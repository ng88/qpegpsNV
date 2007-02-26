/*
  qpegps is a program for displaying a map centered at the current longitude/
  latitude as read from a gps receiver.

  qpeGPS NV >= 1.1 with route navigation Copyright (C) 2006 Nicolas Guillaume <ng@ngsoft-fr.com>
  qpeGPS <= 0.9.2.3.3 Copyright (C) 2002 Ralf Haselmeier <Ralf.Haselmeier@gmx.de>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/



#include "stdlib.h"
#include "signal.h"
#include "qpegps.h"
#include "gpsdata.h"
#include "client.h"
#include "mapdisp.h"
#include "mapinfo.h"
#include "maps.h"
#include "settings.h"
#include "fetchmap.h"
#include "route.h"
#include "about.h"
#include "gpsstatus.h"
#include <sys/types.h>
#include <sys/stat.h>

#include <qmessagebox.h>
#include <qapplication.h>
#include <qpe/qpedebug.h>




/*
  The resumCF function and the handling of the SIGCONT is provided by Petr Doubek.
  Everytime the Z resumes, it wakes the CF-GPS up.
  ** added class wrapper and singelton to prevent need for global ptr - Carsten Roedel
  */
Qpegps *ResumeCF::d_pAppl = NULL;

void ResumeCF::resume(int signal)
{
    QString speed = d_pAppl->gpsData().gpsdArgStr;
    speed = speed.right(speed.length() - speed.find("-s", 0, false) - 3);
    speed.truncate(speed.find(' ', 0, false));

    system( ("READY=`cardctl status 0 | grep ready | wc -l`;"
             "if [ $READY -gt 0 ];"
             "then"
             "  echo \"card ready\";"
             "else"
             "  echo \"executing cardctl resume\";"
             "  cardctl resume;"
             "fi;"
             "SPEED=`stty < /dev/ttyS3 | grep ") + speed + (" | wc -l`;"
                                                             "if [ $SPEED -gt 0 ];"
                                                             "then"
                                                             "  echo \"port settings ok\";"
                                                             "else"
                                                             "  echo \"executing stty\";"
                                                             "  stty ") + speed +
        (" line 0 min 1 time 0 ignbrk -brkint -icrnl -imaxbel -opost"
         "    -onlcr onlret -isig -icanon -iexten -echo -echoe -echok -echoctl"
         "    -echoke < /dev/ttyS3;" "fi;"
         "killall -SIGCONT gpsd" ) );

    if (signal)
        d_pAppl->gpsd->restartGpsd();
}

void ResumeCF::usr(int signal)  /* added by A.Karkhov after 0.9.2.3.2  */
{
    qDebug("usr(%i)\n", signal);
    if (!d_pAppl->gpsData().d_connected || !d_pAppl->gpsData().d_aliveGPS)
        if (signal)
            d_pAppl->gpsd->restartGpsd();       // restart gpsd 
}

void ResumeCF::activate()
{
    signal(SIGCONT, &this->resume);
    signal(SIGUSR1, &this->usr);
}

const char * const Qpegps::_version = "1.1.15 beta";

Qpegps::Qpegps(const QString& qpedir, QWidgetStack * viewer, const char *name, WFlags f)
  : QTabWidget(viewer, name, f), _gpsData(qpedir), d_pViewer(viewer), resumeCF(this)
{
    lastTab = 0;
    places = NULL; /********/

    // make sure this widget is visible in viewer
    d_pViewer->raiseWidget(this);

    // disable "light off" and "power off"
    QCopEnvelope e("QPE/System", "setScreenSaverMode(int)");
    e << (int) QPEApplication::Disable;

    maps.setAutoDelete(TRUE);

    settings = new Settings(&_gpsData, viewer, tr("Config"));

    _gpsData.ManualPosit = _gpsData.startup_mode;                        /* Added by A/ Karhov *//********/

    // restart gpsd, check for CF card and resume it if neccessary
    //restartGpsd(true);
    if (_gpsData.gpsdArgStr.contains("/dev/ttyS3"))
    {
        resumeCF.resume(0);
        resumeCF.activate();
    }
    // Note: the Client is now taking over this part, it tries to connect to the gpsd first
    //       if it succeeds it leaves it alone. If not it restarts the gpsd
    //gpsd = new Client(gpsData);
    gpsd = new Client(this);

    readMaps();

    readPlaces();

    route = new RouteGUI(this, viewer, tr("Route"));
    
    _routeAlert = new RouteAlert(this, this, &(route->currentRoute()) );
    
    connect( &(route->currentRoute()), SIGNAL(routeInfo(const QString&, int))
                , this, SLOT(newComment(const QString&, int)) );

    // create the map viewer, note must not be a child of qpegps or we can't use it in the viewer
    track = 0;
    mapDisp = new MapDisp(this, &maps, d_pViewer, tr("Map"));

    //mapInfo = new MapInfo(gpsData, &maps, viewer, "Info");
    mapInfo = new MapInfo(this, &maps, viewer, tr("Info"));
    //fetchMap = new FetchMap(gpsData, mapInfo, &maps, viewer, "Fetch");
#if 0                           // Disabled until implemented
    route = new Route(gpsData, viewer);
#endif
    //d_pGpsStatus = new GpsStatus(gpsData, viewer, "GPS");
    d_pGpsStatus = new GpsStatus(this, viewer, tr("GPS"));

    track = new Track(this, viewer, tr("Track"));

    QPixmap pixmap;

    pixmap.load(_gpsData.iconsPathStr + "/map16x16.xpm");
    addTab(mapDisp, pixmap, "");        //"Map");

    pixmap.load(_gpsData.iconsPathStr + "/info16x16.xpm");
    addTab(mapInfo, pixmap, "");        //"Info");

    pixmap.load(_gpsData.iconsPathStr + "/gps16x16.xpm");
    addTab(d_pGpsStatus, pixmap, "");   //"GPS");

    pixmap.load(_gpsData.iconsPathStr + "/track16x16.xpm");
    addTab(track, pixmap, "");  //"Track");

    pixmap.load(_gpsData.iconsPathStr + "/route16x16.xpm");
    addTab(route, pixmap, "");  //"Route");

    pixmap.load(_gpsData.iconsPathStr + "/config16x16.xpm");
    addTab(settings, pixmap, "");       //"Config");

    pixmap.load(_gpsData.iconsPathStr + "/about16x16.xpm");
    addTab(new About(viewer, tr("About")), pixmap, "");       //"About");
    
    //pixmap.load(iconPath + "/fetch16x16.xpm");
    //addTab(fetchMap, pixmap,"");//"Fetch");

    connect(this, SIGNAL(currentChanged(QWidget *)),
            SLOT(tabChanged(QWidget *)));

    connect(gpsd, SIGNAL(newData()), SLOT(updateData()));

    connect(settings, SIGNAL(mapPathChanged()), this, SLOT(reReadMaps()));

    connect(settings, SIGNAL(mapPathChanged()),
            mapInfo, SLOT(mapListChanged()));

    connect(mapInfo, SIGNAL(mapListCleared()),
            mapDisp, SLOT(clearActMapList()));

    //    connect( fetchMap, SIGNAL(mapListChanged()),
    //             mapInfo, SLOT(mapListChanged()) );

    connect(qApp, SIGNAL(aboutToQuit()), SLOT(quitInProgress()));

    connect(mapDisp, SIGNAL(mouseClick(QWidget *)),
            this, SLOT(toggleFullScreen(QWidget *)));

    // everything is started up, ready to go
    // need info about satellites as quickly as possible
    // start sniffing for gps
    gpsd->startSniffMode();

}

void Qpegps::newComment(const QString& str, int i)
{
    if(_gpsData.rtDisplayPopup)
        _routeAlert->show(i, _gpsData.rtPopupAutoClose, _gpsData.rtAutoCloseTime);
    
    if(_gpsData.rtUseTts)    
        _tts.current()->sayText(str);
}

void Qpegps::showRouteInfo()
{
    if(route->currentRoute().commentedRoutePoints().isEmpty())
        QMessageBox::warning(this, "QpeGps",
                tr("The current route doesnt contain any information!"),
                QMessageBox::Ok, QMessageBox::NoButton);
    else
        _routeAlert->show();
}


void Qpegps::readPlaces()                                              /* changed by A.Karkhov after 0.9.2.3.2  *//**********/
{
    
    Places *curr, *next;

    QFile placesFile(_gpsData.trackPathStr + "/places.txt");
    
    curr = places = next = (Places *) malloc(sizeof(Places));
    curr->next = NULL;
    curr->name = new QString("none");
    curr->comment = new QString("");
    _gpsData.currPos.setLat(0);
    _gpsData.currPos.setLong(0);
    if (placesFile.open(IO_ReadOnly))
    {
        QTextStream t(&placesFile);
        t.setEncoding(QTextStream::UnicodeUTF8);
        QString pro;
        while (!t.eof())
        {
            t >> pro;
            if (pro[0] == '#')
                pro = t.readLine();
            else
            {
                next = (Places *) malloc(sizeof(Places));
                curr->next = next;
                curr = next;
                curr->name = new QString(pro);
                t >> pro;
                curr->pos.setLat(pro.toDouble());
                t >> pro;
                curr->pos.setLong(pro.toDouble());
                t >> pro;
                curr->altitude = pro.toDouble();
                pro = t.readLine().stripWhiteSpace();
                curr->comment = new QString(pro);

                if (*curr->name == _gpsData.startup_name)
                {
                    _gpsData.currPos.setLat(curr->pos.latitude());
                    _gpsData.currPos.setLong(curr->pos.longitude());
                    _gpsData.altitude.setFromDouble(curr->altitude);
                }
            }
        }
        curr->next = NULL;
        placesFile.close();
    }
}

void Qpegps::writePlaces()      /* added by A.Karkhov after 0.9.2.3.2  */
{
    Places *curr;
    int ok = 0;
    bool newpl;
    QString *filename;
    filename = new QString(_gpsData.trackPathStr);
    filename->append("/places.txt");
    remove(*filename + "~");
    rename(*filename, *filename + "~");
    QFile *placesFile;
    placesFile = new QFile(*filename);
    if (placesFile->exists())
        newpl = FALSE;
    else
        newpl = TRUE;
    if (!(placesFile->open(IO_WriteOnly)))
    {
        while (!QMessageBox::
               warning(this, "Saving places...",
                       "Can't open/create file:\n" + *filename +
                       "\nPlace not saved. Please check file/directory access rights.\n",
                       "Try again", "Ignore", 0, 0, 1))
        {
            ok = placesFile->open(IO_WriteOnly);
            if (ok)
                break;
        }
        if (!ok)
        {
            rename(*filename + "~", *filename);
            return;
        }
    }
    QTextStream t(placesFile);
    t.setEncoding(QTextStream::UnicodeUTF8);

    if (newpl)
    {
        t << "#format is: <Name> <latitude (fload decimal)> <longitude (fload decimal)> <altitude> <comment>\r\n";
    }

    curr = places;
    curr = curr->next;
    while (curr != NULL)
    {
        t << (*curr->name + "\t" + tr("%1").arg(curr->pos.latitude(), 6, 'f') +
              "\t" + tr("%1").arg(curr->pos.longitude(), 6,
                                  'f') + "\t" + tr("%1").arg(curr->altitude,
                                                             6,
                                                             'f') + "\t" +
              *curr->comment + "\r\n");
        curr = curr->next;
    };
    placesFile->close();
    delete placesFile;
    delete filename;
}


void Qpegps::readMaps()
{
    QString filename = _gpsData.mapPathStr;
    filename.append("/maps.txt");
    QFile mapFile(filename);
    int ok = mapFile.open(IO_ReadOnly);
    if (ok)
    {
        QTextStream t(&mapFile);
        QString pro;
        QString *mapInfo = new QString;
        MapLambert *lambertMap;
        MapLin *linearMap;
        MapFritz *fritzMap;
        MapCEA *ceaMap;
        MapMercator *mercatorMap;
        MapUTM *utmMap;
        while (!t.eof())
        {
            t >> pro;

            if (pro == "LAMBERT")
            {
                *mapInfo = t.readLine();
                lambertMap = new MapLambert(mapInfo);
                maps.append(lambertMap);
            }

            else if (pro == "CEA")
            {
                *mapInfo = t.readLine();
                ceaMap = new MapCEA(mapInfo);
                maps.append(ceaMap);
            }

            else if (pro == "MERCATOR")
            {
                *mapInfo = t.readLine();
                mercatorMap = new MapMercator(mapInfo);
                maps.append(mercatorMap);
            }

            else if (pro == "TM")
            {
                *mapInfo = t.readLine();
                utmMap = new MapUTM(mapInfo, FALSE);
                maps.append(utmMap);
            }

            else if (pro == "UTM")
            {
                *mapInfo = t.readLine();
                utmMap = new MapUTM(mapInfo, TRUE);
                maps.append(utmMap);
            }

            else if (pro == "LINEAR")
            {
                *mapInfo = t.readLine();
                linearMap = new MapLin(mapInfo);
                maps.append(linearMap);
            }

            else if (pro == "FRITZ")
            {
                *mapInfo = t.readLine();
                fritzMap = new MapFritz(mapInfo);
                maps.append(fritzMap);
            }

            else
                t.readLine();   // else => comment
        }
        mapFile.close();
        maps.sort();
    }
    else
    {
        QFileInfo mfi(mapFile);
        QMessageBox::critical(0, "qpeGPS",
                              tr("couldn't open \n%1\n"
                                 "If file does not exist it will be created.\n"
                                 "File info:\n"
                                 "exists = %2\n"
                                 "is readable = %3\n"
                                 "is a file = %4\n"
                                 "is a directory = %5\n"
                                 "is a symbolic link = %6\n"
                                 "owner = %7").arg(filename).arg(mfi.
                                                                 exists()).
                              arg(mfi.isReadable()).arg(mfi.isFile()).arg(mfi.
                                                                          isDir
                                                                          ()).
                              arg(mfi.isSymLink()).arg(mfi.owner()));
        if (!mfi.exists())
        {
            mapFile.open(IO_ReadWrite);
            mapFile.close();
            chmod((const char *) filename, 0777);
        }                                                                                                                              /* Added by A. Karhov *//**********/
    }
}

void Qpegps::reReadMaps()
{
    maps.setAutoDelete(TRUE);
    maps.clear();               // FIXME => critical for mapInfo ...
    mapDisp->clearActMapList();
    readMaps();
}


Qpegps::~Qpegps()
{
}

void Qpegps::tabChanged(QWidget * tb)
{
    this->changeTab(tb, tb->name());
    if (lastTab && tb != lastTab)
        this->changeTab(lastTab, "");
    lastTab = tb;

    // timer changes
    gpsd->widgetToReEnable = tb;
    if (gpsd->inSniffMode())
        // nothing to do here, we are in special sniff mode
        // Note: when sniff mode ends, we will reenable timers and settings
        return;


    if (tb == mapDisp)
    {
        // moving map needs updates every 500msec
        // status(S), position(P), waypoint(W), altitude(A), speed(V), heading(H), bearing(B), length(L), date(D)
        gpsd->readyToConnect("SPWAVHBLDC", 500);
    }
    else
    {
        if (tb == d_pGpsStatus)
        {
            // gps status needs update more frequent
            // status(S), position(P), altitude(A), speed(V), heading(H), bearing(B), date(D)
            gpsd->readyToConnect("SPAVHBDX", 250);
        }
        else
        {
            // any other page, no updates (can that be right?)
            gpsd->timer->stop();
        }
    }
    /* tb->setFocus(); 2307 ? */
}

void Qpegps::toggleFullScreen(QWidget * w)
{
    static QSize cachedSize;

    // make sure viewer is visible
    if (d_pViewer->isVisible())
    {
        // freeze the viewer
        d_pViewer->setUpdatesEnabled(false);

        // if the current widget is already visible then switch back to main widget
        if (w == d_pViewer->visibleWidget())
        {
            // switch to normal
            //qDebug("set to normal");

            // remove the widget from the viewer and add it back to the main widget
            // Note: this is neccessary because it is not possible to have a widget belong to two widget-stacks
            d_pViewer->removeWidget(w);

            // this is really nice, have to add the widget back to the main widget
            // which means we need to restore tab nam/icon and position
            QPixmap pixmap;
            QString iconPath = _gpsData.iconsPathStr;
            if (w == mapDisp)
            {
                pixmap.load(iconPath + "/map16x16.xpm");
                insertTab(mapDisp, pixmap, tr("Map"), 0);
                showPage(w);
            }

            // raise the main widget ID=0
            d_pViewer->raiseWidget(0);
            d_pViewer->showNormal();
            // restore the WidgetFlags !
            d_pViewer->reparent(0, WStyle_Customize | WStyle_NoBorder,
                                QPoint(0, 0));
            d_pViewer->showMaximized();
            d_pViewer->resize(cachedSize);
        }
        else
        {
            // raise specified widget and switch to fullscreen
            //qDebug("set to fullscreen");
            cachedSize = d_pViewer->size();
            if (d_pViewer->size() != qApp->desktop()->size())
            {
                d_pViewer->resize(qApp->desktop()->size());
            }

            // remove the widget from the main widget
            // Note: this is necessary since a widget can not belong to two widget-stacks
            removePage(w);

            // add the widget to the viewer as fixed ID=1
            d_pViewer->addWidget(w, 1);

            // make sure widget is valid
            if (d_pViewer->id(w) == 1)
            {
                d_pViewer->raiseWidget(1);
                d_pViewer->showFullScreen();
            }
        }

        // unfreeze the viewer
        d_pViewer->setUpdatesEnabled(true);
        gpsd->readyToConnect("SPWAVHBLDC", 500);

    }
    w->setFocus();
}


void Qpegps::updateData()
{
    // called when GPS Data has changed
    QWidget *tb = currentPage();

    if (d_pViewer->visibleWidget() == mapDisp)
        d_pViewer->visibleWidget()->repaint();
    else
    {
        if (tb == mapDisp)      // update moving map
            mapDisp->repaint();

        else if (tb == d_pGpsStatus)    // update full GPS status
            d_pGpsStatus->update();
    }

    // update track => new track point
    track->update();
}

void Qpegps::quitInProgress()
{
    // re-enable "light off" and "power off"
    QCopEnvelope("QPE/System",
                 "setScreenSaverMode(int)") << (int) QPEApplication::Enable;
    // write track in memory to file
    if (track->wDo)
        track->Write(track->wLog->currentText());
    // close client connection
    gpsd->closeConnection();
}


int main(int argc, char **argv)
{
    QString dir = ".";

    QPEApplication a(argc, argv);
    dir = a.qpeDir();

    QTranslator translator(0);
    if (a.argc() > 1)
    {
        printf("command line parameter \"%s\" overrides global setting of \"locale\"\n", a.argv()[1]);
        translator.load(QString("qpegps_") + a.argv()[1], dir + "/qpegps");
    }
    else
        translator.load(QString("qpegps_") + QTextCodec::locale(),dir + "/qpegps");
        
    a.installTranslator(&translator);

    // create the *viewer* which can switch between full screen and normal
    QWidgetStack viewer;

    // this is the main widget
    viewer.setCaption(QString("qpeGPS NV Edition %1").arg(Qpegps::version()));

    // create the qpegps main appl widget (with viewer as parent)
    Qpegps qpegps(dir, &viewer);

    // main widget to show
    viewer.addWidget(&qpegps, 0);

    // register main widget with appl
    a.showMainWidget(&viewer);

    // start the loop

    return a.exec();
}

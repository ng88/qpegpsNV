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


#include "track.h"

#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <qvector.h>

#include "mathex.h"

// write config file
#define WRITE_CONFIG	application->settings->writeConfig()


/*
 * convert (char*) "hh:mm:ss" to (double) hhmmss
 */

double tmstr2double(char *str)
{
    int h, m, s;
    sscanf(str, "%d:%d:%d", &h, &m, &s);
    return (double) (10000 * h + 100 * m + s);
}

/*
 * convert (double) hhmmss to (char*) "hh:mm:ss"
 */

void tmdouble2str(double tm, char *str)
{
    int h = (int) tm / 10000,
        m = (int) tm / 100 - 100 * h, s = (int) tm % 100;
    sprintf(str, "%02d:%02d:%02d", h, m, s);
}

/*********************************************************************
 * TRACKPOINT constructor - create trackpoint from various formats
 *********************************************************************/


TrackPoint::TrackPoint(const QString& nmea) //in part rewrited and optimized by ng
{

    if(nmea.length() == 0)
        return;
        
    char * str = const_cast<char*>(nmea.latin1());

    switch (nmea[0])
    {
    case '$':
        {                       // assuming NMEA GPGGA messages
        
            QStringList args = QStringList::split(';', nmea, true);
            if (args.count() >= 10)
            {

                double lon, lat, hdop;
                char lonsign, latsign;
                int status, sats;
                
                time = args[1].toDouble();
                lat = args[2].toDouble();
                latsign = (QChar)args[3][0];
                lon = args[4].toDouble();
                lonsign = (QChar)args[5][0];
                status = args[6].toInt();
                sats = args[7].toInt();
                hdop = args[8].toDouble();
                altitude = args[9].toDouble();
    
                latitude = floor(lat / 100) + (lat - floor(lat / 100) * 100) / 60;
                longitude =
                    floor(lon / 100) + (lon - floor(lon / 100) * 100) / 60;
                if (latsign == 'S')
                    latitude = -latitude;
                if (lonsign == 'W')
                    longitude = -longitude;
            }

            break;
        }
    case ';':                  //  assuming qpeGPS NV route format, added by ng
        {
            const QStringList& args = QStringList::split(';', nmea, true);
            //printf("DBG   args.count()== %d\n", args.count());
            if (args.count() >= 5/*7*/)
            {
                latitude = args[1].toDouble();
                longitude = args[2].toDouble();
                altitude = args[3].toDouble();
                time = args[4].toDouble();
                
                //printf("DBG   (lt, lg, alt, tm) == (%e, %e, %e, %e)\n", latitude, longitude, altitude, time);
            }
        }
    case 'T':
        {                       // assuming PCX5 format
            char tm[20], dt[20];
            sscanf(str, "T %lf %lf %s %s %lf", &latitude, &longitude, dt, tm,
                   &altitude);
            time = tmstr2double(tm);
            break;
        }

    case ' ':
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        {                       // assuming gpsdrive format
            char t[5][20];
            sscanf(str, "%lf %lf %lf %s %s %s %s %s",
                   &latitude, &longitude, &altitude, t[0], t[1], t[2], t[3],
                   t[4]);
            for (int i = 0; i < 5; i++)
                if (strchr(t[i], ':'))  // looking for time
                    time = tmstr2double(t[i]);
            break;
        }

    default:
        qWarning(tr("unknown trackpoint format `%1'").arg(nmea));
        break;
    }
}

/*
 * create trackpoint from real values
 */

TrackPoint::TrackPoint(QString tim, double lat, double lon, double alt)
{
    time = tmstr2double((char *) tim.latin1());
    latitude = lat;
    longitude = lon;
    altitude = alt;
}

/*
 * create NMEA GGA message from the trackpoint
 */

QString TrackPoint::toNMEA()
{
    QString nmea, checksum;
    double lat, lg, tdeg, tmin, ndeg, nmin;
    char tsign, nsign;

    // latitude
    lat = fabs(latitude);
    if (latitude > 0)
        tsign = 'N';
    else
        tsign = 'S';
    tdeg = floor(lat);
    tmin = (lat - tdeg) * 60.0;

    // longitude
    lg = fabs(longitude);
    if (longitude > 0)
        nsign = 'E';
    else
        nsign = 'W';
    ndeg = floor(lg);
    nmin = (lg - ndeg) * 60.0;

    nmea.
        sprintf
        ("$GPGGA,%010.3f,%02d%07.4f,%c,%02d%07.4f,%c,1,0,0,%d,M,,,,0000",
         time, (int) tdeg, tmin, tsign, (int) ndeg, nmin, nsign,
         (int) altitude);
    MathEx::NMEAChecksum(nmea);

    return nmea;
}

/*
 * create Garmin PCX5 message for the trackpoint
 */

QString TrackPoint::toPCX5()
{
    QString pcx;
    char tm[20];

    tmdouble2str(time, tm);

    // \TODO: date
    pcx.sprintf("T %+09.6f %+09.6f %s %s %4d\n",
                latitude, longitude, "00-JAN-00", tm, (int) altitude);

    return pcx;
}

/*
 * create gpsdrive trackpoint
 */

QString TrackPoint::toDrive()
{
    QString drive;
    char tm[20];

    tmdouble2str(time, tm);

    // \TODO: date
    drive.sprintf("%9.6f %9.6f %4d %s\n",
                  latitude, longitude, (int) altitude, tm);

    return drive;
}


/*
 * return distance from given point in a strange metric (degree based)
 */

double TrackPoint::dist(double lat, double lon)
{
    return fabs(latitude - lat) + fabs(longitude - lon);
}

/*
 * return time difference from given point in seconds
 */

double TrackPoint::timediff(QString tim)
{
    int h, m, s;
    sscanf(tim.latin1(), "%d:%d:%d", &h, &m, &s);
    return fabs(time - (double) (10000 * h + 100 * m + s));
}

/*********************************************************************
 * TRACK constructor - set default values, create widgets
 *********************************************************************/

void Track::placeSelected(int ind)      /* Added by A. Karhov */
{
    if (ind)
    {
        int i = ind;
        Places *curr = pl;
        do
        {
            curr = (Places *) curr->next;
            i--;
            if (i == 0)
            {
                commMLE->setText(*curr->comment);
                break;
            }
        }
        while (curr != NULL);
    }
}

void Track::delPlace()                                                      /* changed by A.Karkhov after 0.9.2.3.2  *//**********/
{
    int ind = 0;
    int i = ind = mapLatLonCB->currentItem();
    if (ind)
    {
        mapLatLonCB->removeItem(ind);
        StartCB->removeItem(ind);
        StartCB->setCurrentItem(ind - 1);
        startSelected(ind - 1);
        commMLE->setText("");
        mapLatLonCB->setCurrentItem(0);
        Places *curr = pl, *prev;
        do
        {
            prev = curr;
            curr = curr->next;
            i--;
            if (i == 0)
            {
                prev->next = curr->next;
                delete curr->name;
                delete curr->comment;
                free(curr);
                break;
            }
        }
        while (curr->next != NULL);
        application->writePlaces();

/* // commented by A.Karkhov after 0.9.2.3.2  
QString	buf="";
	  QString filename = gpsData->trackPathStr;
	  filename.append("/places.txt");
		  remove(filename+"~");
		  rename(filename,filename+"~");
	  QFile placesFile(filename);
	   int ok =  placesFile.open(IO_WriteOnly);
	   if ( !ok )
		   { while ( !QMessageBox::warning( this, "Saving places...", "Can't open/create file:\n"+filename+ 			  "\nPlace not saved. Please check file/directory access rights.\n","Try again","Ignore", 0, 0, 1 ) )
			 { ok =  placesFile.open(IO_WriteOnly);
			 if (ok) break;
			 }
			 if ( !ok ) {rename(filename+"~",filename); return; }
		      }
		 QTextStream t( &placesFile );
		  QFile oplacesFile(filename+"~");
		  ok =  oplacesFile.open(IO_ReadOnly);
		   QTextStream ot( &oplacesFile );
		  if ( ok )
		  { while ( !ot.eof() )
        		{buf=ot.readLine();
			if ( (oplacesFile.status()) != -1 && (placesFile.status()) != -1 && buf[0]=='#') { buf+="\r\n"; t<<buf; buf="";   }
			else break;
			}
		   }
		   else { t << "# this is places file places.txt format is : <CityName> <latitude (fload decimal)> <longitude (fload decimal)> <short text comment>\r\n"; }

		curr=pl;
		curr=curr->next;
		while ( curr != NULL )  {
t << (*curr->name+"\t"+tr("%1").arg(curr->pos.latitude,6,'f')+"\t"+tr("%1").arg(curr->pos.longitude,6,'f')+"\t"+tr("%1").arg(curr->altitude,6,'f')+"\t"+*curr->comment+"\r\n");
		curr=curr->next;
    		};

		placesFile.close();
		if (ok) oplacesFile.close();
*/
    }
}

void Track::refresh()           /* Added by A. Karhov */
{
    mapLatLonCB->clear();
    pl = application->places;
    Places *curr = application->places;
    do
    {
        mapLatLonCB->insertItem(*curr->name);
        curr = (Places *) curr->next;
    }
    while (curr != NULL);
}

void Track::setStartup()        /* Added by A. Karhov */
{
    StartCB->clear();
    pl = application->places;
    Places *curr = application->places;
    int i = 0;
    do
    {
        StartCB->insertItem(*curr->name);
        if (*curr->name == gpsData->startup_name)
            StartCB->setCurrentItem(i);
        curr = (Places *) curr->next;
        i++;
    }
    while (curr != NULL);
}

void Track::startSelected(int ind)      /* Added by A. Karhov */
{
    if (ind)
    {
        int i = ind;
        Places *curr = pl;
        do
        {
            curr = (Places *) curr->next;
            i--;
            if (i == 0)
            {
                gpsData->startup_name = *curr->name;
                WRITE_CONFIG;
                break;
            }
        }
        while (curr != NULL);
    }
    else
    {
        gpsData->startup_name = "";
        WRITE_CONFIG;
    }
}

Track::Track(Qpegps * appl, QWidget * parent, const char *name,
             WFlags fl):QScrollView(parent, name, fl)
{
    application = appl;
    gpsData = &(application->gpsData());
    wDo = rDo = false;

    logdir = new QDir();
    logdir->setNameFilter("[^.]*");

    setHScrollBarMode(AlwaysOff);
    setVScrollBarMode(Auto);
    mainBox = new QVBox(this);
    addChild(mainBox);

    setResizePolicy(AutoOneFit);
/* Added by A. Karhov */
    hBox = new QHBox(mainBox);
    PlaceGB = new QVGroupBox(tr("Places"), hBox);

    hBox = new QHBox(PlaceGB);

    mapLatLonCB = new QComboBox(hBox);
    refresh();
    connect(mapLatLonCB, SIGNAL(activated(int)), this,
            SLOT(placeSelected(int)));
    QDelButt = new QPushButton("Delete", hBox);
    connect(QDelButt, SIGNAL(pressed()), SLOT(delPlace()));

    hBox = new QHBox(PlaceGB);

    commMLE = new QMultiLineEdit(hBox);
    commMLE->setWordWrap(QMultiLineEdit::WidgetWidth);
    commMLE->setWrapPolicy(QMultiLineEdit::Anywhere);
    commMLE->setReadOnly(TRUE);
    commMLE->setFixedVisibleLines(2);

    // Startup mode

    hBox = new QHBox(mainBox);
    StGB = new QHGroupBox(tr("qpeGPS startup"), hBox);
    StartModeL = new QLabel("Map mode:", StGB);
    sCB = new QCheckBox(StGB);
    sCB->setChecked(gpsData->startup_mode);
    connect(sCB, SIGNAL(toggled(bool)), SLOT(setStartModeCB(bool)));

    hBox = new QHBox(StGB);

    StartCB = new QComboBox(hBox);
    setStartup();
    connect(StartCB, SIGNAL(activated(int)), this, SLOT(startSelected(int)));


        /*** End Added by A. Karhov ***/

    instructions =
        new
        QLabel(tr
               (" - TRACKLOG is saved when unchecking write checkbox\n"
                " - (re)select map to display track in Info"), mainBox);

    // create field for track directory
    tBox = new QHBox(mainBox);
    tBox->setMargin(2);
    tLabel = new QLabel(tr("Track dir: "), tBox);
    tLE = new QLineEdit(tBox);
    tButton = new QPushButton(tr("search"), tBox);
    tLE->setText(gpsData->trackPathStr);
    connect(tButton, SIGNAL(pressed()), SLOT(setTrackPath()));
    connect(tLE, SIGNAL(returnPressed()), SLOT(tLEChanged()));

    // create checkboxes and comboboxes for tracklog files
    wBox = new QHBox(mainBox);
    wBox->setMargin(2);
    wCB = new QCheckBox(wBox);
    wBox->setMargin(2);
    wLabel = new QLabel(tr("write "), wBox);
    wLog = new QComboBox(true, wBox, "write_log_filename");
    connect(wCB, SIGNAL(toggled(bool)), SLOT(setWriteCB(bool)));

    rBox = new QHBox(mainBox);
    rBox->setMargin(2);
    rCB = new QCheckBox(rBox);
    rLabel = new QLabel(tr("read  "), rBox);
    rLog = new QComboBox(true, rBox, "read_log_filename");
    connect(rCB, SIGNAL(toggled(bool)), SLOT(setReadCB(bool)));
    connect(rLog, SIGNAL(activated(const QString &)),
            SLOT(setReadName(const QString &)));

    updateFileList();

    // minimal time difference between 2 positions
    dBox = new QHBox(mainBox);
    dBox->setMargin(2);
    dLabel = new QLabel(tr("min time difference [s]    "), dBox);
    dLE = new QLineEdit(dBox);
    QString buf;
    buf.sprintf("%d", gpsData->updt_freq);
    dLE->setText(buf);
    connect(dLE, SIGNAL(returnPressed()), SLOT(dLEChanged()));


    // track line thickness
    QStringList thickList;
    thickList << "1" << "2" << "3" << "4" << "5";
    lBox = new QHBox(mainBox);
    lBox->setMargin(2);
    lLabel = new QLabel(tr("line thickness"), lBox);
    lMenuB = new MenuButton(thickList, lBox);
    lMenuB->select(gpsData->track_thick - 1);
    connect(lMenuB, SIGNAL(selected(int)), SLOT(lMenuBChanged(int)));

    mapLatLonCB->setFocus();

}

Track::~Track()
{
    if (wDo)
        Write(wLog->currentText());
}

/*
 * update comboboxes with log names
 */

void Track::updateFileList()
{
    // keep old selected filenames
    QString wOld = wLog->currentText(), rOld = rLog->currentText();
    // remove old files
    wLog->clear();
    rLog->clear();

    // if trackPathStr exists, change logdir to it
    if (logdir->exists(gpsData->trackPathStr))
    {
        logdir->setPath(gpsData->trackPathStr);
        // fill list with new files
        const QFileInfoList *list = logdir->entryInfoList();
        QFileInfoListIterator it(*list);
        QFileInfo *fi;
        while ((fi = it.current()))
        {
            // add file to list
            if (fi->fileName() != "places.txt"
                && fi->fileName() != "places.txt~")
            {
                wLog->insertItem(fi->fileName().latin1());
                rLog->insertItem(fi->fileName().latin1());
                // if it's the same as the old one select it
                if (fi->fileName() == wOld)
                    wLog->setCurrentItem(wLog->count() - 1);
                if (fi->fileName() == rOld)
                    rLog->setCurrentItem(rLog->count() - 1);
            }
            ++it;
        }
    }
}

/*
 * write the tracklog which is in mamory
 */

void Track::Write(QString filename, int format)
{
    QString pathfile = gpsData->trackPathStr;
    QTextStream *wStream;
    QFile wFile;
    TrackPoint *tp;

    pathfile.append("/");
    pathfile.append(filename);
    wFile.setName(pathfile);
    if (wFile.open(IO_WriteOnly | IO_Append))
    {
        if ((wStream = new QTextStream(&wFile)))
        {
            while (!wTrack.isEmpty())
            {
                tp = wTrack.first();
                switch (format)
                {
                case NMEA:
                    *wStream << tp->toNMEA();
                    break;
                case PCX5:
                    *wStream << tp->toPCX5();
                    break;
                case GPSDRIVE:
                    *wStream << tp->toDrive();
                    break;
                }
                wTrack.removeFirst();
                delete tp;
            }
            delete wStream;
        }
        wFile.close();
    }
    if (filename == rLog->currentText())
        Read(filename);
    // we've just written and cleared track we are displaying ... reread
    updateFileList();           // fill comboboxes with log names
}

/*
 * read tracklog into the list of trackpoints
 */

void Track::Read(QString filename)
{
    QString pathfile = gpsData->trackPathStr, trkp;
    QTextStream *rStream;
    QFile rFile;
    TrackPoint *tp;

    pathfile.append("/");
    pathfile.append(filename);
    rFile.setName(pathfile);

    while (!rTrack.isEmpty())
    {                           // delete old track if any
        tp = rTrack.first();
        rTrack.removeFirst();
        delete tp;
    }

    if (rFile.open(IO_ReadOnly))
    {                           // open file with new one
        rStream = new QTextStream(&rFile);

        while (!rStream->eof())
        {                       // get points
            trkp = rStream->readLine();
            tp = new TrackPoint(trkp);
            rTrack.append(tp);
        }

        delete rStream;
        rFile.close();
    }
}

/*
 * new gps data, update tracklog
 */

void Track::update()
{

    if (gpsData->ManualPosit)
        return;                 // Added by A. Karhov

    if (wDo && gpsData->status)
    {
        if (wTrack.isEmpty() ||
            (wTrack.last()->dist(gpsData->currPos.latitude(),
                                 gpsData->currPos.longitude()) > MINTRACKDIST &&
             wTrack.last()->timediff(gpsData->ts.time()) > gpsData->updt_freq))
        {

            TrackPoint *tp = new TrackPoint(gpsData->ts.time(),
                                            gpsData->currPos.latitude(),
                                            gpsData->currPos.longitude(),
                                            gpsData->altitude.toDouble());
            wTrack.append(tp);
        }
    }
}

/*
 * toggled write checkbox
 */

void Track::setWriteCB(bool state)
{
    wDo = state;
    if (!wDo)
        Write(wLog->currentText());
}

void Track::setStartModeCB(bool state)
{
    gpsData->startup_mode = state;
    WRITE_CONFIG;
}

/*
 * toggled read checkbox
 */

void Track::setReadCB(bool state)
{
    TrackPoint *tp;

    rDo = state;
    if (rDo)
        Read(rLog->currentText());
    else
    {
        while (!rTrack.isEmpty())
        {
            tp = rTrack.first();
            rTrack.removeFirst();
            delete tp;
        }
    }
}

/*
 * changed read log filename
 */

void Track::setReadName(const QString &)
{
    if (rDo)
        Read(rLog->currentText());
}

/*
 * display track on the screen
 */

void Track::drawTrack(QPainter * painter, MapBase * actmap,
                      int x1, int y1, int mx, int my)
{

    if (!actmap)
        return;



    QList < TrackPoint > *disp[2];

    if (rDo)
    {
        disp[0] = &rTrack;
        // if read and write tracks are the same display also the part in memory
        if (wDo && (wLog->currentText() == rLog->currentText()))
            disp[1] = &wTrack;
        else
            disp[1] = NULL;

        double lg, lt, xwp = 0, ywp = 0;
        int fx = 0, fy = 0;     /* added by ng as a replacement of moveTo/lineTo (now obsoletes) by drawLine */
        int xtp, ytp;
        bool precdef = false;
        bool in = false;

        QPen pen = painter->pen();
        pen.setColor(gpsData->trackColor);
        pen.setWidth(gpsData->track_thick);
        painter->setPen(pen);

        for (int i = 0; i < 2; i++)
            if (disp[i] && !disp[i]->isEmpty())
            {

                TrackPoint *tp = disp[i]->first();
                while (tp)      /* optimized and corrected by ng to fix partial draw bug */
                {
                    lt = MathEx::deg2rad(tp->latitude);
                    lg = MathEx::deg2rad(tp->longitude);


                    actmap->calcxy(&xwp, &ywp, lg, lt);
                    xtp = (int) xwp - x1;
                    ytp = (int) ywp - y1;
                    
                    //printf("tp->latitude=%e, tp->longitude=%e, lt=%e, lg=%e, x1=%d, y1=%d, xwp=%e, ywp=%e, xtp=%d, ytp=%d\n", tp->latitude,tp->longitude,lt,lg,x1,y1,xwp,ywp,xtp,ytp);

                    if (xtp >= 0 && ytp >= 0 && (int) xwp < mx
                        && (int) ywp <= my)
                    {
                        in = true;

                        if (precdef)
                        {
                            //printf("drawline1(%d, %d, %d, %d)\n", fx, fy, xtp, ytp);
                            painter->drawLine(fx, fy, xtp, ytp);
                        }
                        else
                        {
                            gpsData->updt_freq = dLE->text().toInt();
                            WRITE_CONFIG;       // what is this doing here ?!
                        }
                    }
                    else if (in)
                    {
                        //printf("drawline2(%d, %d, %d, %d)\n", fx, fy, xtp, ytp);
                        painter->drawLine(fx, fy, xtp, ytp);
                        in = false;
                    }

                    fx = xtp;
                    fy = ytp;
                    precdef = true;

                    tp = disp[i]->next();
                }

            }
    }
}

/*
 * track path changed
 */

void Track::tLEChanged()
{
    gpsData->trackPathStr = tLE->text();
    WRITE_CONFIG;
    updateFileList();
}

/*
 * searching for track directory
 */

void Track::setTrackPath()
{
    DirDialog getDirDialog(this, 0, TRUE, 0);
    getDirDialog.setCaption(tr("select track directory"));
    getDirDialog.exec();
    if (getDirDialog.result() == QDialog::Accepted)
    {
        gpsData->trackPathStr = getDirDialog.selectedPath();
        tLE->setText(gpsData->trackPathStr);
    }
    WRITE_CONFIG;
    updateFileList();
}

/*
 * minimal time difference between 2 positions changed
 */

void Track::dLEChanged()
{
    gpsData->updt_freq = dLE->text().toInt();
    WRITE_CONFIG;
}

/*
 * line thickness changes
 */

void Track::lMenuBChanged(int idx)
{
    gpsData->track_thick = idx + 1;
    WRITE_CONFIG;
}

/*
 * set rate of message
 */

void Track::setRate(unsigned message, unsigned rate)
{
    QString msg;
    int fd;

    if (gpsData->gpsdArgStr.contains("/dev/ttyS3"))
    {
        msg.sprintf("$PSRF103,%02u,00,%02u,01", message, rate);
        MathEx::NMEAChecksum(msg);
        if ((fd = open("/dev/ttyS3", O_WRONLY)))
        {
            write(fd, msg.latin1(), strlen(msg.latin1()));
            close(fd);
            qWarning(tr("changing msg%d rate to %ds"), message, rate);
        }
    }
}

/*
 * set rate of all message
 */

void Track::cLEChanged()
{
    int i, val = cLE->text().toInt();

    if (val < 1)
        val = 1;
    else if (val > 255)
        val = 255;

    for (i = 0; i < 6; i++)
        setRate(i, val);
}

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

#include "mapinfo.h"
#include "route.h"
#include "track.h"
#include "qpegps.h"
#include "fetchmap.h"
#include <qpopupmenu.h>
#include <unistd.h>
#include "mathex.h"



MapWidget::MapWidget(QWidget * parent, const char *name,
                     WFlags fl):QWidget(parent, name, fl)
{
    xx = -1;
    yy = -1;
}

MapWidget::~MapWidget()
{
}

void MapWidget::mousePressEvent(QMouseEvent * e)
{
    xx = e->x();
    yy = e->y();
    emit mouseClick(e->x(), e->y());
}

void MapWidget::paintEvent(QPaintEvent *)
{
    if (xx > 0)
    {
        QPainter p(this);
        p.drawLine(0, yy, this->width(), yy);
        p.drawLine(xx, 0, xx, this->height());
    }
}


MapInfo::MapInfo(Qpegps * appl, QSortedList < MapBase > *mapList,
                 QWidget * parent, const char *name, WFlags fl):QVBox(parent,
                                                                      name,
                                                                      fl)
{
    application = appl;
    gpsData = &(application->gpsData());
    maps = mapList;

    mapSelect = new QComboBox(FALSE, this);
    //mapDescGB = new QVGroupBox("Map Info", this);
    /*mapDescL = new QLabel(mapDescGB); */
    mapView = new QScrollView(this);
    positionInfo = new QLabel(this);
    positionInfo->setAlignment(Qt::AlignHCenter);

    mapWidget = new MapWidget(mapView);
    mapPixMap = 0;
    mapView->addChild(mapWidget);

    QHBox *hBox;
    hBox = new QHBox(this);
    hBox->setFixedHeight(18);
    removePB = new QPushButton(tr("remove"), hBox);
    downloadPB = new QPushButton(tr("download"), hBox);
    importPB = new QPushButton(tr("import"), hBox);
    propertiesPB = new QPushButton(tr("properties"), hBox);

    connect(mapSelect, SIGNAL(activated(int)), SLOT(selectionChanged(int)));
    connect(mapWidget, SIGNAL(mouseClick(int, int)),
            SLOT(calcPosInfo(int, int)));
    connect(downloadPB, SIGNAL(clicked()), this, SLOT(startDownLoadD()));
    connect(importPB, SIGNAL(clicked()), this, SLOT(startImportD()));
    connect(propertiesPB, SIGNAL(clicked()), this, SLOT(startChangeD()));
    connect(removePB, SIGNAL(clicked()), this, SLOT(startRemoveD()));

    mapsIndex = 0;
    mapListChanged();
}

void MapInfo::mapListChanged()
{
    mapNames.clear();
    /*mapDescList.clear(); */
    mapSelect->clear();
    MapBase *aMap;
    for (aMap = maps->first(); aMap != 0; aMap = maps->next())
    {
        mapNames.append(aMap->name);
        /*mapDescList.append(aMap->getInfo()); */
    }
    mapSelect->insertStringList(mapNames);
    mapSelect->setCurrentItem(mapsIndex);
    selectionChanged(mapsIndex);
    emit mapListCleared();
}

MapInfo::~MapInfo()
{
}

void MapInfo::writeMapFile()
{
    QString filename = gpsData->mapPathStr;
    filename.append("/maps.txt");
    QFile mapFile(filename);
    mapFile.open(IO_WriteOnly);
    QTextStream t(&mapFile);
    MapBase *aMap;
    for (aMap = maps->first(); aMap != 0; aMap = maps->next())
        t << aMap->getParameterStr() << "\n";
    mapFile.close();
}

void MapInfo::selectionChanged(int index)
{
    if (index >= 0 && mapSelect->count())
    {
        mapsIndex = index;
        /*mapDescL->setText(mapDescList[index]); */
        positionInfo->setText("");
        mapWidget->xx = -1;
        mapWidget->yy = -1;
        if (mapPixMap)
            delete mapPixMap;
        QString mapfilename = gpsData->mapPathStr;
        mapfilename.append("/");
        mapfilename.append(mapSelect->currentText());
        mapPixMap = new QPixmap(mapfilename);
        if (!mapPixMap)
        {
            QMessageBox::warning(0, "qpeGPS",
                                 tr("couldn't load map \n%1").
                                 arg(mapfilename));

            mapPixMap = new QPixmap();
        }

        // display track & route on the info map
        if (mapPixMap && mapPixMap->width() && mapPixMap->height())
        {
            MapBase *aMap = maps->at(mapsIndex);
            QPainter painter;
            painter.begin(mapPixMap);

            if (application->route->drawEnabled())
                application->route->draw(&painter, aMap, 0, 0,
                                            mapPixMap->width(),
                                            mapPixMap->height());

            if (application->track && application->track->rDo)
                application->track->drawTrack(&painter, aMap, 0, 0,
                                              mapPixMap->width(),
                                              mapPixMap->height());

            painter.end();

        }

        mapWidget->resize(mapPixMap->width(), mapPixMap->height());
        mapView->setStaticBackground(FALSE);
        mapWidget->setBackgroundPixmap(*mapPixMap);
        //??mapView->setBackgroundPixmap(*mapPixMap);// try, if this works on OPIE 3.2
        mapView->update();
    }
    else
    {
        if (mapPixMap)
            delete mapPixMap;
        mapPixMap = 0;
        positionInfo->setText("");
        mapWidget->xx = -1;
        mapWidget->yy = -1;
    }

    if (mapSelect->currentItem() < mapSelect->count()
        && mapSelect->currentItem() >= 0)
    {
        removePB->setEnabled(TRUE);
        propertiesPB->setEnabled(TRUE);
    }
    else
    {
        removePB->setEnabled(FALSE);
        propertiesPB->setEnabled(FALSE);
    }
}

void MapInfo::calcPosInfo(int x, int y)
{
    double lt, lg;
    QString posInfoStr;
    MapBase *aMap;
    aMap = maps->at(mapsIndex);
    if (aMap->calcltlg(&lt, &lg, (double) x, (double) y))
    {
        mapPos.setLat(MathEx::rad2deg(lt));
        mapPos.setLong(MathEx::rad2deg(lg));
        posInfoStr = tr("Lat: %1, Lon: %2 (%3,%4)")
                        .arg(mapPos.latToString())
                        .arg(mapPos.longToString())
                        .arg(x)
                        .arg(y);      /* Added by A/ Karhov */
    }
    else
        posInfoStr = "";
    positionInfo->setText(posInfoStr);
    mapWidget->update();        // calls paintEvent
}

void MapInfo::startDownLoadD()
{
    QPopupMenu *pMenu = new QPopupMenu; /* Added by A. Karkhov */
    DownLoadDialog dlDialog(gpsData, this, maps, this, "download map", TRUE,
                            0);
    DownloadAreaSpecification *spec;
    spec = new DownloadAreaSpecification(gpsData, this, maps);  // pass the current gps data in here for suggested lat,long,name!
    DownLoadAreaDialog dlDialog1(application, spec, gpsData, this, maps, this,
                                 "download maps", TRUE, 0);
    QString as("downmap");
    pMenu->insertItem("Download few maps cover area from expedia.", 0, 0);
    pMenu->insertItem("Download few maps cover area from multimap.", 1, 1);
    pMenu->insertItem("Download single map from configurable source.", 2, 2);
    pMenu->setCheckable(FALSE);
    pMenu->popup(QPoint(16, 150));
    pMenu->setActiveItem(0);
    switch (pMenu->exec())
    {
    case 2:                    // Standard dialog
        dlDialog.setCaption(tr("Download Map"));
        dlDialog.exec();
        if (dlDialog.result() == QDialog::Accepted)
        {
            writeMapFile();
            mapListChanged();
        }
        break;
    case 1:
        as += "_m";
    case 0:
    default:
        dlDialog1.setCaption(tr("Download Maps"));
        bool valid = 0;
        do
        {
            this->mapPos.setLong( 0 );
            this->mapPos.setLat( 0 );
            dlDialog1.exec();
            if (dlDialog1.result() == QDialog::Accepted)
            {
                valid = 1;
                if (spec->dir.length() >= 140)
                {
                    QMessageBox mb(tr("Map Download"),
                                   tr
                                   ("Downloag directory path too long. You must specify another path."),
                                   QMessageBox::Warning,
                                   QMessageBox::Ok | QMessageBox::Default,
                                   QMessageBox::NoButton,
                                   QMessageBox::NoButton);
                    mb.exec();
                    mb.hide();
                    valid = 0;
                }
                if (spec->scale < 1000)
                {
                    QMessageBox mb(tr("Map Download"),
                                   tr
                                   ("Scale not valid. You must specify another scale."),
                                   QMessageBox::Warning,
                                   QMessageBox::Ok | QMessageBox::Default,
                                   QMessageBox::NoButton,
                                   QMessageBox::NoButton);
                    mb.exec();
                    mb.hide();
                    valid = 0;
                }
                if (spec->IsArea)
                {
                    if (spec->areax < 1 || spec->areay < 1)
                    {
                        QMessageBox mb(tr("Map Download"),
                                       tr
                                       ("Area not valid. You must specify another area."),
                                       QMessageBox::Warning,
                                       QMessageBox::Ok | QMessageBox::Default,
                                       QMessageBox::NoButton,
                                       QMessageBox::NoButton);
                        mb.exec();
                        mb.hide();
                        valid = 0;
                    }
                }
                else
                {
                    if (spec->slatitude >= spec->elatitude)
                    {
                        QMessageBox mb(tr("Map Download"),
                                       tr
                                       ("Start Latitude must be lower then End Latitude."),
                                       QMessageBox::Warning,
                                       QMessageBox::Ok | QMessageBox::Default,
                                       QMessageBox::NoButton,
                                       QMessageBox::NoButton);
                        mb.exec();
                        mb.hide();
                        valid = 0;
                    }
                    if (spec->slongitude >= spec->elongitude)
                    {
                        QMessageBox mb(tr("Map Download"),
                                       tr
                                       ("Start Longitude must be lower then End Longitude."),
                                       QMessageBox::Warning,
                                       QMessageBox::Ok | QMessageBox::Default,
                                       QMessageBox::NoButton,
                                       QMessageBox::NoButton);
                        mb.exec();
                        mb.hide();
                        valid = 0;
                    }
                }
            }
            else
            {
                valid = 0;
                break;
            }
        }
        while (!valid);
        char *com;
        if (valid)
        {
            com = (char *) malloc(512);

            if (spec->IsArea)
            {
                as +=
                    " -la %.5f -lo %.5f -sc %li -a %.2fx%.2f -p -P m -f Dd -md %s -r %1i %s";
                sprintf(com, as.latin1(), spec->latitude, spec->longitude,
                        spec->scale, spec->areax, spec->areay,
                        (const char *) spec->dir, spec->res,
                        (const char *) spec->param);
            }
            else
            {
                as +=
                    " -sla %.5f -slo %.5f -ela %.5f -elo %.5f -sc %li -p -P m -f Dd -md %s -r %1i %s";
                sprintf(com, as.latin1(), spec->slatitude, spec->slongitude,
                        spec->elatitude, spec->elongitude, spec->scale,
                        (const char *) spec->dir, spec->res,
                        (const char *) spec->param);
            }
            QMessageBox mb(tr("Start Downloading maps."),
                           tr
                           ("     Start Downloading maps...\r\n It can take from few miutes to few hours.\n     Push OK button to start."),
                           QMessageBox::Warning,
                           QMessageBox::Ok | QMessageBox::Default,
                           QMessageBox::NoButton, QMessageBox::NoButton);
            printf("%s \n\n", com);
            mb.exec();
            int status = 0;
            if ((status = system(com)) != 0)
            {
                mb.hide();
                if (status == -1)
                {
                    QMessageBox mb(tr("Runing downmap error."),
                                   tr
                                   ("  Can't execute downmap utility.\nCheck is it exist and acces (execute) rigths to it\r\n   Push OK button."),
                                   QMessageBox::Critical,
                                   QMessageBox::Ok | QMessageBox::Default,
                                   QMessageBox::NoButton,
                                   QMessageBox::NoButton);
                    mb.exec();
                    mb.hide();
                }
                else
                {
                    QMessageBox mb(tr("Downloading error."),
                                   tr
                                   ("    Download ERROR.\n If you use proxy: Run Console (terminal)\n and enter command:\n export http_proxy=http://proxy:<port #>\n then run from terminal qpegps\n Example:\n# export http_proxy=http://mypoxy:8080\n# qpegps\n   Push OK button."),
                                   QMessageBox::Critical,
                                   QMessageBox::Ok | QMessageBox::Default,
                                   QMessageBox::NoButton,
                                   QMessageBox::NoButton);
                    mb.exec();
                    mb.hide();
                }
            }
            else
                mb.hide();
            sleep(2);
            free(com);
//        mapListChanged();
            application->reReadMaps();
            mapListChanged();
            break;
        }
        break;
    }
}

void MapInfo::startImportD()
{
    Position tp;
    QString xxx;
    ImportMapDialog imDialog(maps, this, "import map", TRUE, 0);
    imDialog.setCaption(tr("select image"));
    imDialog.exec();
    if (imDialog.result() == QDialog::Accepted && imDialog.imageSelected)
    {
        QFileInfo fi(imDialog.mapImageLnk.file());

        QString fn = gpsData->mapPathStr;
        fn.append("/" + fi.baseName() + ".png");
        QImage *orgImage;
        orgImage = new QImage(fi.filePath());
        if (orgImage->depth() > 8)      // use max. 8 bit colors to reduce memory consumption
            *orgImage = orgImage->convertDepth(8);
        image = new QPixmap();
        image->convertFromImage(*orgImage);
        delete orgImage;
        image = new QPixmap(fi.filePath());

        MapParDialog mapPar(0, this, "set parameter", TRUE, 0);
        mapPar.setCaption(tr("set projection parameter"));
        mapPar.exec();

        if (mapPar.result() == QDialog::Accepted)
        {
            QString str;
            /* transfer map parameters to map entry */
            switch (mapPar.projectionCB->currentItem())
            {
            case 0:            // LINEAR
                MapLin * mapLin;
                str = fi.baseName() + ".png " + mapPar.scaleLE->text() + " " +
                    str.setNum(image->width()) + " " +
                    str.setNum(image->height()) + " " +
                    xxx.setNum(tp.setLongAndRet(mapPar.lg1LE->text())) + " " +
                    xxx.setNum(tp.setLatAndRet(mapPar.lt1LE->text())) + " " +
                    mapPar.x1LE->text() + " " + mapPar.y1LE->text() + " " +
                    xxx.setNum(tp.setLongAndRet(mapPar.lg2LE->text())) + " " +
                    xxx.setNum(tp.setLatAndRet(mapPar.lt2LE->text())) + " " +
                    mapPar.x2LE->text() + " " + mapPar.y2LE->text();
                mapLin = new MapLin(&str);
                maps->append(mapLin);
                break;

            case 1:            // CEA
                MapCEA * mapCea;
                str = fi.baseName() + ".png " + mapPar.scaleLE->text() + " " +
                    str.setNum(image->width()) + " " +
                    str.setNum(image->height()) + " " +
                    xxx.setNum(tp.setLongAndRet(mapPar.lg1LE->text())) + " " +
                    xxx.setNum(tp.setLatAndRet(mapPar.lt1LE->text())) + " " +
                    mapPar.x1LE->text() + " " + mapPar.y1LE->text() + " " +
                    xxx.setNum(tp.setLongAndRet(mapPar.lg2LE->text())) + " " +
                    xxx.setNum(tp.setLatAndRet(mapPar.lt2LE->text())) + " " +
                    mapPar.x2LE->text() + " " + mapPar.y2LE->text();
                mapCea = new MapCEA(&str);
                maps->append(mapCea);
                break;

            case 2:            // UTM
                MapUTM * mapUtm2;
                str = fi.baseName() + ".png " + mapPar.scaleLE->text() + " " +
                    str.setNum(image->width()) + " " +
                    str.setNum(image->height())
                    + " " +
                    mapPar.zoneLE->text() + " " + mapPar.lt1LE->text() + " " +
                    mapPar.lg1LE->text() + " " +
                    mapPar.x1LE->text() + " " + mapPar.y1LE->text() + " " +
                    mapPar.lt2LE->text() + " " + mapPar.lg2LE->text() + " " +
                    mapPar.x2LE->text() + " " + mapPar.y2LE->text();
                mapUtm2 = new MapUTM(&str, TRUE);
                maps->append(mapUtm2);
                break;

            case 3:            // TM
                MapUTM * mapUtm3;
                str = fi.baseName() + ".png " + mapPar.scaleLE->text() + " " +
                    str.setNum(image->width()) + " " +
                    str.setNum(image->height())
                    + " " +
                    xxx.setNum(tp.setLongAndRet(mapPar.lg1LE->text())) + " " +
                    xxx.setNum(tp.setLatAndRet(mapPar.lt1LE->text())) + " " +
                    mapPar.x1LE->text() + " " + mapPar.y1LE->text() + " " +
                    xxx.setNum(tp.setLongAndRet(mapPar.lg2LE->text())) + " " +
                    xxx.setNum(tp.setLatAndRet(mapPar.lt2LE->text())) + " " +
                    mapPar.x2LE->text() + " " + mapPar.y2LE->text() +
                    " " + xxx.setNum(tp.setLongAndRet(mapPar.stdLongLE->text()));
                mapUtm3 = new MapUTM(&str, FALSE);
                maps->append(mapUtm3);
                break;

            case 4:            // MERCATOR
                MapMercator * mapMercator;
                str = fi.baseName() + ".png " + mapPar.scaleLE->text() + " " +
                    str.setNum(image->width()) + " " +
                    str.setNum(image->height()) + " " +
                    xxx.setNum(tp.setLongAndRet(mapPar.lg1LE->text())) + " " +
                    xxx.setNum(tp.setLatAndRet(mapPar.lt1LE->text())) + " " +
                    mapPar.x1LE->text() + " " + mapPar.y1LE->text() + " " +
                    xxx.setNum(tp.setLongAndRet(mapPar.lg2LE->text())) + " " +
                    xxx.setNum(tp.setLatAndRet(mapPar.lt2LE->text())) + " " +
                    mapPar.x2LE->text() + " " + mapPar.y2LE->text();
                mapMercator = new MapMercator(&str);
                maps->append(mapMercator);
                break;

            case 5:            // LAMBERT
                MapLambert * mapLambert;
                str = fi.baseName() + ".png " + mapPar.scaleLE->text() + " " +
                    str.setNum(image->width()) + " " +
                    str.setNum(image->height()) + " " +
                    xxx.setNum(tp.setLongAndRet(mapPar.lg1LE->text())) + " " +
                    xxx.setNum(tp.setLatAndRet(mapPar.lt1LE->text())) + " " +
                    mapPar.x1LE->text() + " " + mapPar.y1LE->text() + " " +
                    xxx.setNum(tp.setLongAndRet(mapPar.lg2LE->text())) + " " +
                    xxx.setNum(tp.setLatAndRet(mapPar.lt2LE->text())) + " " +
                    mapPar.x2LE->text() + " " + mapPar.y2LE->text() + " " +
                    xxx.setNum(tp.setLatAndRet(mapPar.stdLat1LE->text())) + " " +
                    xxx.setNum(tp.setLatAndRet(mapPar.stdLat2LE->text())) + " " +
                    xxx.setNum(tp.setLongAndRet(mapPar.refLongLE->text()));
                mapLambert = new MapLambert(&str);
                maps->append(mapLambert);
                break;

            case 6:            // FRITZ
                MapFritz * mapFritz;
                str = fi.baseName() + ".png " + mapPar.scaleLE->text() + " " +
                    str.setNum(image->width()) + " " +
                    str.setNum(image->height()) + " " +
                    xxx.setNum(tp.setLatAndRet(mapPar.centerLatLE->text())) + " " +
                    xxx.setNum(tp.setLongAndRet(mapPar.centerLongLE->text()));
                mapFritz = new MapFritz(&str);
                maps->append(mapFritz);
                break;
            }

            if (imDialog.bg && imDialog.bg->id(imDialog.bg->selected()))
            {
                // delete original image file/docLnk
                imDialog.mapImageLnk.removeFiles();
                //QFile mapFileOrg(imDialog.mapImageLnk.file());
                //mapFileOrg.remove();
                //QFile mapFileOrgL(imDialog.mapImageLnk.linkFile());
                //mapFileOrgL.remove();
            }

            if (!image->save(fn, "PNG"))
                qWarning(tr("could not create map image"));

            delete image;
            writeMapFile();
            mapListChanged();

        }
    }
}

void MapInfo::startRemoveD()
{
    RemoveMapDialog rmDialog(maps->at(mapSelect->currentItem()), this,
                             "remove map", TRUE, 0);
    rmDialog.setCaption(tr("Remove Map"));
    rmDialog.exec();
    if (rmDialog.result() == QDialog::Accepted)
    {
        if (rmDialog.deleteCB->isChecked())
        {
            QString filename = gpsData->mapPathStr;
            filename.append(mapSelect->currentText());
            QFile mapImage(filename);
            mapImage.remove();
        }
        maps->remove(mapSelect->currentItem());
        writeMapFile();
        if (mapSelect->currentItem() > 0)
            mapsIndex = mapSelect->currentItem() - 1;
        else
            mapsIndex = 0;
        mapListChanged();
    }
}

void MapInfo::startChangeD()
{
    Position tp;
    QString xxx;
    QString fn = gpsData->mapPathStr;
    fn.append("/" + mapSelect->currentText());
    image = new QPixmap(fn);

    MapParDialog mapPar(0, this, "properties", TRUE, 0);
    mapPar.setCaption(tr("Properties - %1").arg(mapSelect->currentText()));
    // set existing parameters:
    MapBase *aMap;
    int idx = 0;
    aMap = maps->first();
    while ((aMap != 0) && (aMap->name != mapSelect->currentText()))
    {
        if (aMap->name != mapSelect->currentText())
        {
            aMap = maps->next();
            idx++;
        }
    }

    int proIdx;
    if (aMap->projection == "LINEAR")
        proIdx = 0;
    else if (aMap->projection == "CEA")
        proIdx = 1;
    else if (aMap->projection == "UTM")
        proIdx = 2;
    else if (aMap->projection == "TM")
        proIdx = 3;
    else if (aMap->projection == "MERCATOR")
        proIdx = 4;
    else if (aMap->projection == "LAMBERT")
        proIdx = 5;
    else if (aMap->projection == "FRITZ")
        proIdx = 6;
    else
    {
        proIdx = 0;
        qWarning(tr("fetchmap: unknown projection type"));
    };

    mapPar.projectionCB->setCurrentItem(proIdx);
    mapPar.scaleLE->setText(xxx.setNum(aMap->scale));

    switch (proIdx)
    {
    case 0:                    // LINEAR
        mapPar.lg1LE->setText(tp.
                              setLongAndRet(180.0 / MathEx::PI *
                                      ((MapLin *) aMap)->longitude1));
        mapPar.lt1LE->setText(tp.
                              setLatAndRet(180.0 / MathEx::PI * ((MapLin *) aMap)->latitude1));
        mapPar.x1LE->setText(xxx.setNum(((MapLin *) aMap)->x1));
        mapPar.y1LE->setText(xxx.setNum(((MapLin *) aMap)->y1));
        mapPar.lg2LE->setText(tp.
                              setLongAndRet(180.0 / MathEx::PI *
                                      ((MapLin *) aMap)->longitude2));
        mapPar.lt2LE->setText(tp.
                              setLatAndRet(180.0 / MathEx::PI * ((MapLin *) aMap)->latitude2));
        mapPar.x2LE->setText(xxx.setNum(((MapLin *) aMap)->x2));
        mapPar.y2LE->setText(xxx.setNum(((MapLin *) aMap)->y2));
        break;

    case 1:                    // CEA
        mapPar.lg1LE->setText(tp.
                              setLongAndRet(180.0 / MathEx::PI *
                                      ((MapCEA *) aMap)->longitude1));
        mapPar.lt1LE->setText(tp.
                              setLatAndRet(180.0 / MathEx::PI * ((MapCEA *) aMap)->latitude1));
        mapPar.x1LE->setText(xxx.setNum(((MapCEA *) aMap)->x1));
        mapPar.y1LE->setText(xxx.setNum(((MapCEA *) aMap)->y1));
        mapPar.lg2LE->setText(tp.
                              setLongAndRet(180.0 / MathEx::PI *
                                      ((MapCEA *) aMap)->longitude2));
        mapPar.lt2LE->setText(tp.
                              setLatAndRet(180.0 / MathEx::PI * ((MapCEA *) aMap)->latitude2));
        mapPar.x2LE->setText(xxx.setNum(((MapCEA *) aMap)->x2));
        mapPar.y2LE->setText(xxx.setNum(((MapCEA *) aMap)->y2));
        break;

    case 2:                    // UTM
        mapPar.zoneLE->setText(((MapUTM *) aMap)->utmZone);
        mapPar.lt1LE->setText(xxx.setNum(((MapUTM *) aMap)->utmNorthing1));
        mapPar.lg1LE->setText(xxx.setNum(((MapUTM *) aMap)->utmEasting1));
        mapPar.x1LE->setText(xxx.setNum(((MapUTM *) aMap)->x1));
        mapPar.y1LE->setText(xxx.setNum(((MapUTM *) aMap)->y1));
        mapPar.lt2LE->setText(xxx.setNum(((MapUTM *) aMap)->utmNorthing2));
        mapPar.lg2LE->setText(xxx.setNum(((MapUTM *) aMap)->utmEasting2));
        mapPar.x2LE->setText(xxx.setNum(((MapUTM *) aMap)->x2));
        mapPar.y2LE->setText(xxx.setNum(((MapUTM *) aMap)->y2));
        break;

    case 3:                    // TM
        mapPar.lg1LE->setText(tp.
                              setLongAndRet(180.0 / MathEx::PI *
                                      ((MapUTM *) aMap)->longitude1));
        mapPar.lt1LE->setText(tp.
                              setLatAndRet(180.0 / MathEx::PI * ((MapUTM *) aMap)->latitude1));
        mapPar.x1LE->setText(xxx.setNum(((MapUTM *) aMap)->x1));
        mapPar.y1LE->setText(xxx.setNum(((MapUTM *) aMap)->y1));
        mapPar.lg2LE->setText(tp.
                              setLongAndRet(180.0 / MathEx::PI *
                                      ((MapUTM *) aMap)->longitude2));
        mapPar.lt2LE->setText(tp.
                              setLatAndRet(180.0 / MathEx::PI * ((MapUTM *) aMap)->latitude2));
        mapPar.x2LE->setText(xxx.setNum(((MapUTM *) aMap)->x2));
        mapPar.y2LE->setText(xxx.setNum(((MapUTM *) aMap)->y2));
        mapPar.stdLongLE->setText(tp.
                                  setLongAndRet(180.0 / MathEx::PI *
                                          ((MapUTM *) aMap)->stdLong));
        break;

    case 4:                    // MERCATOR
        mapPar.lg1LE->setText(tp.
                              setLongAndRet(180.0 / MathEx::PI *
                                      ((MapMercator *) aMap)->longitude1));
        mapPar.lt1LE->setText(tp.
                              setLatAndRet(180.0 / MathEx::PI *
                                     ((MapMercator *) aMap)->latitude1));
        mapPar.x1LE->setText(xxx.setNum(((MapMercator *) aMap)->x1));
        mapPar.y1LE->setText(xxx.setNum(((MapMercator *) aMap)->y1));
        mapPar.lg2LE->setText(tp.
                              setLongAndRet(180.0 / MathEx::PI *
                                      ((MapMercator *) aMap)->longitude2));
        mapPar.lt2LE->setText(tp.
                              setLatAndRet(180.0 / MathEx::PI *
                                     ((MapMercator *) aMap)->latitude2));
        mapPar.x2LE->setText(xxx.setNum(((MapMercator *) aMap)->x2));
        mapPar.y2LE->setText(xxx.setNum(((MapMercator *) aMap)->y2));
        break;

    case 5:                    // LAMBERT
        mapPar.lg1LE->setText(tp.
                              setLongAndRet(180.0 / MathEx::PI *
                                      ((MapLambert *) aMap)->longitude1));
        mapPar.lt1LE->setText(tp.
                              setLatAndRet(180.0 / MathEx::PI *
                                     ((MapLambert *) aMap)->latitude1));
        mapPar.x1LE->setText(xxx.setNum(((MapLambert *) aMap)->x1));
        mapPar.y1LE->setText(xxx.setNum(((MapLambert *) aMap)->y1));
        mapPar.lg2LE->setText(tp.
                              setLongAndRet(180.0 / MathEx::PI *
                                      ((MapLambert *) aMap)->longitude2));
        mapPar.lt2LE->setText(tp.
                              setLatAndRet(180.0 / MathEx::PI *
                                     ((MapLambert *) aMap)->latitude2));
        mapPar.x2LE->setText(xxx.setNum(((MapLambert *) aMap)->x2));
        mapPar.y2LE->setText(xxx.setNum(((MapLambert *) aMap)->y2));
        mapPar.stdLat1LE->setText(tp.
                                  setLatAndRet(180.0 / MathEx::PI *
                                         ((MapLambert *) aMap)->std1Lat));
        mapPar.stdLat2LE->setText(tp.
                                  setLatAndRet(180.0 / MathEx::PI *
                                         ((MapLambert *) aMap)->std2Lat));
        mapPar.refLongLE->setText(tp.
                                  setLongAndRet(180.0 / MathEx::PI *
                                          ((MapLambert *) aMap)->refLong));
        break;

    case 6:                    // FRITZ
        mapPar.centerLongLE->setText(tp.
                                     setLongAndRet(((MapFritz *) aMap)->
                                             center_longitude));
        mapPar.centerLatLE->setText(tp.
                                    setLatAndRet(((MapFritz *) aMap)->
                                           center_latitude));
        break;
    }

    mapPar.exec();
    if (mapPar.result() == QDialog::Accepted)
    {
        QString str;
        // remove orig map entry
        maps->remove(idx);
        /* transfer map parameters to map entry */
        switch (mapPar.projectionCB->currentItem())
        {
        case 0:                // LINEAR
            MapLin * mapLin;
            str =
                mapSelect->currentText() + " " + mapPar.scaleLE->text() +
                " " + str.setNum(image->width()) + " " +
                str.setNum(image->height()) + " " +
                xxx.setNum(tp.setLongAndRet(mapPar.lg1LE->text())) + " " +
                xxx.setNum(tp.setLatAndRet(mapPar.lt1LE->text())) + " " +
                mapPar.x1LE->text() + " " + mapPar.y1LE->text() + " " +
                xxx.setNum(tp.setLongAndRet(mapPar.lg2LE->text())) + " " +
                xxx.setNum(tp.setLatAndRet(mapPar.lt2LE->text())) + " " +
                mapPar.x2LE->text() + " " + mapPar.y2LE->text();
            mapLin = new MapLin(&str);
            maps->append(mapLin);
            break;

        case 1:                // CEA
            MapCEA * mapCea;
            str =
                mapSelect->currentText() + " " + mapPar.scaleLE->text() +
                " " + str.setNum(image->width()) + " " +
                str.setNum(image->height()) + " " +
                xxx.setNum(tp.setLongAndRet(mapPar.lg1LE->text())) + " " +
                xxx.setNum(tp.setLatAndRet(mapPar.lt1LE->text())) + " " +
                mapPar.x1LE->text() + " " + mapPar.y1LE->text() + " " +
                xxx.setNum(tp.setLongAndRet(mapPar.lg2LE->text())) + " " +
                xxx.setNum(tp.setLatAndRet(mapPar.lt2LE->text())) + " " +
                mapPar.x2LE->text() + " " + mapPar.y2LE->text();
            mapCea = new MapCEA(&str);
            maps->append(mapCea);
            break;

        case 2:                // UTM
            MapUTM * mapUtm2;
            str =
                mapSelect->currentText() + " " + mapPar.scaleLE->text() +
                " " + str.setNum(image->width()) + " " +
                str.setNum(image->height()) + " " + mapPar.zoneLE->text() +
                " " + mapPar.lt1LE->text() + " " + mapPar.lg1LE->text() +
                " " + mapPar.x1LE->text() + " " + mapPar.y1LE->text() + " " +
                mapPar.lt2LE->text() + " " + mapPar.lg2LE->text() + " " +
                mapPar.x2LE->text() + " " + mapPar.y2LE->text();
            mapUtm2 = new MapUTM(&str, TRUE);
            maps->append(mapUtm2);
            break;

        case 3:                // TM
            MapUTM * mapUtm3;
            str =
                mapSelect->currentText() + " " + mapPar.scaleLE->text() +
                " " + str.setNum(image->width()) + " " +
                str.setNum(image->height()) + " " +
                xxx.setNum(tp.setLongAndRet(mapPar.lg1LE->text())) + " " +
                xxx.setNum(tp.setLatAndRet(mapPar.lt1LE->text())) + " " +
                mapPar.x1LE->text() + " " + mapPar.y1LE->text() + " " +
                xxx.setNum(tp.setLongAndRet(mapPar.lg2LE->text())) + " " +
                xxx.setNum(tp.setLatAndRet(mapPar.lt2LE->text())) + " " +
                mapPar.x2LE->text() + " " + mapPar.y2LE->text() + " " +
                xxx.setNum(tp.setLongAndRet(mapPar.stdLongLE->text()));
            mapUtm3 = new MapUTM(&str, FALSE);
            maps->append(mapUtm3);
            break;

        case 4:                // MERCATOR
            MapMercator * mapMercator;
            str =
                mapSelect->currentText() + " " + mapPar.scaleLE->text() +
                " " + str.setNum(image->width()) + " " +
                str.setNum(image->height()) + " " +
                xxx.setNum(tp.setLongAndRet(mapPar.lg1LE->text())) + " " +
                xxx.setNum(tp.setLatAndRet(mapPar.lt1LE->text())) + " " +
                mapPar.x1LE->text() + " " + mapPar.y1LE->text() + " " +
                xxx.setNum(tp.setLongAndRet(mapPar.lg2LE->text())) + " " +
                xxx.setNum(tp.setLatAndRet(mapPar.lt2LE->text())) + " " +
                mapPar.x2LE->text() + " " + mapPar.y2LE->text();
            mapMercator = new MapMercator(&str);
            maps->append(mapMercator);
            break;

        case 5:                // LAMBERT
            MapLambert * mapLambert;
            str =
                mapSelect->currentText() + " " + mapPar.scaleLE->text() +
                " " + str.setNum(image->width()) + " " +
                str.setNum(image->height()) + " " +
                xxx.setNum(tp.setLongAndRet(mapPar.lg1LE->text())) + " " +
                xxx.setNum(tp.setLatAndRet(mapPar.lt1LE->text())) + " " +
                mapPar.x1LE->text() + " " + mapPar.y1LE->text() + " " +
                xxx.setNum(tp.setLongAndRet(mapPar.lg2LE->text())) + " " +
                xxx.setNum(tp.setLatAndRet(mapPar.lt2LE->text())) + " " +
                mapPar.x2LE->text() + " " + mapPar.y2LE->text() + " " +
                xxx.setNum(tp.setLatAndRet(mapPar.stdLat1LE->text())) + " " +
                xxx.setNum(tp.setLatAndRet(mapPar.stdLat2LE->text())) + " " +
                xxx.setNum(tp.setLongAndRet(mapPar.refLongLE->text()));
            mapLambert = new MapLambert(&str);
            maps->append(mapLambert);
            break;

        case 6:                // FRITZ
            MapFritz * mapFritz;
            str =
                mapSelect->currentText() + " " + mapPar.scaleLE->text() +
                " " + str.setNum(image->width()) + " " +
                str.setNum(image->height()) + " " +
                xxx.setNum(tp.setLatAndRet(mapPar.centerLatLE->text())) + " " +
                xxx.setNum(tp.setLongAndRet(mapPar.centerLongLE->text()));
            mapFritz = new MapFritz(&str);
            maps->append(mapFritz);
            break;
        }

        writeMapFile();
        mapListChanged();
    }
}

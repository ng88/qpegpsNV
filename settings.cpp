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


#include "settings.h"

Settings::Settings(GpsData * gData, QWidget * parent, const char *name,
                   WFlags fl):QScrollView(parent, name, fl)
{
    gpsData = gData;

    setHScrollBarMode(AlwaysOff);
    setVScrollBarMode(Auto);
    mainBox = new QVBox(this);
    addChild(mainBox);
    setResizePolicy(AutoOneFit);
    mainBox->
        setSizePolicy(QSizePolicy
                      (QSizePolicy::Maximum, QSizePolicy::Maximum));

    horbox3 = new QHBox(mainBox);
    mapDir = new QLabel(tr("Map directory:"), horbox3);
    mapPath = new QLineEdit(horbox3);
    mapPathB = new QPushButton(tr("search"), horbox3);

    geoDatL = new QLabel(tr("GEO Datum:"), mainBox);
    horbox12 = new QHBox(mainBox);
    geoDatGpsL = new QLabel(tr("GPS"), horbox12);
    geoDatGpsL->setAlignment(AlignCenter | AlignVCenter);
    geoDatGpsCB = new QComboBox(horbox12);
    geoDatGpsCB->insertStringList(gpsData->geoDatum.getDatumList());
    geoDatGpsCB->setEditable(false);
    horbox13 = new QHBox(mainBox);
    geoDatMapL = new QLabel(tr("Map"), horbox13);
    geoDatMapL->setAlignment(AlignCenter | AlignVCenter);
    geoDatMapCB = new QComboBox(horbox13);
    geoDatMapCB->insertStringList(gpsData->geoDatum.getDatumList());
    geoDatMapCB->setEditable(false);

    horbox6 = new QHBox(mainBox);
    altL = new QLabel(tr("Altitude"), horbox6);
    altL->setAlignment(AlignCenter | AlignVCenter);
    altCB = new QComboBox(false, horbox6, "Altitude");
    altCB->insertItem(tr("none"));
    altCB->insertItem(tr("m"));
    altCB->insertItem(tr("feet"));
    altCB->insertItem(tr("FL"));
    horbox8 = new QHBox(mainBox);
    speedL = new QLabel(tr("Speed"), horbox8);
    speedL->setAlignment(AlignCenter | AlignVCenter);
    speedCB = new QComboBox(false, horbox8, "Speed");
    speedCB->insertItem(tr("none"));
    speedCB->insertItem(tr("kmh"));
    speedCB->insertItem(tr("kn"));
    speedCB->insertItem(tr("mph"));

    horbox7 = new QHBox(mainBox);
    distL = new QLabel(tr("Distance"), horbox7);
    distL->setAlignment(AlignCenter | AlignVCenter);
    distCB = new QComboBox(false, horbox7, "Distance");
    distCB->insertItem(tr("none"));
    distCB->insertItem(tr("km"));
    distCB->insertItem(tr("nmi"));
    distCB->insertItem(tr("mi"));

    horbox9 = new QHBox(mainBox);
    posL = new QLabel(tr("Position"), horbox9);
    posL->setAlignment(AlignCenter | AlignVCenter);
    posCB = new QComboBox(false, horbox9, "Position");
    posCB->insertItem(tr("DD.d"));
    posCB->insertItem(tr("DDMM.m'"));
    posCB->insertItem(tr("DDMM'SS.s''"));

    horbox10 = new QHBox(mainBox);
    colorB = new QPushButton(tr("Colors"), horbox10);
    textSizeL = new QLabel(tr("Text Size: "), horbox10);
    textSizeL->setAlignment(AlignRight | AlignVCenter);
    textSizeSB = new QSpinBox(4, 30, 1, horbox10, "Text Size");

    horbox1 = new QHBox(mainBox);
    checkLabel = new QLabel(tr("display:"), horbox1);
    bearTB = new QCheckBox(horbox1);
    bearTB->setText(tr("Bearing"));
    headTB = new QCheckBox(horbox1);
    headTB->setText(tr("Heading"));
    timeTB = new QCheckBox(horbox1);
    timeTB->setText(tr("Time"));

    //read config file
    qpegpsConfig = new Config("qpegps");
    qpegpsConfig->setGroup("units");
    gpsData->altitude.altUnit =
        (Altitude::Alt) qpegpsConfig->readNumEntry("altitude",
                                                   Altitude::Feet);
    gpsData->speed.speedUnit =
        (Speed::Sp) qpegpsConfig->readNumEntry("speed", Speed::Knots);
    gpsData->wpDistance.distUnit =
        (Distance::Dist) qpegpsConfig->readNumEntry("distance",
                                                    Distance::Naut);
    gpsData->currPos.posUnit =
        (Position::Pos) qpegpsConfig->readNumEntry("position",
                                                   Position::DegMin);

    qpegpsConfig->setGroup("show");
    gpsData->bearing.setShow( qpegpsConfig->readBoolEntry("bearing", true) );
    gpsData->heading.setShow( qpegpsConfig->readBoolEntry("heading", true) );
    gpsData->showTime = qpegpsConfig->readBoolEntry("time", true);

    qpegpsConfig->setGroup("gps");
    gpsData->gpsdArgStr = qpegpsConfig->readEntry("gpsd", gpsdDefaultArg);
    gpsData->host = qpegpsConfig->readEntry("host", gpsdDefaultHost);
    gpsData->port = qpegpsConfig->readNumEntry("port", gpsdDefaultPort);

    gpsData->startup_name = qpegpsConfig->readEntry("StartupPlace", "");        /* Added by A. Karhov */
    gpsData->startup_mode = qpegpsConfig->readBoolEntry("StartupMode", false);  /* Added by A. Karhov */
    gpsData->draw_places = qpegpsConfig->readBoolEntry("DrawPlaces", false);    /* Added by A. Karhov */


    qpegpsConfig->setGroup("map");
    gpsData->mapPathStr =
        qpegpsConfig->readEntry("path", gpsData->qpedir + "/qpegps/maps");

    qpegpsConfig->setGroup("download");

    QDir md(gpsData->mapPathStr);

    //gpsData->mapPathStr = md.canonicalPath();

    qpegpsConfig->setGroup("icons");
    gpsData->iconsPathStr =
        qpegpsConfig->readEntry("path", gpsData->qpedir + "/qpegps/icons");
    QDir md2(gpsData->iconsPathStr);
    //gpsData->iconsPathStr = md2.canonicalPath();

    qpegpsConfig->setGroup("datum");
    gpsData->gpsDatumIdx = qpegpsConfig->readEntry("GpsDatum", "1").toInt();
    gpsData->mapDatumIdx = qpegpsConfig->readEntry("MapsDatum", "1").toInt();

    qpegpsConfig->setGroup("color");

    gpsData->statusOkColor.setNamedColor(qpegpsConfig->readEntry("ok", "#000000"));
    gpsData->statusNoFixColor.setNamedColor(qpegpsConfig->readEntry("noFix", "#000000"));
    gpsData->headColor.setNamedColor(qpegpsConfig->readEntry("heading", "#00FF00"));
    gpsData->bearColor.setNamedColor(qpegpsConfig->readEntry("bearing", "#FF0000"));
    gpsData->trackColor.setNamedColor(qpegpsConfig->readEntry("trackC", "#FF0000"));
    gpsData->scaleColor.setNamedColor(qpegpsConfig->readEntry("scale", "#FF0000"));
    gpsData->waypointColor.setNamedColor(qpegpsConfig->readEntry("waypoint", "#FF0000"));
    gpsData->routeColor.setNamedColor(qpegpsConfig->readEntry("routeC", "#FF0000"));
    gpsData->routeIconColor.setNamedColor(qpegpsConfig->readEntry("routeIcon", "#FFFFFF"));
    gpsData->routeIconTxtColor.setNamedColor(qpegpsConfig->readEntry("routeIconTxt", "#000000"));
    gpsData->routePosLineColor.setNamedColor(qpegpsConfig->readEntry("routePosLine", "#0000FF"));
        
    
    gpsData->textSize = qpegpsConfig->readEntry("textSize", "15").toInt();

    qpegpsConfig->setGroup("track");
    gpsData->trackPathStr = qpegpsConfig->readEntry("path",
                                                    gpsData->qpedir +
                                                    "/qpegps/tracks");
    QDir md3(gpsData->trackPathStr);
    //gpsData->trackPathStr = md3.canonicalPath();
    gpsData->updt_freq = qpegpsConfig->readEntry("updt_freq", "10").toInt();
    gpsData->track_thick =
        qpegpsConfig->readEntry("track_thick", "2").toInt();

    // create config file
    writeConfig();

    // set buttons...
    altCB->setCurrentItem((int) gpsData->altitude.altUnit);
    speedCB->setCurrentItem((int) gpsData->speed.speedUnit);
    distCB->setCurrentItem((int) gpsData->wpDistance.distUnit);
    posCB->setCurrentItem((int) gpsData->currPos.posUnit);
    bearTB->setChecked(gpsData->bearing.show());
    headTB->setChecked(gpsData->heading.show());
    timeTB->setChecked(gpsData->showTime);
    mapPath->setText(gpsData->mapPathStr);
    textSizeSB->setValue(gpsData->textSize);

    geoDatGpsCB->setCurrentItem(gpsData->gpsDatumIdx - 1);
    geoDatMapCB->setCurrentItem(gpsData->mapDatumIdx - 1);

    connect(altCB, SIGNAL(activated(int)), SLOT(setAlt(int)));
    connect(speedCB, SIGNAL(activated(int)), SLOT(setSpeed(int)));
    connect(distCB, SIGNAL(activated(int)), SLOT(setDist(int)));
    connect(posCB, SIGNAL(activated(int)), SLOT(setPos(int)));
    connect(bearTB, SIGNAL(toggled(bool)), SLOT(setBear(bool)));
    connect(headTB, SIGNAL(toggled(bool)), SLOT(setHead(bool)));
    connect(timeTB, SIGNAL(toggled(bool)), SLOT(setTime(bool)));
    connect(mapPathB, SIGNAL(pressed()), SLOT(setMapPath()));
    connect(colorB, SIGNAL(pressed()), SLOT(setColors()));
    connect(mapPath, SIGNAL(returnPressed()), SLOT(mapPathLEChanged()));

    connect(textSizeSB, SIGNAL(valueChanged(int)),
            SLOT(textSizeChanged(int)));
    connect(geoDatGpsCB, SIGNAL(activated(int)), SLOT(geoDatGpsChanged(int)));
    connect(geoDatMapCB, SIGNAL(activated(int)), SLOT(geoDatMapChanged(int)));


}


Settings::~Settings()
{
}

void Settings::writeConfig()
{

    qpegpsConfig->setGroup("units");
    qpegpsConfig->writeEntry("altitude", (int) gpsData->altitude.altUnit);
    qpegpsConfig->writeEntry("speed", (int) gpsData->speed.speedUnit);
    qpegpsConfig->writeEntry("distance", (int) gpsData->wpDistance.distUnit);
    qpegpsConfig->writeEntry("position", (int) gpsData->currPos.posUnit);
    qpegpsConfig->setGroup("show");
    qpegpsConfig->writeEntry("bearing", gpsData->bearing.show());
    qpegpsConfig->writeEntry("heading", gpsData->heading.show());
    qpegpsConfig->writeEntry("time", gpsData->showTime);
    qpegpsConfig->setGroup("gps");
    qpegpsConfig->writeEntry("gpsd", gpsData->gpsdArgStr);
    qpegpsConfig->writeEntry("host", gpsData->host);
    qpegpsConfig->writeEntry("port", gpsData->port);
    qpegpsConfig->writeEntry("StartupPlace", gpsData->startup_name);    /* Added by A. Karhov */
    qpegpsConfig->writeEntry("StartupMode", gpsData->startup_mode);     /* Added by A. Karhov */
    qpegpsConfig->writeEntry("DrawPlaces", gpsData->draw_places);       /* Added by A. Karhov */
    qpegpsConfig->setGroup("map");
    qpegpsConfig->writeEntry("path", gpsData->mapPathStr);
    qpegpsConfig->setGroup("download");

    qpegpsConfig->setGroup("color");

    qpegpsConfig->writeEntry("ok", gpsData->statusOkColor.name());
    qpegpsConfig->writeEntry("noFix", gpsData->statusNoFixColor.name());
    qpegpsConfig->writeEntry("heading", gpsData->headColor.name());
    qpegpsConfig->writeEntry("bearing", gpsData->bearColor.name());
    qpegpsConfig->writeEntry("trackC", gpsData->trackColor.name());
    qpegpsConfig->writeEntry("scale", gpsData->scaleColor.name());
    qpegpsConfig->writeEntry("waypoint", gpsData->waypointColor.name());
    qpegpsConfig->writeEntry("routeC", gpsData->routeColor.name());
    qpegpsConfig->writeEntry("routeIcon", gpsData->routeIconColor.name());
    qpegpsConfig->writeEntry("routeIconTxt", gpsData->routeIconTxtColor.name());
    qpegpsConfig->writeEntry("routePosLine", gpsData->routePosLineColor.name());

    qpegpsConfig->writeEntry("textSize", gpsData->textSize);
    qpegpsConfig->setGroup("track");
    qpegpsConfig->writeEntry("path", gpsData->trackPathStr);
    qpegpsConfig->writeEntry("updt_freq", gpsData->updt_freq);
    qpegpsConfig->writeEntry("track_thick", gpsData->track_thick);
    qpegpsConfig->setGroup("datum");
    qpegpsConfig->writeEntry("GpsDatum", (int) gpsData->gpsDatumIdx);
    qpegpsConfig->writeEntry("MapsDatum", (int) gpsData->mapDatumIdx);

    qpegpsConfig->~Config();
    qpegpsConfig = new Config("qpegps");        /* uggly, but works ... */

}

void Settings::setAlt(int id)
{
    gpsData->altitude.altUnit = (Altitude::Alt) id;
    writeConfig();
}
void Settings::setSpeed(int id)
{
    gpsData->speed.speedUnit = (Speed::Sp) id;
    writeConfig();
}
void Settings::setDist(int id)
{
    gpsData->wpDistance.distUnit = (Distance::Dist) id;
    writeConfig();
}
void Settings::setPos(int id)
{
    gpsData->currPos.posUnit = (Position::Pos) id;
    writeConfig();
}

void Settings::setBear(bool state)
{
    gpsData->bearing.setShow(state);
    writeConfig();
}

void Settings::setHead(bool state)
{
    gpsData->heading.setShow(state);
    writeConfig();
}

void Settings::setTime(bool state)
{
    gpsData->showTime = state;
    writeConfig();
}


void Settings::mapPathLEChanged()
{
    gpsData->mapPathStr = mapPath->text();
    QDir md(gpsData->mapPathStr);

    //gpsData->mapPathStr = md.canonicalPath();
    writeConfig();
    emit mapPathChanged();
}

void Settings::setMapPath()
{
    // FIXME *gpsData->mapPathStr = QFileDialog::getExistingDirectory(*gpsData->mapPathStr);
    DirDialog getDirDialog(this, 0, true, 0);
    getDirDialog.setCaption(tr("select map directory"));
    getDirDialog.exec();
    if (getDirDialog.result() == QDialog::Accepted)
    {
        gpsData->mapPathStr = getDirDialog.selectedPath();
        QDir md(gpsData->mapPathStr);

        //gpsData->mapPathStr = md.canonicalPath();
        mapPath->setText(gpsData->mapPathStr);
        writeConfig();
        emit mapPathChanged();
    }
}
void Settings::setColors()
{
    QValueList<QColor>colors;
    colors.append(gpsData->statusOkColor);
    colors.append(gpsData->statusNoFixColor);
    colors.append(gpsData->headColor);
    colors.append(gpsData->bearColor);
    colors.append(gpsData->trackColor);
    colors.append(gpsData->scaleColor);
    colors.append(gpsData->waypointColor);
    colors.append(gpsData->routeColor);
    colors.append(gpsData->routeIconColor);
    colors.append(gpsData->routeIconTxtColor);
    colors.append(gpsData->routePosLineColor);
    QStringList items;
    items.append(tr("for status \"GPS OK\""));
    items.append(tr("for status \"no position fix\""));
    items.append(tr("heading"));
    items.append(tr("bearing"));
    items.append(tr("track"));
    items.append(tr("scale"));
    items.append(tr("waypoint"));
    items.append(tr("route"));
    items.append(tr("route icon"));
    items.append(tr("route icon text"));
    items.append(tr("route pos. line"));

    ColorDlg setColorDialog(&colors, items, this, 0, true, 0);
    setColorDialog.setCaption(tr("assign colors"));
    setColorDialog.exec();
    if (setColorDialog.result() == QDialog::Accepted)
    {
        gpsData->statusOkColor = colors[0];
        gpsData->statusNoFixColor = colors[1];
        gpsData->headColor = colors[2];
        gpsData->bearColor = colors[3];
        gpsData->trackColor = colors[4];
        gpsData->scaleColor = colors[5];
        gpsData->waypointColor = colors[6];
        gpsData->routeColor = colors[7];
        gpsData->routeIconColor = colors[8];
        gpsData->routeIconTxtColor = colors[9];
        gpsData->routePosLineColor = colors[10];

        writeConfig();
    }
}


void Settings::textSizeChanged(int idx)
{
    gpsData->textSize = idx;
    writeConfig();
}

void Settings::geoDatGpsChanged(int idx)
{
    gpsData->gpsDatumIdx = idx + 1;
    writeConfig();
}

void Settings::geoDatMapChanged(int idx)
{
    gpsData->mapDatumIdx = idx + 1;
    writeConfig();
}

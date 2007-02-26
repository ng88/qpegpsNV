/*
  qpegps is a program for displaying a map centered at the current longitude/
  latitude as read from a gps receiver.

  Copyright (C) 2002 Ralf Haselmeier <Ralf.Haselmeier@gmx.de>

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

Settings::Settings(GpsData *gData, QWidget *parent, const char *name, WFlags fl):
        QScrollView (parent, name, fl)
{
    gpsData = gData;
#ifndef DESKTOP
    setHScrollBarMode(AlwaysOff);
    setVScrollBarMode(Auto);
    mainBox = new QVBox(this);
    addChild(mainBox);
    setResizePolicy(AutoOneFit);
    mainBox->setSizePolicy(QSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum));

    horbox3 = new QHBox(mainBox);
    mapDir = new QLabel(tr("Map directory:"),horbox3);
    mapPath = new QLineEdit(horbox3);
    mapPathB = new QPushButton(tr("search"),horbox3);
/*
    horbox11 = new QHBox(mainBox);
    proxyUrlL = new QLabel(tr("Use Proxy:"),horbox11);
    proxyUrlLE = new QLineEdit(horbox11);
*/
#ifdef PROXYB

    horbox11 = new QHBox(mainBox);
    proxyCB= new QCheckBox(tr("Use proxy"),horbox11,"Use proxy");
//    horbox3 = new QHBox(mainBox);
    proxyL = new QLabel(tr(" Proxy addres:"),horbox11);
    proxyLE = new QLineEdit(horbox11);
 #endif

    geoDatL = new QLabel(tr("GEO Datum:"),mainBox);
    horbox12= new QHBox(mainBox);
    geoDatGpsL = new QLabel(tr("GPS"), horbox12);
    geoDatGpsL->setAlignment(AlignCenter | AlignVCenter);
    geoDatGpsCB = new QComboBox(horbox12);
    geoDatGpsCB->insertStringList(gpsData->geoDatum.getDatumList());
    geoDatGpsCB->setEditable(FALSE);
    horbox13= new QHBox(mainBox);
    geoDatMapL = new QLabel(tr("Map"), horbox13);
    geoDatMapL->setAlignment(AlignCenter | AlignVCenter);
    geoDatMapCB = new QComboBox(horbox13);
    geoDatMapCB->insertStringList(gpsData->geoDatum.getDatumList());
    geoDatMapCB->setEditable(FALSE);

    horbox6 = new QHBox(mainBox);
    altL = new QLabel(tr("Altitude"), horbox6);
    altL->setAlignment(AlignCenter | AlignVCenter);
    altCB = new QComboBox(FALSE, horbox6, "Altitude");
    altCB->insertItem(tr("none"));
    altCB->insertItem(tr("m"));
    altCB->insertItem(tr("feet"));
    altCB->insertItem(tr("FL"));
    horbox8 = new QHBox(mainBox);
    speedL = new QLabel(tr("Speed"), horbox8);
    speedL->setAlignment(AlignCenter | AlignVCenter);
    speedCB = new QComboBox(FALSE, horbox8, "Speed");
    speedCB->insertItem(tr("none"));
    speedCB->insertItem(tr("kmh"));
    speedCB->insertItem(tr("kn"));
    speedCB->insertItem(tr("mph"));

    horbox7 = new QHBox(mainBox);
    distL = new QLabel(tr("Distance"), horbox7);
    distL->setAlignment(AlignCenter | AlignVCenter);
    distCB = new QComboBox(FALSE, horbox7, "Distance");
    distCB->insertItem(tr("none"));
    distCB->insertItem(tr("km"));
    distCB->insertItem(tr("nmi"));
    distCB->insertItem(tr("mi"));

    horbox9 = new QHBox(mainBox);
    posL = new QLabel(tr("Position"), horbox9);
    posL->setAlignment(AlignCenter | AlignVCenter);
    posCB = new QComboBox(FALSE, horbox9, "Position");
    posCB->insertItem(tr("DD.d"));
    posCB->insertItem(tr("DDMM.m'"));
    posCB->insertItem(tr("DDMM'SS.s''"));

    horbox10 = new QHBox(mainBox);
    colorB = new QPushButton(tr("Colors"),horbox10);
    textSizeL = new QLabel(tr("Text Size: "), horbox10);
    textSizeL->setAlignment(AlignRight | AlignVCenter);
    textSizeSB = new QSpinBox(4, 30, 1, horbox10, "Text Size");

    horbox1 = new QHBox(mainBox);
    checkLabel = new QLabel(tr("display:"), horbox1 );
    bearTB = new QCheckBox(horbox1);
    bearTB->setText(tr("Bearing"));
    headTB = new QCheckBox(horbox1);
    headTB->setText(tr("Heading"));
    timeTB = new QCheckBox(horbox1);
    timeTB->setText(tr("Time"));

/*
    colorList << "black" << "white" << "darkGray" << "gray" << "lightGray" << "red" << "green"
    << "blue" << "cyan" << "magenta" << "yellow" << "darkRed" << "darkGreen" << "darkBlue"
    << "darkCyan" << "darkMagenta" << "darkYellow" << "color0" << "color1";
    qColorPtrList.append(new QColor(Qt::black));
    qColorPtrList.append(new QColor(Qt::white));
    qColorPtrList.append(new QColor(Qt::darkGray));
    qColorPtrList.append(new QColor(Qt::gray));
    qColorPtrList.append(new QColor(Qt::lightGray));
    qColorPtrList.append(new QColor(Qt::red));
    qColorPtrList.append(new QColor(Qt::green));
    qColorPtrList.append(new QColor(Qt::blue));
    qColorPtrList.append(new QColor(Qt::cyan));
    qColorPtrList.append(new QColor(Qt::magenta));
    qColorPtrList.append(new QColor(Qt::yellow));
    qColorPtrList.append(new QColor(Qt::darkRed));
    qColorPtrList.append(new QColor(Qt::darkGreen));
    qColorPtrList.append(new QColor(Qt::darkBlue));
    qColorPtrList.append(new QColor(Qt::darkCyan));
    qColorPtrList.append(new QColor(Qt::darkMagenta));
    qColorPtrList.append(new QColor(Qt::darkYellow));
    qColorPtrList.append(new QColor(Qt::color0));
    qColorPtrList.append(new QColor(Qt::color1));
    horbox4 = new QHBox(mainBox);
    okLabel = new QLabel(tr("GPS-OK"),horbox4);
    okLabel->setAlignment(AlignCenter | AlignVCenter | ExpandTabs);
    okColorCB = new MenuButton(colorList, horbox4);
    noFixLabel = new QLabel(tr("GPS-noFix"),horbox4);
    noFixLabel->setAlignment(AlignCenter | AlignVCenter | ExpandTabs);
    noFixColorCB = new MenuButton(colorList, horbox4);
    horbox5 = new QHBox(mainBox);
    headColorLabel = new QLabel(tr("heading"),horbox5);
    headColorLabel->setAlignment(AlignCenter | AlignVCenter | ExpandTabs);
    headColorCB = new MenuButton(colorList, horbox5);
    bearColorLabel = new QLabel(tr("bearing"),horbox5);
    bearColorLabel->setAlignment(AlignCenter | AlignVCenter | ExpandTabs);
    bearColorCB = new MenuButton(colorList, horbox5);
*/

    //read config file
    qpegpsConfig = new Config("qpegps");
    qpegpsConfig->setGroup("units");
    gpsData->altitude.altUnit = (Altitude::Alt) qpegpsConfig->readNumEntry("altitude", Altitude::Feet);
    gpsData->speed.speedUnit = (Speed::Sp) qpegpsConfig->readNumEntry("speed", Speed::Knots);
    gpsData->wpDistance.distUnit = (Distance::Dist) qpegpsConfig->readNumEntry("distance", Distance::Naut);
    gpsData->currPos.posUnit = (Position::Pos) qpegpsConfig->readNumEntry("position", Position::DegMin);

    qpegpsConfig->setGroup("show");
    gpsData->bearing.show = qpegpsConfig->readBoolEntry("bearing", TRUE);
    gpsData->heading.show = qpegpsConfig->readBoolEntry("heading", TRUE);
    gpsData->showTime = qpegpsConfig->readBoolEntry("time", TRUE);

    qpegpsConfig->setGroup("gps");
    gpsData->gpsdArgStr = qpegpsConfig->readEntry("gpsd",gpsdDefaultArg);
    gpsData->host = qpegpsConfig->readEntry("host", gpsdDefaultHost);
    gpsData->port = qpegpsConfig->readNumEntry("port",gpsdDefaultPort);

    gpsData->startup_name=qpegpsConfig->readEntry("StartupPlace","");       /* Added by A. Karhov */
    gpsData->startup_mode=qpegpsConfig->readBoolEntry("StartupMode", FALSE);       /* Added by A. Karhov */
    gpsData->draw_places=qpegpsConfig->readBoolEntry("DrawPlaces", FALSE);       /* Added by A. Karhov */


    qpegpsConfig->setGroup("map");
    gpsData->mapPathStr = qpegpsConfig->readEntry("path",gpsData->qpedir+"/qpegps/maps");

    qpegpsConfig->setGroup("download");

    gpsData->proxyUrl = qpegpsConfig->readEntry("proxy","");

    QDir md(gpsData->mapPathStr);

    //gpsData->mapPathStr = md.canonicalPath();

    qpegpsConfig->setGroup("icons");
    gpsData->iconsPathStr = qpegpsConfig->readEntry("path",gpsData->qpedir+"/qpegps/icons");
    QDir md2(gpsData->iconsPathStr);
    //gpsData->iconsPathStr = md2.canonicalPath();

    qpegpsConfig->setGroup("datum");
    gpsData->gpsDatumIdx = qpegpsConfig->readEntry("GpsDatum","1").toInt();
    gpsData->mapDatumIdx = qpegpsConfig->readEntry("MapsDatum","1").toInt();

    qpegpsConfig->setGroup("color");

    okColorName = qpegpsConfig->readEntry("ok","black");
    noFixColorName = qpegpsConfig->readEntry("noFix","yellow");
    headColorName = qpegpsConfig->readEntry("heading","green");
    bearColorName = qpegpsConfig->readEntry("bearing","red");
    trackColorName = qpegpsConfig->readEntry("trackC","red");
    scaleColorName = qpegpsConfig->readEntry("scale","red");  /* Added by A. Karhov */
    waypointColorName = qpegpsConfig->readEntry("waypoint","red");  /* Added by A. Karhov */

    gpsData->textSize = qpegpsConfig->readEntry("textSize","15").toInt();

    qpegpsConfig->setGroup("track");
    gpsData->trackPathStr = qpegpsConfig->readEntry("path",
						    gpsData->qpedir+"/qpegps/tracks");
    QDir md3(gpsData->trackPathStr);
    //gpsData->trackPathStr = md3.canonicalPath();
    gpsData->updt_freq = qpegpsConfig->readEntry("updt_freq","10").toInt();
    gpsData->track_thick = qpegpsConfig->readEntry("track_thick","2").toInt();

    // create config file
    writeConfig();

    // set buttons...
    altCB->setCurrentItem((int)gpsData->altitude.altUnit);
    speedCB->setCurrentItem((int)gpsData->speed.speedUnit);
    distCB->setCurrentItem((int)gpsData->wpDistance.distUnit);
    posCB->setCurrentItem((int)gpsData->currPos.posUnit);
    bearTB->setChecked(gpsData->bearing.show);
    headTB->setChecked(gpsData->heading.show);
    timeTB->setChecked(gpsData->showTime);
    mapPath->setText(gpsData->mapPathStr);
    textSizeSB->setValue(gpsData->textSize);
    // proxyUrlLE->setText(gpsData->proxyUrl);
    geoDatGpsCB->setCurrentItem(gpsData->gpsDatumIdx-1);
    geoDatMapCB->setCurrentItem(gpsData->mapDatumIdx-1);

    connect( altCB, SIGNAL(activated(int)),
             SLOT(setAlt(int)) );
    connect( speedCB, SIGNAL(activated(int)),
             SLOT(setSpeed(int)) );
    connect( distCB, SIGNAL(activated(int)),
             SLOT(setDist(int)) );
    connect( posCB, SIGNAL(activated(int)),
             SLOT(setPos(int)) );
    connect( bearTB, SIGNAL(toggled(bool)),
             SLOT(setBear(bool)) );
    connect( headTB, SIGNAL(toggled(bool)),
             SLOT(setHead(bool)) );
    connect( timeTB, SIGNAL(toggled(bool)),
             SLOT(setTime(bool)) );
    connect( mapPathB, SIGNAL(pressed()),
             SLOT(setMapPath()) );
    connect( colorB, SIGNAL(pressed()),
             SLOT(setColors()) );
    connect( mapPath, SIGNAL(returnPressed()),
             SLOT(mapPathLEChanged()) );
/*
    connect( proxyUrlLE, SIGNAL(returnPressed()),
             SLOT(proxyUrlLEChanged()) );
    connect( okColorCB , SIGNAL(selected(int)),
             SLOT(okColorChanged(int)) );
    connect( noFixColorCB , SIGNAL(selected(int)),
             SLOT(noFixColorChanged(int)) );
    connect( headColorCB , SIGNAL(selected(int)),
             SLOT(headColorChanged(int)) );
    connect( bearColorCB , SIGNAL(selected(int)),
             SLOT(bearColorChanged(int)) );
*/
    connect( textSizeSB , SIGNAL(valueChanged(int)),
             SLOT(textSizeChanged(int)) );
    connect( geoDatGpsCB , SIGNAL(activated(int)),
             SLOT(geoDatGpsChanged(int)) );
    connect( geoDatMapCB , SIGNAL(activated(int)),
             SLOT(geoDatMapChanged(int)) );
#ifdef PROXYB
    connect( proxyCB, SIGNAL( toggled(bool) ), this, SLOT( toggledpCB(bool) ) );
    connect( proxyLE, SIGNAL(returnPressed()),   SLOT(proxyLEChanged()) );
 #endif
 #ifdef PROXYB
    if (gpsData->proxyUrl.length() > 1 )
    	{ 	gpsData->useProxy=TRUE;
		proxyLE->setText(gpsData->proxyUrl);
		proxyCB->setChecked(TRUE);
		proxyL->show();
		proxyLE->show();
	}
	else {
	gpsData->useProxy=FALSE;
	proxyLE->setText("http:/x.y:8080");
	proxyCB->setChecked(FALSE);
	proxyL->hide();
	proxyLE->hide();
	}
#endif


/*
    okColorCB->select(okColorName);
    noFixColorCB->select(noFixColorName);
    headColorCB->select(headColorName);
    bearColorCB->select(bearColorName);
*/

    gpsData->statusOkColor = new QColor(okColorName);
    gpsData->statusNoFixColor = new QColor(noFixColorName);
    gpsData->headColor = new QColor(headColorName);
    gpsData->bearColor = new QColor(bearColorName);
    gpsData->trackColor = new QColor(trackColorName);
    gpsData->scaleColor = new QColor(scaleColorName);    /* Added by A. Karhov */
    gpsData->waypointColor = new QColor(waypointColorName);    /* Added by A. Karhov */



#else
    //read config file
    gpsData->altitude.altUnit =  Altitude::Feet;
    gpsData->speed.speedUnit =  Speed::Knots;
    gpsData->wpDistance.distUnit = Distance::Naut;
    gpsData->currPos.posUnit = Position::DegMin;

    gpsData->bearing.show = TRUE;
    gpsData->heading.show = TRUE;
    gpsData->showTime = TRUE;

    gpsData->gpsdArgStr = gpsdDefaultArg;
    gpsData->host = gpsdDefaultHost;
    gpsData->port = gpsdDefaultPort;

    gpsData->mapPathStr = gpsData->qpedir+"/maps";
    QDir md(gpsData->mapPathStr);
    //gpsData->mapPathStr = md.canonicalPath();

    gpsData->iconsPathStr = gpsData->qpedir+"/icons";
    QDir md2(gpsData->iconsPathStr);
    //gpsData->iconsPathStr = md2.canonicalPath();

    okColorName = "black";
    noFixColorName = "yellow";
    headColorName = "green";
    bearColorName = "red";
    trackColorName = "red";
    scaleColorName = "red"; /* Added by A. Karhov */
    waypointColorName = "red"; /* Added by A. Karhov */

    gpsData->statusOkColor = new QColor(Qt::black);
    gpsData->statusNoFixColor = new QColor(Qt::yellow);
    gpsData->headColor = new QColor(Qt::green);
    gpsData->bearColor = new QColor(Qt::red);
    gpsData->trackColor = new QColor(Qt::red);
    gpsData->scaleColor = new QColor(Qt::red);   /* Added by A. Karhov */
    gpsData->waypointColor = new QColor(Qt::red);   /* Added by A. Karhov */

    gpsData->trackPathStr = gpsData->qpedir+"/tracks";
    gpsData->updt_freq = 10;
    gpsData->track_thick = 2;

    gpsData->textSize = 12;

#endif

}

#ifdef PROXYB
void Settings::toggledpCB(bool state)       /* Added by A. Karhov */
{
if (state) {
	proxyL->show();
	proxyLE->show();
	gpsData->useProxy=TRUE;
	}
else {
	proxyL->hide();
	proxyLE->hide();
	gpsData->useProxy=FALSE;
	}
	writeConfig();
}
#endif


Settings::~Settings()
{
}

void Settings::writeConfig()
{
#ifndef DESKTOP
    qpegpsConfig->setGroup("units");
    qpegpsConfig->writeEntry("altitude",(int)gpsData->altitude.altUnit);
    qpegpsConfig->writeEntry("speed",(int)gpsData->speed.speedUnit);
    qpegpsConfig->writeEntry("distance",(int)gpsData->wpDistance.distUnit);
    qpegpsConfig->writeEntry("position",(int)gpsData->currPos.posUnit);
    qpegpsConfig->setGroup("show");
    qpegpsConfig->writeEntry("bearing",gpsData->bearing.show);
    qpegpsConfig->writeEntry("heading",gpsData->heading.show);
    qpegpsConfig->writeEntry("time",gpsData->showTime);
    qpegpsConfig->setGroup("gps");
    qpegpsConfig->writeEntry("gpsd", gpsData->gpsdArgStr);
    qpegpsConfig->writeEntry("host", gpsData->host);
    qpegpsConfig->writeEntry("port", gpsData->port);
    qpegpsConfig->writeEntry("StartupPlace", gpsData->startup_name);       /* Added by A. Karhov */
    qpegpsConfig->writeEntry("StartupMode", gpsData->startup_mode);       /* Added by A. Karhov */
    qpegpsConfig->writeEntry("DrawPlaces", gpsData->draw_places);       /* Added by A. Karhov */
    qpegpsConfig->setGroup("map");
    qpegpsConfig->writeEntry("path", gpsData->mapPathStr);
    qpegpsConfig->setGroup("download");

#ifdef PROXYB
    if (gpsData->useProxy) qpegpsConfig->writeEntry("proxy",gpsData->proxyUrl);       /* Added by A. Karhov */
       		else 		qpegpsConfig->writeEntry("proxy"," ");
#else
    qpegpsConfig->writeEntry("proxy",gpsData->proxyUrl);       /* Added by A. Karhov */
#endif
    qpegpsConfig->setGroup("color");

    qpegpsConfig->writeEntry("ok", okColorName);
    qpegpsConfig->writeEntry("noFix", noFixColorName);
    qpegpsConfig->writeEntry("heading", headColorName);
    qpegpsConfig->writeEntry("bearing", bearColorName);
    qpegpsConfig->writeEntry("trackC", trackColorName);
    qpegpsConfig->writeEntry("scale", scaleColorName);       /* Added by A. Karhov */
    qpegpsConfig->writeEntry("waypoint", waypointColorName);       /* Added by A. Karhov */

    qpegpsConfig->writeEntry("textSize", gpsData->textSize);
    qpegpsConfig->setGroup("track");
    qpegpsConfig->writeEntry("path", gpsData->trackPathStr);
    qpegpsConfig->writeEntry("updt_freq", gpsData->updt_freq);
    qpegpsConfig->writeEntry("track_thick", gpsData->track_thick);
    qpegpsConfig->setGroup("datum");
    qpegpsConfig->writeEntry("GpsDatum", (int)gpsData->gpsDatumIdx);
    qpegpsConfig->writeEntry("MapsDatum", (int)gpsData->mapDatumIdx);

    qpegpsConfig->~Config();
    qpegpsConfig = new Config("qpegps"); /* uggly, but works ... */

#endif
}

void Settings::setAlt(int id)
{
    gpsData->altitude.altUnit = (Altitude::Alt)id;
    writeConfig();
}
void Settings::setSpeed(int id)
{
    gpsData->speed.speedUnit = (Speed::Sp)id;
    writeConfig();
}
void Settings::setDist(int id)
{
    gpsData->wpDistance.distUnit = (Distance::Dist)id;
    writeConfig();
}
void Settings::setPos(int id)
{
    gpsData->currPos.posUnit = (Position::Pos)id;
    writeConfig();
}

void Settings::setBear(bool state)
{
    gpsData->bearing.show = state;
    writeConfig();
}
void Settings::setHead(bool state)
{
    gpsData->heading.show = state;
    writeConfig();
}
void Settings::setTime(bool state)
{
    gpsData->showTime = state;
    writeConfig();
}
#if 0
void Settings::proxyUrlLEChanged()
{
    gpsData->proxyUrl = proxyUrlLE->text();
    writeConfig();
}
#endif

#ifdef PROXYB
void Settings::proxyLEChanged()
{
    gpsData->proxyUrl = proxyLE->text();
    writeConfig();
}
#endif

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
    DirDialog getDirDialog(this, 0, TRUE, 0);
    getDirDialog.setCaption(tr("select map directory"));
    getDirDialog.exec();
    if(getDirDialog.result()==QDialog::Accepted)
    {
        gpsData->mapPathStr = getDirDialog.selectedPath;
        QDir md(gpsData->mapPathStr);


        //gpsData->mapPathStr = md.canonicalPath();
        mapPath->setText(gpsData->mapPathStr);
        writeConfig();
        emit mapPathChanged();
    }
}
void Settings::setColors()
{
    QValueList <QColor>colors;   
    colors.append(*gpsData->statusOkColor);
    colors.append(*gpsData->statusNoFixColor);
    colors.append(*gpsData->headColor);
    colors.append(*gpsData->bearColor);
    colors.append(*gpsData->trackColor);
    colors.append(*gpsData->scaleColor);
    colors.append(*gpsData->waypointColor);
    QStringList items;
    items.append(tr("for status \"GPS OK\""));
    items.append(tr("for status \"no position fix\""));
    items.append(tr("heading"));
    items.append(tr("bearing"));
    items.append(tr("track"));
    items.append(tr("scale"));
    items.append(tr("waypoint"));

    ColorDlg setColorDialog(&colors,&items,this, 0, TRUE, 0);
    setColorDialog.setCaption(tr("assign colors"));
    setColorDialog.exec();
    if(setColorDialog.result()==QDialog::Accepted)
    {
	*gpsData->statusOkColor = colors[0];
	*gpsData->statusNoFixColor = colors[1];
	*gpsData->headColor = colors[2];
	*gpsData->bearColor = colors[3];
	*gpsData->trackColor = colors[4];
	*gpsData->scaleColor = colors[5];
	*gpsData->waypointColor = colors[6];
        okColorName = gpsData->statusOkColor->name();
        noFixColorName = gpsData->statusNoFixColor->name();
        headColorName = gpsData->headColor->name();
        bearColorName = gpsData->bearColor->name();
        trackColorName = gpsData->trackColor->name();
        scaleColorName = gpsData->scaleColor->name();       /* Added by A. Karhov */
        waypointColorName = gpsData->waypointColor->name();       /* Added by A. Karhov */
        writeConfig();
    }
}
#if 0
void Settings::okColorChanged(int idx)
{
    okColorName = okColorCB->currentText();
    gpsData->statusOkColor = qColorPtrList.at(idx);
    writeConfig();
}

void Settings::noFixColorChanged(int idx)
{
    noFixColorName = noFixColorCB->currentText();
    gpsData->statusNoFixColor = qColorPtrList.at(idx);
    writeConfig();
}

void Settings::headColorChanged(int idx)
{
    headColorName = headColorCB->currentText();
    gpsData->headColor = qColorPtrList.at(idx);
    writeConfig();
}

void Settings::bearColorChanged(int idx)
{
    bearColorName = bearColorCB->currentText();
    gpsData->bearColor = qColorPtrList.at(idx);
    writeConfig();
}
#endif

void Settings::textSizeChanged(int idx)
{
#ifndef DESKTOP
    gpsData->textSize = idx;
    writeConfig();
#endif
}

void Settings::geoDatGpsChanged(int idx)
{
#ifndef DESKTOP
    gpsData->gpsDatumIdx = idx+1;
    writeConfig();
#endif
}

void Settings::geoDatMapChanged(int idx)
{
#ifndef DESKTOP
    gpsData->mapDatumIdx = idx+1;
    writeConfig();
#endif
}



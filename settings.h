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
///define PROXYB

#ifndef SETTINGS_H
#define SETTINGS_H

#include <qsocket.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qtextview.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qarray.h>
#include <qwidget.h>
#include <qcombobox.h>
#include <qscrollview.h>
#include <qstringlist.h>
#include <qhbuttongroup.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qcheckbox.h>
#include <qcolor.h>
#include <qspinbox.h>
#include <qsizepolicy.h>

#include <math.h>
#include "gpsdata.h"
#include "dirdialog.h"
#include "colordlg.h"

#ifndef DESKTOP
#include <qpe/global.h>
#include <qpe/config.h>
#include <qpe/menubutton.h>

#else
#include <qsettings.h>
#define Config QSettings 

#include <qpushbutton.h>
#define MenuButton QPushButton
#endif

#define gpsdDefaultArg "-p /dev/ttyS0 -s 4800"
#define gpsdDefaultHost "localhost"
#define gpsdDefaultPort 2947


class Settings : public QScrollView
{
    Q_OBJECT

public:
    Settings(GpsData *, QWidget *parent=0, const char *name=0, WFlags fl=0);
    ~Settings();

    GpsData *gpsData;
    Config *qpegpsConfig;
    void writeConfig();

signals:
    void mapPathChanged();

private:
    QVBox *mainBox;
    QComboBox *altCB, *speedCB, *distCB, *posCB;
    QLabel *altL, *speedL, *distL, *posL;
    QHBox  *horbox1,*horbox2,*horbox3, *horbox4, *horbox5;
    QHBox  *horbox6, *horbox7, *horbox8, *horbox9, *horbox10;
    QHBox *horbox11, *horbox12, *horbox13;
    QCheckBox *bearTB;
    QCheckBox *headTB;
    QCheckBox *timeTB;
    QLabel *mapDir, *checkLabel;
    QLineEdit *mapPath;
    QLineEdit *proxyUrlLE;
    QPushButton *mapPathB, *colorB;
    /*MenuButton *okColorCB, *noFixColorCB, *headColorCB, *bearColorCB;*/
    QString okColorName, noFixColorName, headColorName, bearColorName, trackColorName, scaleColorName, waypointColorName;
    QStringList colorList;
    QList <QColor>qColorPtrList;
    QLabel *textSizeL;
    /*QLabel *okLabel, *noFixLabel, *headColorLabel, *bearColorLabel, *proxyUrlL;*/
    QSpinBox *textSizeSB;
    QLabel *geoDatL,*geoDatGpsL, *geoDatMapL;
    QComboBox *geoDatGpsCB, *geoDatMapCB;

#ifdef PROXYB
        QLabel *proxyL;		/* Added by A. Karkhov */
    QLineEdit *proxyLE;
    QCheckBox *proxyCB;
#endif

private slots:
    void setAlt(int);
    void setSpeed(int);
    void setDist(int);
    void setPos(int);
    void setBear(bool);
    void setHead(bool);
    void setTime(bool);
    void setMapPath();
    void mapPathLEChanged();

#ifdef PROXYB
//        void toggledpCB(bool state);		/* Added by A. Karkhov */
//	void proxyLEChanged();
#endif

/*
    void proxyUrlLEChanged();
    void okColorChanged(int);
    void noFixColorChanged(int);
    void headColorChanged(int);
    void bearColorChanged(int);
*/
    void setColors();
    void textSizeChanged(int);
    void geoDatGpsChanged(int);
    void geoDatMapChanged(int);
};

#endif

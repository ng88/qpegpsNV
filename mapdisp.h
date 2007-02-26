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

#ifndef MAP_DISP_H
#define MAP_DISP_H


#include "maps.h"

#include <qsocket.h>
#include <time.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qvgroupbox.h>
#include <qmultilineedit.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qdialog.h>
#include <qsortedlist.h>
#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qlistview.h>
#include <qvalidator.h>
#include <qmessagebox.h>
#include <qpalette.h>

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
#include <math.h>
#include <qimage.h>

#include "gpsdata.h"
#include "track.h"

class MapDisp:public QWidget
{
  Q_OBJECT public:
    MapDisp(Qpegps *, QSortedList < MapBase > *mapList, QWidget * parent =
            0, const char *name = 0, WFlags fl = 0);
     ~MapDisp();

    Qpegps *application;
    GpsData *gpsData;
    QPixmap *mapdisp;
    QImage *map;
    MapBase *actmap;
      QSortedList < MapBase > *maps;
      QSortedList < MapBase > actMapList;
    double selectedScale;

    QColor statColor;

    int centerX, centerY;       /* Added by A. Karhov */
    double mPointLat, mPointLong, accDist, destPointLong, destPointLat;
    bool ManualWp, noDBLC;

    void mousePressEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *);
    void paintEvent(QPaintEvent *);
    void createMap();
    double coverage(MapBase *, double, double, int, int);
    void MapDispAddPos(double londitude, double latitude, double altitude);     /* Added by A. Karhov */
    void MapDispDist(int mouseEventX, int mouseEventY);
    void MapDispMesure(int X, int Y);   /* Added by A. Karhov */
//    void      mouseDoubleClickEvent(QMouseEvent *e);   /* Added by A/ Karhov */
    void mouseReleaseEvent(QMouseEvent *);
    QTimer *timer;              /* Added by A/ Karhov */
    QTimer *atimer;             /* Added by A/ Karhov */
    QFile *placesFile;
    QString ts;
    double newlongitude, newlatitude, newaltitude, timeAccelerator;
    QTextStream *t;
    int currtime, newtime, min, timeZone;
    int mouseEventX, mouseEventY;
    void swichMode();           /* Added by A/ Karhov */
//    int WTconunter;
    time_t WTTimer, WriteTimeout;
    bool FirstKey, inanimation, shortcutTime;

    public slots: void chooseLessDetailedMap();
    void chooseMoreDetailedMap();
    void showAvailableMaps();
    void clearActMapList();
    void timerDone();
    void animateTrack();

      signals: void mouseClick(QWidget *);
    void moreDetail();
    void lessDetail();
    void debugMaps();
};

/* Added by A. Karhov */
class MapCoordEditorWidget:public QVBox
{
  Q_OBJECT public:
    MapCoordEditorWidget(GpsData * gpsData, Places * places,
                         QWidget * parent = 0, const char *name = 0);
     ~MapCoordEditorWidget();

    QLineEdit *AltLE;

    QLineEdit *LatiLEd;
    QLineEdit *LonLEd;
    QLineEdit *LatiLEm;
    QLineEdit *LonLEm;
    QLineEdit *LatiLEs;
    QLineEdit *LonLEs;
    QComboBox *mapLatLonCB;

    protected slots:void placeSelected(int ind);
    void toggledDDddB(bool state);
    void toggledDDMMmmB(bool state);
    void toggledDDMMSSssB(bool state);
    void editedLtLe(const QString &);

  protected:
      QMultiLineEdit * commMLE;
    QLabel *AltL;
    QLabel *LonL;
    QLabel *LatiL;
    QLabel *LonLd;
    QLabel *LatiLd;
    QLabel *LonLm;
    QLabel *LatiLm;
    QLabel *LonLs;
    QLabel *LatiLs;
    QLabel *LonLS;
    QLabel *LatiLS;
    QVGroupBox *CoordGB;
    QVGroupBox *PlaceGB;
    QButtonGroup *LatLonBG;
    QRadioButton *DDddB;
    QRadioButton *DDMMmmB;
    QRadioButton *DDMMSSssB;

    Places *pl;
    Altitude *alt;
};

class MapCoordEditorDialog:public QDialog
{
  Q_OBJECT public:
    MapCoordEditorDialog(GpsData * gpsData, Places * places, QWidget *,
                         const char *, bool, WFlags);
     ~MapCoordEditorDialog();
    MapCoordEditorWidget *mapSrcEditW;
};

class MapPlaceEditorWidget:public QVBox
{
  Q_OBJECT public:
    MapPlaceEditorWidget(QWidget * parent = 0, const char *name = 0);
     ~MapPlaceEditorWidget();

    QLineEdit *CommentLEd;
    QLineEdit *NameLEd;

  protected:
      QVGroupBox * PlaceGB;
    QVGroupBox *PlaceGB1;
};

class MapPlaceEditorDialog:public QDialog
{
  Q_OBJECT public:
    MapPlaceEditorDialog(QWidget *, const char *, bool, WFlags);
     ~MapPlaceEditorDialog();
    MapPlaceEditorWidget *mapSrcEditW;
};


class AnimateParamsWidget:public QVBox
{
  Q_OBJECT public:
    AnimateParamsWidget(GpsData * gpsData, QWidget * parent =
                        0, const char *name = 0);
     ~AnimateParamsWidget();

    QLabel *AccelL, *AccelLS;
    QLineEdit *AccelLEd;
    QLabel *TZL, *TZLS;
    QLineEdit *TZLEd;
    QCheckBox *scTimeTB;
    QComboBox *trackFileCB;

};

class AnimateParamsDialog:public QDialog
{
  Q_OBJECT public:
    AnimateParamsDialog(GpsData * gpsData, QWidget *, const char *, bool,
                        WFlags);
     ~AnimateParamsDialog();
    AnimateParamsWidget *paramW;
};


#endif

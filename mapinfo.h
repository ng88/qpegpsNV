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

#ifndef MAP_INFO_H
#define MAP_INFO_H

#include <qsocket.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qtextview.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qvgroupbox.h>
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
#include <math.h>

#include <qpe/fileselector.h>
#include <qpe/applnk.h>


#include "gpsdata.h"
#include "maps.h"

class Qpegps;

class MapWidget:public QWidget
{
  Q_OBJECT public:
    MapWidget(QWidget * parent = 0, const char *name = 0, WFlags fl = 0);
     ~MapWidget();

    void mousePressEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);

    int xx, yy;

      signals: void mouseClick(int, int);
};

class MapInfo:public QVBox
{
  Q_OBJECT public:
    MapInfo(Qpegps *, QSortedList < MapBase > *mapList, QWidget * parent =
            0, const char *name = 0, WFlags fl = 0);
     ~MapInfo();
    void writeMapFile();

    Qpegps *application;
    GpsData *gpsData;
      QSortedList < MapBase > *maps;
    int mapsIndex;
    QComboBox *mapSelect;
    /*QLabel    *mapDescL; */
    QVGroupBox *mapDescGB;
    QLabel *positionInfo;
    QScrollView *mapView;
    QPixmap *mapPixMap;
    MapWidget *mapWidget;
    QPushButton *removePB;
    QPushButton *downloadPB;
    QPushButton *importPB;
    QPushButton *propertiesPB;
    QStringList mapNames;
    /*QStringList mapDescList; */
    Position mapPos;
    QPixmap *image;

      signals: void mapListCleared();
    private slots: void mapListChanged();
    void selectionChanged(int);
    void calcPosInfo(int, int);
    void startDownLoadD();
    void startImportD();
    void startChangeD();
    void startRemoveD();
};


#endif

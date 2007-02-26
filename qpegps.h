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

#ifndef QPEGPS_H
#define QPEGPS_H

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
#include <qtabwidget.h>
#include <qwidgetstack.h>
#include <qmessagebox.h>
#include <qtextcodec.h>

#ifndef DESKTOP
#include <qpe/qpeapplication.h>
#include <qpe/qcopenvelope_qws.h>
#endif

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <signal.h>
#include "gpsdata.h"

class GpsData;
class Client;
class MapDisp;
class MapInfo;
class Settings;
class FetchMap;
class Route;
class GpsStatus;
class Track;
class Qpegps;

typedef struct Places {  /* Added by A. Karhov */
 QString 	*name;
 Position	pos;
 double		altitude;
 QString 	*comment;
 Places 	*next;
};

#include "maps.h"

class Qpegps;


class ResumeCF
{
public:
    ResumeCF(Qpegps * theAppl) {d_pAppl = theAppl; } //:d_pAppl(theAppl) {}
    ~ResumeCF() {}

public:
    void activate();
    static void resume(int);    
    static void usr(int);
private:
    static Qpegps * d_pAppl;
};


class Qpegps : public QTabWidget
{
    Q_OBJECT

public:
    Qpegps(QString, QWidgetStack *parent, const char *name=0, WFlags fl=0);
    ~Qpegps();

    GpsData *gpsData;
    Client *gpsd;
    MapDisp *mapDisp;
    MapInfo *mapInfo;
    Settings *settings;
    FetchMap *fetchMap;
    Route *route;
    GpsStatus *d_pGpsStatus;
    Track *track;
    QWidgetStack * d_pViewer;

    QSortedList<MapBase> maps;
    ResumeCF resumeCF;

    int ManualPosit;	/* Added by A. Karhov */
    Places *places;	/* Added by A. Karhov */
    void readPlaces();	/* Added by A. Karhov */
    void writePlaces();	/* added by A.Karkhov after 0.9.2.3.2  */ /**********/

protected:
    QWidget *lastTab;

public slots:
    void tabChanged(QWidget *);
    void updateData();
    void quitInProgress();
    void readMaps();
    void reReadMaps();
    void toggleFullScreen(QWidget *);

signals:
    void currentChanged(QWidget*);

};



#endif

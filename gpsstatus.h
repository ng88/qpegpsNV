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

#ifndef GPSSTATUS_H
#define GPSSTATUS_H
#include <qsocket.h>
#include <qpe/qpeapplication.h>
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
#include <qhbuttongroup.h>
#include <qradiobutton.h>
#include <qvgroupbox.h>
#include <qdatetime.h>


#include <vector>

#include "mathex.h"
#include <sys/time.h>
#include <qpe/timeconversion.h>
#include <qpe/qcopenvelope_qws.h>
#include <qpe/tzselect.h>

#include "qpegps.h"
#include "maps.h"
#include "gpsdata.h"


using std::pair;

class SatSNR:public QFrame
{
  public:
    SatSNR(GpsData * gpsdata, QWidget * parent = 0, const char *name =
           0, WFlags f = 0);
     ~SatSNR();

  public:
    void updateInfo();

  private:
    void drawContents(QPainter * painter);


  public:
    GpsData * gpsData;
    // cache
    SatInfo d_pSatInfo[12];

};


class SatStat:public QFrame
{
  Q_OBJECT public:

    SatStat(GpsData * gpsdata, QWidget * parent = 0, const char *name =
            0, WFlags f = 0);
     ~SatStat();

  public:
    void updateInfo();
    void shiftSamples();
    void drawSamples();

  private:
    void drawContents(QPainter * painter);


  public:
    GpsData * gpsData;

    double *d_pSpeedSamples;
    double *d_pAltitudeSamples;
    pair < int, int >*d_pSatelliteSamples;
    static const int d_pixSatellites = 12;

    double d_maxSpeed;
    double d_minSpeed;
    static const int d_pixSpeed = 12;

    double d_maxAltitude;
    double d_minAltitude;
    static const int d_pixAltitude = 12;

    static const int d_numSamples = 80;

    QTimer *d_pTimer;

    private slots: void updateSamples();

};



class GpsStatus:public QVBox
{
  Q_OBJECT public:
    //GpsStatus(GpsData *gdata, QWidget *parent=0, const char *name=0, WFlags fl=0);
    GpsStatus(Qpegps * appl, QWidget * parent = 0, const char *name =
              0, WFlags fl = 0);
     ~GpsStatus();


  public:
    void updateQuick();
    void update();


  private:
    GpsData * gpsData;
    Qpegps *application;
    Settings *settings;
    Client *gpsd;

    QLabel *gpsdOpt, *gpsdHost, *gpsdPort;
    QLineEdit *gpsdArguments, *gpsdHostArg, *gpsdPortArg;
    QPushButton *gpsdArgumentsB, *gpsdHostPortB;

    QPushButton *d_pStatus;
    QLabel *d_pReceiverStatus;
    QLabel *d_pGpsdStatus;

    QLabel *d_pLongitude;
    QLabel *d_pLatitude;

    SatSNR *d_pSatSNR;
    SatStat *d_pSatStat;

    bool d_fullUpdate, requestTimeAdj;

    QGroupBox *group1, *group2;

  protected:
    virtual void paintEvent(QPaintEvent *);

    signals: void gpsdArgChanged();

    private slots: void setGpsdDefaultArg();
    void setGpsdDefaultHostPort();
    void gpsdArgLEChanged();
    void gpsdHostArgLEChanged();
    void gpsdPortArgLEChanged();
    void setSysTime();
};


#endif

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

#ifndef GPS_DATA_H
#define GPS_DATA_H

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
#include <qcolor.h>
#include <qregexp.h>
#include <math.h>

#include "datum/datum.h"
#include "datum/ellipse.h"

class Track;
class Qpegps;

class Altitude : public QObject
{
    Q_OBJECT
public:
    Altitude(QWidget *parent=0, const char *name=0):QObject(parent,name){altitude=0.0;};
    ~Altitude(){};
    enum	Alt { None=0, Meter, Feet, FL };
    static Alt altUnit;

    double altitude;
    double Altitude::getAlt(double meters);
    QString toString();
};

class Speed : public QObject
{
    Q_OBJECT
public:
    Speed(QWidget *parent=0, const char *name=0):QObject(parent,name){speed=0.0;};
    ~Speed(){};

    double speed;
    enum Sp { None=0, Kmh, Knots, Mph };
    static Sp speedUnit;
    QString toString();
};

class Distance : public QObject
{
    Q_OBJECT
public:
    Distance(QWidget *parent=0, const char *name=0):QObject(parent,name){distance=0.0;};
    ~Distance(){};

    double distance;
    enum Dist { None=0, Km, Naut, Statute };
    static Dist distUnit;
    QString toString();
};

class Position : public QObject
{
    Q_OBJECT
public:
    Position(QWidget *parent=0, const char *name=0);
    ~Position(){};

    double longitude, latitude;
    enum Pos { Degree=0, DegMin, DegMinSec};
    static Pos posUnit;
    QString latToString(double lat); /* Added by A. Karhov */
    QString longToString(double lon); /* Added by A. Karhov */
    double setLong(QString);
    double setLat(QString);
    QString setLong(double);
    QString setLat(double);
};

class Angle : public QObject
{
    Q_OBJECT
public:
    Angle(QWidget *parent=0, const char *name=0):QObject(parent,name){angle=0;}; /* Fix by A. Karhov */
    ~Angle(){};

    double angle;
    bool show;
    QString toString();
};

class Time : public QObject
{
    Q_OBJECT
public:
    Time(QWidget *parent=0, const char *name=0):QObject(parent,name){};
    ~Time(){};

    QString toString();
};

class TimeStamp : public QObject
{
    Q_OBJECT
public:
    TimeStamp(QWidget *parent=0, const char *name=0):QObject(parent,name){};
    ~TimeStamp(){};

    QString date;
    QString time;

    QString toString();
};

class SatInfo : public QObject
{
    Q_OBJECT
public:
    SatInfo(QWidget *parent=0, const char *name=0):
            QObject(parent,name),
            d_elevation(0), d_azimut(0), d_snr(0), d_updated(false)
    {};
    ~SatInfo(){};

public:
    bool operator == (const SatInfo & other) const;
    SatInfo & operator =(const class SatInfo & other);

    QString d_satName;

    // elevation of satellite
    int d_elevation;

    // azimut angle
    int d_azimut;

    // signal noise ratio (db)
    int d_snr;

    // flag if the value has changed
    bool d_updated;

};

class GeoDatum : public QObject
{
   Q_OBJECT
public:
   GeoDatum();
   ~GeoDatum();
   QStringList getDatumList();
   void convertDatum(long fromIdx, long toIdx, double *lt, double *lg, double *altitude);
 private:
   long errorCode, datumCount;
   QStringList datumList;
};

class GpsData : public QObject
{
    Q_OBJECT
public:
    GpsData(QWidget *parent=0, const char *name=0);
    ~GpsData();

    void adjustDatum();

    QString host;
    Q_UINT16 port;

    QString gpsdArgStr, mapPathStr, iconsPathStr, qpedir, trackPathStr, proxyUrl;

    GeoDatum geoDatum;
    long gpsDatumIdx, mapDatumIdx;

    Altitude altitude;
    Speed speed, avspeed, wpSpeed;
    Distance wpDistance;
    Position currPos, wpPos;
    Angle heading, bearing;
    TimeStamp ts;
    int textSize;

    bool d_connected; // indicator if we are connected to gpsd and receiving data ok
    bool d_aliveGPS;  // indicator if we are receiving data from GPS alright
    QString d_Receiver;

    int status; /* 0 = no fix, 1 = fix, 2 = dgps fix */
    QString *statusStr;
    QColor *statusOkColor, *statusNoFixColor, *headColor, *bearColor, *trackColor, *scaleColor, *waypointColor;
    double ttwph, ttwpm, ttwps; //time to waypoint h,min,sec
    bool showTime;

    int d_no_of_satellites;
    int d_no_of_fix_satellites;
    SatInfo d_pSatInfo[12];

    // Track *track;
    // tracklogs:
    int updt_freq,		// minimal time difference between 2 positionsm
    track_thick;	// trackline thickness

    QString timeToString();

    double latitudeGps,longitudeGps;	/* Added by A/ Karhov */
    QString startup_name;	/* Added by A/ Karhov */
    bool startup_mode, draw_places;	/* Added by A/ Karhov */
    bool useProxy;


};



#endif

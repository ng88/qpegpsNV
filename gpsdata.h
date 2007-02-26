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

#ifndef GPS_DATA_H
#define GPS_DATA_H

#include <qcolor.h>
#include <qstringlist.h>

/* improvement of all the small classes here by ng */

class Altitude
{
 private:
    double _altitude;
  
 public:
 
    enum Alt  { None = 0, Meter, Feet, FL };
    static Alt altUnit;
 
    Altitude(double alt = 0.0) :  _altitude(alt) {}

    ~Altitude() { }
    
    inline void setFromDouble(const double &v) { _altitude = v; }
    inline double toDouble() const { return _altitude; }

    
    static double getAlt(double d);
    
    inline double getAlt() const { return getAlt(_altitude); }
    QString toString() const;
};

class Speed
{

 private:
     double _speed;
     
 public:
 
    enum Sp { None = 0, Kmh, Knots, Mph };
    static Sp speedUnit;
 
    Speed(double sp = 0.0) : _speed(sp) {} 
    ~Speed() { }
    
    inline void setFromDouble(const double &v) { _speed = v; }
    inline double toDouble() const { return _speed; }

    QString toString() const;
};

class Distance
{

 private:
     double _distance;
     
 public:
 
    enum Dist { None = 0, Km, Naut, Statute };
    static Dist distUnit;
    
    inline void setFromDouble(const double &v) { _distance = v; }
    inline double toDouble() const { return _distance; }
 
 
    Distance(double dist = 0.0) : _distance(dist) {}
    ~Distance() {}


    QString toString() const;
    
    static QString toStringFromKm(double d);
};

class Position
{

  private:
  
    double _longitude, _latitude;
  
  public:
  
    enum Pos { Degree = 0, DegMin, DegMinSec };
    static Pos posUnit;
  
    Position(double lat = 0.0, double lgn = 0.0) : _longitude(lgn), _latitude(lat) {}
    ~Position() {}
    
    static QString latToString(double v);
    static QString longToString(double v);
    
    inline QString latToString() const { return latToString(_latitude); }
    inline QString longToString() const { return longToString(_longitude); }
    
    void setLong(const QString& s);
    void setLat(const QString& s);
    
    inline void setLong(double v ) { _longitude = v; }
    inline void setLat(double v) { _latitude = v; }
    
    inline double longitude() const { return _longitude; }
    inline double latitude() const { return _latitude; }
    
    
    
    
     /* kept for code compatibility reason */
    inline double setLongAndRet(const QString& s) { setLong(s); return _longitude; }
     /* kept for code compatibility reason */
    inline double setLatAndRet(const QString& s) { setLat(s); return _latitude; }
    /* kept for code compatibility reason */
    inline QString setLongAndRet(double v ) { _longitude = v; return QString::number(v); } 
    /* kept for code compatibility reason */
    inline QString setLatAndRet(double v) { _latitude = v; return QString::number(v);}
    
};

class Angle
{

 private:
 
     double _angle;
     bool   _show;
 
 public:
    Angle(double angle = 0.0) :  _angle(angle) {}

    ~Angle() {}
    
    
    inline void setFromDouble(const double &v) { _angle = v; }
    inline double toDouble() const { return _angle; }

    inline void setShow(const bool &v) { _show = v; }
    inline bool show() const { return _show; }


    QString toString() const;
};

class Time
{

  public:
    Time() {}
    ~Time() {}

    QString toString() const;
};

class TimeStamp
{

private:

    QString _date, _time;
  
public:

    TimeStamp(const QString& date = "", const QString& time = "")
        : _date(date),  _time(time) {}
    ~TimeStamp() {}
    
    
    inline void setDate(const QString &v) { _date = v; }
    inline const QString& date() const { return _date; }

    inline void setTime(const QString &v) { _time = v; }
    inline const QString& time() const { return _time; }


    QString toString() const;
};

class SatInfo
{
  
 private:
 
    QString _satName;
    // elevation of satellite
    int _elevation;
    // azimut angle
    int _azimut;
    // signal noise ratio (db)
    int _snr;
    // flag if the value has changed
    bool _updated;
 
 public:
    SatInfo(const QString& name = "", int elev = 0, int azimut = 0, int snr = 0)
        : _satName(name), _elevation(elev), _azimut(azimut), _snr(snr), _updated(false) {}
        
    ~SatInfo() {}

    bool operator ==(const SatInfo & other) const;
    SatInfo & operator =(const SatInfo & other);

    inline void setSatName(const QString &v) { _satName = v; }
    inline const QString& satName() const { return _satName; }
    
    inline void setElevation(const int &v) { _elevation = v; }
    inline int elevation() const { return _elevation; }
    
    inline void setAzimut(const int &v) { _azimut = v; }
    inline int azimut() const { return _azimut; }
    
    inline void setSignalNoiseRatio(const int &v) { _snr = v; }
    inline int signalNoiseRatio() const { return _snr; }
    
    inline void setUpdated(const bool &v) { _updated = v; }
    inline bool updated() const { return _updated; }



};

class GeoDatum
{
  
  private:
  
    long errorCode, datumCount;
    QStringList datumList;
  
    
  public:
  
    GeoDatum(const QString& qpedir);
    ~GeoDatum() {}
    
    inline const QStringList& getDatumList() const { return datumList; }
    void convertDatum(long fromIdx, long toIdx, double *lt, double *lg,  double *altitude);

};

//TODO membres privés + accesseur à faire
class GpsData 
{
  public:
  
    GpsData(const QString& qpedir);
     ~GpsData();

    void adjustDatum();

    QString host;
    Q_UINT16 port;

    QString gpsdArgStr, mapPathStr, iconsPathStr, qpedir, trackPathStr;

    GeoDatum geoDatum;
    long gpsDatumIdx, mapDatumIdx;

    Altitude altitude;
    Speed speed, avspeed, wpSpeed;
    Distance wpDistance;
    Position currPos, wpPos;
    Angle heading, bearing;
    TimeStamp ts;
    int textSize;
    
    bool ManualPosit;

    bool d_connected;           // indicator if we are connected to gpsd and receiving data ok
    bool d_aliveGPS;            // indicator if we are receiving data from GPS alright
    QString d_Receiver;

    int status;                 /* 0 = no fix, 1 = fix, 2 = dgps fix */
    QString statusStr;
    
    QColor statusOkColor,
           statusNoFixColor,
           headColor,
           bearColor,
           trackColor,
           scaleColor,
           waypointColor,
           routeColor,
           routeIconColor,
           routeIconTxtColor,
           routePosLineColor;
    
    
    bool rtDisplayIcon,
         rtDisplayPopup,
         rtPopupAutoClose,
         rtUseTts,
         rtDrawLine;
         
    int  rtAutoCloseTime,
         rtPreventValue,
         rtLineThick;

           
    double ttwph, ttwpm, ttwps; //time to waypoint h,min,sec
    bool showTime;

    int d_no_of_satellites;
    int d_no_of_fix_satellites;
    SatInfo d_pSatInfo[12];

    int updt_freq,              // minimal time difference between 2 positionsm
      track_thick;              // trackline thickness

    QString timeToString();

    double latitudeGps, longitudeGps;   /* Added by A/ Karhov */
    QString startup_name;       /* Added by A/ Karhov */
    bool startup_mode, draw_places;     /* Added by A/ Karhov */


};



#endif

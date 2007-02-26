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

#include "gpsdata.h"

#include <qregexp.h>
#include "datum/datum.h"
#include "datum/ellipse.h"
#include "track.h"
#include "mathex.h"

// initialization of static elements:
Altitude::Alt Altitude::altUnit = Altitude::Feet;
Speed::Sp Speed::speedUnit = Speed::Knots;
Distance::Dist Distance::distUnit = Distance::Naut;
Position::Pos Position::posUnit = Position::DegMin;

QString Altitude::toString() const
{
    switch (altUnit)
    {

    case Meter: return QObject::tr("%1 m").arg(_altitude, 0, 'f', 0);
    case Feet:  return QObject::tr("%1 ft").arg(MathEx::meters2feet(_altitude), 0, 'f', 0);
    case FL:    return QObject::tr("%1 FL").arg(MathEx::meters2FL(_altitude), 0, 'f', 2);
    default:
    case None:  return "";
    }
}

double Altitude::getAlt(double d)
{
    switch (altUnit)
    {
        case Meter: return d;
        case Feet:  return MathEx::meters2feet(d);
        case FL:    return MathEx::meters2FL(d);
        default:
        case None:  return 0;
    }
}


QString Speed::toString() const
{
    switch (speedUnit)
    {
        case Kmh:   return QObject::tr("%1 kmh").arg(MathEx::kn2kmh(_speed), 0, 'f', 1);
        case Knots: return QObject::tr("%1 kn").arg(_speed, 0, 'f', 1);
        case Mph:   return QObject::tr("%1 mph").arg(MathEx::kn2mph(_speed), 0, 'f', 1);
        default:
        case None:  return "";
    }
}

QString Distance::toString() const
{
    switch (distUnit)
    {

    case Km:      return QObject::tr("%1 km").arg(MathEx::nmi2km(_distance), 0, 'f', 3); 
    case Naut:    return QObject::tr("%1 nmi").arg(_distance, 0, 'f', 3);
    case Statute: return QObject::tr("%1 mi").arg(MathEx::nmi2mi(_distance), 0, 'f', 3);
    default:
    case None: return "";
    }
}

QString Distance::toStringFromKm(double d)
{
    switch (distUnit)
    {

    case Km:      return QObject::tr("%1 km").arg(d, 0, 'f', 3); 
    case Naut:    return QObject::tr("%1 nmi").arg(MathEx::km2nmi(d), 0, 'f', 3);
    case Statute: return QObject::tr("%1 mi").arg(MathEx::km2mi(d) , 0, 'f', 3);
    default:
    case None: return "";
    }
}



QString Position::latToString(double v)
{
    double deg, min, sec;
    
    QString sign = (v > 0) ? "N" : "S";
    double lat = fabs(v);

        
    switch (posUnit)
    {
        case DegMin:
            deg = floor(lat);
            min = (lat - deg) * 60.0;
            return QObject::tr("%1\260%2'%3").arg(deg, 0, 'f', 0)
                                    .arg(min, 0, 'f', 3)
                                    .arg(sign);
        case DegMinSec:
            deg = floor(lat);
            min = floor((lat - deg) * 60.0);
            sec = (lat - (deg + min / 60.0)) * 3600.0;
            return QObject::tr("%1\260%2'%3\"%4").arg(deg, 0, 'f', 0)
                                        .arg(min, 0, 'f', 0)
                                        .arg(sec, 0, 'f', 2)
                                        .arg(sign);
        default:
        case Degree:
            return QObject::tr("%1\260%2").arg(lat, 0, 'f', 5)
                                .arg(sign);
    }
}

QString Position::longToString(double v)
{
    double deg, min, sec;

    double lg = fabs(v);
    QString sign = (v > 0) ?"E" : "W";
    
    switch (posUnit)
    {

    case DegMin:
        deg = floor(lg);
        min = (lg - deg) * 60.0;
        return QObject::tr("%1\260%2'%3").arg(deg, 0, 'f', 0)
                                .arg(min, 0, 'f', 3)
                                .arg(sign);
    case DegMinSec:
        deg = floor(lg);
        min = floor((lg - deg) * 60.0);
        sec = (lg - (deg + min / 60.0)) * 3600.0;
        return QObject::tr("%1\260%2'%3\"%4").arg(deg, 0, 'f', 0)
                                    .arg(min, 0, 'f', 0)
                                    .arg(sec, 0, 'f', 2)
                                    .arg(sign);
    default:
    case Degree:
        return QObject::tr("%1\260%2").arg(lg, 0, 'f', 5)
                             .arg(sign);
    }
}

void Position::setLong(const QString& lgString)
{
    int p;
    float deg = 0, min = 0, sec = 0, west = 1;
    QString lgStr;
    QRegExp re("[\\s'\"]");

    lgStr = lgString.simplifyWhiteSpace();

    p = lgStr.find("w", 0, FALSE);
    if (p >= 0)
    {
        west = -1.0;
        lgStr.remove(p, 1);
        lgStr = lgStr.simplifyWhiteSpace();
    }
    p = lgStr.find("e", 0, FALSE);
    if (p >= 0)
    {
        lgStr.remove(p, 1);
        lgStr = lgStr.simplifyWhiteSpace();
    }

    deg = lgStr.toFloat();
    p = lgStr.find(re);
    if (p > 0)
    {
        lgStr = lgStr.mid(p + 1);
        min = lgStr.toFloat();
        p = lgStr.find(re);
        if (p > 0)
        {
            lgStr = lgStr.mid(p + 1);
            sec = lgStr.toFloat();
        }
    }
    _longitude = west * (deg + (min / 60.0) + (sec / 3600.0));
}

void Position::setLat(const QString& ltString)
{
    int p;
    float deg = 0, min = 0, sec = 0, south = 1;
    QString ltStr;
    QRegExp re("[\\s'\"]");

    ltStr = ltString.simplifyWhiteSpace();

    p = ltStr.find("s", 0, FALSE);
    if (p >= 0)
    {
        south = -1.0;
        ltStr.remove(p, 1);
        ltStr = ltStr.simplifyWhiteSpace();
    }
    p = ltStr.find("n", 0, FALSE);
    if (p >= 0)
    {
        ltStr.remove(p, 1);
        ltStr = ltStr.simplifyWhiteSpace();
    }

    deg = ltStr.toFloat();
    p = ltStr.find(re);
    if (p > 0)
    {
        ltStr = ltStr.mid(p + 1);
        min = ltStr.toFloat();
        p = ltStr.find(re);
        if (p > 0)
        {
            ltStr = ltStr.mid(p + 1);
            sec = ltStr.toFloat();
        }
    }
    _latitude = south * (deg + (min / 60.0) + (sec / 3600.0));
}


QString Angle::toString() const
{
    if (_show)
        return QString("%1").arg(_angle, 0, 'f', 0);
    else
        return "";
}

QString TimeStamp::toString() const
{
    if (_date.isEmpty() || _time.isEmpty())
        return QObject::tr("* No GMT Signal rcvd *");

    return _date + " " + _time;
}

bool SatInfo::operator ==(const SatInfo & other) const
{
    return ((_satName == other._satName) &&
            (_elevation == other._elevation) &&
            (_azimut == other._azimut) && (_snr == other._snr));
}

SatInfo & SatInfo::operator =(const SatInfo & other)
{
    // memberwise copy (needed here because QObject operator = is private)
    _satName = other._satName;
    _elevation = other._elevation;
    _azimut = other._azimut;
    _snr = other._snr;
    _updated = other._updated;

    return *this;
}

GeoDatum::GeoDatum(const QString& qpedir)
{

    setenv("DATUM_DATA", qpedir + "/qpegps", 1);        //datum conversion needs env variable to the .dat files
    setenv("ELLIPSOID_DATA", qpedir + "/qpegps", 1);    //datum conversion needs env variable to the .dat files

    errorCode = Initialize_Ellipsoids();
    if (errorCode)
        qWarning(QObject::tr("Ellipsoid table couldn't be initialized (check existance and path of ellipse.dat)"));
    errorCode = Initialize_Datums();
    if (errorCode)
    {
        qWarning(QObject::tr("Datum table couldn't be initialized (check existance and path of 3_param.dat and 7_param.dat"));
        datumCount = 0;
        datumList = 0;
    }
    else
    {
        char datumName[64];
        QString datName;
        Datum_Count(&datumCount);
        for (int i = 1; i < datumCount; i++)
        {
            Datum_Name(i, datumName);
            datName = datumName;
            datName = datName.simplifyWhiteSpace();
            //datName=datName.lower();//datName.truncate(26);
            datumList.append(datName);
        }
    }
}

void GeoDatum::convertDatum(long fromIdx, long toIdx, double *lt, double *lg, double *altitude)
{
    long error;
    error = Geodetic_Datum_Shift(fromIdx, *lt, *lg, *altitude, toIdx, lt, lg, altitude);

    if (error)
    {
        QString msg =
            "Error at the datum conversion of the current position:\n ";
        /*DATUM_NOT_INITIALIZED_ERROR */
        if (error & 0x00001)
            msg.append("Datum module has not been initialized\n ");
        /*DATUM_7PARAM_FILE_OPEN_ERROR */
        if (error & 0x00002)
            msg.append("7 parameter file opening error\n ");
        /*DATUM_7PARAM_FILE_PARSING_ERROR */
        if (error & 0x00004)
            msg.append("7 parameter file structure error\n ");
        /*DATUM_7PARAM_OVERFLOW_ERROR */
        if (error & 0x00008)
            msg.append("7 parameter table overflow\n ");
        /*DATUM_3PARAM_FILE_OPEN_ERROR */
        if (error & 0x00010)
            msg.append("3 parameter file opening error\n ");
        /*DATUM_3PARAM_FILE_PARSING_ERROR */
        if (error & 0x00020)
            msg.append("3 parameter file structure error\n ");
        /*DATUM_3PARAM_OVERFLOW_ERROR */
        if (error & 0x00040)
            msg.append("3 parameter table overflow\n ");
        /*DATUM_INVALID_INDEX_ERROR */
        if (error & 0x00080)
            msg.append
                ("Index out of valid range (less than one or more than Datum_Count)\n ");
        /*DATUM_INVALID_SRC_INDEX_ERROR */
        if (error & 0x00100)
            msg.append("Source datum index invalid\n ");
        /*DATUM_INVALID_DEST_INDEX_ERROR */
        if (error & 0x00200)
            msg.append("Destination datum index invalid\n ");
        /*DATUM_INVALID_CODE_ERROR */
        if (error & 0x00400)
            msg.append("Datum code not found in table\n ");
        /*DATUM_LAT_ERROR */
        if (error & 0x00800)
            msg.append("Latitude out of valid range (-90 to 90 in rad)\n ");
        /*DATUM_LON_ERROR */
        if (error & 0x01000)
            msg.append
                ("Longitude out of valid range (-180 to 360 in rad)\n ");
        /*DATUM_SIGMA_ERROR */
        if (error & 0x02000)
            msg.append
                ("Standard error values must be positive(or -1 if unknown)\n ");
        /*DATUM_DOMAIN_ERROR */
        if (error & 0x04000)
            msg.append("Domain of validity not well defined\n ");
        /*DATUM_ELLIPSE_ERROR */
        if (error & 0x08000)
            msg.append
                ("Error in ellipsoid module (check existance and path of ellipse.dat)\n ");
        /*DATUM_NOT_USERDEF_ERROR */
        if (error & 0x10000)
            msg.append
                ("Datum code is not user defined - cannot be deleted\n ");
        qDebug(msg);
    }
}

GpsData::GpsData(const QString& qpefolder)
 : qpedir(qpefolder), geoDatum(qpefolder)
{
    d_connected = false;
    d_aliveGPS = false;
    ttwph = 99;
    ttwpm = 99;
    ttwps = 99;
    showTime = false;
    d_no_of_satellites = 0;
    d_no_of_fix_satellites = 0;
    d_Receiver = "";
    latitudeGps = 0;            /* Added by A/ Karhov */
    longitudeGps = 0;           /* Added by A/ Karhov */
   

}

GpsData::~GpsData()
{
}

void GpsData::adjustDatum()
{
    double lt, lg, alt;
    lt = MathEx::deg2rad(currPos.latitude());
    lg = MathEx::deg2rad(currPos.longitude());
    alt = altitude.toDouble();
    geoDatum.convertDatum(gpsDatumIdx, mapDatumIdx, &lt, &lg, &alt);
    currPos.setLat(MathEx::rad2deg(lt));
    currPos.setLong(MathEx::rad2deg(lg));
    altitude.setFromDouble(alt);

    // waypoint position ? altitude of waypoint ?
};

QString GpsData::timeToString()
{
    if (avspeed.toDouble() > 0)
    {
        ttwph = wpDistance.toDouble() / avspeed.toDouble();
        ttwpm = (ttwph - floor(ttwph)) * 60.0;
        ttwph = floor(ttwph);
        ttwps = floor((ttwpm - floor(ttwpm)) * 60.0);
        ttwpm = floor(ttwpm);
    }

    if (showTime)
        return QString("%1:%2:%3").arg(ttwph, 2, 'f', 0)
                             .arg(ttwpm, 2, 'f', 0)
                             .arg(ttwps, 2, 'f', 0);
    else
        return "";
}

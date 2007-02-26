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


#include "gpsdata.h"
#include "track.h"

// initialization of static elements:
Altitude::Alt Altitude::altUnit = Altitude::Feet;
Speed::Sp Speed::speedUnit = Speed::Knots;
Distance::Dist Distance::distUnit = Distance::Naut;
Position::Pos Position::posUnit = Position::DegMin;

QString Altitude::toString()
{
    switch(altUnit)
    {
    case None:
        return tr("");
        break;
    case Meter:
        return tr("%1 m").arg(altitude,0,'f',0);
        break;
    case Feet:
        return tr("%1 ft").arg(altitude*3.2808399,0,'f',0);
        break;
    case FL:
        return tr("%1 FL").arg(altitude*0.032808399,0,'f',2);
        break;
    }
    return 0;
}


double Altitude::getAlt(double meters)
{
    switch(altUnit)
    {
    case None:
    default:
        return 0;
        break;
    case Meter:
        return meters;
        break;
    case Feet:
        return meters*3.2808399;
        break;
    case FL:
        return meters*0.032808399;
        break;
    }
}


QString Speed::toString()
{
    switch(speedUnit)
    {
    case None:
        return tr("");
        break;
    case Kmh:
        return tr("%1 kmh").arg(speed*1.852,0,'f',1);
        break;
    case Knots:
        return tr("%1 kn").arg(speed,0,'f',1);
        break;
    case Mph:
        return tr("%1 mph").arg(speed*1.1507794,0,'f',1);
        break;
    }
    return 0;
}

QString Distance::toString()
{
    switch(distUnit)
    {
    case None:
        return tr("");
        break;
    case Km:
        return tr("%1 km").arg(distance*1.852,0,'f',3); /* Added by A/ Karhov */
        break;
    case Naut:
        return tr("%1 nmi").arg(distance,0,'f',3); /* Added by A/ Karhov */
        break;
    case Statute:
        return tr("%1 mi").arg(distance*1.1507794,0,'f',3); /* Added by A/ Karhov */
        break;
    }
    return 0;
}

Position::Position(QWidget *parent, const char *name):QObject(parent,name)
{
    latitude=0;
    longitude=0;
}

QString Position::latToString(double latit) /* Added by A/ Karhov */
{
    double lat,deg,min,sec;
    QString sign;
    lat = fabs(latit);
    if(latit > 0)
        sign = tr("N");
    else
        sign = tr("S");
    switch(posUnit)
    {
    case Degree:
        return tr("%1\260%2").arg(lat,0,'f',5).arg(sign);
        break;
    case DegMin:
        deg = floor(lat);
        min = (lat-deg)*60.0;
        return tr("%1\260%2'%3").arg(deg, 0,'f',0)
               .arg(min,0,'f',3).arg(sign);
        break;
    case DegMinSec:
        deg = floor(lat);
        min = floor((lat-deg)*60.0);
        sec = (lat-(deg+min/60.0))*3600.0;
        return tr("%1\260%2'%3\"%4").arg(deg, 0,'f',0)
               .arg(min, 0,'f',0)
               .arg(sec, 0,'f',2).arg(sign);
        break;
    }
    return 0;
}

QString Position::longToString(double longi) /* Added by A/ Karhov */
{
    double lg,deg,min,sec;
    QString sign;
    lg = fabs(longi);
    if(longi > 0)
        sign = tr("E");
    else
        sign = tr("W");
    switch(posUnit)
    {
    case Degree:
        return tr("%1\260%2").arg(lg,0,'f',5).arg(sign);
        break;
    case DegMin:
        deg = floor(lg);
        min = (lg-deg)*60.0;
        return tr("%1\260%2'%3").arg(deg, 0,'f',0)
               .arg(min,0,'f',3).arg(sign);
        break;
    case DegMinSec:
        deg = floor(lg);
        min = floor((lg-deg)*60.0);
        sec = (lg-(deg+min/60.0))*3600.0;
        return tr("%1\260%2'%3\"%4").arg(deg, 0,'f',0)
               .arg(min, 0,'f',0)
               .arg(sec, 0,'f',2).arg(sign);
        break;
    }
    return 0;
}

double Position::setLong(QString lgString)
{
    int p;
    float deg=0, min=0, sec=0, west=1;
    QString lgStr;
    QRegExp re("[\\s'\"]");

    lgStr = lgString.simplifyWhiteSpace();

    p=lgStr.find(tr("w"),0,FALSE);
    if(p>=0)
    {
        west = -1.0;
        lgStr.remove(p,1);
        lgStr = lgStr.simplifyWhiteSpace();
    }
    p=lgStr.find(tr("e"),0,FALSE);
    if(p>=0)
    {
        lgStr.remove(p,1);
        lgStr = lgStr.simplifyWhiteSpace();
    }

    deg = lgStr.toFloat();
    p = lgStr.find(re);
    if(p>0)
    {
        lgStr = lgStr.mid(p+1);
        min = lgStr.toFloat();
        p = lgStr.find(re);
        if(p>0)
        {
            lgStr = lgStr.mid(p+1);
            sec = lgStr.toFloat();
        }
    }
    longitude = west * (deg + (min/60.0) + (sec/3600.0));
    return longitude;
}

double Position::setLat(QString ltString)
{
    int p;
    float deg=0, min=0, sec=0, south=1;
    QString ltStr;
    QRegExp re("[\\s'\"]");

    ltStr = ltString.simplifyWhiteSpace();

    p=ltStr.find(tr("s"),0,FALSE);
    if(p>=0)
    {
        south = -1.0;
        ltStr.remove(p,1);
        ltStr = ltStr.simplifyWhiteSpace();
    }
    p=ltStr.find(tr("n"),0,FALSE);
    if(p>=0)
    {
        ltStr.remove(p,1);
        ltStr = ltStr.simplifyWhiteSpace();
    }

    deg = ltStr.toFloat();
    p = ltStr.find(re);
    if(p>0)
    {
        ltStr = ltStr.mid(p+1);
        min = ltStr.toFloat();
        p = ltStr.find(re);
        if(p>0)
        {
            ltStr = ltStr.mid(p+1);
            sec = ltStr.toFloat();
        }
    }
    latitude = south * (deg + (min/60.0) + (sec/3600.0));
    return latitude;
}

QString Position::setLong(double lg)
{
    longitude = lg;
    return longToString(lg); /* Added by A/ Karhov */
}

QString Position::setLat(double lt)
{
    latitude = lt;
    return latToString(lt); /* Added by A/ Karhov */
}

QString Angle::toString()
{
    if(show)
        return tr("%1").arg(angle,0,'f',0);
    else
        return tr("");
}

QString TimeStamp::toString()
{
    if (date.isEmpty() || time.isEmpty())
        return tr("* No GMT Signal rcvd *");

    return date + " " + time;
}

GeoDatum::GeoDatum()
{
    errorCode=Initialize_Ellipsoids();
    if(errorCode)
      qWarning(tr("Ellipsoid table couldn't be initialized (check existance and path of ellipse.dat)"));
    errorCode=Initialize_Datums();
    if(errorCode)
    {
	qWarning(tr("Datum table couldn't be initialized (check existance and path of 3_param.dat and 7_param.dat"));
	datumCount = 0;
	datumList=0;
    }
    else
    {
	char datumName[64];
        QString datName;
	Datum_Count(&datumCount);
	for(long i=1; i < datumCount; i++)
	{
	    Datum_Name( i, datumName);
            datName = datumName;
            datName=datName.simplifyWhiteSpace();
            //datName=datName.lower();//datName.truncate(26);
            datumList.append(datName);
	}
    }
}

GeoDatum::~GeoDatum()
{
}

QStringList GeoDatum::getDatumList()
{
    return datumList;
}

void GeoDatum::convertDatum(long fromIdx, long toIdx, double *lt, double *lg, double *altitude)
{
    long error;
    error = Geodetic_Datum_Shift ( fromIdx,
				   *lt,
				   *lg,
				   *altitude,
				   toIdx,
				   lt,
				   lg,
				   altitude);
   
    if(error)
    {
	QString msg="Error at the datum conversion of the current position:\n ";
	/*DATUM_NOT_INITIALIZED_ERROR*/
	if(error&0x00001)
	    msg.append("Datum module has not been initialized\n ");
	    /*DATUM_7PARAM_FILE_OPEN_ERROR*/ 
	    if(error&0x00002)
	    msg.append("7 parameter file opening error\n ");
	    /*DATUM_7PARAM_FILE_PARSING_ERROR*/
	    if(error&0x00004)
	    msg.append("7 parameter file structure error\n ");
	    /*DATUM_7PARAM_OVERFLOW_ERROR*/
	    if(error&0x00008)
	    msg.append("7 parameter table overflow\n ");
	    /*DATUM_3PARAM_FILE_OPEN_ERROR*/
            if(error&0x00010)
	    msg.append("3 parameter file opening error\n ");
	    /*DATUM_3PARAM_FILE_PARSING_ERROR*/
	    if(error&0x00020)
	    msg.append("3 parameter file structure error\n ");
	    /*DATUM_3PARAM_OVERFLOW_ERROR*/
	    if(error&0x00040)
	    msg.append("3 parameter table overflow\n ");
	    /*DATUM_INVALID_INDEX_ERROR*/
	    if(error&0x00080)
	    msg.append("Index out of valid range (less than one or more than Datum_Count)\n ");
	    /*DATUM_INVALID_SRC_INDEX_ERROR*/
	    if(error&0x00100)
	    msg.append("Source datum index invalid\n ");
	    /*DATUM_INVALID_DEST_INDEX_ERROR*/
	    if(error&0x00200)
	    msg.append("Destination datum index invalid\n ");
	    /*DATUM_INVALID_CODE_ERROR*/
	    if(error&0x00400)
	    msg.append("Datum code not found in table\n ");
	    /*DATUM_LAT_ERROR*/
	    if(error&0x00800)
	    msg.append("Latitude out of valid range (-90 to 90 in rad)\n ");
	    /*DATUM_LON_ERROR*/
	    if(error&0x01000)
	    msg.append("Longitude out of valid range (-180 to 360 in rad)\n ");
	    /*DATUM_SIGMA_ERROR*/
	    if(error&0x02000)
	    msg.append("Standard error values must be positive(or -1 if unknown)\n ");
	    /*DATUM_DOMAIN_ERROR*/
	    if(error&0x04000)
	    msg.append("Domain of validity not well defined\n ");
	    /*DATUM_ELLIPSE_ERROR*/
	    if(error&0x08000)
	    msg.append("Error in ellipsoid module (check existance and path of ellipse.dat)\n ");
	    /*DATUM_NOT_USERDEF_ERROR*/
	    if(error&0x10000)
	    msg.append("Datum code is not user defined - cannot be deleted\n ");
	    qDebug(msg);
	    }
}

bool SatInfo::operator == (const SatInfo & other) const
{
    return ((d_satName == other.d_satName) &&
            (d_elevation == other.d_elevation) &&
            (d_azimut == other.d_azimut) &&
            (d_snr == other.d_snr));
}

SatInfo & SatInfo::operator =(const class SatInfo & other)
{
    // memberwise copy (needed here because QObject operator = is private)
    d_satName = other.d_satName;
    d_elevation = other.d_elevation;
    d_azimut = other.d_azimut;
    d_snr = other.d_snr;
    d_updated = other.d_updated;

    return *this;
}

GpsData::GpsData(QWidget *parent, const char *name):
        QObject(parent,name)
{
    d_connected = false;
    d_aliveGPS = false;
    ttwph=99; ttwpm=99; ttwps=99;
    showTime = FALSE;
    ts.date = "";
    ts.time = "";
    d_no_of_satellites = 0;
    d_no_of_fix_satellites = 0;
    d_Receiver = "";
    latitudeGps=0;	/* Added by A/ Karhov */
    longitudeGps=0;	/* Added by A/ Karhov */
    useProxy=FALSE;

}

GpsData::~GpsData()
{
};

void GpsData::adjustDatum()
{
  double lt, lg;
  lt = currPos.latitude * M_PI / 180.0;
  lg = currPos.longitude * M_PI / 180.0;
  geoDatum.convertDatum(gpsDatumIdx, mapDatumIdx,
      &lt, &lg, &(altitude.altitude) );
  currPos.latitude = lt * 180.0 / M_PI;
  currPos.longitude= lg * 180.0 / M_PI;

  // waypoint position ? altitude of waypoint ?
};

QString GpsData::timeToString()
{
    if(avspeed.speed > 0)
    {
        ttwph = wpDistance.distance / avspeed.speed;
        ttwpm = (ttwph - floor(ttwph)) * 60.0;
        ttwph = floor(ttwph);
        ttwps = floor((ttwpm - floor(ttwpm)) * 60.0);
        ttwpm = floor(ttwpm);
    }

    if(showTime)
        return tr("%1:%2:%3").arg(ttwph,2,'f',0).arg(ttwpm,2,'f',0).arg(ttwps,2,'f',0);
    else
        return tr("");
}


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


#include "maps.h"

const double deg2rad = M_PI / 180.0;
const double rad2deg = 180.0 / M_PI;
const double PI_1_2 = M_PI / 2.0;
const double PI_1_4 = M_PI / 4.0;
const double PI_3_4 = M_PI * 0.75;

inline double checkLg(double lg)
{
    if (lg > M_PI)
        lg -= 2 * M_PI;
    else if (lg < -M_PI)
        lg += 2 * M_PI;
    return lg;
}


MapBase::MapBase(QString * mapInfo)
{
    // read map info
    mapIStream = new QTextIStream(mapInfo);
    *mapIStream >> name >> scale >> mapSizeX >> mapSizeY;
}

MapBase::MapBase()
{

}

MapBase::~MapBase()
{
}

int MapBase::operator<(MapBase & map)
{
    return (this->scale < map.scale);
}

int MapBase::operator==(MapBase & map)
{
    return (this->scale == map.scale);
}

int MapBase::operator>(MapBase & map)
{
    return (this->scale > map.scale);
}

MapLin::MapLin(QString * mapInfo):MapBase(mapInfo)
{
    // read additional map info
    *mapIStream >> longitude1 >> latitude1 >> x1 >> y1
        >> longitude2 >> latitude2 >> x2 >> y2;
    longitude1 *= deg2rad;
    latitude1 *= deg2rad;
    longitude2 *= deg2rad;
    latitude2 *= deg2rad;
    projection = "LINEAR";
    checkLg(longitude1);
    checkLg(longitude2);
}

MapLin::MapLin():MapBase()
{
    projection = "LINEAR";
}

MapLin::~MapLin()
{
}

bool MapLin::calcxy(double *x, double *y, double lg, double lt) // return x,y in file (not screen) coordinates, lt, lg - in radians
{
    *x = x1 +
        (checkLg(lg - longitude1) * (x2 - x1) /
         checkLg(longitude2 - longitude1));
    *y = y1 + ((lt - latitude1) * (y2 - y1) / (latitude2 - latitude1));
    if ((*x < 0) || (*x >= mapSizeX) || (*y < 0) || (*y >= mapSizeY))
        return (FALSE);
    return (TRUE);
}

bool MapLin::calcltlg(double *lt, double *lg, double x, double y)
{
    *lg =
        checkLg(longitude1 +
                ((x - x1) * checkLg(longitude2 - longitude1) / (x2 - x1)));
    *lt = latitude1 + ((y - y1) * (latitude2 - latitude1) / (y2 - y1));
    return TRUE;
}

QString MapLin::getInfo()
{
    QString info;
    info =
        tr("Linear Projection, SCALE 1:%1,\nLat: %2 to %3, Lon: %4 to %5").
        arg(scale).arg(latitude1 * rad2deg).arg(latitude2 *
                                                rad2deg).arg(longitude1 *
                                                             rad2deg).
        arg(longitude2 * rad2deg);
    return info;
}

QString MapLin::getParameterStr()
{
    QString param;
/*    param = tr("%1 %2 %3 %4 %5 %6 %7 %8 %9")
            .arg(projection).arg(name).arg(scale).arg(mapSizeX).arg(mapSizeY)
            .arg(longitude1*rad2deg).arg(latitude1*rad2deg).arg(x1).arg(y1);
    param.append(tr(" %1 %2 %3 %4")
                 .arg(longitude2*rad2deg).arg(latitude2*rad2deg).arg(x2).arg(y2));
*/
    QTextOStream os(&param);
    os.precision(10);
    os << projection << " " << name << " " << scale << " " << mapSizeX
        << " " << mapSizeY << " " << longitude1 *
        rad2deg << " " << latitude1 *
        rad2deg << " " << x1 << " " << y1 << " " << longitude2 *
        rad2deg << " " << latitude2 * rad2deg << " " << x2 << " " << y2;

    return param;
}

MapCEA::MapCEA(QString * mapInfo):MapBase(mapInfo)
{
    // read additional map info
    *mapIStream >> longitude1 >> latitude1 >> x1 >> y1
        >> longitude2 >> latitude2 >> x2 >> y2;

    longitude1 *= deg2rad;
    latitude1 *= deg2rad;
    longitude2 *= deg2rad;
    latitude2 *= deg2rad;
    projection = "CEA";

    // precalc
    xlong1 = checkLg(longitude1);
    ylat1 = sin(latitude1);
    xlong2 = checkLg(longitude2);
    ylat2 = sin(latitude2);
}

MapCEA::MapCEA():MapBase()
{
    projection = "CEA";
}

MapCEA::~MapCEA()
{
}

bool MapCEA::calcxy(double *x, double *y, double lg, double lt)
{
    double xt, yt;
    xt = lg;
    yt = sin(lt);
    *x = x1 + (checkLg(xt - xlong1) * (x2 - x1) / checkLg(xlong2 - xlong1));
    *y = y1 + ((yt - ylat1) * (y2 - y1) / (ylat2 - ylat1));
    if ((*x < 0) || (*x >= mapSizeX) || (*y < 0) || (*y >= mapSizeY))
        return (FALSE);
    return (TRUE);
}

bool MapCEA::calcltlg(double *lt, double *lg, double x, double y)
{
    double xt, yt;
    xt = xlong1 + ((x - x1) * checkLg(xlong2 - xlong1) / (x2 - x1));
    yt = ylat1 + ((y - y1) * (ylat2 - ylat1) / (y2 - y1));
    *lt = asin(yt);
    *lg = checkLg(xt);
    return TRUE;
}

QString MapCEA::getInfo()
{
    QString info;
    info = tr("Cylindrical Equal Area Projection, SCALE 1:%1,\n"
              "Lat: %2 to %3, Lon: %4 to %5").arg(scale).arg(latitude1 *
                                                             rad2deg).
        arg(latitude2 * rad2deg).arg(longitude1 * rad2deg).arg(longitude2 *
                                                               rad2deg);
    return info;
}

QString MapCEA::getParameterStr()
{
    QString param;
/*
    param = tr("%1 %2 %3 %4 %5 %6 %7 %8 %9")
            .arg(projection).arg(name).arg(scale).arg(mapSizeX).arg(mapSizeY)
            .arg(longitude1*rad2deg).arg(latitude1*rad2deg).arg(x1).arg(y1);
    param.append(tr(" %1 %2 %3 %4")
                 .arg(longitude2*rad2deg).arg(latitude2*rad2deg).arg(x2).arg(y2));
*/
    QTextOStream os(&param);
    os.precision(10);
    os << projection << " " << name << " " << scale << " " << mapSizeX
        << " " << mapSizeY << " " << longitude1 *
        rad2deg << " " << latitude1 *
        rad2deg << " " << x1 << " " << y1 << " " << longitude2 *
        rad2deg << " " << latitude2 * rad2deg << " " << x2 << " " << y2;

    return param;
}

MapUTM::MapUTM(QString * mapInfo, bool utmCoord):MapBase(mapInfo)
{
    // read additional map info
    if (utmCoord)
    {
        universal = TRUE;
        *mapIStream >> utmZone >> utmNorthing1 >> utmEasting1 >> x1 >> y1
            >> utmNorthing2 >> utmEasting2 >> x2 >> y2;
        UTMtoLL(utmNorthing1, utmEasting1, utmZone, latitude1, longitude1,
                stdLong);
        UTMtoLL(utmNorthing2, utmEasting2, utmZone, latitude2, longitude2,
                stdLong);
        projection = "UTM";
    }
    else
    {
        universal = FALSE;
        *mapIStream >> longitude1 >> latitude1 >> x1 >> y1
            >> longitude2 >> latitude2 >> x2 >> y2 >> stdLong;
        projection = "TM";
    }
    longitude1 *= deg2rad;
    latitude1 *= deg2rad;
    longitude2 *= deg2rad;
    latitude2 *= deg2rad;
    stdLong *= deg2rad;

    // precalc
    xlong1 = 0.5 * atanh(cos(latitude1) * sin(checkLg(longitude1 - stdLong)));
    ylat1 = atan(tan(latitude1) / cos(checkLg(longitude1 - stdLong)));
    xlong2 = 0.5 * atanh(cos(latitude2) * sin(checkLg(longitude2 - stdLong)));
    ylat2 = atan(tan(latitude2) / cos(checkLg(longitude2 - stdLong)));
}

MapUTM::MapUTM(bool utmCoord):MapBase()
{
    if (utmCoord)
    {
        universal = TRUE;
        projection = "UTM";
    }
    else
    {
        universal = FALSE;
        projection = "TM";
    }
}

void MapUTM::UTMtoLL(double UTMNorthing, double UTMEasting, QString UTMZone,
                     double &Lat, double &Long, double &stdLong)
{
    //converts UTM coords to lat/long.  Equations from USGS Bulletin 1532
    //East Longitudes are positive, West longitudes are negative.
    //North latitudes are positive, South latitudes are negative
    //Lat and Long are in decimal degrees.
    //Written by Chuck Gantz- chuck.gantz@globalstar.com

    double k0 = 0.9996;
    double a = 6378137;
    double eccSquared = 0.00669438;
    double eccPrimeSquared;
    double e1 = (1 - sqrt(1 - eccSquared)) / (1 + sqrt(1 - eccSquared));
    double N1, T1, C1, R1, D, M;
    double LongOrigin;
    double mu, phi1, phi1Rad;
    double x, y;
    int ZoneNumber;
    char *ZoneLetter;
    int NorthernHemisphere;     //1 for northern hemispher, 0 for southern

    x = UTMEasting - 500000.0;  //remove 500,000 meter offset for longitude
    y = UTMNorthing;

    ZoneNumber = strtoul(UTMZone.latin1(), &ZoneLetter, 10);
    if ((*ZoneLetter - 'N') >= 0)
        NorthernHemisphere = 1; //point is in northern hemisphere
    else
    {
        NorthernHemisphere = 0; //point is in southern hemisphere
        y -= 10000000.0;        //remove 10,000,000 meter offset used for southern hemisphere
    }


    eccPrimeSquared = (eccSquared) / (1 - eccSquared);

    M = y / k0;
    mu = M / (a *
              (1 - eccSquared / 4 - 3 * eccSquared * eccSquared / 64 -
               5 * eccSquared * eccSquared * eccSquared / 256));

    phi1Rad = mu + (3 * e1 / 2 - 27 * e1 * e1 * e1 / 32) * sin(2 * mu)
        + (21 * e1 * e1 / 16 - 55 * e1 * e1 * e1 * e1 / 32) * sin(4 * mu)
        + (151 * e1 * e1 * e1 / 96) * sin(6 * mu);
    phi1 = phi1Rad * deg2rad;

    N1 = a / sqrt(1 - eccSquared * sin(phi1Rad) * sin(phi1Rad));
    T1 = tan(phi1Rad) * tan(phi1Rad);
    C1 = eccPrimeSquared * cos(phi1Rad) * cos(phi1Rad);
    R1 = a * (1 - eccSquared) / pow(1 -
                                    eccSquared * sin(phi1Rad) * sin(phi1Rad),
                                    1.5);
    D = x / (N1 * k0);

    Lat =
        phi1Rad - (N1 * tan(phi1Rad) / R1) * (D * D / 2 -
                                              (5 + 3 * T1 + 10 * C1 -
                                               4 * C1 * C1 -
                                               9 * eccPrimeSquared) * D * D *
                                              D * D / 24 + (61 + 90 * T1 +
                                                            298 * C1 +
                                                            45 * T1 * T1 -
                                                            252 *
                                                            eccPrimeSquared -
                                                            3 * C1 * C1) * D *
                                              D * D * D * D * D / 720);
    Lat = Lat * rad2deg;

    switch (ZoneNumber)
    {
    case 31:
        if (Lat >= 72.0 && Lat < 84.0)
            LongOrigin = 4.5;   //special case Svalbard
        else if (Lat >= 56.0 && Lat < 64.0)
            LongOrigin = 1.5;   //special case southwest Norway
        else
            LongOrigin = (ZoneNumber - 1) * 6 - 180 + 3;        //+3 puts origin in middle of zone
        break;
    case 32:
        if (Lat >= 56.0 && Lat < 64.0)
            LongOrigin = 7.5;   //special case southwest Norway
        else
            LongOrigin = (ZoneNumber - 1) * 6 - 180 + 3;        //+3 puts origin in middle of zone
        break;
    case 33:
        if (Lat >= 72.0 && Lat < 84.0)
            LongOrigin = 15;    //special case Svalbard
        else
            LongOrigin = (ZoneNumber - 1) * 6 - 180 + 3;        //+3 puts origin in middle of zone
        break;
    case 35:
        if (Lat >= 72.0 && Lat < 84.0)
            LongOrigin = 27;    //special case Svalbard
        else
            LongOrigin = (ZoneNumber - 1) * 6 - 180 + 3;        //+3 puts origin in middle of zone
        break;
    case 37:
        if (Lat >= 72.0 && Lat < 84.0)
            LongOrigin = 37.5;  //special case Svalbard
        else
            LongOrigin = (ZoneNumber - 1) * 6 - 180 + 3;        //+3 puts origin in middle of zone
        break;

    default:
        LongOrigin = (ZoneNumber - 1) * 6 - 180 + 3;    //+3 puts origin in middle of zone
        break;
    }
    stdLong = LongOrigin;

    Long =
        (D - (1 + 2 * T1 + C1) * D * D * D / 6 +
         (5 - 2 * C1 + 28 * T1 - 3 * C1 * C1 + 8 * eccPrimeSquared +
          24 * T1 * T1) * D * D * D * D * D / 120) / cos(phi1Rad);
    Long = LongOrigin + Long * rad2deg;
}

MapUTM::~MapUTM()
{
}

bool MapUTM::calcxy(double *x, double *y, double lg, double lt)
{
    double xt, yt;
    xt = 0.5 * atanh(cos(lt) * sin(checkLg(lg - stdLong)));
    yt = atan(tan(lt) / cos(checkLg(lg - stdLong)));
    *x = x1 + (checkLg(xt - xlong1) * (x2 - x1) / checkLg(xlong2 - xlong1));
    *y = y1 + ((yt - ylat1) * (y2 - y1) / (ylat2 - ylat1));
    if ((*x < 0) || (*x >= mapSizeX) || (*y < 0) || (*y >= mapSizeY))
        return (FALSE);
    return (TRUE);
}

bool MapUTM::calcltlg(double *lt, double *lg, double x, double y)
{
    double xt, yt;
    xt = xlong1 + ((x - x1) * checkLg(xlong2 - xlong1) / (x2 - x1));
    yt = ylat1 + ((y - y1) * (ylat2 - ylat1) / (y2 - y1));
    *lt = asin(sin(yt) / cosh(xt));
    *lg = checkLg(stdLong + atan(sinh(xt) / cos(yt)));
    return TRUE;
}

QString MapUTM::getInfo()
{
    QString info;
    if (universal)
    {
        info =
            tr
            ("Universal Transverse Mercator Projection, SCALE 1:%1,\nLat: %2 to %3, Lon: %4 to %5\n"
             "std.Longitude: %6").arg(scale).arg(latitude1 *
                                                 rad2deg).arg(latitude2 *
                                                              rad2deg).
            arg(longitude1 * rad2deg).arg(longitude2 * rad2deg).arg(stdLong *
                                                                    rad2deg);
    }
    else
    {
        info =
            tr
            ("Transverse Mercator Projection, SCALE 1:%1,\nLat: %2 to %3, Lon: %4 to %5\n"
             "std.Longitude: %6").arg(scale).arg(latitude1 *
                                                 rad2deg).arg(latitude2 *
                                                              rad2deg).
            arg(longitude1 * rad2deg).arg(longitude2 * rad2deg).arg(stdLong *
                                                                    rad2deg);
    }

    return info;
}

QString MapUTM::getParameterStr()
{
    QString param;
    if (universal)
    {
/*        param = tr("%1 %2 %3 %4 %5 %6 %7 %8 %9")
                .arg(projection).arg(name).arg(scale).arg(mapSizeX).arg(mapSizeY)
                .arg(utmZone).arg(utmNorthing1).arg(utmEasting1).arg(x1);
        param.append(tr(" %1 %2 %3 %4 %5")
                     .arg(y1).arg(utmNorthing2).arg(utmEasting2).arg(x2).arg(y2));
   */
        QTextOStream os(&param);
        os.precision(10);
        os << projection << " " << name << " " << scale << " " <<
            mapSizeX << " " << mapSizeY << " " << utmZone << " " <<
            utmNorthing1 << " " << utmEasting1 << " " << x1 << " " << y1 <<
            " " << utmNorthing2 << " " << utmEasting2 << " " << x2 << " " <<
            y2;
        return param;
    }
    else
    {
/*        param = tr("%1 %2 %3 %4 %5 %6 %7 %8 %9")
                .arg(projection).arg(name).arg(scale).arg(mapSizeX).arg(mapSizeY)
                .arg(longitude1*rad2deg).arg(latitude1*rad2deg).arg(x1).arg(y1);
        param.append(tr(" %1 %2 %3 %4 %5")
                     .arg(longitude2*rad2deg).arg(latitude2*rad2deg).arg(x2).arg(y2).arg(stdLong*rad2deg));
   */
        QTextOStream os(&param);
        os.precision(10);
        os << projection << " " << name << " " << scale << " " <<
            mapSizeX << " " << mapSizeY << " " << longitude1 *
            rad2deg << " " << latitude1 *
            rad2deg << " " << x1 << " " << y1 << " " << longitude2 *
            rad2deg << " " << latitude2 *
            rad2deg << " " << x2 << " " << y2 << " " << stdLong * rad2deg;
        return param;
    }
}

MapMercator::MapMercator(QString * mapInfo):MapBase(mapInfo)
{
    // read additional map info
    *mapIStream >> longitude1 >> latitude1 >> x1 >> y1
        >> longitude2 >> latitude2 >> x2 >> y2;

    longitude1 *= deg2rad;
    latitude1 *= deg2rad;
    longitude2 *= deg2rad;
    latitude2 *= deg2rad;
    projection = "MERCATOR";

    // precalc
    xlong1 = checkLg(longitude1);
    //ylat1 = log(tan(latitude1)+1.0/cos(latitude1));
    ylat1 = atanh(sin(latitude1));
    xlong2 = checkLg(longitude2);
    //ylat2 = log(tan(latitude2)+1.0/cos(latitude2));
    ylat2 = atanh(sin(latitude2));
}

MapMercator::MapMercator():MapBase()
{
    projection = "MERCATOR";
}

MapMercator::~MapMercator()
{
}

bool MapMercator::calcxy(double *x, double *y, double lg, double lt)
{
    double xt, yt;
    xt = lg;
    //yt = log(tan(lt)+1.0/cos(lt));
    yt = atanh(sin(lt));
    *x = x1 + (checkLg(xt - xlong1) * (x2 - x1) / checkLg(xlong2 - xlong1));
    *y = y1 + ((yt - ylat1) * (y2 - y1) / (ylat2 - ylat1));
    if ((*x < 0) || (*x >= mapSizeX) || (*y < 0) || (*y >= mapSizeY))
        return (FALSE);
    return (TRUE);
}

bool MapMercator::calcltlg(double *lt, double *lg, double x, double y)
{
    double xt, yt;
    xt = xlong1 + ((x - x1) * checkLg(xlong2 - xlong1) / (x2 - x1));
    yt = ylat1 + ((y - y1) * (ylat2 - ylat1) / (y2 - y1));
    *lt = atan(sinh(yt));
    *lg = checkLg(xt);
    return TRUE;
}

QString MapMercator::getInfo()
{
    QString info;
    info =
        tr("Mercator Projection, SCALE 1:%1,\nLat: %2 to %3, Lon: %4 to %5").
        arg(scale).arg(latitude1 * rad2deg).arg(latitude2 *
                                                rad2deg).arg(longitude1 *
                                                             rad2deg).
        arg(longitude2 * rad2deg);
    return info;
}

QString MapMercator::getParameterStr()
{
    QString param;
    /* param = tr("%1 %2 %3 %4 %5 %6 %7 %8 %9")
       .arg(projection).arg(name).arg(scale).arg(mapSizeX).arg(mapSizeY)
       .arg(longitude1*rad2deg).arg(latitude1*rad2deg).arg(x1).arg(y1);
       param.append(tr(" %1 %2 %3 %4")
       .arg(longitude2*rad2deg).arg(latitude2*rad2deg).arg(x2).arg(y2)); */
    QTextOStream os(&param);
    os.precision(10);
    os << projection << " " << name << " " << scale << " " <<
        mapSizeX << " " << mapSizeY << " " << longitude1 *
        rad2deg << " " << latitude1 *
        rad2deg << " " << x1 << " " << y1 << " " << longitude2 *
        rad2deg << " " << latitude2 * rad2deg << " " << x2 << " " << y2;
    return param;
}

MapLambert::MapLambert(QString * mapInfo):MapBase(mapInfo)
{
    double p;
    // read additional map info
    *mapIStream >> longitude1 >> latitude1 >> x1 >> y1
        >> longitude2 >> latitude2 >> x2 >> y2
        >> std1Lat >> std2Lat >> refLong;

    longitude1 *= deg2rad;
    latitude1 *= deg2rad;
    longitude2 *= deg2rad;
    latitude2 *= deg2rad;
    std1Lat *= deg2rad;
    std2Lat *= deg2rad;
    refLong *= deg2rad;
    projection = "LAMBERT";

    // precalc
    n = log(cos(std1Lat) * (1 / cos(std2Lat))) /
        log(tan(PI_1_4 + std2Lat / 2) * (-tan(PI_3_4 + std1Lat / 2)));
    F = (cos(n) * pow(tan(PI_1_4 + std1Lat / 2), n)) / n;
    p0 = F * pow((-tan(PI_3_4)), n);
    p = F * pow((-tan(PI_3_4 + latitude1 / 2)), n);
    xlong1 = p * sin(n * (longitude1 - refLong));
    ylat1 = p0 - p * cos(n * (longitude1 - refLong));
    p = F * pow((-tan(PI_3_4 + latitude2 / 2)), n);
    xlong2 = p * sin(n * (longitude2 - refLong));
    ylat2 = p0 - p * cos(n * (longitude2 - refLong));
    if (n > 0)
        n_sign = 1.0;
    else if (n == 0)
        n_sign = 0.0;
    else
        n_sign = -1.0;
}

MapLambert::MapLambert():MapBase()
{
}

MapLambert::~MapLambert()
{
}

bool MapLambert::calcxy(double *x, double *y, double lg, double lt)
{
    double p, xt, yt;
    p = F * pow((-tan(PI_3_4 + lt / 2)), n);
    xt = p * sin(n * (lg - refLong));
    yt = p0 - p * cos(n * (lg - refLong));
    *x = x1 + ((xt - xlong1) * (x2 - x1) / (xlong2 - xlong1));
    *y = y1 + ((yt - ylat1) * (y2 - y1) / (ylat2 - ylat1));
    if ((*x < 0) || (*x >= mapSizeX) || (*y < 0) || (*y >= mapSizeY))
        return (FALSE);
    return (TRUE);
}

bool MapLambert::calcltlg(double *lt, double *lg, double x, double y)
{
    double p, s, xt, yt;
    xt = xlong1 + ((x - x1) * (xlong2 - xlong1) / (x2 - x1));
    yt = ylat1 + ((y - y1) * (ylat2 - ylat1) / (y2 - y1));
    p = n_sign * sqrt(xt * xt + (p0 - yt) * (p0 - yt));
    s = atan(xt / (p0 - yt));
    *lt = 2.0 * atan(pow(F / p, 1.0 / n)) - (PI_1_2);
    *lg = refLong + (s / n);
    return TRUE;
}

QString MapLambert::getInfo()
{
    QString info;
    info =
        tr("Lambert Projection, SCALE 1:%1,\nLat: %2 to %3, Lon: %4 to %5\n"
           "std.Latitudes: %6 %7, ref longitude %8").arg(scale).
        arg(latitude1 * rad2deg).arg(latitude2 * rad2deg).arg(longitude1 *
                                                              rad2deg).
        arg(longitude2 * rad2deg).arg(std1Lat * rad2deg).arg(std2Lat *
                                                             rad2deg).
        arg(refLong * rad2deg);
    return info;
}

QString MapLambert::getParameterStr()
{
    QString param;
/*    param = tr("%1 %2 %3 %4 %5 %6 %7 %8")
            .arg(projection).arg(name).arg(scale).arg(mapSizeX).arg(mapSizeY)
            .arg(longitude1*rad2deg).arg(latitude1*rad2deg).arg(x1);
    param.append(tr(" %1 %2 %3 %4 %5 %6 %7 %8")
                 .arg(y1).arg(longitude2*rad2deg).arg(latitude2*rad2deg).arg(x2).arg(y2)
                 .arg(std1Lat*rad2deg).arg(std2Lat*rad2deg).arg(refLong*rad2deg));
*/
    QTextOStream os(&param);
    os.precision(10);
    os << projection << " " << name << " " << scale << " " <<
        mapSizeX << " " << mapSizeY << " " << longitude1 *
        rad2deg << " " << latitude1 *
        rad2deg << " " << x1 << " " << y1 << " " << longitude2 *
        rad2deg << " " << latitude2 *
        rad2deg << " " << x2 << " " << y2 << " " << std1Lat *
        rad2deg << " " << std2Lat * rad2deg << " " << refLong * rad2deg;
    return param;
}

// create static Ra
double MapFritz::Ra[201] = { 0 };

MapFritz::MapFritz(QString * mapInfo):MapBase(mapInfo)
{
    pixelfact = 10000 / 2817.947378;
    zero_long = 0;
    zero_lat = 0;
    projection = "FRITZ";
    /*
     * Build array for earth radii
     */
    if (!Ra[0])
    {
        int i;
        for (i = -100; i <= 100; i++)
            Ra[i + 100] = calcR(i);
    }

    // read additional map info
    *mapIStream >> center_latitude >> center_longitude;

    mapSizeX2 = mapSizeX / 2;
    mapSizeY2 = mapSizeY / 2;

}

MapFritz::MapFritz():MapBase()
{
    projection = "FRITZ";
}

MapFritz::~MapFritz()
{
}

bool MapFritz::calcxy(double *x, double *y, double lg, double lt)
{
    double dif;
    zero_long = center_longitude;
    zero_lat = center_latitude;
    pixelfact = scale / PIXELFACT;
    if (pixelfact == 0)
        return (FALSE);         /* Added by A. Karhov */
    lg *= rad2deg;              // FIXME, optimize equations
    lt *= rad2deg;
    *x = (Ra[(int) (100 + lt)] * deg2rad) *
        cos(lt * deg2rad) * (lg - zero_long);
    *y = (Ra[(int) (100 + lt)] * deg2rad) * (lt - zero_lat);
    dif = Ra[(int) (100 + lt)] * (1 - (cos((deg2rad * (lg - zero_long)))));
    *y = *y + dif / 1.85;
    *x = *x / pixelfact;
    *y = *y / pixelfact;
    *x += mapSizeX2;
    *y = mapSizeY2 - *y;
    if ((*x < 0) || (*x >= mapSizeX) || (*y < 0) || (*y >= mapSizeY))
        return (FALSE);
    return (TRUE);
}

bool MapFritz::calcltlg(double *lt, double *lg, double x, double y)
{
    double dif, tempLt;         // FIXME, optimize equations/iteration
    int i = 0;
    zero_long = center_longitude;
    zero_lat = center_latitude;
    pixelfact = scale / PIXELFACT;
    x = (mapSizeX2 - x) * pixelfact;
    y = (y - mapSizeY2) * pixelfact;
    *lt = center_latitude;
    do
    {
        tempLt = *lt;
        *lt = zero_lat - y / (Ra[(int) (100 + tempLt)] * deg2rad);
        i++;
    }
    while ((fabs(*lt - tempLt) >= 1.0) && (i < 4));
    *lt = zero_lat - y / (Ra[(int) (100 + tempLt)] * deg2rad);
    *lg =
        zero_long -
        x / ((Ra[(int) (100 + *lt)] * deg2rad) * cos(deg2rad * *lt));
    dif = *lt * (1 - (cos((deg2rad * fabs(*lg - zero_long)))));
    *lt = *lt - dif / 1.5;
    *lg =
        zero_long -
        x / ((Ra[(int) (100 + *lt)] * M_PI / 180.0) * cos(deg2rad * *lt));
    *lt *= deg2rad;
    *lg *= deg2rad;
    return TRUE;
}

QString MapFritz::getInfo()
{
    QString info;
    info =
        tr("Fritz Projection, SCALE 1:%1,\nCenter Lat: %2, Center Lon: %3").
        arg(scale).arg(center_latitude).arg(center_longitude);
    return info;
}

QString MapFritz::getParameterStr()
{
    QString param;
    /*   param = tr("%1 %2 %3 %4 %5 %6 %7")
       .arg(projection).arg(name).arg(scale).arg(mapSizeX).arg(mapSizeY)
       .arg(center_latitude).arg(center_longitude); */
    QTextOStream os(&param);
    os.precision(10);
    os << projection << " " << name << " " << scale << " " <<
        mapSizeX << " " << mapSizeY << " " << center_latitude << " " <<
        center_longitude;

    return param;
}

double MapFritz::calcR(double lat)
{
    double a = 6378.137, r, sc, x, y, z;
    double e2 = 0.081082 * 0.081082;
    /*
     * the radius of curvature of an ellipsoidal Earth in the plane of the
     * meridian is given by
     *
     * R' = a * (1 - e^2) / (1 - e^2 * (sin(lat))^2)^(3/2)
     *
     * where a is the equatorial radius, b is the polar radius, and e is
     * the eccentricity of the ellipsoid = sqrt(1 - b^2/a^2)
     *
     * a = 6378 km (3963 mi) Equatorial radius (surface to center distance)
     * b = 6356.752 km (3950 mi) Polar radius (surface to center distance) e
     * = 0.081082 Eccentricity
     */

    lat = lat * deg2rad;
    sc = sin(lat);
    x = a * (1.0 - e2);
    z = 1.0 - e2 * sc * sc;
    y = pow(z, 1.5);
    r = x / y;
    r = r * 1000.0;
    return r;
}

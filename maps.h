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

#ifndef MAPS_H
#define MAPS_H

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
#include <qsortedlist.h>
#include <math.h>
#include <stdlib.h>


class MapBase : public QObject
{
    Q_OBJECT

public:
    MapBase(QString *mapInfo);
    MapBase();
    ~MapBase();
    QTextIStream *mapIStream;
    QString projection;
    QString 	name;
    double	scale;
    int mapSizeX, mapSizeY;
    int operator==(MapBase&);
    int operator<(MapBase&);
    int operator>(MapBase&);

    // all positions in RAD !!!
    virtual bool calcxy(double *x, double *y, double lg, double lt) =  0;
    virtual bool calcltlg(double *lt, double *lg, double x, double y) =  0;
    virtual QString getParameterStr() = 0;
    virtual QString getInfo() = 0;
};

class MapLin : public MapBase
{
    Q_OBJECT

public:
    MapLin(QString *mapInfo);
    MapLin();
    ~MapLin();

    bool calcxy(double *x, double *y, double lg, double lt);
    bool calcltlg(double *lt, double *lg, double x, double y);
    QString getInfo();
    QString getParameterStr();

    double	longitude1;
    double	latitude1;
    int	x1;
    int	y1;
    double	longitude2;
    double	latitude2;
    int	x2;
    int	y2;
};


class MapCEA : public MapBase
{
    Q_OBJECT

public:
    MapCEA(QString *mapInfo);
    MapCEA();
    ~MapCEA();
    bool calcxy(double *x, double *y, double lg, double lt);
    bool calcltlg(double *lt, double *lg, double x, double y);
    QString getInfo();
    QString getParameterStr();

    double	longitude1;
    double	latitude1;
    int	x1;
    int	y1;
    double	longitude2;
    double	latitude2;
    int	x2;
    int	y2;

private:
    double xlong1, ylat1, xlong2, ylat2;
};

class MapUTM : public MapBase
{
    Q_OBJECT

public:
    MapUTM(QString *mapInfo, bool);
    MapUTM(bool);
    ~MapUTM();
    bool calcxy(double *x, double *y, double lg, double lt);
    bool calcltlg(double *lt, double *lg, double x, double y);
    QString getInfo();
    QString getParameterStr();

    void UTMtoLL(double UTMNorthing, double UTMEasting,
                 QString UTMZone, double& Lat, double& Long, double& stdLong );

    QString utmZone;
    double utmNorthing1, utmEasting1, utmNorthing2, utmEasting2;
    double	longitude1;
    double	latitude1;
    int	x1;
    int	y1;
    double	longitude2;
    double	latitude2;
    int	x2;
    int	y2;
    double stdLong;

private:
    double xlong1, ylat1, xlong2, ylat2;
    bool universal;
};


class MapMercator : public MapBase
{
    Q_OBJECT

public:
    MapMercator(QString *mapInfo);
    MapMercator();
    ~MapMercator();
    bool calcxy(double *x, double *y, double lg, double lt);
    bool calcltlg(double *lt, double *lg, double x, double y);
    QString getInfo();
    QString getParameterStr();
    double	longitude1;
    double	latitude1;
    int	x1;
    int	y1;
    double	longitude2;
    double	latitude2;
    int	x2;
    int	y2;

private:
    double xlong1, ylat1, xlong2, ylat2;
};

class MapLambert : public MapBase
{
    Q_OBJECT

public:
    MapLambert(QString *mapInfo);
    MapLambert();
    ~MapLambert();
    bool calcxy(double *x, double *y, double lg, double lt);
    bool calcltlg(double *lt, double *lg, double x, double y);
    QString getInfo();
    QString getParameterStr();
    double	longitude1;
    double	latitude1;
    int	x1;
    int	y1;
    double	longitude2;
    double	latitude2;
    int	x2;
    int	y2;
    double std1Lat;
    double std2Lat;
    double refLong;

private:
    double F, p0;
    double xlong1, ylat1, xlong2, ylat2;
    double n;
    double n_sign;
};


#define PIXELFACT 2817.947378

class MapFritz : public MapBase
{
    Q_OBJECT

public:
    MapFritz(QString *mapInfo);
    MapFritz();
    ~MapFritz();

    bool calcxy(double *x, double *y, double lg, double lt);
    bool calcltlg(double *lt, double *lg, double x, double y);
    QString getInfo();
    QString getParameterStr();

    int mapSizeX2;
    int mapSizeY2;
    double	center_longitude;
    double	center_latitude;

private:
    static double Ra[201];
    double calcR (double lat);
    double zero_long;
    double zero_lat;
    double pixelfact;
};


#endif

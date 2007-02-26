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

#ifndef MATHEX_H
#define MATHEX_H

/* Extented mathematical functions */

/* file and classe added by ng <ng@ngsoft-fr.com> */

#include <math.h>

class QString;

class MathEx
{
public:
    
    static const double PI;
    
    inline static double computeDistance(int x1, int y1, int x2, int y2)
     { return sqrt( square(x1 - x2) + square(y1 - y2) ); }
     
    /* lt/lg to km */
    inline static double computeDistanceLtLgRad(double lt1, double lg1, double lt2, double lg2);
    inline static double computeDistanceLtLgDeg(double lt1, double lg1, double lt2, double lg2);
    
    /* lt/lg to m */
    inline static double computeDistanceLtLgRadM(double lt1, double lg1, double lt2, double lg2);
    
    
    inline static double deg2rad(double v) { return v * PI / 180; }
    inline static double rad2deg(double v) { return v * 180 / PI; }
    
    inline static double meters2feet(double v) { return v * 3.2808399; }
    inline static double meters2FL(double v) { return v * 0.032808399; }
    
    inline static double kn2kmh(double v) { return v * 1.852; }
    inline static double kmh2kn(double v) { return v / 1.852; }
    inline static double kn2mph(double v) { return v * 1.1507794; }
    
    inline static double nmi2km(double v) { return kn2kmh(v); }
    inline static double km2nmi(double v) { return kmh2kn(v); }
    inline static double nmi2mi(double v) { return kn2mph(v); }
    inline static double km2mi(double v) { return nmi2mi(km2nmi(v)); }
    
    #if 0 /* bug with arm-linux-g++ 2.95.2 */
    template<class T>
    inline static T square(T v) { return v * v; }
    #else
    inline static int square(int v) { return v * v; }
    inline static double square(double v) { return v * v; }
    #endif
    
    
    /* calculate NMEA checksum */
    static void NMEAChecksum(QString& ns);
    
};



double MathEx::computeDistanceLtLgRad(double lt1, double lg1, double lt2, double lg2)
{
    /*return
        acos(cos(lt1) * cos(lg1) * cos(lt2) * cos(lg2) +
                cos(lt1) * sin(lg1) * cos(lt2) * sin(lg2) +
                sin(lt1) * sin(lt2)) * 6378;*/
    //simplifié par ng
    return acos( cos(lt1) * cos(lt2) * cos(lg1 - lg2) + sin(lt1) * sin(lt2) ) * 6378;
     
}
double MathEx::computeDistanceLtLgRadM(double lt1, double lg1, double lt2, double lg2)
{
    return computeDistanceLtLgRad(lt1, lg1, lt2, lg2) * 1000;
}
double MathEx::computeDistanceLtLgDeg(double lt1, double lg1, double lt2, double lg2)
{
    return computeDistanceLtLgRad(deg2rad(lt1), deg2rad(lg1), deg2rad(lt2), deg2rad(lg2));
}


#endif


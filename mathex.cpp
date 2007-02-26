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

#include "mathex.h"

#include <values.h>
#include <qstring.h>


const double MathEx::PI = M_PI;

void MathEx::NMEAChecksum(QString& ns)
{
    unsigned int len = ns.length();
    
    if(len < 3)
        return;
        
    unsigned int checksum =(QChar)ns[1];
    QString nend;

    for(unsigned int i = 2; i < len; i++)
        checksum ^= (QChar)ns[i];
        
    nend.sprintf("*%02x\r\n", checksum);
    ns.append(nend);
    
}


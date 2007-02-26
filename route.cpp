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


#include "route.h"


Route::Route(GpsData *, QWidget *parent, const char *name, WFlags fl):
        QVBox (parent, name, fl)
{
    routeSourceBG = new QHButtonGroup("Route data source",this);
    gpsRouteRB = new QRadioButton("GPS",routeSourceBG);
    zaurusRouteRB = new QRadioButton("THIS",routeSourceBG);
    routeSourceBG->setButton(0);
    gpsRouteRB->setDisabled(TRUE);
    zaurusRouteRB->setDisabled(TRUE);
    info = new QLabel(this);
    info->setText("\nToDo:\n"
                  "Routing, waypoints... inside the PDA\n\n"
                  "Routing with the information read\n"
                  "from the GPS is supported.\n");
}

Route::~Route()
{
}


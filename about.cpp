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

#include "about.h"
#include <qlabel.h>
#include <qvbox.h>
#include "qpegps.h"

About::About(QWidget * parent, const char *name, WFlags fl)
 : QScrollView(parent, name, fl)
{

    /** GUI **/
    setHScrollBarMode(Auto);
    setVScrollBarMode(Auto);

    QVBox *mainBox = new QVBox(this);
    addChild(mainBox);

    setResizePolicy(AutoOneFit);
    
    new QLabel( tr(
                    "<center><b>qpeGPS</b> is released under the <b>GNU General Public License version 2</b></center>"
                    "<br><br>"
                    "<b>qpeGPS NV Edition %1</b> (c) 2006 by Nicolas GUILLAUME &lt;<font color=blue><u>ng@ngsoft-fr.com</u></font>&gt; and the qpeGPS team"
                    "<br><br>"
                    "based on <b>qpeGPS 0.9.2.3.3</b> (c) 2002 by Ralf HASELMEIER &lt;<font color=blue><u>Ralf.Haselmeier@gmx.de</u></font>&gt; and the qpeGPS team"
                    "<br><br>"
                    "Developpers (alphabetical order):<br>"
                    "&nbsp;&nbsp;<b>Petr DOUBEK</b> &lt;<font color=blue><u>pdou@users.sourceforge.net</u></font>&gt;"
                    "<br>&nbsp;&nbsp;&nbsp;&nbsp;developed the Tracklog stuff and the CF GPSCard support<br><br>"
                    "&nbsp;&nbsp;<b>Bob GREEN</b> &lt;<font color=blue><u>bob_green@users.sourceforge.net</u></font>&gt;"
                    "<br>&nbsp;&nbsp;&nbsp;&nbsp;added the Download of maps and designed the main tab with its icons<br><br>"
                    "&nbsp;&nbsp;<b>Nicolas GUILLAUME</b> &lt;<font color=blue><u>ng@ngsoft-fr.com</u></font>&gt;"
                    "<br>&nbsp;&nbsp;&nbsp;&nbsp;added the Route tab, all the route stuff and some code optimization<br><br>"
                    "&nbsp;&nbsp;<b>Ralf HASELMEIER</b> &lt;<font color=blue><u>Ralf.Haselmeier@gmx.de</u></font>&gt;"
                    "<br>&nbsp;&nbsp;&nbsp;&nbsp;the main author and project founder<br><br>"
                    "&nbsp;&nbsp;<b>Alexander KARKHOV</b> &lt;<font color=blue><u>voblin@users.sourceforge.net</u></font>&gt;"
                    "<br>&nbsp;&nbsp;&nbsp;&nbsp;developed map mode and numerous additions<br><br>"
                    "&nbsp;&nbsp;<b>Carsten ROEDEL</b> &lt;<font color=blue><u>croedel@users.sourceforge.net</u></font>&gt;"
                    "<br>&nbsp;&nbsp;&nbsp;&nbsp;contributed the GPS Status page<br><br>"
                    "&nbsp;&nbsp;<b>Andreas WENZEL</b> &lt;<font color=blue><u>knn30@users.sourceforge.net</u></font>&gt;"
                    "<br>&nbsp;&nbsp;&nbsp;&nbsp;general debugging<br><br>"
                    ).arg(Qpegps::version())
                , mainBox);
                            
}


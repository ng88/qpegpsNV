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

#ifndef COLOR_DLG_H
#define COLOR_DLG_H

#include <qdialog.h>
#include <qvaluelist.h>
#include <qcolor.h>

class QListBox;
class QVBox;
class QHBox;
class QLabel;
class QGridLayout;

/* rewrited & optimized by ng */

class ColorDlg:public QDialog
{
  Q_OBJECT
  
  public:
    ColorDlg(QValueList<QColor> *, const QStringList&, QWidget *, const char *, bool, WFlags);
    ~ColorDlg();

  protected:
  
    static const uint COLORCOUNT = 19;
    static const QColor colors[COLORCOUNT];
  
    QValueList<QColor> *selectedColors;
    
    QListBox * itemCB;
    QVBox    * vBox;
    QHBox    * hBox;
    QLabel   * colorIndicatorL;
    QGridLayout * grid;

  private slots:
    
    void setColor(int);
    void setInd(int);
};

#endif

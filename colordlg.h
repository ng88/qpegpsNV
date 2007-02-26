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

#ifndef COLOR_DLG_H
#define COLOR_DLG_H

#include <qvbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtextstream.h>
#include <qarray.h>
#include <qwidget.h>
#include <qdialog.h>
#include <qsortedlist.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qpalette.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qlistbox.h>

class ColorDlg : public QDialog
{
    Q_OBJECT
public:
    ColorDlg(QValueList<QColor>*, QStringList *, QWidget *, const char *, bool, WFlags);
    ~ColorDlg();

protected:
    QList <QColor> qColorPtrList;
    QValueList<QColor> *selectedColors;
    QListBox *itemCB;
    QVBox *vBox;
    QHBox *hBox;
    QLabel *colorIndicatorL;
    QButtonGroup *colorBG;
    QGridLayout *grid;
    QPushButton *c[19];

private slots:
    void setColor(int);
    void setInd(int);
};

#endif

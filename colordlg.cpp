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

#include "colordlg.h"

#include <qvbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qvaluelist.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>

/* rewrited & optimized by ng */

const QColor ColorDlg::colors[ColorDlg::COLORCOUNT] = 
    {
        Qt::black,
        Qt::white,
        Qt::darkGray,
        Qt::gray,
        Qt::lightGray,
        Qt::red,
        Qt::green,
        Qt::blue,
        Qt::cyan,
        Qt::magenta,
        Qt::yellow,
        Qt::darkRed,
        Qt::darkGreen,
        Qt::darkBlue,
        Qt::darkCyan,
        Qt::darkMagenta,
        Qt::darkYellow,
        Qt::color0,
        Qt::color1,
    };

ColorDlg::ColorDlg(QValueList<QColor> * sc, const QStringList& isl,
                   QWidget * parent, const char *name, bool modal,
                   WFlags f)
 : QDialog(parent, name, modal, f), selectedColors(sc)
{

    vBox = new QVBox(this);
    QButtonGroup * colorBG = new QButtonGroup(vBox);
    colorBG->setFrameShape(QFrame::NoFrame);
    
    QGridLayout * grid = new QGridLayout(colorBG, 4, 5, 2);
    
    QPushButton * btn;
    
    for (uint i = 0; i < COLORCOUNT; ++i)
    {
        btn = new QPushButton("", colorBG);
        btn->setBackgroundColor(colors[i]);
        btn->setFlat(true);
        grid->addWidget(btn, i / 5, i % 5);
    }

    hBox = new QHBox(vBox);
    itemCB = new QListBox(hBox);
    itemCB->insertStringList(isl);
    colorIndicatorL = new QLabel(tr(" selected color "), hBox);
    
    resize(parent->geometry().size());
    vBox->resize(geometry().size());

    connect(colorBG, SIGNAL(clicked(int)), SLOT(setColor(int)));
    connect(itemCB, SIGNAL(highlighted(int)), SLOT(setInd(int)));

    itemCB->setCurrentItem(0);
}

ColorDlg::~ColorDlg()
{
};

void ColorDlg::setColor(int colorIdx)
{
   colorIndicatorL->setBackgroundColor(colors[colorIdx]);
   (*selectedColors)[itemCB->currentItem()] = colors[colorIdx];
}

void ColorDlg::setInd(int itemIdx)
{
  colorIndicatorL->setBackgroundColor((*selectedColors)[itemIdx]);
}

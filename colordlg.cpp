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
#include "colordlg.h"

ColorDlg::ColorDlg(QValueList<QColor>*sc, QStringList *isl, QWidget *parent,
                        const char *name, bool modal,WFlags f) : QDialog(parent,name,modal,f)
{
    uint i;

#define total 19
    qColorPtrList.append(new QColor(Qt::black));
    qColorPtrList.append(new QColor(Qt::white));
    qColorPtrList.append(new QColor(Qt::darkGray));
    qColorPtrList.append(new QColor(Qt::gray));
    qColorPtrList.append(new QColor(Qt::lightGray));
    qColorPtrList.append(new QColor(Qt::red));
    qColorPtrList.append(new QColor(Qt::green));
    qColorPtrList.append(new QColor(Qt::blue));
    qColorPtrList.append(new QColor(Qt::cyan));
    qColorPtrList.append(new QColor(Qt::magenta));
    qColorPtrList.append(new QColor(Qt::yellow));
    qColorPtrList.append(new QColor(Qt::darkRed));
    qColorPtrList.append(new QColor(Qt::darkGreen));
    qColorPtrList.append(new QColor(Qt::darkBlue));
    qColorPtrList.append(new QColor(Qt::darkCyan));
    qColorPtrList.append(new QColor(Qt::darkMagenta));
    qColorPtrList.append(new QColor(Qt::darkYellow));
    qColorPtrList.append(new QColor(Qt::color0));
    qColorPtrList.append(new QColor(Qt::color1));

    selectedColors = sc;

    vBox = new QVBox(this);
    colorBG = new QButtonGroup(vBox);
    colorBG->setFrameShape(QFrame::NoFrame);
    grid = new QGridLayout(colorBG,4,5,2);    
    for(i=0;i<total;i++)
    {
	c[i]= new QPushButton("",colorBG);
        c[i]->setBackgroundColor(*qColorPtrList.at(i));
	c[i]->setFlat(TRUE);
        grid->addWidget(c[i],i/5,i%5);
    }    

    hBox = new QHBox(vBox);
    itemCB = new QListBox(hBox);
    itemCB->insertStringList(*isl);
    colorIndicatorL = new QLabel(tr(" selected color "),hBox);

    //colorIndicatorL->setBackgroundColor(*qColorPtrList.at(0));    

    resize(parent->geometry().size());
    vBox->resize(geometry().size());

    connect( colorBG , SIGNAL(clicked(int)),
             SLOT(setColor(int)) );
    connect( itemCB , SIGNAL(highlighted(int)),
             SLOT(setInd(int)) );

    itemCB->setCurrentItem(0);
}
ColorDlg::~ColorDlg(){};

void ColorDlg::setColor(int colorIdx)
{
    colorIndicatorL->setBackgroundColor(*qColorPtrList.at(colorIdx));
    (*selectedColors)[itemCB->currentItem()] = *qColorPtrList.at(colorIdx);   
}

void ColorDlg::setInd(int itemIdx)
{
    colorIndicatorL->setBackgroundColor((*selectedColors)[itemIdx]);
}


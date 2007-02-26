
/*
 * tts.cpp
 *
 * ---------------------
 *
 * copyright   : (c) 2003 by Ralf Haselmeier
 * email       : rhaselme@users.sourceforge.net
 */
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "tts.h"



extern "C" cst_voice *register_cmu_us_kal();
extern "C" cst_voice *unregister_cmu_us_kal(cst_voice *vox);

Tts::Tts(QWidget * parent):QWidget(parent)
{

    textChannel = new QCopChannel("QPE/Tts", this);
    connect(textChannel, SIGNAL(received(const QCString &, const QByteArray &)),
	    this, SLOT(getMessage(const QCString &, const QByteArray &)) );

    enabled = FALSE;
    pmoff = Resource::loadPixmap("tts/ttsoff");
    pmon = Resource::loadPixmap("tts/ttson");

    setFixedSize(pmoff.size());
}

Tts::~Tts()
{

}


void Tts::mousePressEvent(QMouseEvent *)
{
    QPopupMenu *menu = new QPopupMenu(this);

    if (enabled)
    {
	menu->insertItem(tr("Disable"), 0);
	menu->insertItem(tr("Paste"), 1);
    }
    else
    {
	menu->insertItem(tr("Enable"), 0);
    }


    QPoint p = mapToGlobal(QPoint(0, 0));
    QSize s = menu->sizeHint();
    int opt = menu->exec(QPoint(p.x() + (width() / 2) - (s.width() / 2),
				p.y() - s.height()), 0);

    switch(opt)
    {
	case 0:
            if(enabled)
              disable();
	    else
	      enable();
	    break;

        case 1:
            // paste from clipboard and say text
            QClipboard *cb = QApplication::clipboard();
            QString a;
            a = cb->text();
            flite_text_to_speech(a.latin1(),v,"play");
	    break;
    }

    delete menu;
}


void Tts::enable()
{
    extra_feats = new_features();
    flite_init();
    v = register_cmu_us_kal();
    feat_copy_into(extra_feats,v->features);
    enabled = TRUE;
    QWidget::repaint();
}

void Tts::disable()
{
    delete_features(extra_feats);
    unregister_cmu_us_kal(v);
    enabled = FALSE;
    QWidget::repaint();
}

void Tts::getMessage(const QCString & msg, const QByteArray &data)
{
    if (enabled)
	if(msg == "sayText(QString)")
	{
	    QDataStream stream( data, IO_ReadOnly );
	    QString a;
	    stream >> a;

	    // flite say text a
	    flite_text_to_speech(a.latin1(),v,"play");
	}
}


void Tts::paintEvent(QPaintEvent *)
{

    QPainter p(this);
    if(enabled)
	p.drawPixmap(0, 0, pmon);
    else
	p.drawPixmap(0, 0, pmoff);
}

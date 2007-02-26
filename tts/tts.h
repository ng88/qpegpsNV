/*
 * tts.h
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

#ifndef TTS_H
#define TTS_H

#include <qwidget.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qstring.h>
#include <qclipboard.h>
#include <qcopchannel_qws.h>
#include <qpe/resource.h>

#include <qpainter.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qpe/qpeapplication.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#ifndef __cplusplus
#define __cplusplus
#endif

#include "flite.h"
//#include "flite_version.h"
//#include "voxdefs.h"   

class Tts : public QWidget {
    Q_OBJECT
public:
    Tts( QWidget *parent = 0 );
    ~Tts();
    QCopChannel *textChannel;
private slots:
    void getMessage( const QCString &msg, const QByteArray & );
    void enable();
    void disable();

protected:
    void paintEvent( QPaintEvent* );
    void mousePressEvent( QMouseEvent * );

private:
    bool enabled;
    cst_voice *v;
    cst_features *extra_feats;

    QPixmap pmon,pmoff;
 };

#endif


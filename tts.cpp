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

#include "tts.h"
#include <qpe/qcopenvelope_qws.h>
#include <qcopchannel_qws.h>
#include <qpe/process.h>

#include <cstdlib>


GenericQCopSpeaker::GenericQCopSpeaker()
{
}

void GenericQCopSpeaker::sayText(const QString & txt, const QString &)
{
    QCopEnvelope e("QPE/Tts", "sayText(QString)");
    e << txt;
}

const QString& GenericQCopSpeaker::name() const
{
    static QString r(QObject::tr("qcop QPE/Tts"));
    return r;
}

bool GenericQCopSpeaker::isAvailable() const
{
    return QCopChannel::isRegistered("QPE/Tts");
}



//////////////////////////////////////////////////////


FliteSpeaker::FliteSpeaker()
{
}

void FliteSpeaker::sayText(const QString & txt, const QString &)
{
    QString out;
    
    /*Process p;
    
    p.addArgument("flite");
    
    if(!p.exec(txt, out))
        qDebug("flite execution error, output was: " + out);*/
    system("echo " + txt + " | flite");
}

const QString& FliteSpeaker::name() const
{
    static QString r(QObject::tr("festival lite"));
    return r;
}

bool FliteSpeaker::isAvailable() const
{
    return true;
}


//////////////////////////////////////////////////////


LliaphonMbrolaSpeaker::LliaphonMbrolaSpeaker()
{
}

void LliaphonMbrolaSpeaker::sayText(const QString &, const QString &)
{
}

const QString& LliaphonMbrolaSpeaker::name() const
{
    static QString r(QObject::tr("lliaphon/mbrola"));
    return r;
}

bool LliaphonMbrolaSpeaker::isAvailable() const
{
    return true;
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////


Speakers::Speakers()
{
    _spks[0] = new GenericQCopSpeaker();
    _spks[1] = new FliteSpeaker();
    _spks[2] = new LliaphonMbrolaSpeaker();
    
    _current = 0;
    
    for(int i = 0; i < SPEAKERS_COUNT; ++i)
        _names.append(_spks[i]->name());
}

Speakers::~Speakers()
{
    for(int i = 0; i < SPEAKERS_COUNT; ++i)
        delete _spks[i];
}




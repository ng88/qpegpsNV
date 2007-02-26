#ifndef TTS_H
#define TTS_H

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


/* File and classed added by ng <ng@ngsoft-fr.com> */

#include <qstringlist.h>

class Speakers;

class Speaker
{
  public:

    virtual void sayText(const QString & txt, const QString & lang = QString("en")) = 0;
    
    virtual const QString& name() const;
    virtual bool isAvailable() const;
    
};

/* use QPE/Tts qcop channel */
class GenericQCopSpeaker : public Speaker
{
    GenericQCopSpeaker();
    
  public:

    void sayText(const QString & txt, const QString & lang = QString("en"));
    const QString& name() const;
    bool isAvailable() const;
    
    friend  Speakers;
    
};


/* festival lite support (english only) */
class FliteSpeaker : public Speaker
{
    FliteSpeaker();
    
  public:

    void sayText(const QString & txt, const QString & lang = QString("en"));
    const QString& name() const;
    bool isAvailable() const;
    
    friend  Speakers;

};

/* lliaphon & Mbrola support */
class LliaphonMbrolaSpeaker : public Speaker
{
    LliaphonMbrolaSpeaker();

  public:
  
    void sayText(const QString & txt, const QString & lang = QString("en"));
    const QString& name() const;
    bool isAvailable() const;
    
    friend  Speakers;
};


class Speakers
{
 public:
 
     enum { SPEAKERS_COUNT = 3 };
 
 private:
 
     QStringList _names;
     Speaker*    _spks[SPEAKERS_COUNT];
     Speaker*    _current;
 
 public:
 
     Speakers();
     ~Speakers();
     
     inline QStringList& speakerNames() { return _names; }
     inline void setCurrent(int i) { _current = (i >=0 && i < SPEAKERS_COUNT) ? _spks[i] : 0; }
     inline Speaker* current() { return _current; }

};


#endif

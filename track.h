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

#ifndef TRACK_H
#define TRACK_H
#include <qsocket.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qmultilineedit.h>
#include <qtextview.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qarray.h>
#include <qwidget.h>
#include <qcheckbox.h>
#include <qlist.h>
#include <qcombobox.h>

#include <math.h>

#include "qpegps.h"
#include "maps.h"
#include "gpsdata.h"
#include "dirdialog.h"
#include "settings.h"

#define MINTRACKDIST	0.0005  // minimal distance between tracks [app. degrees]

// track formats
enum
{
    NMEA,
    PCX5,
    GPSDRIVE,
};
#define DEF_FORMAT	GPSDRIVE


/* one track point */

class TrackPoint:public QObject
{
  Q_OBJECT public:

    double time, longitude, latitude, altitude;

    TrackPoint(const QString& );       // create trackpoint from nmea gga sentence
    TrackPoint(QString tim, double lat, double lon, double alt);
   ~TrackPoint() { }
    QString toNMEA();
    QString toPCX5();
    QString toDrive();
    double dist(double lat, double lon);
    double timediff(QString tim);
};

/* writing and reading tracklogs */


class Track:public QScrollView
{
  Q_OBJECT public:

    // boxes, labels
    QVBox * mainBox;
    QHBox *tBox, *wBox, *rBox, *dBox, *cBox, *lBox;
    QLabel *tLabel, *wLabel, *rLabel, *dLabel, *cLabel, *lLabel,
        *instructions;

    QDir *logdir;               // mapdir, we have .log files there
    QComboBox *wLog, *rLog;     // write/read tracklog filenames
    QCheckBox *wCB, *rCB;       // write/read checkboxes
    QLineEdit *tLE, *dLE, *cLE; // trackdir, min. time diff [s], cf period [s]
    QPushButton *tButton;       // trackdir browse button
    MenuButton *lMenuB;         // track line thickness

    bool wDo, rDo, cDo;         // write track? display track? display current?

    Qpegps *application;
    GpsData *gpsData;           // common qpegps data

      QList < TrackPoint > wTrack, rTrack;

      Track(Qpegps * appl, QWidget * parent = 0, const char *name =
            0, WFlags fl = 0);
     ~Track();

    void updateFileList();      // update comboboxes with filenames
    void Write(QString filename, int format = DEF_FORMAT);
    // write tracklog in memory to file
    void Read(QString filename);        // read tracklog from file to memory
    void update();              // new data from gps, update track
    void drawTrack(QPainter * painter, MapBase * actmap, int x1, int y1,
                   int mx, int my);
    // draw tracklog
    void setRate(unsigned message, unsigned rate);      // set rate of message
    /* Added by A. Karhov */
    void refresh();
    void setStartup();
    QHBox *hBox;
    QComboBox *mapLatLonCB;
    QVGroupBox *PlaceGB;
    QHGroupBox *StGB;
    QPushButton *QDelButt;

    QLabel *StartModeL;
    QCheckBox *sCB;
    QComboBox *StartCB;


    private slots: void setWriteCB(bool state); // (un)checked write checkbox
    void setReadCB(bool state); // (un)checked read checkbox
    void setReadName(const QString &);  // changed read log name
    void tLEChanged();          // trackdir line edit changed
    void setTrackPath();        // searching for trackdir
    void dLEChanged();          // trackdir line edit changed
    void lMenuBChanged(int idx);        // track line thickness changed
    void cLEChanged();          // CF period line edit changed

    void placeSelected(int ind);        /* Added by A. Karhov */
    void delPlace();
    void setStartModeCB(bool);
    void startSelected(int);

  protected:
      QMultiLineEdit * commMLE; /* Added by A. Karhov */
    Places *pl;

};



#endif

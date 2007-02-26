/*
  qpegps is a program for displaying a map centered at the current longitude/
  latitude as read from a gps receiver.

  Copyright (C) 2002 Carsten Roedel <croedel@users.sourceforge.net>
 
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
 

  Todo (as of 11/26/2002):

  - Tooltips to better explain what each filed supposed to mean

*/


#include "gpsstatus.h"
#include "settings.h"
#include "client.h"

#include <qpainter.h>
#include <qtooltip.h>

#define X_ADJ(a) ((int)(float(a)*xScreenCorr))
#define Y_ADJ(a) ((int)(float(a)*yScreenCorr))
#define S_ADJ(a) ((int)(float(a)*xyScreenCorr))


float xScreenCorr=1, yScreenCorr=1, xyScreenCorr=1; // global, bad  but easy

//GpsStatus::GpsStatus(GpsData *gData, QWidget *parent, const char *name, WFlags fl):
GpsStatus::GpsStatus(Qpegps *appl, QWidget *parent, const char *name, WFlags fl):
        QVBox (parent, name, fl)
{
    application = appl;
    //gpsData = gData;
    gpsData = application->gpsData;
    //settings = gData->appl()->settings;
    settings = application->settings;
    //gpsd = gData->appl()->gpsd;
    gpsd = application->gpsd;
    d_fullUpdate = false;

    //QToolTip * tooltips = new QToolTip(this);

    group1 = new QVGroupBox("gpsd Settings",this);

    QHBox *horbox1 = new QHBox(group1);
    gpsdOpt = new QLabel("Args: ",horbox1);
    gpsdArguments = new QLineEdit(horbox1);
    gpsdArgumentsB = new QPushButton(tr("default"),horbox1);

    gpsdArguments->setText(gpsData->gpsdArgStr);

    QHBox * horbox2 = new QHBox(group1);
    gpsdHost = new QLabel(tr("Host: "),horbox2);
    gpsdHostArg = new QLineEdit(horbox2);

    gpsdHostArg->setText(gpsData->host);

    gpsdPort = new QLabel(tr("  Port: "),horbox2);
    gpsdPortArg = new QLineEdit(horbox2);

    gpsdPortArg->setText(tr("%1").arg(gpsData->port));
    gpsdPortArg->setMaxLength(4);
    gpsdHostPortB = new QPushButton(tr("default"),horbox2);

    if (gpsData->host.compare(gpsdDefaultHost))
        gpsdArguments->setDisabled(TRUE);
    else
        gpsdArguments->setDisabled(FALSE);

    group1->setMaximumHeight(72);

    QHBox *hsep = new QHBox(this);

    group2 = new QVGroupBox(tr("Data Status"), hsep);

    QHBox *horbox3 = new QHBox(group2);

    new QLabel("gpsd:", horbox3);
    d_pGpsdStatus = new QLabel(tr("???"), horbox3);
    d_pGpsdStatus->setBackgroundColor(Qt::red);
    d_pGpsdStatus->setFrameStyle( QFrame::Panel | QFrame::Raised );
    d_pGpsdStatus->setUpdatesEnabled(TRUE);
    d_pGpsdStatus->setAlignment(Qt::AlignCenter);
    d_pGpsdStatus->setFixedWidth(30);
    //tooltips->add(d_pGpsdStatus, "Status of gps daemon");

    new QLabel(" GPS:", horbox3);

    d_pReceiverStatus = new QLabel(tr("???"), horbox3);
    d_pReceiverStatus->setBackgroundColor(Qt::red);
    d_pReceiverStatus->setFrameStyle( QFrame::Panel | QFrame::Raised );
    d_pReceiverStatus->setUpdatesEnabled(TRUE);
    d_pReceiverStatus->setAlignment(Qt::AlignCenter);
    d_pReceiverStatus->setFixedWidth(30);

    /*   99/99/9999 99:99:99  */
    d_pStatus = new QPushButton(tr("* No GMT Signal rcvd *"), group2);
    d_pStatus->setUpdatesEnabled(TRUE);
    d_pStatus->setFlat(TRUE);
    d_pStatus->setBackgroundColor(Qt::red);
    // d_pStatus->setFrameStyle( QFrame::Panel | QFrame::Raised );
    //d_pStatus->setAlignment(Qt::AlignCenter);
    requestTimeAdj = FALSE;

    group2->setMaximumHeight(106);

    d_pSatStat = new SatStat(gpsData, hsep);
    d_pSatStat->setMinimumSize(100, 110);
    //d_pSatStat->setMaximumHeight(130);
    //d_pSatStat->setFrameStyle( QFrame::Panel | QFrame::Raised );
    //d_pSatStat->setBackgroundColor(Qt::lightGray);

    QHBox * horbox4 = new QHBox(group2);
    new QLabel(tr("Lat: "), horbox4);
    //d_pLatitude  = new QLineEdit(horbox4);
    d_pLatitude  = new QLabel("", horbox4);
    d_pLatitude->setText(tr("???"));
    d_pLatitude->setAlignment(Qt::AlignRight);
    //d_pLatitude->setReadOnly(true);

    QHBox * horbox5 = new QHBox(group2);
    new QLabel(tr("Lon: "), horbox5);
    //d_pLongitude = new QLineEdit(horbox5);
    d_pLongitude = new QLabel("", horbox5);
    d_pLongitude->setText(tr("???"));
    d_pLongitude->setAlignment(Qt::AlignRight);
    //d_pLongitude->setReadOnly(true);


    d_pSatSNR = new SatSNR(gpsData, this);
    d_pSatSNR->setMinimumSize(220, 73);
    d_pSatSNR->setFrameStyle( QFrame::Panel | QFrame::Raised );
    d_pSatSNR->setBackgroundColor(Qt::lightGray);
    //d_pSatSNR->setBackgroundMode(QWidget::NoBackground);

    connect( gpsdArgumentsB, SIGNAL(pressed()),
             SLOT(setGpsdDefaultArg()) );
    connect( gpsdArguments, SIGNAL(returnPressed()),
             SLOT(gpsdArgLEChanged()) );

    connect( gpsdHostPortB, SIGNAL(pressed()),
             SLOT(setGpsdDefaultHostPort()) );
    connect( gpsdHostArg, SIGNAL(returnPressed()),
             SLOT(gpsdHostArgLEChanged()) );
    connect( gpsdPortArg, SIGNAL(returnPressed()),
             SLOT(gpsdPortArgLEChanged()) );

    //connect( this, SIGNAL(gpsdArgChanged()),
    //         gData->appl()->gpsd, SLOT(restartGpsd()) );
    connect( this, SIGNAL(gpsdArgChanged()),
             gpsd, SLOT(restartGpsd()) );

    connect( d_pStatus, SIGNAL(clicked()),
             this, SLOT(setSysTime()) );


}

GpsStatus::~GpsStatus()
{
}

void GpsStatus::setSysTime()
{
    requestTimeAdj = TRUE;
}

void GpsStatus::setGpsdDefaultArg()
{
    //gpsData->gpsdArgStr = gpsdDefaultArg;
    application->gpsData->gpsdArgStr = gpsdDefaultArg;
    //gpsdArguments->setText(gpsData->gpsdArgStr);
    gpsdArguments->setText(application->gpsData->gpsdArgStr);

    settings->writeConfig();
    emit gpsdArgChanged();
}
void GpsStatus::setGpsdDefaultHostPort()
{
    //gpsData->host = gpsdDefaultHost;
    application->gpsData->host = gpsdDefaultHost;
    //gpsdHostArg->setText(gpsData->host);
    gpsdHostArg->setText(application->gpsData->host);

    //gpsData->port = gpsdDefaultPort;
    application->gpsData->port = gpsdDefaultPort;
    //gpsdPortArg->setText(tr("%1").arg(gpsData->port));
    gpsdPortArg->setText(tr("%1").arg(application->gpsData->port));

    gpsdArguments->setDisabled(FALSE);

    settings->writeConfig();

    //gpsd->socket->connectToHost( gpsData->host, gpsData->port );
    gpsd->socket->connectToHost( application->gpsData->host, application->gpsData->port );

}

void GpsStatus::gpsdArgLEChanged()
{
    //gpsData->gpsdArgStr = gpsdArguments->text();
    application->gpsData->gpsdArgStr = gpsdArguments->text();
    settings->writeConfig();

    emit gpsdArgChanged();
}

void GpsStatus::gpsdHostArgLEChanged()
{
    //gpsData->host = gpsdHostArg->text();
    application->gpsData->host = gpsdHostArg->text();
    settings->writeConfig();

    //if (gpsData->host.compare(gpsdDefaultHost))
    if (application->gpsData->host.compare(gpsdDefaultHost))
        gpsdArguments->setDisabled(TRUE);
    else
        gpsdArguments->setDisabled(FALSE);

    emit gpsdArgChanged();
}

void GpsStatus::gpsdPortArgLEChanged()
{
    //gpsData->port = atoi(gpsdPortArg->text());
    application->gpsData->port = atoi(gpsdPortArg->text());
    settings->writeConfig();

    emit gpsdArgChanged();
}


void GpsStatus::paintEvent( QPaintEvent * ev)
{
    //get tab size and calc adjustment values
    xScreenCorr = width()/236.0;
    yScreenCorr = height()/256.0;
    xScreenCorr = (xScreenCorr<1.0)? 1.0:xScreenCorr;// 240X320 is smallest !
    yScreenCorr = (yScreenCorr<1.0)? 1.0:yScreenCorr;
    xyScreenCorr = (xScreenCorr<yScreenCorr)? xScreenCorr:yScreenCorr;
    //qDebug(tr("screen w=%1,h=%2, xc=%3, yc=%4, xyc=%5").arg(width()).arg(height()).arg(xScreenCorr).arg(yScreenCorr).arg(xyScreenCorr));

    // fon of "Data Status"

    QFont f=font();
    f.setPointSize(S_ADJ(10));
    group2->setFont(f);

    // set width/height again
    group1->setMaximumHeight(Y_ADJ(72));
    d_pGpsdStatus->setFixedWidth(X_ADJ(30));
    d_pReceiverStatus->setFixedWidth(X_ADJ(30));
    group2->setMaximumHeight(Y_ADJ(106));

    d_pSatStat->setMinimumSize(X_ADJ(100), Y_ADJ(110));
    d_pSatSNR->setMinimumSize(X_ADJ(220), Y_ADJ(73));

    // any repaint event will force a full update
    d_fullUpdate = true;

    QVBox::paintEvent(ev);
}


void GpsStatus::updateQuick()
{
    static QString lastFix = "";
    static QString lastReceiver = "";
    static int status = -1;
    static bool connected = false;
    static bool alive = false;

    //qDebug(tr("update called. connected=%1 alive=%2 status=%3").arg(gpsData->d_connected).arg(gpsData->d_aliveGPS).arg(gpsData->status));

    //QString ts = gpsData->ts.toString();
    QString ts = application->gpsData->ts.toString();

    bool needTextUpdate = false;
    bool needColorUpdate = false;
    bool needUpdate = false;
    bool needReceiverUpdate = false;
    bool needAliveUpdate = false;

    if (ts.compare(lastFix) != 0) {
        // new time stamp came in, reset counter
        lastFix = ts;
        needTextUpdate = true;
    }

    //if (connected != gpsData->d_connected) {
    if (connected != application->gpsData->d_connected) {
        //connected = gpsData->d_connected;
        connected = application->gpsData->d_connected;
        needUpdate = true;
    }
    //if (gpsData->d_Receiver.compare(lastReceiver) != 0) {
    if (application->gpsData->d_Receiver.compare(lastReceiver) != 0) {
        //lastReceiver = gpsData->d_Receiver;
        lastReceiver = application->gpsData->d_Receiver;
        needReceiverUpdate = true;
    }

    //if (status != gpsData->status) {
    if (status != application->gpsData->status) {
        //status = gpsData->status;
        status = application->gpsData->status;
        needColorUpdate = true;
    }

    if (alive!= gpsData->d_aliveGPS) {
        alive = gpsData->d_aliveGPS;
        needAliveUpdate = true;
    }

    if (needTextUpdate)
    {
        // new time stamp
        if(requestTimeAdj && gpsData->status )
        {
	    requestTimeAdj = FALSE;
	    QStringList slst = QStringList::split(QRegExp("[/: ]"),ts,TRUE);

            //set TZ env. variable to UTC time => for settimeofday and QDateTime
	    QString tzenv = getenv("TZ");
            setenv("TZ","UTC",1);

	    QDate tsDate(slst[2].toUInt(),slst[0].toUInt(),slst[1].toUInt());
	    QTime tsTime(slst[3].toUInt(),slst[4].toUInt(),slst[5].toUInt());
	    QDateTime tsDT(tsDate, tsTime);
	    // Set system clock
	    if ( tsDT.isValid() )
	    {
                // consider timezone !
		struct timeval myTv;
		int t = TimeConversion::toUTC( tsDT ); // isn't UTC, it's local time in seconds..
                tsDT.setTime_t(t);
		t = TimeConversion::toUTC( tsDT );
		myTv.tv_sec = t;
		myTv.tv_usec = 0;

 		if ( myTv.tv_sec != -1 )
		    ::settimeofday( &myTv, 0 );

                // the TZ env. variable must be set correctly
                // No! it must be set to the same value than before
                setenv("TZ",tzenv,1);

                // cause update of other apps (badly, the timeZone didn't change)
                TimeZoneSelector ts;
                QString tz = ts.currentZone();

                QCopEnvelope setTimeZone( "QPE/System", "timeChange(QString)" );
                setTimeZone << tz;
	    }
	    else
		qWarning(tr("y=%1,m=%2,d=%3,h=%4,min=%5,s=%6")
		       .arg(slst[2].toUInt()).arg(slst[0].toUInt()).arg(slst[1].toUInt())
		       .arg(slst[3].toUInt()).arg(slst[4].toUInt()).arg(slst[5].toUInt()));

        }
        d_pStatus->setText(ts);
    }
    if (needColorUpdate) {
        // new fix
        // notify user about change in status
        QApplication::beep();

        d_pStatus->setBackgroundColor( (gpsData->status ? Qt::green : Qt::red) );
        // Any FIX status change forces full update
        d_fullUpdate = true;
    }

    if (needReceiverUpdate)
        // new Receiver name
        d_pReceiverStatus->setText(gpsData->d_Receiver);

    if (needUpdate) {
        // connected ?
        d_pGpsdStatus->setBackgroundColor( (gpsData->d_connected ? Qt::green : Qt::red) );
        d_pGpsdStatus->setText( (gpsData->d_connected ? tr("OK") : tr("ERR")) );
        d_pGpsdStatus->repaint();
    }

    if (gpsData->status) {
        // new position
        //QColor * color = (gpsData->status ? gpsData->statusNoFixColor : gpsData->statusOkColor);
        //d_pGpsdStatus->setBackgroundColor( (gpsData->d_connected ? Qt::green : Qt::red) );
        //if (!d_pLongitude->hasMarkedText())
        d_pLongitude->setText(gpsData->currPos.longToString(gpsData->longitudeGps)); /* Added by A/ Karhov */
        //if (!d_pLatitude->hasMarkedText())
        d_pLatitude->setText(gpsData->currPos.latToString(gpsData->latitudeGps)); /* Added by A/ Karhov */
    }

    if (needAliveUpdate) {
        // GPS is alive?
        d_pReceiverStatus->setBackgroundColor( (gpsData->d_aliveGPS ? Qt::green : Qt::red) );
        // Any GPS status change forces full update
        d_fullUpdate = true;
    }

    if (needAliveUpdate || needReceiverUpdate)
        d_pReceiverStatus->repaint();

    if (needTextUpdate || needColorUpdate)
        d_pStatus->repaint();

}


void GpsStatus::update()
{
    static QString lastInfo = "";

    updateQuick();

    bool needUpdate = d_fullUpdate;
    for (int i = 0; i < 12; ++i) {

        SatInfo & satInfo = gpsData->d_pSatInfo[i];

        // check if satellite data has changed
        if (!(d_pSatSNR->d_pSatInfo[i] == satInfo) || d_fullUpdate) {
            d_pSatSNR->d_pSatInfo[i] = satInfo;

            // force update of every single sat info
            if (d_fullUpdate)
                d_pSatSNR->d_pSatInfo[i].d_updated = true;

            needUpdate = true;
        }
    }
    if (needUpdate) d_pSatSNR->repaint(false);


    // mangle all strings together
    QString tmp = gpsData->bearing.toString()  + gpsData->heading.toString() +
                  gpsData->altitude.toString() + gpsData->speed.toString() +
                  tr("%1").arg(gpsData->d_no_of_satellites);

    if (lastInfo.compare(tmp) != 0) {
        // new info arrived
        lastInfo = tmp;
        d_pSatStat->repaint();
    }

    // if it was requested, it is now updated
    d_fullUpdate = false;
}




SatSNR::SatSNR(GpsData * gpsdata, QWidget * parent, const char * name, WFlags f):
        QFrame(parent, name, f),
        gpsData(gpsdata)
{
}

SatSNR::~SatSNR()
{
}


void SatSNR::drawContents ( QPainter * painter )
{
    int xSPC = X_ADJ(4);
    int ySPC = Y_ADJ(4);
    int WIDTH = X_ADJ(15);
    int offset = X_ADJ(2);

    int x = xSPC + offset;
    int y = minimumHeight() - ySPC;
    int min = x;
    int max = min + 11*(WIDTH + xSPC) + WIDTH - 1;
    int threshold = Y_ADJ(7); // 7% mark

    painter->setPen( QPen( Qt::black, 0, QPen::SolidLine) );

    for (int i = 0; i < 12; ++i) {
        SatInfo & satInfo = d_pSatInfo[i];

        int strength = satInfo.d_snr; //scaled 50pixel == 100%

        int clip = x + WIDTH + xSPC - 1;
        if (clip > max) clip = max;

        if (satInfo.d_satName.isEmpty()) {
            // satellite disappeared, clear its area
            painter->eraseRect(x, y+Y_ADJ(10), WIDTH, Y_ADJ(-99));

            // draw the marker lines
            for (int p = threshold; p <= Y_ADJ(50); p+=Y_ADJ(10) )
                painter->drawLine ( x, y-Y_ADJ(12)-p, clip, y-Y_ADJ(12)-p );

        }
        else if (satInfo.d_updated) {
            // sat has name and info has changed

            // since we don't erase the background erase text area
            // Note: this will still flicker a little since it erases the solid line
            painter->eraseRect(x, y+Y_ADJ(10), WIDTH, Y_ADJ(-99));

            // repair the marker lines
            for (int p = threshold; p <= Y_ADJ(50); p+=Y_ADJ(10) )
                painter->drawLine ( x, y-Y_ADJ(12)-p, clip, y-Y_ADJ(12)-p );

            painter->drawText(x+X_ADJ(2),y, satInfo.d_satName);

            // when signal is lower than threshold, gps does not use it for reading
            const QColor &c = (gpsData->d_aliveGPS ? (strength < threshold ? Qt::red : Qt::green) : Qt::red);
            painter->setBrush( QBrush( c, QBrush::SolidPattern) );

            painter->drawRect(x, y-Y_ADJ(13), WIDTH, Y_ADJ(-strength));

            // unmark
            satInfo.d_updated = false;
        }

        x = x + WIDTH + xSPC;
    }

    // 0% mark
    painter->setPen( QPen( Qt::black, 2, QPen::SolidLine) );
    painter->drawLine ( min, y-Y_ADJ(12), max+1, y-Y_ADJ(12) );



}


void SatSNR::updateInfo()
{
}




SatStat::SatStat(GpsData * gpsdata, QWidget * parent, const char * name, WFlags f):
        QFrame(parent, name, f),
        gpsData(gpsdata)
{
    d_pSpeedSamples = new double [d_numSamples];
    d_pAltitudeSamples = new double [d_numSamples];
    d_pSatelliteSamples = new pair<int,int> [d_numSamples];

    pair<int,int> nulP; nulP.first = 0; nulP.second = 0;

    for (int index = 0; index < d_numSamples; ++index) {
        d_pSpeedSamples [index]    = 0.0;
        d_pAltitudeSamples [index] = 0.0;
        d_pSatelliteSamples [index]= nulP;
    }

    d_maxSpeed = double(INT_MIN);
    d_minSpeed = double(INT_MAX);

    d_maxAltitude = double(INT_MIN);
    d_minAltitude = double(INT_MAX);

    d_pTimer = new QTimer;
    d_pTimer->start(2000);

    connect( d_pTimer, SIGNAL(timeout()), SLOT(updateSamples()) );

}

SatStat::~SatStat()
{
    delete [] d_pSpeedSamples;
    delete [] d_pAltitudeSamples;
    delete [] d_pSatelliteSamples;
}


//********     do the repaint of the timeline with a fixed timer (2.0 sec)
//********     during that function call, erase the rectangle of the timeline and draw
void SatStat::updateSamples()
{
    shiftSamples();
    drawSamples();
}

void SatStat::drawSamples()
{
    QPainter painter(this);

    QPen drawP  = painter.pen();
    drawP.setColor(Qt::blue);

    QPen drawGP  = painter.pen();
    drawGP.setColor(Qt::green);
    QPen drawRP  = painter.pen();
    drawRP.setColor(Qt::red);

    QPen eraseP = painter.pen();
    eraseP.setColor(painter.backgroundColor());


    // Number of Satellites
    const int yU1 = 3+34+1;
    const int yL1 = yU1 + 12;

    //painter.eraseRect( 18, yU1, d_numSamples, 12);
    for (int index = 0; index < d_numSamples; ++index) {
        pair<int,int> & satcount = d_pSatelliteSamples [index];
        int numsatinview = satcount.first;
        int numsatinfix  = satcount.second;

        if (numsatinfix > numsatinview) {
            // more satellites for fix than in view --> ERROR
            painter.setPen(drawRP);
            painter.drawLine(X_ADJ(18+index), Y_ADJ(yL1), X_ADJ(18+index), Y_ADJ(yU1));
        }
        else {
            int h  = yL1 - numsatinview;
            int h1 = yL1 - numsatinfix;
            if (h1 < yL1) {
                painter.setPen(drawGP);
                painter.drawLine(X_ADJ(18+index), Y_ADJ(yL1), X_ADJ(18+index), Y_ADJ(h1));
            }
            if (h1 > h) {
                painter.setPen(drawP);
                if (h1 == yL1) h1 = yL1 + 1;
                painter.drawLine(X_ADJ(18+index), Y_ADJ(h1-1), X_ADJ(18+index), Y_ADJ(h));
            }
            if (h > yU1) {
                painter.setPen(eraseP);
                painter.drawLine(X_ADJ(18+index), Y_ADJ(h-1), X_ADJ(18+index), Y_ADJ(yU1));
            }
        }
    }

    // calculate scale factor for speed
    const int yU2 = 3+0+1;
    const int yL2 = yU2 + 12;
    double fSpeed = 0;
    if ((d_maxSpeed - d_minSpeed) > 0) fSpeed = d_pixSpeed / (d_maxSpeed - d_minSpeed);
    //painter.eraseRect( 18, yU2, d_numSamples, 12);
    // draw speed samples
    for (int index = 0; index < d_numSamples; ++index) {
        double value = (d_pSpeedSamples [index] - d_minSpeed) * fSpeed;
        int h = yL2 - (int)rint(value);
        painter.setPen(drawP);
        painter.drawLine(X_ADJ(18+index), Y_ADJ(yL2), X_ADJ(18+index), Y_ADJ(h));
        if (h > yU2) {
            painter.setPen(eraseP);
            painter.drawLine(X_ADJ(18+index), Y_ADJ(h-1), X_ADJ(18+index), Y_ADJ(yU2));
        }
    }



    // calculate scale factor for altitude
    const int yU3 = 3+17+1;
    const int yL3 = yU3 + 12;
    double fAltitude = 0;
    if ((d_maxAltitude - d_minAltitude) > 0) fAltitude = d_pixAltitude / (d_maxAltitude - d_minAltitude);
    //painter.eraseRect( 18, yR3, d_numSamples, 12);
    // draw altitude samples
    for (int index = 0; index < d_numSamples; ++index) {
        double value = (d_pAltitudeSamples [index] - d_minAltitude) * fAltitude;
        int h = yL3 - (int)rint(value);
        painter.setPen(drawP);
        painter.drawLine(X_ADJ(18+index), Y_ADJ(yL3), X_ADJ(18+index), Y_ADJ(h));
        if (h > yU3) {
            painter.setPen(eraseP);
            painter.drawLine(X_ADJ(18+index), Y_ADJ(h-1), X_ADJ(18+index), Y_ADJ(yU3));
        }
    }


}

void SatStat::shiftSamples()
{
    // shift all samples by one to the left
    for (int index = 1; index < d_numSamples; ++index) {
        d_pSpeedSamples [index-1]    = d_pSpeedSamples [index];
        d_pAltitudeSamples [index-1] = d_pAltitudeSamples [index];
        d_pSatelliteSamples [index-1]= d_pSatelliteSamples [index];
    }

    double speed = gpsData->speed.speed;
    if (speed > d_maxSpeed) d_maxSpeed = speed;
    if (speed < d_minSpeed) d_minSpeed = speed;
    d_pSpeedSamples [d_numSamples-1] = speed;

    double alt =  gpsData->altitude.altitude;
    if (alt > d_maxAltitude) d_maxAltitude = alt;
    if (alt < d_minAltitude) d_minAltitude = alt;
    d_pAltitudeSamples [d_numSamples-1] = alt;

    pair<int,int> p;
    p.first  = gpsData->d_no_of_satellites;
    p.second = gpsData->d_no_of_fix_satellites;
    if (!gpsData->d_connected) p.second = 99; // no connection to gpsd? --> force error display
    if (!gpsData->d_aliveGPS)  p.second = 99; // no connection to GPS? --> force error display

    d_pSatelliteSamples[d_numSamples-1] = p;
}

void SatStat::drawContents ( QPainter * painter )
{
    painter->save();

    painter->translate(X_ADJ(5), Y_ADJ(3));

    int xoff = X_ADJ(12);
    int yoff = Y_ADJ(11);
    int yR1 =  0;
    int yR2 = Y_ADJ(17);
    int yR3 = Y_ADJ(34);

    painter->drawText( 0, yR1+yoff,  tr("S:"));
    painter->drawRect( xoff, yR1, X_ADJ(d_numSamples+2), Y_ADJ(d_pixSpeed+3));


    painter->drawText( 0, yR2+yoff,  tr("A:"));
    painter->drawRect( xoff, yR2, X_ADJ(d_numSamples+2), Y_ADJ(d_pixAltitude+3));

    painter->drawText( 0, yR3+yoff,  tr("#:"));
    painter->drawRect( xoff, yR3, X_ADJ(d_numSamples+2), Y_ADJ(d_pixSatellites+3));

    drawSamples();

    painter->restore();

    painter->setPen( QPen( *gpsData->headColor, 1, QPen::SolidLine) );
    painter->drawText(X_ADJ(70), Y_ADJ(93), gpsData->heading.toString());

    painter->setPen( QPen( *gpsData->bearColor, 1, QPen::SolidLine) );
    painter->drawText(X_ADJ(70), Y_ADJ(106), gpsData->bearing.toString());

    painter->setPen( QPen( Qt::black, 1, QPen::SolidLine) );
    painter->drawText(X_ADJ(60), Y_ADJ(67), gpsData->speed.toString());
    painter->drawText(X_ADJ(60), Y_ADJ(80), gpsData->altitude.toString());

    int compassX = X_ADJ(28);
    int compassY = Y_ADJ(83);
    int compassOuter = S_ADJ(48); //h=24
    int compassInner = S_ADJ(24); //h=12

    // draw compass
    painter->drawEllipse( compassX-compassOuter/2, compassY-compassOuter/2, compassOuter, compassOuter);
    painter->drawEllipse( compassX-compassInner/2, compassY-compassInner/2, compassInner, compassInner);

    painter->save();
    painter->translate(compassX, compassY);

    painter->drawText(S_ADJ(-3),  S_ADJ(-13),  tr("N"));
    painter->drawText(S_ADJ(-3),  S_ADJ(23),  tr("S"));
    painter->drawText(S_ADJ(-22), S_ADJ(4),   tr("W"));
    painter->drawText(S_ADJ(16), S_ADJ(4),   tr("E"));

    painter->drawLine(S_ADJ(9), S_ADJ(9), S_ADJ(17), S_ADJ(17));
    painter->drawLine(S_ADJ(-9), S_ADJ(9), S_ADJ(-17), S_ADJ(17));
    painter->drawLine(S_ADJ(9), S_ADJ(-9), S_ADJ(17), S_ADJ(-17));
    painter->drawLine(S_ADJ(-9), S_ADJ(-9), S_ADJ(-17), S_ADJ(-17));

    painter->setRasterOp(Qt::OrROP);

    int tw, th, sw, sh;

    // heading
    if (gpsData->heading.show) {
        tw = (int)((double)(sin(gpsData->heading.angle*M_PI/180.0)*(double)14.0));
        th = (int)((double)(cos(gpsData->heading.angle*M_PI/180.0)*(double)14.0));
        sw = -tw / 7;
        sh = -th / 7;

        painter->setPen( QPen( *gpsData->headColor, 3, QPen::SolidLine) );
        painter->drawLine(S_ADJ(tw),S_ADJ(-th), S_ADJ(sw),S_ADJ(-sh));
    }

    // bearing
    if (gpsData->bearing.show) {
        tw = (int)((double)(sin(gpsData->bearing.angle*M_PI/180.0)*(double)28.0));
        th = (int)((double)(cos(gpsData->bearing.angle*M_PI/180.0)*(double)28.0));
        sw = (int)((double)(sin(gpsData->bearing.angle*M_PI/180.0)*(double)18.0));
        sh = (int)((double)(cos(gpsData->bearing.angle*M_PI/180.0)*(double)18.0));

        painter->setPen( QPen( *gpsData->bearColor, 3, QPen::SolidLine) );
        painter->drawLine(S_ADJ(tw),S_ADJ(-th), S_ADJ(sw),S_ADJ(-sh));
    }

    painter->restore();

}


void SatStat::updateInfo()
{
}


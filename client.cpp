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


#include "client.h"
#include "qpegps.h"
#include "settings.h"
#include "gpsstatus.h"
#include <qmessagebox.h>
#include <qapplication.h>
#include <qsocket.h>
#include <qwidget.h>
#include <qtimer.h>
#include <qpe/qpedebug.h>
#include "gpsdata.h"

#define MAGIC_WORD "GPSD"       // gpsd signature

//Client::Client( GpsData *gData )
Client::Client(Qpegps * appl)
{
    application = appl;

    d_opMode = MODE_NORMAL;
    widgetToReEnable = NULL;
    oldTS = "";
    bootMode = true;

    positionChanged = FALSE;

    timer = new QTimer(application);
    gpsdRequest = "S\n";

    connect(timer, SIGNAL(timeout()), SLOT(sendToServer()));
    //connect( application, SIGNAL(aboutToQuit()), SLOT(closeConnection()) );

    socket = new QSocket(this);
    connect(socket, SIGNAL(connected()), SLOT(socketConnected()));
    connect(socket, SIGNAL(connectionClosed()),
            SLOT(socketConnectionClosed()));
    connect(socket, SIGNAL(readyRead()), SLOT(socketReadyRead()));
    connect(socket, SIGNAL(error(int)), SLOT(socketError(int)));
    // connect to gpsd
    gpsData = &(appl->gpsData());
    if (gpsData->ManualPosit)
        gpsData->statusStr = "X";                                                   /* Added by A/ Karhov *//********/
    else
        gpsData->statusStr = "Try to connect to gpsd";
    socket->connectToHost(gpsData->host, gpsData->port);

    // this is a special one-shot timer to reliable detect loss of GPS data stream
    rawtimer = new QTimer(application);
    connect(rawtimer, SIGNAL(timeout()), SLOT(lostGPSConnection()));

    // this is a special one-shot timer to reliable detect loss of gpsd data stream
    timeout = new QTimer(application);
    connect(timeout, SIGNAL(timeout()), SLOT(lostGPSdConnection()));
    // set timer to give tcp connection to gpsd some time (3 sec),
    // if after initial time no connection succeeded restart gpsd
    timeout->start(3000, true);
}

Client::~Client()
{
}

void Client::closeConnection()
{
    socket->close();
    if (socket->state() == QSocket::Closing)
    {
        connect(socket, SIGNAL(delayedCloseFinished()), SLOT(socketClosed()));
    }
    else
    {
        socketClosed();
    }
}

void Client::readyToConnect(QString request, int dt)
{
    gpsdRequest = request + "\n";

    if (bootMode)
    {
        // that's nice that we're ready but where is gpsd?
        // wait for a connection before starting the whole nine yards
        // Note: this depends on that 'readyToConnect' is started once a connection is found
        return;
    }

    // timer to send request to gpsd
    timer->start(dt);
    // watchdog to trace loss of gpsd connection
    timeout->start(3000, true);
    // watchdog to trace loss of GPS connection
    rawtimer->start(3000, true);
}

void Client::sendToServer()
{
    //qDebug(tr("sendToServer t=%1").arg(timer->isActive()));

    // write to gpsd
    QTextStream os(socket);

    os << gpsdRequest;
    socket->flush();
}

void Client::startSniffMode()
{
    if (d_opMode == MODE_NORMAL)
    {
        // normal op mode, start sniffing
        widgetToReEnable = application->currentPage();

        // start the timers
        readyToConnect("SPAVHBDX", 250);

        // mode enabled
        d_opMode = MODE_SNIFF;
    }
}

void Client::endSniffMode()
{
    if (d_opMode == MODE_SNIFF)
    {
        // switch off sniffing
        d_opMode = MODE_NORMAL;
        if (widgetToReEnable)
            application->tabChanged(widgetToReEnable);
    }
}


void Client::socketReadyRead()
{
    //qDebug("socketReadyRead()");
    //read from gpsd
//QPE_DEBUGTIMEDESC("new data arrived");
    if (!socket->canReadLine())
        socket->waitForMore(25);

    while (socket->canReadLine())
    {
        QString dataStr(socket->readLine());
        QChar dataType;

        //qDebug("Received: "+dataStr+" "+(inSniffMode() ? "Sniffing" : "Normal"));

        if (dataStr[0] == '$')
        {
            // ok, gps send someting NMEA like
            gpsData->d_aliveGPS = true;

            // start one shot timeout timer for 3 sec
            // when timout is reached before timer can be reactivated
            // indicates signal from GPS is lost
            rawtimer->start(3000, true);

            // raw data found
            // Decode raw data with own optimized nmea parser
            // fetch $GPGSV for Satellites in view
            if (dataStr.startsWith("$GPGSV"))
            {
                parse_GPGSV(dataStr);
            }
            // fetch $GPGGA for Satellite fix data and DGPS info
            else if (dataStr.startsWith("$GPGGA"))
            {
                parse_GPGGA(dataStr);
            }
            // fetch $Pxxx  for device type info (e.g. GRM for Garmin)
            else if (dataStr.startsWith("$P"))
            {
                // vendor specific code detected
                QString vendorCode = dataStr.mid(2, 3);
                // for now just print the vendor code
                // Note: we need some lookup table for the most common vendors
                gpsData->d_Receiver = vendorCode.upper();
            }

            // skip further processing of *raw* data
            continue;
        }
        else if (dataStr.upper().startsWith(MAGIC_WORD))
        {
            // received the 'magic' word
            // flag that we are connected, data seems to come
            if (!gpsData->d_connected)
            {
                gpsData->d_connected = true;

                if (inSniffMode())
                    endSniffMode();
            }
            // start one shot timeout timer for 3 sec
            // when timout is reached before timer can be reactivated
            // indicates signal from gpsd is lost
            timeout->start(3000, true);

            if (gpsData->d_Receiver.isEmpty())
                gpsData->d_Receiver = "???";
        }
        else if (gpsData->d_connected)
        {
            // what? someone changed the baud rate on me?
            gpsData->d_connected = false;
        }

        // Don't try to make sense of garbeling, wait for the magic word
        // Prevent flickering of status because gpsd is dummer than we thought and
        // keeps sending status OK although GPS is not sending any more
        if (!gpsData->d_connected || !gpsData->d_aliveGPS)
            continue;

        int dataPos = dataStr.find('=') - 1;
        while (dataPos > 0)
        {
            dataType = dataStr[dataPos];
            dataStr = dataStr.mid(dataPos + 2);
            
            QTextIStream iStream(&dataStr);
            double d;
            
            switch (dataType)
            {
            case 'S':
                iStream >> gpsData->status;
                break;

            case 'P':
                iStream >> gpsData->latitudeGps >> gpsData->longitudeGps;
                if (!gpsData->ManualPosit)
                {
                    gpsData->currPos.setLat(gpsData->latitudeGps);
                    gpsData->currPos.setLong(gpsData->longitudeGps);
                }
                
                positionChanged = true;
                break;

            case 'W':
                iStream >> d; gpsData->wpPos.setLat(d);
                iStream >> d; gpsData->wpPos.setLong(d);
                break;

            case 'A':
                iStream >> d; gpsData->altitude.setFromDouble(d);
                // positionChanged = TRUE; never updated without long or lat
                break;

            case 'V':
                iStream >> d; gpsData->speed.setFromDouble(d);

                if (d > 0.5) // average speed for travel time calc.
                    gpsData->avspeed.setFromDouble( gpsData->avspeed.toDouble() * 0.99 + d * 0.01 );
                        
                break;

            case 'G':
                iStream >> d; gpsData->wpSpeed.setFromDouble(d);
                break;

            case 'H':
                iStream >> d; gpsData->heading.setFromDouble(d);
                break;

            case 'B':
                iStream >> d; gpsData->bearing.setFromDouble(d);
                break;

            case 'L':
                iStream >> d; gpsData->wpDistance.setFromDouble(d);
                break;

            case 'D':
                QString s;
                iStream >> s; gpsData->ts.setDate(s);
                iStream >> s; gpsData->ts.setTime(s);

                // check if the 'time' stamp has changed since a while
                // if not, this is an indication that the GPS data stream has stopped
                // and only gpsd server deamon is sending old data over and over again
                if (oldTS.compare(gpsData->ts.time()) != 0)
                {
                    // new time stamp has arrived, good
                    oldTS = gpsData->ts.time();
                    // restart watchdog timer
                    rawtimer->start(3000, true);
                }
                break;

            }
            dataPos = dataStr.find('=') - 1;
        }
    }
    if (positionChanged)
    {
        gpsData->adjustDatum();
        positionChanged = FALSE;
    }
    emit newData();
//QPE_DEBUGTIMEDESC("data processed, map drawn");
}

void Client::lostGPSdConnection()
{
    static bool dialogActive = false;

    gpsData->d_connected = false;
    gpsData->d_aliveGPS = false;
    gpsData->status = 0;
    gpsData->d_no_of_satellites = 0;


    //    qDebug(tr("***** gpsd signal lost! Try to regain connection... %1").arg(bootMode));

    // check if this is happening during boot mode
    if (bootMode && !gpsData->ManualPosit)
    {
        // switch to GPS setup page (makes sense right now)
        application->showPage(application->d_pGpsStatus);

        if (!dialogActive)
        {                       // do not display dialog if there's already one
            dialogActive = true;

            QMessageBox mb("QpeGps",
                           tr("Cannot connect to GPS!\n\n"
                              "Abort: Exit QpeGps now.\n"
                              "Retry: Start gpsd with default settings.\n"
                              "Ignore: Change setup on GPS page."),
                           QMessageBox::Critical,
                           QMessageBox::Abort,
                           QMessageBox::Retry | QMessageBox::Default,
                           QMessageBox::Ignore);

            // run modal
            mb.exec();
            mb.hide();

            switch (mb.result())
            {
            case QMessageBox::Abort:
                {
                    socket->close();
                    QApplication::exit(1);
                    return;
                }
            case QMessageBox::Retry:
                {
                    // OK, we never had a working gpsd connection, restart gpsd
                    restartGpsd();
                    // we give it about 10 secs to connect, until we're back here
                    // Note: we need more time than 3 seconds here, since the initial
                    //       gpsd start can be delayed due to Compact Flash card
                    timeout->start(10000, true);

                }
            default:
                // user asked to ignore this - alright, let him deal with this
                break;
            }
            dialogActive = false;
        }
        return;
    }

    // No socket connction exist, so it is allowed to force RAW data mode
    d_opMode = MODE_NORMAL;
    startSniffMode();

    emit newData();
}

void Client::restartGpsd()
{
    socket->close();

    startGpsd();

    socket->connectToHost(gpsData->host, gpsData->port);

    startSniffMode();
}

void Client::startGpsd()
{
    // only start gpsd if hostname is set to localhost
    // because if hostname is another, gpsd is supposed to run on remote machine
    if (!gpsData->gpsdArgStr.isEmpty()
        && (gpsData->host.compare(gpsdDefaultHost) == 0))
    {

        system("killall gpsd");
        system(gpsData->qpedir + "/bin/gpsd " + gpsData->gpsdArgStr);

        timespec waitForGpsd;
        waitForGpsd.tv_sec = 3;
        waitForGpsd.tv_nsec = 0;
        nanosleep(&waitForGpsd, 0);

        qDebug("gpsd started\n");
    }
}


void Client::lostGPSConnection()
{
    gpsData->d_aliveGPS = false;
    gpsData->d_Receiver = "???";
    gpsData->status = 0;
    gpsData->d_no_of_satellites = 0;

    // we force sniff mode
    d_opMode = MODE_NORMAL;
    startSniffMode();

    emit newData();
}

void Client::socketConnected()
{
    if (bootMode)
    {
        // first time called
        bootMode = false;
    }
    if (gpsData->ManualPosit)
        gpsData->statusStr = "X";                                                   /* Added by A/ Karhov *//********/
    else
        gpsData->statusStr = "";       // hide label, if everything is OK
    timer->start(0, FALSE);     // creates always an event, if there is no other one running
}

void Client::socketConnectionClosed()
{
    gpsData->statusStr = "Connection closed by gpsd";
    emit newData();
}

void Client::socketClosed()
{
    gpsData->statusStr = "Connection closed";
    emit newData();
}

void Client::socketError(int e)
{
    if (bootMode)
    {
        // don't wait much longer, we know there is something wrong
        timeout->stop();
        lostGPSdConnection();
    }
    
    if(!gpsData->ManualPosit)
        gpsData->statusStr = tr("Error number %1 occured").arg(e);
        
    emit newData();
}

void Client::parse_GPGGA(QString & str)
{
    // Quick hack to get Satellite fix data to run
    // needs to be rewritten and *rubostified*

    QStringList l;
    l = QStringList::split(',', str, TRUE);

    // adjust checksum
    QStringList x;
    x = QStringList::split('*', l.last(), TRUE);

    // adjust last entry in orig input
    l.last() = x.first();

    enum
    { GPTAG = 0, GMT,
        LATITUDE, LAT_HEMI, LONGITUDE, LONG_HEMI,
        RCVMODE, NUMFIXSAT, HDOP,
        ALTITUDE, ALTUNITS,
        SEPARATION, SEP_UNITS,
        DGPSAGE, DGPSID
    };

    gpsData->d_no_of_fix_satellites = l[NUMFIXSAT].toInt();

    // next step, instead of using static ints use signals
    // emit updateNumSat();
}

void Client::parse_GPGSV(QString & str)
{
    // Quick hack to get Satellite status to run
    // needs to be rewritten and *rubostified*

    QStringList l;
    l = QStringList::split(',', str, TRUE);

    // adjust checksum
    QStringList x;
    x = QStringList::split('*', l.last(), TRUE);

    // adjust last entry in orig input
    l.last() = x.first();

    enum
    { GPTAG = 0, NUMLINES, LINENO, SATSINVIEW, NAME, ELEVATION, AZIMUT, SNR };

    const int SATSPERLINE = 4;
    int lineno = l[LINENO].toInt();
    int offset = (lineno - 1) * SATSPERLINE;

    gpsData->d_no_of_satellites = l[SATSINVIEW].toInt();

    QString t;

    for (int index = 0; index < SATSPERLINE; ++index)
    {
        // four satellite info per line
        int satnumber = index + offset;
        int pos = index * SATSPERLINE;

        // sanity check
        if ((satnumber < 0) || (satnumber >= 12)
            || (satnumber >= gpsData->d_no_of_satellites))
            // invalid satellite number
            continue;

        SatInfo & satInfo = gpsData->d_pSatInfo[satnumber];

        satInfo.setElevation(0);
        satInfo.setAzimut(0);
        satInfo.setSignalNoiseRatio(0);
        satInfo.setSatName("");

        t = l[pos + ELEVATION];
        if (!t.isEmpty())
            satInfo.setElevation(t.toInt());

        t = l[pos + AZIMUT];
        if (!t.isEmpty())
            satInfo.setAzimut(t.toInt());

        t = l[pos + SNR];
        if (!t.isEmpty())
            satInfo.setSignalNoiseRatio(t.toInt());

        t = l[pos + NAME];
        if (!t.isEmpty())
            satInfo.setSatName(t);

        satInfo.setUpdated(true);
    }


}

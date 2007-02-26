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

#ifndef CLIENT_H
#define CLIENT_H


// #include <math.h>
// #include <time.h>
// #include <stdlib.h>

#include <qobject.h>

class Qpegps;
class QSocket;
class QWidget;
class QTimer;
class GpsData;

class Client : public QObject
{
  Q_OBJECT
  
  public:
    QTimer * timer, *timeout, *rawtimer;
    QString gpsdRequest;

      Client(Qpegps * appl);
     ~Client();

    Qpegps *application;
    QSocket *socket;

    enum
    { MODE_NORMAL, MODE_SNIFF } d_opMode;

    void startSniffMode();
    void endSniffMode();

    bool inSniffMode() const
    {
        return (d_opMode == MODE_SNIFF);
    }

    QWidget *widgetToReEnable;

    void readyToConnect(QString request, int dt);
    void startGpsd();

  private:
      QString oldTS;
    bool bootMode;
    bool positionChanged;
    
    GpsData * gpsData;

  private:
    void parse_GPGGA(QString & str);
    void parse_GPGSV(QString & str);

    private slots: void sendToServer();
    void socketReadyRead();
    void socketConnected();
    void socketConnectionClosed();
    void socketClosed();
    void socketError(int e);

    public slots:void closeConnection();
    void lostGPSdConnection();
    void lostGPSConnection();
    void restartGpsd();

      signals: void newData();

};



#endif

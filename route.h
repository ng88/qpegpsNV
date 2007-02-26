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

#ifndef ROUTE_H
#define ROUTE_H


/* File and classes added by ng <ng@ngsoft-fr.com>, as a replacement of the old unterminated Route class */

#include "maps.h"
#include "gpsdata.h"

#include <qdialog.h>
#include "mathex.h"

class QCheckBox;
class QVBox;
class QHBox;
class QLabel;
class QPushButton;
class QComboBox;
class QSpinBox;
class QFile;
class Qpegps;
class GpsData;
class QStringList;
class QPoint;
class QListBox;
class QFrame;
class QTimer;
class QWidgetStack;


class RoutePoint                /* I prefered not use TrackPoint or inherit from TrackPoint */
{
  protected:
  
    double _lat, _lng; // in RAD
    //double alt      // unused
    //Qtime time      //unused
    double _meters2prec; //in meters
    QString _comment;
    int _commentPos;

    
    RoutePoint() {}
    
  public:
  
    RoutePoint(const QString & str);
    RoutePoint(double lat, double lng, double meters2prec, const QString & comment =
                QString(""), int commentPos = 0);
                 
    ~RoutePoint() { }

    /* in rad */
    inline void setLongitude(double v) { _lng = v; }
    inline double longitude() { return _lng; }
    
    /* in deg */
    inline double longitudeDeg() { return MathEx::rad2deg(_lng); }

    /* in rad */
    inline void setLatitude(double v) { _lat = v; }
    inline double latitude() { return _lat; }
    
    /* in deg */
    inline double latitudeDeg() { return MathEx::rad2deg(_lat); }
    
    inline void setMeters2prec(double v) { _meters2prec = v; }
    inline double meters2prec() { return _meters2prec; }

    inline void setComment(const QString & v) { _comment = v; }
    inline const QString & comment() const { return _comment; }

    inline void setCommentPos(const int &v) {  _commentPos = v; }
    inline int commentPos() const { return _commentPos; }
   

};

/* a route */
class Route : public QObject
{
    Q_OBJECT
    
  public:
  
      typedef QList<RoutePoint> RoutePoints;
   
  private:

    RoutePoints  _route;
    RoutePoints  _comments;

    void dispose();

    RoutePoint * _closer;
    
    bool _routeLost;
    
    double _length; /* in meters */
    
    double _distanceBeforeNextComment; /* in meters */
    
    double _routeDone; /* in meters */
    
    /* last read comment */
    int _lastCommentNb;
    
    /* last reached comment */
    int _lastReachedComment;
    

  public:

      Route();
     ~Route();

    /* myPosLt & myPosLg in RAD */
    void draw(QPainter * painter, MapBase * actmap, int x1, int y1, int mx,
              int my, GpsData & data);
              
    void readFromFile(const QString & file);
    void readFromFile(QFile & file);

    inline bool isEmpty()  const { return _route.isEmpty(); }
    inline void clear() {  dispose(); }
    
    /* return route length in meters */
    inline double length() const { return _length; }
    inline double done() const { return _routeDone; }
    inline double distanceBeforeNextComment() const { return _distanceBeforeNextComment; }
    inline bool lost() const { return _routeLost; }
    inline int lastAnnouncedComment() const { return _lastCommentNb; }
    inline int lastReachedComment() const { return _lastReachedComment; }
   
    /* call this when we've left the route */
    inline void revertToRoute() { _routeLost = false; _closer = 0; }

    inline RoutePoints& commentedRoutePoints() { return _comments;}
    
 signals:
     void positionChanged();
     void routeChanged();
     void routeLost();
     void routeInfo(const QString& comment, int commentIndex);


};

/* GUI class that is the Route tab in the main window */
class RouteGUI:public QScrollView
{
  Q_OBJECT
  
  protected:
    Route _route;

    QComboBox *_cboRoutes;
    QPushButton *_btnOK;
    QCheckBox *_chkDspIcon;
    QLabel *_lblCurrent;

    QCheckBox *_chkDspPopup;
    QCheckBox *_chkPopupAutoClose;
    QSpinBox *_spnAutoCloseTime;
    QSpinBox *_spnPreventDist;
    QCheckBox *_chkTts;
    QComboBox *_cboTts;
    
    QCheckBox *_chkDrawLine;
    QSpinBox *_spnLineThick;


    Qpegps *_app;


    void loadSetting();
    void saveSetting();

  public:

      RouteGUI(Qpegps * appl, QWidget * parent = 0, const char *name =
               0, WFlags fl = 0);
     ~RouteGUI();

    inline Route & currentRoute() {  return _route; }
    inline bool drawEnabled() { return !_route.isEmpty(); }
    
    void draw(QPainter * painter, MapBase * actmap, int x1, int y1, int mx, int my);

  private slots:
  
    void currentTabChanged(QWidget * e);
    void updateGUI();
    void btnOkPressed();
    void ttsEngineSelected(int v);
    void valuesUpdated();

};


/* GUI class for alerting the driver */
class RouteAlert : public QDialog
{
    Q_OBJECT
    
protected:

    Qpegps * _app;

    int timeLeft;
    uint currentComment;
    
    Route * route;
    
    QLabel* lblInfo;
    
    QLabel* lblComment;
    QListBox* lbComments;

    
    QLabel* lblRouteDoneTotal;
    //QLabel* lblRouteBeforeComment;
    
    QFrame * autoCloseLine;
    QCheckBox* chkAutoClose;
    QTimer * autoCloseTimer;
    
    QPushButton* btnList;
    
    QWidgetStack* widgetStack1;
    
public:

    RouteAlert(Qpegps * app, QWidget * p, Route * rt);
    
    void show(int commentIndex = 1, bool autoClose = false, int autoCloseTime = 30);
    
protected slots:
    void timerDone();
    
public slots:

    void btnsTopClicked(int);
    void chkAutoCloseClicked();
    void listSel(int);
    void showComment(int);
    void routeInfoUpdated();
    void routeChanged();
    

};

#endif

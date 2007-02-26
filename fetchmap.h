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

#ifndef FETCH_MAP_H
#define FETCH_MAP_H

#include <qvbox.h>
#include <qhbox.h>
#include <qvgroupbox.h>
#include <qmultilineedit.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qarray.h>
#include <qwidget.h>
#include <qdialog.h>
#include <qsortedlist.h>
#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qlistview.h>
#include <qvalidator.h>
#include <qimage.h>
#include <qmessagebox.h>
#include <qpalette.h>
//not in qpe :-( #include <qtopia/services.h>

#include <math.h>

#include "maps.h"
#include "gpsdata.h"
#include "mapinfo.h"

class DownloadSpecification;

class MapSource:public QObject
{
  Q_OBJECT public:
    MapSource(QString * sourceInfo);
    MapSource(QString lname, int pWidth, int pHeight, QString lurl);
     ~MapSource();
    int operator==(MapSource &);
    int operator<(MapSource &);
    int operator>(MapSource &);
    QString makeURLString(DownloadSpecification *);

    QString name;
    QString url;
    int pixelWidth;
    int pixelHeight;

};

typedef QSortedList < MapSource > MapSourceList;

class MapSourceWidget:public QVBox
{
  Q_OBJECT public:
    MapSourceWidget(QWidget * parent = 0, const char *name = 0);
     ~MapSourceWidget();
    void setMapSourceList(MapSourceList *);
    int getSourceIndex();

  protected:
      QVGroupBox * sourceGB;
    QComboBox *sourceCB;
    QLabel *urlL;
    QMultiLineEdit *urlMLE;
    //QPushButton *sourceDeletePB;
    //QPushButton *sourceEditPB;
    //QPushButton *sourceAddPB;

  private:
      MapSourceList * _mapSourceList;

    private slots: void sourceChanged(int);
    void remove();
    void edit();
    void add();
};

class MapSourceFile:public QObject
{
  Q_OBJECT public:
    MapSourceFile(QString mapSourceFilename);
    ~MapSourceFile();
    MapSourceList *makeMapSourceList();
    void write(MapSourceList * mapSourceList);
  private:
      QString filename;
};

class MapSourceEditorWidget:public QVBox
{
  Q_OBJECT public:
    MapSourceEditorWidget(QWidget * parent = 0, const char *name = 0);
      MapSourceEditorWidget(MapSource *, QWidget * parent =
                            0, const char *name = 0);
     ~MapSourceEditorWidget();
  protected:
      QVGroupBox * sourceGB;
    QLabel *nameL;
    QLineEdit *nameLE;
    QLabel *urlL;
    QMultiLineEdit *urlMLE;
    QLabel *mapLatLonL;
    QComboBox *mapLatLonCB;
    QVGroupBox *gifGB;
    QLabel *gifL;
    QCheckBox *gifCB;
    QCheckBox *optimizeCB;
};

class MapSourceEditorDialog:public QDialog
{
  Q_OBJECT public:
    MapSourceEditorDialog(QWidget *, const char *, bool, WFlags);
     ~MapSourceEditorDialog();

  protected:
      MapSourceEditorWidget * mapSrcEditW;

};

class DownloadSpecification:public QObject
{
  Q_OBJECT public:
    DownloadSpecification(GpsData *, MapInfo *, QSortedList < MapBase > *);
    ~DownloadSpecification();
    void download(MapSource * mapSource);

    QString name;
    double latitude;
    double longitude;
    enum LatHemisphere
    { NorthernHemisphere = 1, SouthernHemisphere = -1 } latHem;
    enum LonHemisphere
    { EasternHemisphere = 1, WesternHemisphere = -1 } lonHem;
    long unsigned int scale;
  protected:
      GpsData * gpsData;
      QSortedList < MapBase > *mapList;
};

class DownloadSpecificationWidget:public QVBox
{
  Q_OBJECT public:
    DownloadSpecificationWidget(DownloadSpecification *, QWidget * parent =
                                0, const char *name = 0);
     ~DownloadSpecificationWidget();
    void setDownloadSpecification(DownloadSpecification *);
  protected:
      bool validate();

    DownloadSpecification *spec;
    QVGroupBox *detailsGB;
    QLabel *nameL;
    QLineEdit *nameLE;
    QLabel *latitudeL;
    QLineEdit *latitudeLE;
    QComboBox *latitudeCB;
    QLabel *longitudeL;
    QLineEdit *longitudeLE;
    QComboBox *longitudeCB;
    QLabel *scaleL;
    QComboBox *scaleCB;
    //QPushButton *scalePB;
    public slots: bool accept();
};

class DownLoadDialog:public QDialog
{
  Q_OBJECT public:
    DownLoadDialog(GpsData *, MapInfo *, QSortedList < MapBase > *, QWidget *,
                   const char *, bool, WFlags);
     ~DownLoadDialog();

  protected:
      GpsData * gpsData;
      QSortedList < MapBase > *mapList;

    DownloadSpecification *spec;
    MapSourceList *mapSrcL;
    MapSourceWidget *mapSrcW;
    DownloadSpecificationWidget *dlSpecW;

    protected slots:void accept();
};

class MapParDialog:public QDialog
{
  Q_OBJECT public:
    MapParDialog(MapBase *, QWidget *, const char *, bool, WFlags);
     ~MapParDialog();
    QVBox *vBox;
    QHBox *hBox;
    QComboBox *projectionCB;
    QLabel *scaleL;
    QLineEdit *scaleLE;
    QScrollView *mapView;
    MapWidget *mapWidget;

    QHBox *point1HB;
    QVBox *p1xyLVB, *p1xyLEVB, *p1llLVB, *p1llLEVB;
    QLabel *x1L, *y1L, *lt1L, *lg1L;
    QLineEdit *x1LE, *y1LE, *lt1LE, *lg1LE;
    QHBox *point2HB;
    QVBox *p2xyLVB, *p2xyLEVB, *p2llLVB, *p2llLEVB;
    QLabel *x2L, *y2L, *lt2L, *lg2L;
    QLineEdit *x2LE, *y2LE, *lt2LE, *lg2LE;

    QVBox *p1utmVB, *p2utmVB;

    QHBox *stdLongHB;
    QLabel *stdLongL;
    QLineEdit *stdLongLE;

    QHBox *stdLat12HB;
    QLabel *stdLat1L, *stdLat2L, *refLongL;
    QLineEdit *stdLat1LE, *stdLat2LE, *refLongLE;

    QHBox *centerHB;
    QLabel *centerLatL, *centerLongL;
    QLineEdit *centerLatLE, *centerLongLE;

    QHBox *utmHB;
    QLabel *zoneL;
    QLineEdit *zoneLE;

    private slots:void showProjectionPar(int);
    void clickPosition(int, int);
};

class ImportMapDialog:public QDialog
{
  Q_OBJECT public:
    ImportMapDialog(QSortedList<MapBase> *, QWidget *, const char *, bool,
                    WFlags);
     ~ImportMapDialog();

    QVBox *vBox;

    FileSelector *imageDialog;


    DocLnk mapImageLnk;

    bool imageSelected;

    QVButtonGroup *bg;
    QRadioButton *cpOrg, *delOrg;

    private slots:void docLnkSelected(const DocLnk &);
};

class ChangeMapParDialog:public QDialog
{
  Q_OBJECT public:
    ChangeMapParDialog(QSortedList < MapBase > *, QWidget *, const char *,
                       bool, WFlags);
     ~ChangeMapParDialog();
    QVBox *vBox;
    QComboBox *mapSelect;
    QStringList mapNames;
    QPixmap *image;
};

/*class RemoveMapDialog : public QDialog
{
   Q_OBJECT
   public:
   RemoveMapDialog(QSortedList <MapBase>*, QWidget *, const char *, bool, WFlags);
   ~RemoveMapDialog();
   QVBox *vBox;
   QComboBox  *mapSelect;
   QStringList mapNames;
   QVButtonGroup *bg;
   QRadioButton *remMap, *delMap;
};
*/

class RemoveMapDialog:public QDialog
{
  Q_OBJECT public:
    RemoveMapDialog(MapBase *, QWidget *, const char *, bool, WFlags);
     ~RemoveMapDialog();
    QCheckBox *deleteCB;
  protected:
      QVBox * vBox;
    QLabel *mapNameL;
    QLabel *textL;
};


class DownloadAreaSpecification:public QObject  /* Added by A. Karkhov */
{
  Q_OBJECT public:
    DownloadAreaSpecification(GpsData *, MapInfo *,
                              QSortedList < MapBase > *);
    ~DownloadAreaSpecification();

    QString prefix;
    double latitude;
    double longitude;
    double elatitude;
    double elongitude;
    double slatitude;
    double slongitude;
    bool IsArea;
    double areax, areay;
    QString dir;
    unsigned int res;
    long unsigned int scale;
    QString param;

  protected:
      GpsData * gpsData;
      QSortedList < MapBase > *mapList;
};


class DownloadAreaWidget:public QVBox   /* Added by A. Karkhov */
{
  Q_OBJECT public:
    DownloadAreaWidget(Qpegps * appl, DownloadAreaSpecification *,
                       QWidget * parent = 0, const char *name = 0);
     ~DownloadAreaWidget();
    void setDownloadSpecification(DownloadAreaSpecification *);
  protected:
      DownloadAreaSpecification * spec;
    QVGroupBox *detailsGB;
//    QVBox *detailsGB;
    QLabel *latitudeL;
    QLineEdit *latitudeLE;
    QLabel *longitudeL;
    QLineEdit *longitudeLE;

    QLabel *elatitudeL;
    QLineEdit *elatitudeLE;
    QLabel *elongitudeL;
    QLineEdit *elongitudeLE;

    QLabel *slatitudeL;
    QLineEdit *slatitudeLE;
    QLabel *slongitudeL;
    QLineEdit *slongitudeLE;

    QLabel *areaxL;
    QLineEdit *areaxLE;

    QLabel *areayL;
    QLineEdit *areayLE;

    QLabel *scaleL;
    QComboBox *scaleCB;

    QButtonGroup *SE_ABG;
    QRadioButton *SEB;
    QRadioButton *ASB;
    QPushButton *PlacePB;
    QPushButton *PlaceRTPB;
    QPushButton *PlaceLBPB;
    QLabel *dirL;
    QComboBox *dirCB;
    QLabel *resL;
    QComboBox *resCB;
    QLabel *paramL;
    QLineEdit *paramLE;
    Qpegps *application;
    bool IsArea;

    void PlaceSel(double *lat, double *lon);

    public slots:bool accept();
    void toggledASB(bool);
    void toggledSEB(bool);
    void PlaceSelC();
    void PlaceSelRT();
    void PlaceSelLB();
};

class DownLoadAreaDialog:public QDialog /* Added by A. Karkhov */
{
  Q_OBJECT public:
    DownLoadAreaDialog(Qpegps * application, DownloadAreaSpecification * spec,
                       GpsData *, MapInfo *, QSortedList < MapBase > *,
                       QWidget *, const char *, bool, WFlags);
     ~DownLoadAreaDialog();

  protected:
      GpsData * gpsData;
      QSortedList < MapBase > *mapList;

    DownloadAreaWidget *dlSpecW;

    protected slots:void accept();
};

#endif

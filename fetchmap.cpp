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

#include "fetchmap.h"
#include "mapdisp.h"


const double deg2rad = M_PI / 180.0;
const double rad2deg = 180.0 / M_PI;

MapSource::MapSource(QString * sourceInfo)
{
    QTextIStream *sourceIStream;
    sourceIStream = new QTextIStream(sourceInfo);
    *sourceIStream >> name >> pixelWidth >> pixelHeight >> url;
}

MapSource::MapSource(QString lname, int pWidth, int pHeight, QString lurl)
{
    name = lname;
    pixelWidth = pWidth;
    pixelHeight = pHeight;
    url = lurl;
}

MapSource::~MapSource()
{
};

int MapSource::operator<(MapSource & source)
{
    return (this->name < source.name);
}

int MapSource::operator==(MapSource & source)
{
    return (this->name == source.name);
}

int MapSource::operator>(MapSource & source)
{
    return (this->name > source.name);
}

QString MapSource::makeURLString(DownloadSpecification * spec)
{
    QString urlstr = url;
    QString string;
    const QString lon = "<LON>";
    const QString lat = "<LAT>";
    const QString sca = "<SCA>";

    const QString lonVal = string.setNum(spec->longitude * spec->lonHem);
    const QString latVal = string.setNum(spec->latitude * spec->latHem);
    const QString scaVal = string.setNum(spec->scale);

    urlstr.replace(lon, lonVal);
    urlstr.replace(lat, latVal);
    urlstr.replace(sca, scaVal);

    return urlstr;
}

MapSourceFile::MapSourceFile(QString mapSourceFilename)
{
    filename = mapSourceFilename;
}

MapSourceFile::~MapSourceFile()
{
};

MapSourceList *MapSourceFile::makeMapSourceList()
{
    MapSourceList *mapSourceList;
    mapSourceList = new MapSourceList();
    QFile sourceFile(filename);
    int ok = sourceFile.open(IO_ReadOnly);
    if (ok)
    {
        MapSource *ms;
        QString line;
        QTextStream t(&sourceFile);
        mapSourceList->setAutoDelete(TRUE);
        while (!t.eof())
        {
            line = t.readLine().stripWhiteSpace();
            if (line.length() > 0)
            {
                if (line[0] != '#')
                {
                    ms = new MapSource(&line);
                    mapSourceList->append(ms);
                }
            }
        }
    }
    return mapSourceList;
}

void MapSourceFile::write(MapSourceList *)
{
    //write the sources to the file
}

MapSourceWidget::MapSourceWidget(QWidget * parent,
                                 const char *name):QVBox(parent, name)
{
    sourceGB = new QVGroupBox(tr("Map Source"), this);
    sourceCB = new QComboBox(sourceGB);
    urlL = new QLabel(tr("URL:"), sourceGB);
    urlMLE = new QMultiLineEdit(sourceGB);
    urlMLE->setWordWrap(QMultiLineEdit::WidgetWidth);
    urlMLE->setWrapPolicy(QMultiLineEdit::Anywhere);
    urlMLE->setReadOnly(TRUE);

    //QHBox *hBox;
    //hBox = new QHBox(sourceGB);
    //sourceDeletePB = new QPushButton("delete",hBox);
    //sourceEditPB = new QPushButton("edit...",hBox);
    //sourceAddPB = new QPushButton("add...",hBox);

    connect(sourceCB, SIGNAL(highlighted(int)), SLOT(sourceChanged(int)));
    //connect(sourceDeletePB,SIGNAL(clicked()),SLOT(remove()));
    //connect(sourceEditPB,SIGNAL(clicked()),SLOT(edit()));
    //connect(sourceAddPB,SIGNAL(clicked()),SLOT(add()));
}

MapSourceWidget::~MapSourceWidget()
{
};

void MapSourceWidget::setMapSourceList(MapSourceList * mapSourceList)
{
    _mapSourceList = mapSourceList;

    for (MapSource * source = _mapSourceList->first(); source != 0;
         source = _mapSourceList->next())
    {
        sourceCB->insertItem(source->name);
    }
    if (_mapSourceList->count() > 0)
        urlMLE->setText(_mapSourceList->first()->url);
}

int MapSourceWidget::getSourceIndex()
{
    return sourceCB->currentItem();
}

void MapSourceWidget::sourceChanged(int index)
{
    urlMLE->setText(_mapSourceList->at(index)->url);
}

void MapSourceWidget::remove()
{
    qDebug("delete clicked");
}

void MapSourceWidget::edit()
{
    MapSourceEditorDialog msDialog(this, "edit map source", TRUE, 0);
    msDialog.setCaption(tr("Edit"));
    msDialog.exec();
    if (msDialog.result() == QDialog::Accepted)
    {

    }
}

void MapSourceWidget::add()
{
    MapSourceEditorDialog msDialog(this, "add map source", TRUE, 0);
    msDialog.setCaption(tr("Add"));
    msDialog.exec();
    if (msDialog.result() == QDialog::Accepted)
    {

    }
}

MapSourceEditorDialog::MapSourceEditorDialog(QWidget * parent,
                                             const char *name, bool modal,
                                             WFlags f):QDialog(parent, name,
                                                               modal, f)
{
    resize(240, 300);
    mapSrcEditW = new MapSourceEditorWidget(this);
}

MapSourceEditorDialog::~MapSourceEditorDialog()
{
};

MapSourceEditorWidget::MapSourceEditorWidget(QWidget * parent,
                                             const char *name):QVBox(parent,
                                                                     name)
{
    QHBox *hBox;
    QVBox *vBox;

    resize(parent->geometry().size());

    vBox = new QVBox(this);
    sourceGB = new QVGroupBox(tr("Download Source Information"), vBox);
    hBox = new QHBox(sourceGB);
    nameL = new QLabel(tr("name:"), hBox);
    nameLE = new QLineEdit(hBox);
    urlL = new QLabel(tr("URL:"), sourceGB);
    urlMLE = new QMultiLineEdit(sourceGB);
    urlMLE->setWordWrap(QMultiLineEdit::WidgetWidth);
    urlMLE->setWrapPolicy(QMultiLineEdit::Anywhere);
    hBox = new QHBox(sourceGB);
    mapLatLonL = new QLabel(tr("<LAT>,<LON> specify map:"), hBox);
    mapLatLonCB = new QComboBox(hBox);
    mapLatLonCB->insertItem(tr("Center"));
    mapLatLonCB->insertItem(tr("Lower Left"));
    mapLatLonCB->insertItem(tr("Upper Left"));
    mapLatLonCB->insertItem(tr("Lower Right"));
    mapLatLonCB->insertItem(tr("Upper Right"));
    gifGB = new QVGroupBox(tr("GIF Conversion"), this);
    gifL = new QLabel(tr("GIF maps must be converted to PNG"), gifGB);
    gifCB = new QCheckBox(tr("Downloaded maps are in GIF format"), gifGB);
    optimizeCB = new QCheckBox(tr("Optimize converted PNGs"), gifGB);
}

MapSourceEditorWidget::MapSourceEditorWidget(MapSource *, QWidget * parent,
                                             const char *name):QVBox(parent,
                                                                     name)
{
    MapSourceEditorWidget();
}

MapSourceEditorWidget::~MapSourceEditorWidget()
{
};

DownloadSpecification::DownloadSpecification(GpsData * gData,
                                             MapInfo * mapInfo,
                                             QSortedList < MapBase > *maps)
{
    gpsData = gData;
    mapList = maps;

    // default to current position from map info page
    longitude = mapInfo->mapPos.longitude();
    latitude = mapInfo->mapPos.latitude();

    if ((longitude == 0) && (latitude == 0))
    {
        // use current location from GPS
        longitude = gpsData->currPos.longitude();
        latitude = gpsData->currPos.latitude();
    }

    scale = 100000;
}

DownloadSpecification::~DownloadSpecification()
{
};

void DownloadSpecification::download(MapSource * mapSource)
{
    QString fullPath = gpsData->mapPathStr + "/" + name;
    QString urlStr = mapSource->makeURLString(this);

    // --quick and dirty implemtation with wget for now - this will be replaced with a fully Qt solution
    //   with a progress bar and everything

    system("wget -O " + fullPath + ".gif \"" + urlStr + "\"");

    // not in Qtopia 1.5 :-(
    //ServiceRequest srv("WebAccess", "getURL(QString,QString)");
    //srv << urlStr << fullPath;
    //if(!srv.send())
    //qWarning("service getURL not available");



    // we're assuming we're downloading a gif for now.  this should be configurable in the MapSource
    // Qt/E comes compiled w/o gif support by default for licensing reasons - i think allowing users
    // to install a gif2png ipk if they want to use this feature might be a good option
    system("gif2png -O -d " + fullPath + ".gif");

    QString str;
    str = name + ".png " + str.setNum(scale) + " " +
        str.setNum(mapSource->pixelWidth) + " " +
        str.setNum(mapSource->pixelHeight) + " " +
        str.setNum(latitude * latHem) + " " + str.setNum(longitude * lonHem);
    MapFritz *mapFritz;
    mapFritz = new MapFritz(&str);
    mapList->append(mapFritz);
}

DownloadSpecificationWidget::
DownloadSpecificationWidget(DownloadSpecification * spec, QWidget * parent,
                            const char *name):QVBox(parent, name)
{
    QHBox *hBox;
    detailsGB = new QVGroupBox(tr("Download"), this);
    hBox = new QHBox(detailsGB);
    nameL = new QLabel(tr("map name:"), hBox);
    nameLE = new QLineEdit(hBox);
    hBox = new QHBox(detailsGB);
    latitudeL = new QLabel(tr("latitude:"), hBox);
    latitudeLE = new QLineEdit(hBox);
    latitudeLE->
        setValidator(new QDoubleValidator(-1000, 1000, 10, latitudeLE));
    latitudeCB = new QComboBox(hBox);
    latitudeCB->insertItem(tr("North"));
    latitudeCB->insertItem(tr("South"));
    hBox = new QHBox(detailsGB);
    longitudeL = new QLabel(tr("longitude:"), hBox);
    longitudeLE = new QLineEdit(hBox);
    longitudeLE->
        setValidator(new QDoubleValidator(-1000, 1000, 10, longitudeLE));
    longitudeCB = new QComboBox(hBox);
    longitudeCB->insertItem(tr("East"));
    longitudeCB->insertItem(tr("West"));
    hBox = new QHBox(detailsGB);
    scaleL = new QLabel(tr("scale 1:"), hBox);
    scaleCB = new QComboBox(TRUE, hBox);
    scaleCB->setValidator(new QIntValidator(1, 100000000, scaleCB));
    scaleCB->insertItem("002500");
    scaleCB->insertItem("005000");
    scaleCB->insertItem("007500");
    scaleCB->insertItem("010000");
    scaleCB->insertItem("015000");
    scaleCB->insertItem("025000");
    scaleCB->insertItem("050000");
    scaleCB->insertItem("075000");
    scaleCB->insertItem("100000");
    scaleCB->insertItem("250000");
    scaleCB->insertItem("500000");
    //scalePB = new QPushButton( "...",hBox,"edit scales");

    setDownloadSpecification(spec);
}

DownloadSpecificationWidget::~DownloadSpecificationWidget()
{
};

bool DownloadSpecificationWidget::validate()
{
    bool valid = TRUE;

    if (longitudeLE->text().stripWhiteSpace().isEmpty())
    {
        QMessageBox mb(tr("Map Download"),
                       tr
                       ("Longitude not specified.\n\nYou must specify a longitude."),
                       QMessageBox::Warning,
                       QMessageBox::Ok | QMessageBox::Default,
                       QMessageBox::NoButton, QMessageBox::NoButton);
        mb.exec();
        mb.hide();
        valid = FALSE;
    }
    else if (latitudeLE->text().stripWhiteSpace().isEmpty())
    {
        QMessageBox mb(tr("Map Download"),
                       tr
                       ("Latitude not specified.\n\nYou must specify a latitude."),
                       QMessageBox::Warning,
                       QMessageBox::Ok | QMessageBox::Default,
                       QMessageBox::NoButton, QMessageBox::NoButton);
        mb.exec();
        mb.hide();
        valid = FALSE;
    }
    else if (scaleCB->currentText().stripWhiteSpace().isEmpty())
    {
        QMessageBox mb(tr("Map Download"),
                       tr
                       ("Scale not specified.\n\nYou must specify a scale."),
                       QMessageBox::Warning,
                       QMessageBox::Ok | QMessageBox::Default,
                       QMessageBox::NoButton, QMessageBox::NoButton);
        mb.exec();
        mb.hide();
        valid = FALSE;
    }
    else if (nameLE->text().stripWhiteSpace().isEmpty())
    {
        QMessageBox mb(tr("Map Download"),
                       tr("Map name not specified.\n"
                          "You must specify a map name.\n\n"
                          "Click 'OK' for a default name."),
                       QMessageBox::Warning,
                       QMessageBox::Ok | QMessageBox::Default,
                       QMessageBox::Cancel, QMessageBox::NoButton);
        mb.exec();
        mb.hide();

        valid = (mb.result() == QMessageBox::Ok);
    }
    return valid;
}

void DownloadSpecificationWidget::
setDownloadSpecification(DownloadSpecification * dlSpec)
{
    spec = dlSpec;

    double latitude;
    double longitude;
    if (spec->latitude < 0)
    {
        latitude = spec->latitude * -1.0;
        latitudeCB->setCurrentItem(1);
    }
    else
    {
        latitude = spec->latitude;
    }
    if (spec->longitude < 0)
    {
        longitude = spec->longitude * -1.0;
        longitudeCB->setCurrentItem(1);
    }
    else
    {
        longitude = spec->longitude;
    }

    QString string;
    nameLE->setText(spec->name);
    latitudeLE->setText(string.setNum(latitude));
    longitudeLE->setText(string.setNum(longitude));
    scaleCB->setEditText(string.setNum(spec->scale));
}

bool DownloadSpecificationWidget::accept()
{
    bool valid = validate();
    if (valid)
    {
        spec->name = nameLE->text();
        spec->latitude = latitudeLE->text().toDouble();
        spec->longitude = longitudeLE->text().toDouble();
        spec->scale = scaleCB->currentText().toULong();

        if (latitudeCB->currentItem() == 0)
            spec->latHem = DownloadSpecification::NorthernHemisphere;
        else
            spec->latHem = DownloadSpecification::SouthernHemisphere;
        if (longitudeCB->currentItem() == 0)
            spec->lonHem = DownloadSpecification::EasternHemisphere;
        else
            spec->lonHem = DownloadSpecification::WesternHemisphere;

        // user accepted a default name
        if (nameLE->text().stripWhiteSpace().isEmpty())
        {
            // no name given, create one based on scale factor and position
            QString str;
            str = "map_" + str.setNum(spec->scale) + "-" +
                str.setNum(spec->latitude * spec->latHem) + "--" +
                str.setNum(spec->longitude * spec->lonHem);
            nameLE->setText(str);
            valid = false;
        }
    }
    return valid;
}

DownLoadDialog::DownLoadDialog(GpsData * gData, MapInfo * mapInfo, QSortedList < MapBase > *maps, QWidget * parent, const char *name, bool modal, WFlags f):
QDialog(parent, name, modal, f)
{
    gpsData = gData;
    mapList = maps;

    QVBox *vBox;
    vBox = new QVBox(this);

    MapSourceFile mapSrcF(gpsData->mapPathStr + "/sources.txt");
    mapSrcL = mapSrcF.makeMapSourceList();
    mapSrcW = new MapSourceWidget(vBox);
    mapSrcW->setMapSourceList(mapSrcL);

    spec = new DownloadSpecification(gpsData, mapInfo, mapList);        // pass the current gps data in here for suggested lat,long,name!
    dlSpecW = new DownloadSpecificationWidget(spec, vBox);

    resize(parent->geometry().size());
    vBox->resize(geometry().size());
}

DownLoadDialog::~DownLoadDialog()
{
};

void DownLoadDialog::accept()
{
    if (dlSpecW->accept())
    {
        spec->download(mapSrcL->at(mapSrcW->getSourceIndex()));
        QDialog::accept();
    }
}

ImportMapDialog::ImportMapDialog(QSortedList < MapBase > *, QWidget * parent,
                                 const char *name, bool modal,
                                 WFlags f):QDialog(parent, name, modal, f)
{
    imageSelected = FALSE;
    vBox = new QVBox(this);

    imageDialog =
        new FileSelector("image/*", vBox, tr("image dialog"), FALSE, FALSE);


    bg = new QVButtonGroup("", vBox);
    cpOrg = new QRadioButton(tr("copy image"), bg);
    delOrg = new QRadioButton(tr("remove original image"), bg);
    bg->setButton(1);


    connect(imageDialog, SIGNAL(fileSelected(const DocLnk &)),
            this, SLOT(docLnkSelected(const DocLnk &)));
    resize(parent->geometry().size());
    vBox->resize(geometry().size());
}
void ImportMapDialog::docLnkSelected(const DocLnk & d)
{
    imageSelected = TRUE;
    mapImageLnk = d;
}

ImportMapDialog::~ImportMapDialog()
{
};

MapParDialog::MapParDialog(MapBase *, QWidget * parent,
                           const char *name, bool modal,
                           WFlags f):QDialog(parent, name, modal, f)
{
    vBox = new QVBox(this);
    mapView = new QScrollView(vBox);
    mapWidget = new MapWidget(mapView);
    mapView->addChild(mapWidget);

    // this typecasting of parent is bad and this is why
    //mapWidget->resize(((FetchMap*)parent)->image->width(), ((FetchMap*)parent)->image->height());
    //mapWidget->setBackgroundPixmap(*(((FetchMap*)parent)->image));
    mapWidget->resize(((MapInfo *) parent)->image->width(),
                      ((MapInfo *) parent)->image->height());
    mapWidget->setBackgroundPixmap(*(((MapInfo *) parent)->image));

    hBox = new QHBox(vBox);
    projectionCB = new QComboBox(hBox);
    projectionCB->insertItem("LINEAR");
    projectionCB->insertItem("CEA");
    projectionCB->insertItem("UTM");
    projectionCB->insertItem("TM");
    projectionCB->insertItem("MERCATOR");
    projectionCB->insertItem("LAMBERT");
    projectionCB->insertItem("FRITZ");

    scaleL = new QLabel(tr("  scale 1:"), hBox);
    scaleLE = new QLineEdit("10000", hBox);
    scaleLE->setValidator(new QIntValidator(1, 100000000, scaleLE));

    point1HB = new QHBox(vBox);
    p1xyLVB = new QVBox(point1HB);
    x1L = new QLabel(tr(" x1"), p1xyLVB);
    y1L = new QLabel(tr(" y1"), p1xyLVB);
    p1xyLEVB = new QVBox(point1HB);
    x1LE = new QLineEdit("", p1xyLEVB);
    x1LE->setMaximumHeight(x1LE->fontMetrics().height() + 2);
    //x1LE->setValidator(new QIntValidator(0,((FetchMap*)parent)->image->width(),x1LE));
    x1LE->
        setValidator(new
                     QIntValidator(0, ((MapInfo *) parent)->image->width(),
                                   x1LE));
    y1LE = new QLineEdit("", p1xyLEVB);
    y1LE->setMaximumHeight(y1LE->fontMetrics().height() + 2);
    //y1LE->setValidator(new QIntValidator(0,((FetchMap*)parent)->image->height(),y1LE));
    y1LE->
        setValidator(new
                     QIntValidator(0, ((MapInfo *) parent)->image->height(),
                                   y1LE));
    p1llLVB = new QVBox(point1HB);
    lg1L = new QLabel(tr(" long1"), p1llLVB);
    lt1L = new QLabel(tr(" lat1"), p1llLVB);
    p1llLEVB = new QVBox(point1HB);
    lg1LE = new QLineEdit("", p1llLEVB);
    lg1LE->setMaximumHeight(lg1LE->fontMetrics().height() + 2);
    //lg1LE->setValidator(new QDoubleValidator(-180,180,7,lg1LE));
    // FIXME: write validators vor lat/longitude
    lt1LE = new QLineEdit("", p1llLEVB);
    lt1LE->setMaximumHeight(lt1LE->fontMetrics().height() + 2);
    //lt1LE->setValidator(new QDoubleValidator(-90,90,7,lt1LE));
    // FIXME: write validators vor lat/longitude

    point2HB = new QHBox(vBox);
    p2xyLVB = new QVBox(point2HB);
    x2L = new QLabel(tr(" x2"), p2xyLVB);
    y2L = new QLabel(tr(" y2"), p2xyLVB);
    p2xyLEVB = new QVBox(point2HB);
    x2LE = new QLineEdit("", p2xyLEVB);
    x2LE->setMaximumHeight(x2LE->fontMetrics().height() + 2);
    //x2LE->setValidator(new QIntValidator(0,((FetchMap*)parent)->image->width(),x2LE));
    x2LE->
        setValidator(new
                     QIntValidator(0, ((MapInfo *) parent)->image->width(),
                                   x2LE));
    y2LE = new QLineEdit("", p2xyLEVB);
    y2LE->setMaximumHeight(y2LE->fontMetrics().height() + 2);
    //y2LE->setValidator(new QIntValidator(0,((FetchMap*)parent)->image->height(),y2LE));
    y2LE->
        setValidator(new
                     QIntValidator(0, ((MapInfo *) parent)->image->height(),
                                   y2LE));
    p2llLVB = new QVBox(point2HB);
    lg2L = new QLabel(tr(" long2"), p2llLVB);
    lt2L = new QLabel(tr(" lat2"), p2llLVB);
    p2llLEVB = new QVBox(point2HB);
    lg2LE = new QLineEdit("", p2llLEVB);
    lg2LE->setMaximumHeight(lg2LE->fontMetrics().height() + 2);
    //lg2LE->setValidator(new QDoubleValidator(-180,180,7,lg2LE));
    // FIXME: write validators for lat/longitude
    lt2LE = new QLineEdit("", p2llLEVB);
    lt2LE->setMaximumHeight(lt2LE->fontMetrics().height() + 2);
    //lt2LE->setValidator(new QDoubleValidator(-90,90,7,lt2LE));
    // FIXME: write validators for lat/longitude

    stdLat12HB = new QHBox(vBox);
    stdLat1L = new QLabel(tr("stdLat1"), stdLat12HB);
    stdLat1LE = new QLineEdit("", stdLat12HB);
    stdLat1LE->setMaximumHeight(stdLat1LE->fontMetrics().height() + 2);
    //stdLat1LE->setValidator(new QDoubleValidator(-90,90,7,stdLat1LE));
    // FIXME: write validators for lat/longitude
    stdLat2L = new QLabel(tr("stdLat2"), stdLat12HB);
    stdLat2LE = new QLineEdit("", stdLat12HB);
    stdLat2LE->setMaximumHeight(stdLat2LE->fontMetrics().height() + 2);
    //stdLat2LE->setValidator(new QDoubleValidator(-90,90,7,stdLat2LE));
    // FIXME: write validators for lat/longitude
    refLongL = new QLabel(tr("refLong"), stdLat12HB);
    refLongLE = new QLineEdit("", stdLat12HB);
    refLongLE->setMaximumHeight(refLongLE->fontMetrics().height() + 2);
    //refLongLE->setValidator(new QDoubleValidator(-180,180,7,refLongLE));
    // FIXME: write validators for lat/longitude


    stdLongHB = new QHBox(vBox);
    stdLongL = new QLabel(tr("stdLongitude"), stdLongHB);
    stdLongLE = new QLineEdit("", stdLongHB);
    stdLongLE->setMaximumHeight(stdLongLE->fontMetrics().height() + 2);
    //stdLongLE->setValidator(new QDoubleValidator(-180,180,7,stdLongLE));
    // FIXME: write validators for lat/longitude

    centerHB = new QHBox(vBox);
    centerLatL = new QLabel(tr("center latitude"), centerHB);
    centerLatLE = new QLineEdit("", centerHB);
    //centerLatLE->setValidator(new QDoubleValidator(-90,90,7,centerLatLE));
    // FIXME: write validators for lat/longitude
    centerLongL = new QLabel(tr(" longitude"), centerHB);
    centerLongLE = new QLineEdit("", centerHB);
    //centerLongLE->setValidator(new QDoubleValidator(-180,180,7,centerLongLE));
    // FIXME: write validators for lat/longitude

    utmHB = new QHBox(vBox);
    zoneL = new QLabel(tr("Zone"), utmHB);
    zoneLE = new QLineEdit("", utmHB);

    connect(projectionCB, SIGNAL(highlighted(int)),
            this, SLOT(showProjectionPar(int)));
    connect(mapWidget, SIGNAL(mouseClick(int, int)),
            SLOT(clickPosition(int, int)));

    projectionCB->setCurrentItem(6);

    resize(parent->geometry().size());
    vBox->resize(geometry().size());
    scaleLE->setFocus();
}

MapParDialog::~MapParDialog()
{
};

void MapParDialog::showProjectionPar(int idx)
{
    // hide all
    point1HB->hide();
    point2HB->hide();
    stdLongHB->hide();
    stdLat12HB->hide();
    centerHB->hide();
    utmHB->hide();
    lt1L->setText(tr("lat1"));
    lg1L->setText(tr("long1"));
    lt2L->setText(tr("lat2"));
    lg2L->setText(tr("long2"));

    switch (idx)
    {
    case 0:
    case 1:
    case 4:
        // 2 ref points
        point1HB->show();
        point2HB->show();

        break;

    case 2:
        // UTM
        utmHB->show();
        point1HB->show();
        point2HB->show();
        lt1L->setText(tr("north1"));
        lg1L->setText(tr("east1"));
        lt2L->setText(tr("north2"));
        lg2L->setText(tr("east2"));
        break;

    case 3:
        // TM: 2 points + stdLong
        point1HB->show();
        point2HB->show();
        stdLongHB->show();
        break;

    case 5:
        // Lambert: 2 points + std1Lat, std2Lat, refLong
        point1HB->show();
        point2HB->show();
        stdLat12HB->show();
        break;

    case 6:
        // Fritz: center_latitude, center_longitude
        centerHB->show();
        break;
    }
}

void MapParDialog::clickPosition(int x, int y)
{
    if (x1LE->hasFocus() || y1LE->hasFocus())
    {
        x1LE->setText(tr("%1").arg(x));
        y1LE->setText(tr("%1").arg(y));
    }
    else if (x2LE->hasFocus() || y2LE->hasFocus())
    {
        x2LE->setText(tr("%1").arg(x));
        y2LE->setText(tr("%1").arg(y));
    }

    mapWidget->update();
}

/*RemoveMapDialog::RemoveMapDialog(QSortedList <MapBase>*maps, QWidget *parent=0,
				 const char *name=0, bool modal=FALSE,WFlags f=0):
    QDialog(parent,name,modal,f)
{
    vBox = new QVBox(this);
    mapSelect = new QComboBox(FALSE, vBox);
    bg = new QVButtonGroup("",vBox);
    remMap = new QRadioButton("remove map entry",bg);
    delMap = new QRadioButton("delete map image",bg);
    bg->setButton(0);
    MapBase *aMap;
    for (aMap = maps->first (); aMap != 0; aMap = maps->next ())
	mapNames.append(aMap->name);
    mapSelect->insertStringList(mapNames);
    resize(parent->geometry().size());
    vBox->resize(geometry().size());
}*/
RemoveMapDialog::RemoveMapDialog(MapBase * map, QWidget * parent,
                                 const char *name, bool modal,
                                 WFlags f):QDialog(parent, name, modal, f)
{
    vBox = new QVBox(this);
    mapNameL = new QLabel(map->name, vBox);
    mapNameL->setAlignment(Qt::AlignHCenter);
    QPalette pal = mapNameL->palette();
    pal.setColor(QColorGroup::Foreground, red);
    mapNameL->setPalette(pal);
    textL =
        new
        QLabel(tr("Warning!\nYou are about to remove this map from qpeGPS."),
               vBox);
    textL->setAlignment(Qt::AlignHCenter);
    deleteCB = new QCheckBox(tr("Delete map image file from storage"), vBox);
    resize(parent->geometry().size().width(), 100);
    vBox->resize(geometry().size());
}

RemoveMapDialog::~RemoveMapDialog()
{
};

ChangeMapParDialog::ChangeMapParDialog(QSortedList < MapBase > *maps, QWidget * parent = 0, const char *name = 0, bool modal = FALSE, WFlags f = 0):
QDialog(parent, name, modal, f)
{
    vBox = new QVBox(this);
    mapSelect = new QComboBox(FALSE, vBox);

    MapBase *aMap;
    for (aMap = maps->first(); aMap != 0; aMap = maps->next())
        mapNames.append(aMap->name);
    mapSelect->insertStringList(mapNames);
    resize(parent->geometry().size());
    vBox->resize(geometry().size());
}

ChangeMapParDialog::~ChangeMapParDialog()
{
};



void DownloadAreaWidget::setDownloadSpecification(DownloadAreaSpecification * dlSpec)   /* Added by A. Karkhov */
{
    spec = dlSpec;
    QString string;
    latitudeLE->setText(string.setNum(spec->latitude));
    longitudeLE->setText(string.setNum(spec->longitude));

    slatitudeLE->setText(string.setNum(spec->slatitude));
    slongitudeLE->setText(string.setNum(spec->slongitude));

    elatitudeLE->setText(string.setNum(spec->elatitude));
    elongitudeLE->setText(string.setNum(spec->elongitude));

    areaxLE->setText(string.setNum(spec->areax));
    areayLE->setText(string.setNum(spec->areay));

}

DownloadAreaSpecification::~DownloadAreaSpecification()
{
}                               /* Added by A. Karkhov */

DownloadAreaSpecification::DownloadAreaSpecification(GpsData * gData,
                                                     MapInfo * mapInfo,
                                                     QSortedList < MapBase >
                                                     *maps)
{
    gpsData = gData;
    mapList = maps;

    // default to current position from map info page
    longitude = mapInfo->mapPos.longitude();
    latitude = mapInfo->mapPos.latitude();

    slongitude = slatitude = elongitude = elatitude = 0;
    areax = areay = 200;
    prefix = "";
    IsArea = 1;
    dir = "";
    res = 0;

    if ((longitude == 0) && (latitude == 0))
    {
        // use current location from GPS
        longitude = gpsData->currPos.longitude();
        latitude = gpsData->currPos.latitude();
    }

    scale = 200000;
}


DownLoadAreaDialog::DownLoadAreaDialog(Qpegps * application, DownloadAreaSpecification * spec, GpsData * gData, MapInfo * mapInfo, QSortedList < MapBase > *maps, QWidget * parent,     /* Added by A. Karkhov */
                                       const char *name, bool modal, WFlags f):QDialog(parent, name, modal, f)  /* Added by A. Karkhov */
{
    gpsData = gData;
    mapList = maps;

    QVBox *vBox;
    vBox = new QVBox(this);

    dlSpecW = new DownloadAreaWidget(application, spec, vBox);

    resize(parent->geometry().size());
    vBox->resize(parent->geometry().size());

}

void DownLoadAreaDialog::accept()
{
    if (dlSpecW->accept())
    {
        QDialog::accept();
    }
}


DownLoadAreaDialog::~DownLoadAreaDialog()
{
}                               /* Added by A. Karkhov */

DownloadAreaWidget::~DownloadAreaWidget()
{
}                               /* Added by A. Karkhov */

DownloadAreaWidget::DownloadAreaWidget(Qpegps * appl, DownloadAreaSpecification * spec, QWidget * parent, const char *name)     /* Added by A. Karkhov */
:QVBox(parent, name)
{
    QHBox *hBox;
    application = appl;
    detailsGB = new QVGroupBox(this);
//#define detailsGB this
//      detailsGB = new QVBox(this);

    hBox = new QHBox(detailsGB);
    latitudeL = new QLabel(tr("Lat. (D.d):"), hBox);
    latitudeLE = new QLineEdit(hBox);
    latitudeLE->
        setValidator(new QDoubleValidator(-1000, 1000, 10, latitudeLE));
    longitudeL = new QLabel(tr(" Long.(D.d):"), hBox);
    longitudeLE = new QLineEdit(hBox);
    longitudeLE->
        setValidator(new QDoubleValidator(-1000, 1000, 10, longitudeLE));

    slatitudeL = new QLabel(tr("Start Lat. (D.d):"), hBox);
    slatitudeLE = new QLineEdit(hBox);
    slatitudeLE->
        setValidator(new QDoubleValidator(-1000, 1000, 10, latitudeLE));
    slongitudeL = new QLabel(tr("Start Long.(D.d):"), hBox);
    slongitudeLE = new QLineEdit(hBox);
    slongitudeLE->
        setValidator(new QDoubleValidator(-1000, 1000, 10, longitudeLE));

    hBox = new QHBox(detailsGB);
    PlacePB =
        new QPushButton("Select Cental Point", hBox, "Select Cental Point");
    connect(PlacePB, SIGNAL(clicked()), this, SLOT(PlaceSelC()));

//    hBox = new QHBox(detailsGB);

//    hBox = new QHBox(detailsGB);
    PlaceLBPB =
        new QPushButton("Select Left Buttom point coordinates", hBox,
                        "Select Left Buttom point coordinates");
    connect(PlaceLBPB, SIGNAL(clicked()), this, SLOT(PlaceSelLB()));

    hBox = new QHBox(detailsGB);
    elatitudeL = new QLabel(tr("End   Lat. (D.d):"), hBox);
    elatitudeLE = new QLineEdit(hBox);
    elatitudeLE->
        setValidator(new QDoubleValidator(-1000, 1000, 10, latitudeLE));
    elongitudeL = new QLabel(tr(" End  Long.(D.d):"), hBox);
    elongitudeLE = new QLineEdit(hBox);
    elongitudeLE->
        setValidator(new QDoubleValidator(-1000, 1000, 10, longitudeLE));

    areaxL = new QLabel(tr("    East-West  area  size  (km):"), hBox);
    areaxLE = new QLineEdit(hBox);
    areaxLE->
        setValidator(new QDoubleValidator(-20000, 20000, 10, longitudeLE));

    hBox = new QHBox(detailsGB);
    PlaceRTPB =
        new QPushButton("Select Right Top point coordinates", hBox,
                        "Select Right Top point coordinates");
    connect(PlaceRTPB, SIGNAL(clicked()), this, SLOT(PlaceSelRT()));

//    hBox = new QHBox(detailsGB);
    areayL = new QLabel(tr("    North-South area size (km):"), hBox);
    areayLE = new QLineEdit(hBox);
    areayLE->
        setValidator(new QDoubleValidator(-20000, 20000, 10, longitudeLE));

    hBox = new QHBox(detailsGB);
    scaleL = new QLabel(tr("Scale:"), hBox);
    scaleCB = new QComboBox(TRUE, hBox);
    scaleCB->setValidator(new QIntValidator(1000, 100000000, scaleCB));
    scaleCB->insertItem("4000");
    scaleCB->insertItem("10000");
    scaleCB->insertItem("20000");
    scaleCB->insertItem("30000");
    scaleCB->insertItem("50000");
    scaleCB->insertItem("100000");
    scaleCB->insertItem("200000");
    scaleCB->insertItem("500000");
    scaleCB->insertItem("3000000");
    scaleCB->insertItem("10000000");
    scaleCB->insertItem("30000000");
    scaleCB->setEditable(TRUE);
    scaleCB->setCurrentItem(5);
    resL = new QLabel(tr("  Resolution:"), hBox);
    resCB = new QComboBox(TRUE, hBox);
    resCB->insertItem("1332x1332");
    resCB->insertItem("1998x1998");
    resCB->setEditable(FALSE);
    resCB->setCurrentItem(1);

    SE_ABG = new QButtonGroup(2, Qt::Horizontal, detailsGB);
    ASB = new QRadioButton("Area size", SE_ABG);
    connect(ASB, SIGNAL(toggled(bool)), this, SLOT(toggledASB(bool)));
    SEB = new QRadioButton("Start-End coordinates", SE_ABG);
    connect(SEB, SIGNAL(toggled(bool)), this, SLOT(toggledSEB(bool)));
    ASB->setChecked(TRUE);
    hBox = new QHBox(detailsGB);
    dirL = new QLabel(tr("Download dir:"), hBox);
    dirCB = new QComboBox(TRUE, hBox);
    dirCB->insertItem(appl->gpsData().mapPathStr);
    QString s;
    s = appl->gpsData().mapPathStr;
    s.replace(QRegExp("/+"), "/");
    if (s != "/home/QtPalmtop/qpegps/maps/")
        dirCB->insertItem("/home/QtPalmtop/qpegps/maps/");
    dirCB->insertItem("/home/samba/Main_Memory/map_");
    dirCB->insertItem("/home/samba/SD_Card/map_");
    dirCB->insertItem("/home/samba/CF_Card/map_");
    dirCB->setEditable(TRUE);
    dirCB->setCurrentItem(0);

    hBox = new QHBox(detailsGB);
    paramL = new QLabel(tr("Params:"), hBox);
    paramLE = new QLineEdit(hBox);


    setDownloadSpecification(spec);
}

void DownloadAreaWidget::PlaceSelC()
{
    double PointLat, PointLong;

    PlaceSel(&PointLat, &PointLong);

    QString string;
    latitudeLE->setText(string.setNum(PointLat));
    longitudeLE->setText(string.setNum(PointLong));
}

void DownloadAreaWidget::PlaceSelLB()
{
    double PointLat, PointLong;

    PlaceSel(&PointLat, &PointLong);

    QString string;
    slatitudeLE->setText(string.setNum(PointLat));
    slongitudeLE->setText(string.setNum(PointLong));
}

void DownloadAreaWidget::PlaceSelRT()
{
    double PointLat, PointLong;

    PlaceSel(&PointLat, &PointLong);

    QString string;
    elatitudeLE->setText(string.setNum(PointLat));
    elongitudeLE->setText(string.setNum(PointLong));
}


void DownloadAreaWidget::PlaceSel(double *PointLat, double *PointLong)
{
    MapCoordEditorDialog mDialog(&(application->gpsData()), application->places, this, "edit map coord", TRUE, 0);   /*********//* Added by A. Karhov */
    *PointLat = *PointLong = 0;
    mDialog.setCaption(tr("Select Central Point"));
    if (mDialog.exec() != 0)
    {
        if (mDialog.mapSrcEditW->mapLatLonCB->currentItem())
        {
            int i = mDialog.mapSrcEditW->mapLatLonCB->currentItem();
            Places *curr = application->places;
            do
            {
                curr = (Places *) curr->next;
                i--;
                if (i == 0)     // destPointLat
                {
                    *PointLat = curr->pos.latitude();
                    *PointLong = curr->pos.longitude();
                    break;
                }
            }
            while (curr != NULL);
        }
        else
        {
            double g, m, s;
            g = mDialog.mapSrcEditW->LatiLEd->text().toDouble();
            m = copysign(mDialog.mapSrcEditW->LatiLEm->text().toDouble(), g);
            s = copysign(mDialog.mapSrcEditW->LatiLEs->text().toDouble(), g);
            *PointLat = (g + m / 60 + s / 3600);
            g = mDialog.mapSrcEditW->LonLEd->text().toDouble();
            m = copysign(mDialog.mapSrcEditW->LonLEm->text().toDouble(), g);
            s = copysign(mDialog.mapSrcEditW->LonLEs->text().toDouble(), g);
            *PointLong = (g + m / 60 + s / 3600);
        }
    }
}


void DownloadAreaWidget::toggledSEB(bool state)
{
    if (state)
    {
        IsArea = 0;
        latitudeL->hide();
        latitudeLE->hide();
        longitudeL->hide();
        longitudeLE->hide();
        areaxL->hide();
        areaxLE->hide();
        areayL->hide();
        areayLE->hide();
        PlacePB->hide();

        slatitudeL->show();
        slatitudeLE->show();
        slongitudeL->show();
        slongitudeLE->show();
        elatitudeL->show();
        elatitudeLE->show();
        elongitudeL->show();
        elongitudeLE->show();;
        PlaceLBPB->show();
        PlaceRTPB->show();
        repaint();
    }
}

void DownloadAreaWidget::toggledASB(bool state)
{
    if (state)
    {
        IsArea = 1;
        latitudeL->show();
        latitudeLE->show();
        longitudeL->show();
        longitudeLE->show();
        areaxL->show();
        areaxLE->show();
        areayL->show();
        areayLE->show();
        PlacePB->show();

        slatitudeL->hide();
        slatitudeLE->hide();
        slongitudeL->hide();
        slongitudeLE->hide();
        elatitudeL->hide();
        elatitudeLE->hide();
        elongitudeL->hide();
        elongitudeLE->hide();;
        PlaceLBPB->hide();
        PlaceRTPB->hide();
        repaint();
    }
}

bool DownloadAreaWidget::accept()
{
    spec->latitude = latitudeLE->text().toDouble();
    spec->longitude = longitudeLE->text().toDouble();
    spec->slatitude = slatitudeLE->text().toDouble();
    spec->slongitude = slongitudeLE->text().toDouble();
    spec->elatitude = elatitudeLE->text().toDouble();
    spec->elongitude = elongitudeLE->text().toDouble();
    spec->areax = areaxLE->text().toDouble();
    spec->areay = areayLE->text().toDouble();
    spec->scale = scaleCB->currentText().toULong();
    spec->IsArea = IsArea;
    spec->dir = dirCB->currentText();
    spec->param = paramLE->text();
    spec->res = resCB->currentItem();
    return 1;
}

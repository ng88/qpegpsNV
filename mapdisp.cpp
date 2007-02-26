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
#include "mapdisp.h"
#include "route.h"
#include <math.h>
#include <qdatetime.h>
#include <qpopupmenu.h>

#include "mathex.h"

MapDisp::MapDisp(Qpegps * appl, QSortedList < MapBase > *mapList,
                 QWidget * parent, const char *name,
                 WFlags fl):QWidget(parent, name, fl)
{

    application = appl;
    gpsData = &(application->gpsData());
    mapdisp = new QPixmap();
    map = new QImage();

    maps = mapList;

    actmap = 0;
    createMap();
    setBackgroundMode(NoBackground);

    mPointLat = 0;              /* Added by A. Karhov */
    mPointLong = 0;
    centerX = 0;
    centerY = 0;
    accDist = 0;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerDone()));

//      atimer = new QTimer( this );
//      connect( atimer, SIGNAL(timeout()), this, SLOT(animateTrack()));

    selectedScale = 0;
    actMapList.setAutoDelete(false);

    connect(this, SIGNAL(lessDetail()), SLOT(chooseLessDetailedMap()));
    connect(this, SIGNAL(moreDetail()), SLOT(chooseMoreDetailedMap()));
    connect(this, SIGNAL(debugMaps()), SLOT(showAvailableMaps()));

    setFocusPolicy(QWidget::StrongFocus);
    setFocus();                 /* Added by A. Karhov */
    ManualWp = false;
    destPointLong = destPointLat = 0;
//      WTconunter=0;
    WTTimer = time(NULL);
    WriteTimeout = 1200;
    FirstKey = true;
    inanimation = false;
    timeZone = 0;
    timeAccelerator = 300;

}

MapDisp::~MapDisp()
{
    delete map;
    delete mapdisp;
}

double MapDisp::coverage(MapBase * aMap, double x, double y, int width,
                         int height)
{
    // determines the area on the display, which is not covered by aMap
    // 0 = map fills the display
    // <0 = -1 * pixels without map

    double coverX, coverY;

    if (x > aMap->mapSizeX - x)
        coverX = aMap->mapSizeX - x;
    else
        coverX = x;
    if (y > aMap->mapSizeY - y)
        coverY = aMap->mapSizeY - y;
    else
        coverY = y;
    coverX -= width / 2;
    coverY -= height / 2;
    if (coverX > 0)
        coverX = 0;
    else
        coverX *= height;
    if (coverY > 0)
        coverY = 0;
    else
        coverY *= width;

    return coverX + coverY;
}

void MapDisp::chooseLessDetailedMap()
{
    MapBase *aMap;

    if (actmap)
    {
        if (actmap->scale < actMapList.last()->scale)
        {
            aMap = actMapList.first();
            while (aMap
                   && (aMap->scale < /*actmap->scale */ selectedScale + 1))
            {
                aMap = actMapList.next();
            }
            if (aMap)
                selectedScale = aMap->scale;
            else
                selectedScale = actmap->scale;
        }
        else
            selectedScale = actmap->scale;
    }
}

void MapDisp::chooseMoreDetailedMap()
{
    MapBase *aMap;

    if (actmap && actmap->scale > actMapList.first()->scale)
    {
        aMap = actMapList.last();
        while (aMap && (aMap->scale > /*actmap->scale */ selectedScale - 1))
            aMap = actMapList.prev();
        if (aMap)
            selectedScale = aMap->scale;
    }
    else
        selectedScale = 0;
}

void MapDisp::showAvailableMaps()
{
    MapBase *aMap;

    qDebug(tr("Maps at current position:"));
    aMap = actMapList.first();
    while (aMap)
    {
        qDebug(tr("%1 %2").arg(aMap->scale).arg(aMap->name));
        aMap = actMapList.next();
    }
}

void MapDisp::clearActMapList()
{
    actMapList.clear();
    actmap = 0;
}

double x2(double x)
{
    return x * x;
}                               /* Added by A. Karhov */
double calcAngle(double destPointLat, double destPointLong, double t,
                 double g)
{
    double a, b, angle;         /* Added by A. Karhov */
    /*a = acos(cos(destPointLat) * cos(g) * cos(t) * cos(g) +
             cos(destPointLat) * sin(g) * cos(t) * sin(g) +
             sin(destPointLat) * sin(t)) * 6378;
    b = acos(cos(t) * cos(destPointLong) * cos(t) * cos(g) +
             cos(t) * sin(destPointLong) * cos(t) * sin(g) +
             sin(t) * sin(t)) * 6378;*/

    a = MathEx::computeDistanceLtLgRad(destPointLat, g, t, g);
    b = MathEx::computeDistanceLtLgRad(t, destPointLong, t, g);
    
    if (a != 0 || b != 0)
    {
        angle = acos(a / sqrt(x2(a) + x2(b))) * 180.0 / MathEx::PI;
        if (destPointLong < g)
        {
            angle = angle + 180;
            if (destPointLat > t)
                angle = (360 + 180) - angle;
        }
        else if (destPointLat < t)
            angle = 180 - angle;
    }
    else
        angle = 0;
    return angle;
}

void MapDisp::createMap()
{

    QRect tempRect;
    QPainter painter;
    bool mapavailable = false, mapchanged = false;
    double lg, lt;
    double xwp = 0, ywp = 0;
    int tx, ty, tw, th;
    double xc, yc, cover, scale, tcover;
    MapBase *aMap, *theMap;

    if (FirstKey)
    {
        emit mouseClick(this);
        emit mouseClick(this);
        FirstKey = false;
    }

    /* Debug only */
//      gpsData->currPos.longitude=37.62325; gpsData->currPos.latitude=55.75729; gpsData->altitude.altitude=111.1;      *gpsData->statusStr="";


    lg = MathEx::deg2rad(gpsData->currPos.longitude());
    lt = MathEx::deg2rad(gpsData->currPos.latitude());

    mapdisp->resize(geometry().size());


    // find map
    // and get position in map coordinates

    // check if the actual maps are still valid
    aMap = actMapList.first();
    while (aMap)
    {
        if (!aMap->calcxy(&xc, &yc, lg, lt))
        {
            actMapList.take();
            aMap = actMapList.current();        //next item
        }
        else
            aMap = actMapList.next();
    }

    // check all maps, if actMapList is empty !
    if (actMapList.isEmpty())
    {
        aMap = maps->first();
        while (aMap)
        {
            if (aMap->calcxy(&xc, &yc, lg, lt))
                actMapList.append(aMap);
            aMap = maps->next();
        }
        aMap = maps->first();   // set to start, for following checks
    }
    else
    {
        // check several maps (not all!) from global list, if they would fit the current position
        unsigned int numberOfChecks;
        numberOfChecks = maps->count() / 10;
        if (numberOfChecks > 20)
            numberOfChecks = 20;
        else if (numberOfChecks < 1 && !maps->isEmpty())
            numberOfChecks = 1;
        aMap = maps->current();
        bool mapAppended = false;
        while (numberOfChecks)
        {
            if (aMap && !actMapList.containsRef(aMap)
                && aMap->calcxy(&xc, &yc, lg, lt))
            {
                mapAppended = true;
                actMapList.append(aMap);
            }
            if (aMap)
                aMap = maps->next();
            if (!aMap)
                aMap = maps->first();
            numberOfChecks--;
        }
        if (mapAppended)
            actMapList.sort();
    }

    if (!actMapList.isEmpty())
    {
        mapavailable = true;
        // get Map with best scale + coverage
        // list is still sorted by scale
        // scroll to the map, with appropriate scale
        aMap = actMapList.first();
        scale = aMap->scale;
        while (aMap && (aMap->scale < 0.9 * selectedScale))
        {
            aMap = actMapList.next();
        };
        // all maps have a lower scale => take map with highest scale
        if (!aMap)
            aMap = actMapList.last();

        theMap = aMap;
        scale = aMap->scale;
        // get map with best coverage with approx. the same scale
        aMap->calcxy(&xc, &yc, lg, lt);
        cover = coverage(aMap, xc, yc, mapdisp->width(), mapdisp->height());
        aMap = actMapList.next();
        while (aMap && (cover < -1) && (2.1 * scale > aMap->scale))
        {
            aMap->calcxy(&xc, &yc, lg, lt);
            tcover =
                coverage(aMap, xc, yc, mapdisp->width(), mapdisp->height());
            if (tcover > cover)
            {
                theMap = aMap;
                cover = tcover;
            }
            aMap = actMapList.next();
        };

        if (theMap != actmap)
        {
            mapchanged = true;
            actmap = theMap;
        }
    }

    // get map data !!!!

    if (mapavailable)
    {
        actmap->calcxy(&xc, &yc, lg, lt);
        xc = rint(xc);
        yc = rint(yc);

        centerX = (int) xc;     /* Added by A. Karhov */
        centerY = (int) yc;

        if (mapchanged)
        {
            //if(map) delete(map);
            delete map;
            QString mapfilename = gpsData->mapPathStr + "/" + actmap->name;
            map = new QImage(mapfilename);
            if (!map)
            {
                mapavailable = false;
                qWarning(tr("Couldn't open mapfile %1").arg(mapfilename));
            }
        }
        if (map)
        {
            // get waypoint in display coordinates
            if (ManualWp)       /* Added by A. Karhov */
            {
                lg = destPointLong;
                lt = destPointLat;
            }
            else
            {
                lg = MathEx::deg2rad(gpsData->wpPos.longitude());
                lt = MathEx::deg2rad(gpsData->wpPos.latitude());
            }
            actmap->calcxy(&xwp, &ywp, lg, lt);
            xwp = rint(xwp - xc);
            ywp = rint(ywp - yc);

            //  get part of map, with center xc,yc
            QImage tempImage(mapdisp->width(), mapdisp->height(), 8);
            tempImage =
                map->copy((int) xc - mapdisp->width() / 2,
                          (int) yc - mapdisp->height() / 2, mapdisp->width(),
                          mapdisp->height());

            mapdisp->convertFromImage(tempImage);
        }
        else
            map = new QImage();
    }
    else
    {
        actmap = 0;
        mapdisp->fill();
    }
    tempRect = mapdisp->rect();
    painter.begin(mapdisp);
    QFont f = painter.font();
    f.setPointSize(gpsData->textSize);
    painter.setFont(f);
    QPen p = painter.pen();
    p.setWidth(1);
    if (gpsData->status)
        statColor = gpsData->statusOkColor;
    else
        statColor = gpsData->statusNoFixColor;
    p.setColor(statColor);
    painter.setPen(p);
//
    int xcenter = mapdisp->width() / 2; /* Added by A. Karhov */
    int ycenter = mapdisp->height() / 2;

    if (gpsData->statusStr.length() > 0)
    {

        painter.drawText(tempRect, AlignVCenter | AlignHCenter, gpsData->statusStr);

        if (application->route->drawEnabled())
            application->route->draw(&painter, actmap,
                                        (int) (xc - xcenter),
                                        (int) (yc - ycenter),
                                        (int) (xc + xcenter),
                                        (int) (yc + ycenter));

        /* Added by A. Karhov */
        if (application->track)
        {
            application->track->drawTrack(&painter, actmap,
                                          (int) (xc - xcenter),
                                          (int) (yc - ycenter),
                                          (int) (xc + xcenter),
                                          (int) (yc + ycenter));
            if ((time(NULL) - WTTimer) >= WriteTimeout)
            {
                application->track->Write(application->track->wLog->
                                          currentText());
                WTTimer = time(NULL);
            }
        }
        painter.drawText(tempRect, AlignVCenter | AlignLeft, gpsData->currPos.latToString()); 
        painter.drawText(tempRect, AlignVCenter | AlignRight, gpsData->currPos.longToString());

        if (!inanimation)
        {
            tx = ty = th = 99;
            painter.drawText(tempRect, AlignTop | AlignHCenter, gpsData->altitude.toString());  /* Added by A. Karhov */
        }
        else
        {
            painter.drawText(tempRect, AlignTop | AlignHCenter, gpsData->altitude.toString());  /* Added by A. Karhov */
            tx = currtime / (60 * 60 * 1000);
            ty = (currtime - tx * (60 * 60 * 1000)) / 60000;
            th = (currtime - tx * (60 * 60 * 1000) - ty * 60000) / 1000;
        }
        painter.drawText(tempRect, AlignBottom | AlignHCenter, tr("%1 %2\n%3 %4 %5:%6:%7")
                          .arg("0")     //gpsData->heading.toString())
                         .arg("0.0") //gpsData->speed.toString())
                         .arg("0")      //Bearing.toString())
                         .arg("0.000")      //dist.toString())
                         .arg(tx)
                         .arg(ty)
                         .arg(th));     //gpsData->timeToString()) );
        f.setPixelSize(f.pixelSize() - 2);
        painter.setFont(f);
    }
    else
    {
//        int xcenter = mapdisp->width()/2; /* Removed by A. Karhov */
//        int ycenter = mapdisp->height()/2;
        int lgth;
        if (xcenter < ycenter)
            lgth = xcenter / 6;
        else
            lgth = ycenter / 6;
        //cross
        painter.drawLine(xcenter, ycenter, xcenter, ycenter + lgth);
        painter.drawLine(xcenter - lgth, ycenter, xcenter + lgth, ycenter);
        p.setWidth(2);
        painter.setPen(p);
        painter.drawLine(xcenter, ycenter - lgth, xcenter, ycenter);
        // bearing line
        if (gpsData->bearing.show())
        {
            if (!mapavailable
                || ((xwp + xcenter > mapdisp->width() - lgth / 2.0)
                    || (xwp + xcenter < lgth / 2.0)
                    || (ywp + ycenter > mapdisp->height() - lgth / 2.0)
                    || (ywp + ycenter < lgth / 2.0)))
            {
                tx = xcenter;
                ty = ycenter;
                double BearingAngle;
                if (!ManualWp)  /* Added by A. Karhov */
                {
                    BearingAngle = MathEx::deg2rad(gpsData->bearing.toDouble());
                }
                else            /* Added by A. Karhov */
                {
                    BearingAngle =
                        MathEx::deg2rad( calcAngle(destPointLat, destPointLong,
                                                   MathEx::deg2rad(gpsData->currPos.latitude()),
                                                   MathEx::deg2rad(gpsData->currPos.longitude())) );
                }
                tw = xcenter +
                    (int)
                    rint((double) (sin(BearingAngle) * (double) lgth * 3.0));
                th = ycenter -
                    (int)
                    rint((double) (cos(BearingAngle) * (double) lgth * 3.0));

                p.setWidth(2);
                p.setColor(gpsData->bearColor);
                painter.setPen(p);
                painter.drawLine(tx, ty, tw, th);
            }
            else
            {
                tx = xcenter;
                ty = ycenter;
                tw = xcenter + (int) xwp;
                th = ycenter + (int) ywp;
                p.setWidth(2);
                p.setColor(gpsData->bearColor);
                painter.setPen(p);
                painter.drawLine(tx, ty, tw, th);

                tx = xcenter + (int) (xwp - lgth / 4);
                ty = ycenter + (int) (ywp - lgth / 4);
                tw = (int) lgth / 2;
                th = (int) lgth / 2;
                painter.drawRect(tx, ty, tw, th);
            }
        }
        // heading line
        if (gpsData->heading.show())
        {
            tx = xcenter;
            ty = ycenter;
            tw = xcenter +
                (int) ((double)
                       (sin(MathEx::deg2rad(gpsData->heading.toDouble())) *
                        (double) lgth * 2.0));
            th = ycenter -
                (int) ((double)
                       (cos(MathEx::deg2rad(gpsData->heading.toDouble())) *
                        (double) lgth * 2.0));
            p.setColor(gpsData->headColor);
            p.setWidth(3);
            painter.setPen(p);
            painter.drawLine(tx, ty, tw, th);

        }

        if (application->route->drawEnabled())
            application->route->draw(&painter, actmap,
                                    (int) (xc - xcenter),
                                    (int) (yc - ycenter),
                                    (int) (xc + xcenter),
                                    (int) (yc + ycenter));

        if (application->track)
        {
            application->track->drawTrack(&painter, actmap,
                                          (int) (xc - xcenter),
                                          (int) (yc - ycenter),
                                          (int) (xc + xcenter),
                                          (int) (yc + ycenter));
            if ((time(NULL) - WTTimer) >= WriteTimeout)
            {
                application->track->Write(application->track->wLog->
                                          currentText());
                WTTimer = time(NULL);
            }
        }
        p.setColor(statColor);
        painter.setPen(p);
        if (!mapavailable)
        {
            painter.drawText(tempRect, AlignVCenter | AlignLeft, gpsData->currPos.latToString()); 
            painter.drawText(tempRect, AlignVCenter | AlignRight, gpsData->currPos.longToString());
        }
        QString mapSelectString;
        if (selectedScale > 1)  // !=0
        {
            if (actmap && actmap->scale < actMapList.last()->scale)
                mapSelectString.append(tr("-/"));
            else
                mapSelectString.append(tr(" /"));
            if (actmap && actmap->scale > actMapList.first()->scale)
                mapSelectString.append(tr("+"));
            else
                mapSelectString.append(tr("D"));
        }
        //if(actmap) // only for debugging, don't forget to remove this !!!!
        //QTextOStream(&mapSelectString) << mapSelectString << actmap->scale << " " << selectedScale;

        painter.drawText(tempRect, AlignTop | AlignLeft, mapSelectString);

        Distance dist;
        dist.setFromDouble(gpsData->wpDistance.toDouble());
        dist.distUnit = gpsData->wpDistance.distUnit;

        Angle Bearing;
        Bearing.setFromDouble(gpsData->bearing.toDouble());

        if (ManualWp)           /* Added by A. Karhov */
        {
            if (gpsData->wpDistance.distUnit != Distance::None)
                dist.distUnit = gpsData->wpDistance.distUnit;
            else
                dist.distUnit = Distance::Km;
            lt = MathEx::deg2rad(gpsData->currPos.latitude());
            lg = MathEx::deg2rad(gpsData->currPos.longitude());

            /*dist.distance =
                acos(cos(destPointLat) * cos(destPointLong) * cos(lt) *  cos(lg) +
                     cos(destPointLat) * sin(destPointLong) * cos(lt) * sin(lg) +
                     sin(destPointLat) * sin(lt)) * 6378;*/
            dist.setFromDouble( MathEx::km2nmi( MathEx::computeDistanceLtLgRad(destPointLat, destPointLong, lt, lg) ) );

            gpsData->wpDistance.setFromDouble( dist.toDouble() );
            Bearing.setFromDouble( calcAngle(destPointLat, destPointLong,
                                            MathEx::deg2rad(gpsData->currPos.latitude()),
                                            MathEx::deg2rad(gpsData->currPos.longitude()) ) );
        }
        painter.drawText(tempRect, AlignTop | AlignHCenter,
                         gpsData->altitude.toString());
        painter.drawText(tempRect, AlignBottom | AlignHCenter,
                         tr("%1 %2\n%3 %4 %5").arg(gpsData->heading.toString())
                                              .arg(gpsData->speed.toString())
                                              .arg(Bearing.toString())
                                              .arg(dist.toString())
                                              .arg(gpsData->timeToString()));
        f.setPixelSize(f.pixelSize() - 2);
        painter.setFont(f);

    }
    /* Added by A. Karhov */
    double dlg, x, y, x1, y1;   /* Draw scale line */
    QString sc;
    if (cos(MathEx::deg2rad(gpsData->currPos.latitude())) != 0 && actmap)
    {
        dlg = (2.0 * MathEx::PI) * (actmap->scale / 100000.0) / (40074.16 * 
                        cos(MathEx::deg2rad(gpsData->currPos.latitude())));
        if (dlg != 0)
        {
            QPen pen = painter.pen();
            
            double ltc = MathEx::deg2rad(gpsData->currPos.latitude());
            double lgc = MathEx::deg2rad(gpsData->currPos.longitude());
            
            actmap->calcxy(&x, &y, lgc + dlg, ltc);
            actmap->calcxy(&x1, &y1, lgc, ltc);

            x = fabs(x - x1);
//      painter.save();
            pen.setColor(gpsData->scaleColor);
            pen.setWidth(3);
            painter.setPen(pen);
            f.setPointSize(16);
            painter.setFont(f);
            painter.drawLine(xcenter - mapdisp->width() / 2 + 6,
                             ycenter + mapdisp->height() / 2 - 19,
                             (int) ceil(x) + 2 + xcenter -
                             mapdisp->width() / 2 + 6,
                             ycenter + mapdisp->height() / 2 - 19);
            sc = Distance::toStringFromKm(actmap->scale / 100000.0);
            painter.drawText(tempRect, AlignBottom | AlignLeft, sc);    /* Added by A. Karhov *//* End of Drawing scale line  */

            /* Added by A. Karhov version after 0.9.3.1 *//* Drawing Waypoints */

            double minlt, minlg, maxlt, maxlg;
            if (gpsData->draw_places)
            {
                Places *curr = application->places;
                actmap->calcltlg(&lt, &lg, xc + mapdisp->width() / 2,
                                 yc + mapdisp->height() / 2);
                actmap->calcltlg(&xwp, &ywp, xc - mapdisp->width() / 2,
                                 yc - mapdisp->height() / 2);
                if (lt < xwp)
                {
                    maxlt = MathEx::rad2deg(xwp);
                    minlt = MathEx::rad2deg(lt);
                }
                else
                {
                    minlt = MathEx::rad2deg(xwp);
                    maxlt = MathEx::rad2deg(lt);
                }
                if (lg < ywp)
                {
                    maxlg = MathEx::rad2deg(ywp);
                    minlg = MathEx::rad2deg(lg);
                }
                else
                {
                    minlg = MathEx::rad2deg(ywp);
                    maxlg = MathEx::rad2deg(lg);
                }
                do
                {
                    if ((curr->pos.longitude() > minlg)
                        && (curr->pos.longitude() < maxlg)
                        && (curr->pos.latitude() > minlt)
                        && (curr->pos.latitude() < maxlt))
                    {
                        actmap->calcxy(&xwp, &ywp,
                                       MathEx::deg2rad(curr->pos.longitude()),
                                       MathEx::deg2rad(curr->pos.latitude()));
                        xwp = rint(xwp - xc) + xcenter; // conversion in screen ciirdinates
                        ywp = rint(ywp - yc) + ycenter;
                        tx = int (xwp);
                        ty = int (ywp);

                        if (tx < mapdisp->width() - 2 && ty > 16)
                        {
                            pen.setColor(gpsData->waypointColor);
                            pen.setWidth(4);
                            painter.setPen(pen);
                            painter.drawRect(tx - 1, ty - 1, 2, 2);
                            pen.setWidth(3);
                            painter.setPen(pen);
                            f.setPointSize(16);
                            painter.setFont(f);
                            painter.drawText(tx + 3, ty, *curr->name);
                            //WAYPOINT / POI
                        }
                    }
                    curr = (Places *) curr->next;
                }
                while (curr != NULL);
            }
            /* end of Drawing waypoints *//* End Added by A. Karhov version after 0.9.3.1 */
            painter.setPen(p);
//      painter.restore();
        }
    }

    painter.end();
}

void MapDisp::paintEvent(QPaintEvent *)
{
    createMap();
    bitBlt(this, 0, 0, mapdisp);
}

void MapDisp::timerDone()       // Added by A. Karhov
{
    if (gpsData->ManualPosit)
    {
        if (actmap)
            MapDispMesure(mouseEventX, mouseEventY);
    }
    else
    {
        if (actmap)
            MapDispDist(mouseEventX, mouseEventY);
    }
}

void MapDisp::mousePressEvent(QMouseEvent * e)  // Added by A/ Karhov
{
    noDBLC = true;

    if (timer->isActive())
    {
        timer->stop();
        noDBLC = false;
        swichMode();
    }
    mouseEventX = e->x();
    mouseEventY = e->y();
}

void MapDisp::mouseReleaseEvent(QMouseEvent * e)        // Added by A/ Karhov
{
    if ((abs(mouseEventX - e->x()) + abs(mouseEventY - e->y()) > 8)
        && gpsData->ManualPosit)
    {
        double lt, lg, lt1, lg1;
        if (actmap)
        {
            if (actmap->
                calcltlg(&lt, &lg,
                         (double) ((centerX - mapdisp->width() / 2) + e->x()),
                         (double) ((centerY - mapdisp->height() / 2) +
                                   e->y()))
                && actmap->calcltlg(&lt1, &lg1,
                                    (double) ((centerX -
                                               mapdisp->width() / 2) +
                                              mouseEventX),
                                    (double) ((centerY -
                                               mapdisp->height() / 2) +
                                              mouseEventY)))
            {
                if (timer->isActive())
                    timer->stop();
                    
                gpsData->currPos.setLong( gpsData->currPos.longitude() + MathEx::rad2deg(lg1 - lg) );
                gpsData->currPos.setLat( gpsData->currPos.latitude() + MathEx::rad2deg(lt1 - lt) );
            }
        }
    }
    else if (noDBLC)
    {
        timer->start(200, true);
        noDBLC = true;
    }
}

void MapDisp::swichMode()       /* Added by A/ Karhov */
{
    if (gpsData->ManualPosit)
    {
        gpsData->ManualPosit = false;
        gpsData->statusStr = "";
        gpsData->currPos.setLat( gpsData->latitudeGps );
        gpsData->currPos.setLong( gpsData->longitudeGps );
        mapdisp->resize(geometry().size());
    }
    else
    {
        gpsData->ManualPosit = true;
        gpsData->statusStr = "X";
        mapdisp->resize(geometry().size());
    }
}

 /* Added by A/ Karhov */
//void MapDisp::mouseDoubleClickEvent(QMouseEvent *) {  timer->stop();  swichMode(); }

void MapDisp::MapDispDist(int mouseEventX, int mouseEventY)
{
    double lt, lg, dist;
    int mx, my;
    QString posInfoStr, pos2InfoStr, DistStr;
    mx = (centerX - mapdisp->width() / 2) + mouseEventX;
    my = (centerY - mapdisp->height() / 2) + mouseEventY;
    if (actmap->calcltlg(&lt, &lg, (double) mx, (double) my))   // lt & lg in RAD
    {

        pos2InfoStr = tr("Lat=%1, Lon=%2").arg(gpsData->currPos.latToString())
                                          .arg(gpsData->currPos.longToString());
                                          
        posInfoStr = tr("Lat=%1, Lon=%2").arg(Position::latToString(MathEx::rad2deg(lt)))
                                         .arg(Position::longToString(MathEx::rad2deg(lg)));
        
        
        /*dist =
       acos(cos(gpsData->currPos.latitude * MathEx::PI / 180.0) * cos(gpsData->currPos.longitude * MathEx::PI / 180.0) * cos(lt) * cos(lg) +
       cos(gpsData->currPos.latitude * MathEx::PI / 180.0) * sin(gpsData->currPos.longitude * MathEx::PI / 180.0) * cos(lt) * sin(lg) +
        sin(gpsData->currPos.latitude * MathEx::PI / 180.0) * sin(lt)) * 6378;*/

        dist = MathEx::computeDistanceLtLgRad(MathEx::deg2rad(gpsData->currPos.latitude()),
                                              MathEx::deg2rad(gpsData->currPos.longitude()), lt, lg);
        
        DistStr = tr("\n\n Distance (aprox.)= %1").arg(dist, 0, 'f', 2);        //.arg(dist1, 0, 'f',3);

        switch (QMessageBox::
                information(this, "Pont coordinates",
                            "Current position: " + pos2InfoStr +
                            "\nClicked point: " + posInfoStr + DistStr +
                            "km." +
                            "\nPress:\n \"Cancel\" - Quit,\n \"Destination\" / \"OK\" - Set clicked point as distination,\n \"Save\" - save place.\n\nSave? or Set Destination?",
                            "Destination", "Save", "Cancel", 0, 2))
        {
        case 0:                /* Set Destination */
            destPointLat = lt;
            destPointLong = lg;
            ManualWp = true;
            break;
        case 1:                /* Save  */
            MapDispAddPos(lg * 180.0 / MathEx::PI, lt * 180.0 / MathEx::PI, 0);
            break;
        case 2:                /* Cancel */
            break;
        }
    }
//    setFocus();
}

void MapDisp::MapDispMesure(int mouseEventX, int mouseEventY)
{
    double lt, lg, dist;
    int mx, my;
    QString posInfoStr, pos2InfoStr, DistStr;
    mx = (centerX - mapdisp->width() / 2) + mouseEventX;
    my = (centerY - mapdisp->height() / 2) + mouseEventY;
    if (actmap->calcltlg(&lt, &lg, (double) mx, (double) my))   // lt & lg in RAD
    {
        posInfoStr =
            tr("Lat=%1, Lon=%2").arg(Position::latToString(MathEx::rad2deg(lt)))
                                .arg(Position::longToString(MathEx::rad2deg(lg)));
                                
        if (mPointLat || mPointLong)
        {
            pos2InfoStr =
                tr("Lat=%1, Lon=%2").arg(Position::latToString(MathEx::rad2deg(mPointLat)))
                                    .arg(Position::longToString(MathEx::rad2deg(mPointLong)));
            /*dist =
                acos(cos(mPointLat) * cos(mPointLong) * cos(lt) * cos(lg) +
                     cos(mPointLat) * sin(mPointLong) * cos(lt) * sin(lg) +
                     sin(mPointLat) * sin(lt)) * 6378;*/
                     
            dist =  MathEx::computeDistanceLtLgRad(mPointLat, mPointLong, lt, lg);
                 
            if (accDist == 0)
                DistStr = tr("\n\n Distance (aprox.)= %1").arg(dist, 0, 'f', 2);        //.arg(dist1, 0, 'f',3);
            else
                DistStr = tr("\n\nDistance(aprox.) = %1 + %2 = %3").arg(accDist, 0, 'f', 2).arg(dist, 0, 'f', 2).arg(dist + accDist, 0, 'f', 2);        //.arg(dist1, 0, 'f',3);

            switch (QMessageBox::
                    information(this, "Mesuring result",
                                "Prev. point: " + pos2InfoStr +
                                "\nLast point : " + posInfoStr + DistStr +
                                "km." +
                                "\nPress:\n \"Cancel\" - Quit,\n \"Measure\" / \"OK\" - measure summary distance to next point,\n \"Save&Measure\" - save place and measure distance.\n\nSave&Measure? or Measure (distance)?",
                                "Measure", "Save&&Measure", "Cancel", 0, 2))
            {
            case 0:            /* Measure */
                mPointLat = lt; // in RAD
                mPointLong = lg;        // in RAD
                accDist = accDist + dist;
                break;
            case 1:            /* Save & Measure  */
                MapDispAddPos(MathEx::rad2deg(lg), MathEx::rad2deg(lt), 0);
                mPointLat = lt; // in RAD
                mPointLong = lg;        // in RAD
                accDist = accDist + dist;
                break;
            case 2:            /* Cancel */
                accDist = 0;
                mPointLat = 0;
                mPointLong = 0;
                break;
            }
        }
        else
        {
            switch (QMessageBox::information(this, "Coordinates", posInfoStr +
                                             "\nPress:\n \"Cancel\" - Quit,\n \"Save\" / \"OK\" - save place,\n \"Measure\" - measure distance.\n\nSave (place)? or Measure (distance)?",
                                             "Save", "Measure", "Cancel", 0,
                                             2))
            {
            case 0:
                {
                    MapDispAddPos(MathEx::rad2deg(lg), MathEx::rad2deg(lt), 0);
                }
                break;
            case 1:
                mPointLat = lt; // in RAD
                mPointLong = lg;        // in RAD
                break;
            case 2:
                break;
            }
        }
    }
//    setFocus();
}

void MapDisp::keyPressEvent(QKeyEvent * event)
{



    if (gpsData->ManualPosit)
    {                           /* Added by A/ Karhov */
        switch (event->key())
        {
        case Qt::Key_F33:
        case Qt::Key_Space:    // Select
        case Qt::Key_Escape:   // Select
            emit mouseClick(this);
            break;

        case Qt::Key_Return:
            if (event->state() && ShiftButton)
            {
                MapDispAddPos(gpsData->currPos.longitude(),
                              gpsData->currPos.latitude(),
                              gpsData->altitude.toDouble());
            }
            else
            {
                QPopupMenu *pMenu = new QPopupMenu();
                pMenu->insertItem(tr("Go to Place/Waypoint"), 0, 0);
                pMenu->insertItem(tr("Save Place/Waypoint"), 1, 1);
                if (gpsData->draw_places)
                {
                    pMenu->insertItem(tr("Don't draw places"), 2, 2);
                }
                else
                {
                    pMenu->insertItem(tr("Draw places"), 2, 2);
                }
                pMenu->insertItem(tr("Change View"), 3, 3);
                if (inanimation)
                    pMenu->insertItem(tr("Stop track animation"), 4, 4);
                else
                    pMenu->insertItem(tr("Animate track"), 4, 4);
                pMenu->insertItem(tr("Switch to GPS mode"), 5, 5);
                pMenu->insertItem(tr("More detailed map (Shift+Up)"), 6, 6);
                pMenu->insertItem(tr("Less detailed map (Shift+Down)"), 7, 7);
                
                if (application->route->drawEnabled())
                    pMenu->insertItem(tr("Show route informations"), 8, 8);

                pMenu->setCheckable(false);
                pMenu->popup(this->
                             mapToGlobal(QPoint
                                         (mapdisp->width() / 2 - 100,
                                          mapdisp->height() / 2 - 100)));
                pMenu->setActiveItem(0);
                switch (pMenu->exec())
                {
                case 0:        // Go to Place/Waypoint
                    {
                        MapCoordEditorDialog mDialog(gpsData,
                                                     application->places,
                                                     this, tr("edit map coord"),
                                                     true, 0);
                        mDialog.setCaption(tr("Place Coordinates"));
                        if (mDialog.exec() != 0)
                        {
                            if (mDialog.mapSrcEditW->mapLatLonCB->
                                currentItem())
                            {
                                int i =
                                    mDialog.mapSrcEditW->mapLatLonCB->
                                    currentItem();
                                Places *curr = application->places;
                                do
                                {
                                    curr = (Places *) curr->next;
                                    i--;
                                    if (i == 0)
                                    {
                                        gpsData->currPos.setLat(curr->pos.latitude());
                                        gpsData->currPos.setLong(curr->pos.longitude());
                                        gpsData->altitude.setFromDouble(curr->altitude);
                                        break;
                                    }
                                }
                                while (curr != NULL);
                            }
                            else
                            {
                                double g, m, s;
                                g = mDialog.mapSrcEditW->LatiLEd->text().
                                    toDouble();
                                m = copysign(mDialog.mapSrcEditW->LatiLEm->
                                             text().toDouble(), g);
                                s = copysign(mDialog.mapSrcEditW->LatiLEs->
                                             text().toDouble(), g);
                                gpsData->currPos.setLat(g + m / 60 + s / 3600);
                                g = mDialog.mapSrcEditW->LonLEd->text().
                                    toDouble();
                                m = copysign(mDialog.mapSrcEditW->LonLEm->
                                             text().toDouble(), g);
                                s = copysign(mDialog.mapSrcEditW->LonLEs->
                                             text().toDouble(), g);
                                gpsData->currPos.setLong(g + m / 60 +   s / 3600);
                                gpsData->altitude.setFromDouble( mDialog.mapSrcEditW->AltLE->text().toDouble() );
//                              gpsData->altitude.altUnit= Altitude::Meter;

                            }
                            //      printf("Long=%e Lat=%e\n",gpsData->currPos.longitude,gpsData->currPos.latitude);
                        }
                        gpsData->adjustDatum();
                        mapdisp->resize(geometry().size());
                    }
                    break;
                case 1:        // Save Place/Waypoint
                    MapDispAddPos(gpsData->currPos.longitude(),
                                  gpsData->currPos.latitude(),
                                  gpsData->altitude.toDouble());
                    break;
                case 6:        //      "More detailed map (<shift> + Up)"
                    emit moreDetail();
                    break;
                case 7:        //      Less detailed map (<shift> + Down)"
                    emit lessDetail();
                    break;
                case 8:
                    application->showRouteInfo();
                    break;
                case 3:        //      "Change View"
                    emit mouseClick(this);
                    break;
                case 5:        // Switch mode /* Added by A. Karhov */
                    swichMode();        /* Added by A/ Karhov */
                    break;
                case 2:        // Draw Places /* Added by A. Karhov */
                    gpsData->draw_places = !gpsData->draw_places;
                    application->settings->writeConfig();
                    break;
                case 4:        // Animate track
                    {
                        AnimateParamsDialog aDialog(gpsData,
                                                    this, "Animate track",
                                                    true, 0);
                        if (!inanimation)
                        {
                            aDialog.setCaption(tr("Animation"));
                            if (aDialog.exec() != 0)
                            {
                                timeAccelerator =
                                    aDialog.paramW->AccelLEd->text().
                                    toDouble();
                                timeZone =
                                    aDialog.paramW->TZLEd->text().toInt();
                                shortcutTime =
                                    aDialog.paramW->scTimeTB->isOn();

                                placesFile =
                                    new QFile(gpsData->trackPathStr + "/" +
                                              aDialog.paramW->trackFileCB->
                                              currentText());
                                placesFile->open(IO_ReadOnly);
                                t = new QTextStream(placesFile);
                                atimer = new QTimer(this);
                                connect(atimer, SIGNAL(timeout()), this,
                                        SLOT(animateTrack()));
                                atimer->start(50, true);
                            }
                        }
                        else
                        {
                            inanimation = false;
                            atimer->stop();
                            delete atimer;
                            delete t;
                            placesFile->close();
                            delete placesFile;
                            break;
                        }
                    }
                    break;
                }
                delete pMenu;
            }
            break;
            /* Changed by A. Karhov version after 0.9.3.1 */
        case Qt::Key_Down:
            if (!(event->state() && ShiftButton))
            {
                if (actmap->scale != 0)
                    gpsData->currPos.setLat( gpsData->currPos.latitude() - 0.5 * 0.25 * (actmap->scale / 592500) );
                else
                    gpsData->currPos.setLat( gpsData->currPos.latitude() - 1 );
                    
                gpsData->altitude.setFromDouble(0);
                gpsData->adjustDatum();
                paintEvent(NULL);       //added by ng to fix map unmoved bug
            }
            else
            {
                emit lessDetail();
            }
            break;

        case Qt::Key_Right:
            if (!(event->state() && ShiftButton))
            {

                if (actmap->scale != 0)
                    gpsData->currPos.setLong( gpsData->currPos.longitude()
                                                + ((actmap->scale / 592500) * 0.3 /
                                                 fabs(cos(MathEx::deg2rad(gpsData->currPos.latitude())))) * 0.5 );
                else
                    gpsData->currPos.setLong( gpsData->currPos.longitude() + 1 );
                    
                gpsData->altitude.setFromDouble(0);
                gpsData->adjustDatum();
                paintEvent(NULL);       //added by ng to fix map unmoved bug
            }
            else
                emit lessDetail();
            break;

        case Qt::Key_Up:
            if (!(event->state() && ShiftButton))
            {
                if (actmap->scale != 0)
                    gpsData->currPos.setLat( gpsData->currPos.latitude() + 0.5 * 0.25 * (actmap->scale / 592500) );
                else
                    gpsData->currPos.setLat( gpsData->currPos.latitude() + 1);
                    
                gpsData->altitude.setFromDouble(0);
                gpsData->adjustDatum();
                paintEvent(NULL);       //added by ng to fix map unmoved bug
            }
            else
            {
                emit moreDetail();
            }
            break;

        case Qt::Key_Left:
            if (!(event->state() && ShiftButton))
            {
                if (actmap->scale != 0)
                    gpsData->currPos.setLong( gpsData->currPos.longitude() - 
                                                ((actmap->scale / 592500) * 0.3 /
                                                 fabs(cos(MathEx::deg2rad(gpsData->currPos.latitude())))) * 0.5);
                else
                    gpsData->currPos.setLong( gpsData->currPos.longitude() - 1 );
                    
                gpsData->altitude.setFromDouble(0);
                gpsData->adjustDatum();
                paintEvent(NULL);       //added by ng to fix map unmoved bug
            }
            else
                emit moreDetail();
            break;
            /* Changed by A. Karhov version after 0.9.3.1 */

        case Qt::Key_Backspace:
            if (inanimation)
            {
                inanimation = false;
                atimer->stop();
                delete atimer;
                delete t;
                placesFile->close();
                delete placesFile;
            }
            swichMode();        /* Added by A/ Karhov */
            break;

        default:
            event->ignore();
            break;
        }
    }
    else
    {
        switch (event->key())
        {
        case Qt::Key_F33:
        case Qt::Key_Space:    // Select
        case Qt::Key_Escape:   // Select
            emit mouseClick(this);
            break;

        case Qt::Key_Return:   /* Added by A. Karhov */
            if (!(event->state() && ShiftButton))
            {
                QPopupMenu *pMenu = new QPopupMenu();
                pMenu->insertItem(tr("Save Place/Waypoint"), 0, 0);
                pMenu->insertItem(tr("Set/Clear Destination"), 1, 1);
                if (gpsData->draw_places)
                {
                    pMenu->insertItem(tr("Don't draw places"), 2, 2);
                }
                else
                {
                    pMenu->insertItem(tr("Draw places"), 2, 2);
                }
                pMenu->insertItem(tr("Change View"), 3, 3);
                pMenu->insertItem(tr("Switch to Map mode"), 4, 4);
                
                if (application->route->drawEnabled())
                    pMenu->insertItem(tr("Show route informations"), 5, 5);

                pMenu->setCheckable(false);
                pMenu->popup(this->
                             mapToGlobal(QPoint
                                         (mapdisp->width() / 2,
                                          mapdisp->height() / 2)));
                pMenu->setActiveItem(0);
                switch (pMenu->exec())
                {
                case 0:        // Save Place/Waypoint
                    MapDispAddPos(gpsData->currPos.longitude(),
                                  gpsData->currPos.latitude(),
                                  gpsData->altitude.toDouble());
                    break;

                case 1:        // Set/Clear Destination
                    {
                        MapCoordEditorDialog mDialog(gpsData,
                                                     application->places,
                                                     this, tr("edit map coord"),
                                                     true, 0);
                        mDialog.setCaption(tr("Set/Clear Destination"));
                        if (mDialog.exec() != 0)
                        {
                            ManualWp = true;
                            if (mDialog.mapSrcEditW->mapLatLonCB->
                                currentItem())
                            {
                                int i =
                                    mDialog.mapSrcEditW->mapLatLonCB->
                                    currentItem();
                                Places *curr = application->places;
                                do
                                {
                                    curr = (Places *) curr->next;
                                    i--;
                                    if (i == 0) // destPointLat
                                    {
                                        destPointLat = MathEx::deg2rad(curr->pos.latitude());
                                        destPointLong = MathEx::deg2rad(curr->pos.longitude());
                                        gpsData->altitude.setFromDouble(curr->altitude);
                                        break;
                                    }
                                }
                                while (curr != NULL);
                            }
                            else
                            {
                                double g, m, s;
                                g = mDialog.mapSrcEditW->LatiLEd->text().
                                    toDouble();
                                m = copysign(mDialog.mapSrcEditW->LatiLEm->
                                             text().toDouble(), g);
                                s = copysign(mDialog.mapSrcEditW->LatiLEs->
                                             text().toDouble(), g);
                                destPointLat =
                                    (g + m / 60 + s / 3600) * MathEx::PI / 180.0;
                                g = mDialog.mapSrcEditW->LonLEd->text().
                                    toDouble();
                                m = copysign(mDialog.mapSrcEditW->LonLEm->
                                             text().toDouble(), g);
                                s = copysign(mDialog.mapSrcEditW->LonLEs->
                                             text().toDouble(), g);
                                destPointLong =
                                    (g + m / 60 + s / 3600) * MathEx::PI / 180.0;
                                gpsData->altitude.setFromDouble(mDialog.mapSrcEditW->AltLE->text().
                                    toDouble());
                            }
//                      printf("Long=%e Lat=%e\n",gpsData->currPos.longitude,gpsData->currPos.latitude);
                        }
                        else
                        {
                            destPointLat = destPointLong = 0;
                            ManualWp = false;
                        }

                        gpsData->adjustDatum();
                        mapdisp->resize(geometry().size());

                    }
                    break;

                case 2:        // Draw Places /* Added by A. Karhov */
                    gpsData->draw_places = !gpsData->draw_places;
                    application->settings->writeConfig();
                    break;

                case 3:        //      "Change View"
                    emit mouseClick(this);
                    break;

                case 4:        /* Added by A. Karhov */
                    swichMode();        /* Added by A/ Karhov */
                    break;
                case 5:
                    application->showRouteInfo();
                    break;
                default:
                    break;
                }
                delete pMenu;
            }
            else
                MapDispAddPos(gpsData->currPos.longitude(),
                              gpsData->currPos.latitude(),
                              gpsData->altitude.toDouble());
            break;

        case Qt::Key_Down:
        case Qt::Key_Right:
            emit lessDetail();
            break;

        case Qt::Key_Up:
        case Qt::Key_Left:
            emit moreDetail();
            break;

            //case Qt::Key_Left:
            //emit debugMaps(); // only for debugging, don't forget to remove !!!!
            //break;

        case Qt::Key_Backspace:        /* Added by A. Karhov */
            swichMode();        /* Added by A/ Karhov */
            break;

        default:
            event->ignore();
            break;
        }
    }
//    setFocus();       /* Added by A/ Karhov */
}

void MapDisp::animateTrack()
{
    if (inanimation)
    {
        gpsData->currPos.setLat(newlatitude);
        gpsData->currPos.setLong(newlongitude);
        gpsData->altitude.setFromDouble(newaltitude);
        currtime = newtime;;
        if (t->atEnd())
        {
            inanimation = false;
            delete atimer;
            atimer = NULL;
            delete t;
            placesFile->close();
            delete placesFile;
            gpsData->adjustDatum();
            mapdisp->resize(geometry().size());
            return;
        }
    }
    newlatitude = 99999999;
    t->skipWhiteSpace();
    *t >> newlatitude;
    if (t->atEnd() || newlatitude == 99999999)
    {
        inanimation = false;
        delete atimer;
        atimer = NULL;
        delete t;
        placesFile->close();
        delete placesFile;
        gpsData->adjustDatum();
        mapdisp->resize(geometry().size());
        return;
    }
    newlongitude = 99999999;
    t->skipWhiteSpace();
    *t >> newlongitude;
    if (t->atEnd() || newlongitude == 99999999)
    {
        inanimation = false;
        delete atimer;
        atimer = NULL;
        delete t;
        placesFile->close();
        delete placesFile;
        gpsData->adjustDatum();
        mapdisp->resize(geometry().size());
        return;
    }
    newaltitude = 99999999;
    t->skipWhiteSpace();
    *t >> newaltitude;
    if (t->atEnd() || newaltitude == 99999999)
    {
        inanimation = false;
        delete atimer;
        atimer = NULL;
        delete t;
        placesFile->close();
        delete placesFile;
        gpsData->adjustDatum();
        mapdisp->resize(geometry().size());
        return;
    }
    t->skipWhiteSpace();
    *t >> ts;
    ts[2] = ts[5] = ' ';
    newtime = ts.left(2).toInt();
    newtime = (newtime + timeZone) * 60 * 60 * 1000;
    ts[0] = ts[3];
    ts[1] = ts[4];
    min = ts.mid(3, 2).toInt();
    newtime = newtime + min * 60 * 1000;
    ts[0] = ts[6];
    ts[1] = ts[7];
    min = ts.mid(6, 2).toInt();
    newtime = newtime + min * 1000;
//      printf("Time:%i lat:%f Long:%f Alt:%f\n",newtime,newlongitude, newlatitude, newaltitude);
    if (inanimation)
    {
        if (newtime < 0)
        {
            newtime = currtime + (gpsData->updt_freq * 1000);
        }
        min = int ((newtime - currtime) / timeAccelerator);
        if ((min < 0) || ((min > 60000) && shortcutTime))
            atimer->start(5000, true);
        else
            atimer->start(min, true);
    }
    else
    {
        inanimation = true;
        atimer->start(50, true);
    }

    return;
}



AnimateParamsDialog::AnimateParamsDialog(GpsData * gpsData, QWidget * parent, const char *name, bool modal, WFlags f):QDialog(parent, name, modal,
        f)
{
    resize(220, 160);
    paramW = new AnimateParamsWidget(gpsData, this);
}

AnimateParamsDialog::~AnimateParamsDialog()
{
};

AnimateParamsWidget::~AnimateParamsWidget()
{
}

AnimateParamsWidget::AnimateParamsWidget(GpsData * gpsData, QWidget * parent, const char *name = 0):QVBox(parent,
      name)
{
    QHBox *hBox, *vBox;

    resize(parent->geometry().size());
    vBox = new QVBox(this);

    hBox = new QHBox(vBox);
    trackFileCB = new QComboBox(hBox);

    QDir *logdir = new QDir();
    // if trackPathStr exists, change logdir to it
    if (logdir->exists(gpsData->trackPathStr))
    {
        logdir->setPath(gpsData->trackPathStr);
        // fill list with new files
        const QFileInfoList *list = logdir->entryInfoList();
        QFileInfoListIterator it(*list);
        QFileInfo *fi;
        while ((fi = it.current()))
        {
            // add file to list
            if (fi->fileName()[0] != '.' && fi->fileName() != "places.txt"
                && fi->fileName() != "places.txt~")
            {
                trackFileCB->insertItem(fi->fileName().latin1());
            }
            ++it;
        }
    }

    hBox = new QHBox(vBox);
    AccelL = new QLabel(tr("Time Acceleration:"), hBox);
    AccelLEd = new QLineEdit(hBox);
    AccelLEd->setValidator(new QDoubleValidator(0.01, 10000, 3, AccelLEd));
    AccelLS = new QLabel(tr("X"), hBox);
    AccelLEd->setText("100");

    hBox = new QHBox(vBox);
    TZL = new QLabel(tr("Time Zone: GMT +"), hBox);
    TZLEd = new QLineEdit(hBox);
    TZLEd->setValidator(new QIntValidator(-23, 23, TZLEd));
    TZLS = new QLabel(tr("h"), hBox);
    TZLEd->setText("0");

    hBox = new QHBox(vBox);
    scTimeTB = new QCheckBox(hBox);
    scTimeTB->setText(tr("Shortcut large time intervals"));
    scTimeTB->setChecked(true);
}



void MapDisp::MapDispAddPos(double longitude, double latitude, double altitude)                                                /* changed by A.Karkhov after 0.9.2.3.2  *//**********/
{
    MapPlaceEditorDialog pDialog(this, "Saving Places", true, 0);       /*********//* Added by A. Karhov */
    pDialog.setCaption(tr("Saving Place"));
    if (pDialog.exec() != 0)
    {
        Places *curr;
        curr = (Places *) malloc(sizeof(Places));
        if (pDialog.mapSrcEditW->NameLEd->text().length() == 0)
        {
            curr->name = new QString(tr("p%1").arg(time(NULL), 0, 16));
        }
        else
            curr->name = new QString(pDialog.mapSrcEditW->NameLEd->text());
        *curr->name = curr->name->stripWhiteSpace();
        *curr->name = curr->name->simplifyWhiteSpace();
        curr->name->truncate(30);
        curr->name->replace(QRegExp("\40+"), "_");
        curr->pos.setLat(latitude);
        curr->pos.setLong(longitude);
        curr->altitude = altitude;      // Save Altitude
        curr->comment = new QString(pDialog.mapSrcEditW->CommentLEd->text());
        *curr->comment = curr->comment->stripWhiteSpace();
        *curr->comment = curr->comment->simplifyWhiteSpace();
        *curr->comment += " :: " + QDateTime::currentDateTime().toString();

        curr->next = application->places->next;
        application->places->next = curr;


        application->writePlaces();
/*	 // commented by A.Karkhov after 0.9.2.3.2  	  
QString	buf="";
		 QString filename = gpsData->trackPathStr;
		  filename.append("/places.txt");
		  remove(filename+"~");
		  rename(filename,filename+"~");
		 QFile placesFile(filename);
		  int ok =  placesFile.open(IO_WriteOnly);
		   if ( !ok )
		   { while ( !QMessageBox::warning( this, "Saving places...", "Can't open/create file:\n"+filename+  "\nPlace not saved. Please check file/directory access rights.\n","Try again","Ignore", 0, 0, 1 ) )
			 { ok =  placesFile.open(IO_WriteOnly);
			 if (ok) break;
			 }
			 if ( !ok ) {rename(filename+"~",filename); return; }
		      }
		 QTextStream t( &placesFile );		  
		  QFile oplacesFile(filename+"~");
		  ok =  oplacesFile.open(IO_ReadOnly);
		   QTextStream ot( &oplacesFile );
		  if ( ok )
		  { while ( !ot.eof() )
        		{buf=ot.readLine();
			if ( (oplacesFile.status()) != -1 && (placesFile.status()) != -1 && buf[0]=='#') { buf+="\r\n"; t<<buf; buf="";   }
			else break;
			}
		   }
		   else { t << "# this is places file places.txt format is : <CityName> <latitude (fload decimal)> <longitude (fload decimal)> <short text comment>\r\n"; }
		   t << (*curr->name+"\t"+tr("%1").arg(curr->pos.latitude,6,'f')+"\t"+tr("%1").arg(curr->pos.longitude,6,'f')+"\t"+tr("%1").arg(curr->altitude,1,'f')+"\t"+*curr->comment+"\r\n");

		curr=curr->next;
		while ( curr != NULL )   {
		   t << (*curr->name+"\t"+tr("%1").arg(curr->pos.latitude,6,'f')+"\t"+tr("%1").arg(curr->pos.longitude,6,'f')+"\t"+tr("%1").arg(curr->altitude,1,'f')+"\t"+*curr->comment+"\r\n");
		curr=curr->next;
    		};

		placesFile.close();
		if (ok) oplacesFile.close();
*/
        application->track->refresh();
        application->track->setStartup();

    }
}

MapCoordEditorDialog::MapCoordEditorDialog(GpsData * gpsData, Places * places,
                                           QWidget * parent, const char *name,
                                           bool modal,
                                           WFlags f):QDialog(parent, name,
                                                             modal, f)
{
/*QPoint po=pos(); po.setY(60);//po.setX(120); move(po);*/
    resize(245, 305);
    mapSrcEditW = new MapCoordEditorWidget(gpsData, places, this, "");
}

MapCoordEditorDialog::~MapCoordEditorDialog()
{
};

void MapCoordEditorWidget::placeSelected(int ind)
{
    if (ind)
    {
        int i = ind;
        Places *curr = pl;
        do
        {
            curr = (Places *) curr->next;
            i--;
            if (i == 0)
            {
                commMLE->setText(*curr->comment);
/*		  switch(curr->pos.posUnit)
    		{	case curr->pos.Degree:	DDddB->setChecked(true);
        		break;
    			case curr->pos.DegMin:	DDMMmmB->setChecked(true);
        		break;
    			case curr->pos.DegMinSec:	DDMMSSssB->setChecked(true);
		        break;
    		}
*/
                DDddB->setChecked(true);
                LatiLEd->setText(tr("%1").arg(curr->pos.latitude(), 0, 'f'));
                LonLEd->setText(tr("%1").arg(curr->pos.longitude(), 0, 'f'));
                AltLE->setText(tr("%1").arg(Altitude::getAlt(curr->altitude), 0, 'f'));
                break;
            }
        }
        while (curr != NULL);
    }
}

MapCoordEditorWidget::MapCoordEditorWidget(GpsData * gpsData, Places * places,
                                           QWidget * parent,
                                           const char *name):QVBox(parent,
                                                                   name)
{
    QHBox *hBox, *vBox;
    QFont f;

    LatiLEd = LatiLEm = LatiLEs = NULL;
    LonLEd = LonLEm = LonLEs = NULL;
    AltLE = NULL;
    resize(parent->geometry().size());
    vBox = new QVBox(this);
//    PlaceGB = new QVGroupBox(tr("Place"),vBox);
//    hBox = new QHBox(PlaceGB);

    hBox = new QHBox(vBox);
    mapLatLonCB = new QComboBox(hBox);
    pl = places;
    Places *curr = places;
    do
    {
        mapLatLonCB->insertItem(*curr->name);
        curr = (Places *) curr->next;
    }
    while (curr != NULL);
    connect(mapLatLonCB, SIGNAL(activated(int)), this,
            SLOT(placeSelected(int)));

    commMLE = new QMultiLineEdit(vBox);
    commMLE->setWordWrap(QMultiLineEdit::WidgetWidth);
    commMLE->setWrapPolicy(QMultiLineEdit::Anywhere);
    commMLE->setReadOnly(true);
    commMLE->setFixedVisibleLines(3);
    CoordGB = new QVGroupBox(tr("Coordinates"), vBox);

    LatLonBG = new QButtonGroup(3, Qt::Horizontal, CoordGB);
    DDddB = new QRadioButton("D\260", LatLonBG);
    connect(DDddB, SIGNAL(toggled(bool)), this, SLOT(toggledDDddB(bool)));
    DDMMmmB = new QRadioButton("D\260M'", LatLonBG);
    connect(DDMMmmB, SIGNAL(toggled(bool)), this, SLOT(toggledDDMMmmB(bool)));
    DDMMSSssB = new QRadioButton("D\260M'S\"", LatLonBG);
    connect(DDMMSSssB, SIGNAL(toggled(bool)), this,
            SLOT(toggledDDMMSSssB(bool)));

    hBox = new QHBox(CoordGB);
    LatiL = new QLabel(tr("   Lat:"), hBox);
    LatiLEd = new QLineEdit(hBox);
    connect(LatiLEd, SIGNAL(textChanged(const QString &)), this,
            SLOT(editedLtLe(const QString &)));
    LatiLd = new QLabel(tr("\260 "), hBox);
    f = LatiL->font();
    f.setPixelSize(f.pixelSize() + 3);
    LatiLd->setFont(f);
    LatiLEm = new QLineEdit(hBox);
    connect(LatiLEm, SIGNAL(textChanged(const QString &)), this,
            SLOT(editedLtLe(const QString &)));
    LatiLm = new QLabel(tr("\' "), hBox);
    LatiLm->setFont(f);
    LatiLEs = new QLineEdit(hBox);
    connect(LatiLEs, SIGNAL(textChanged(const QString &)), this,
            SLOT(editedLtLe(const QString &)));
    LatiLs = new QLabel(tr("\""), hBox);
    LatiLs->setFont(f);
    LatiLS = new QLabel(tr("N"), hBox);

    hBox = new QHBox(CoordGB);
    LonL = new QLabel(tr("Long:"), hBox);
    LonLEd = new QLineEdit(hBox);
    connect(LonLEd, SIGNAL(textChanged(const QString &)), this,
            SLOT(editedLtLe(const QString &)));
    LonLd = new QLabel(tr("\260 "), hBox);
    LonLd->setFont(f);
    LonLEm = new QLineEdit(hBox);
    connect(LonLEm, SIGNAL(textChanged(const QString &)), this,
            SLOT(editedLtLe(const QString &)));
    LonLm = new QLabel(tr("\' "), hBox);
    LonLm->setFont(f);
    LonLEs = new QLineEdit(hBox);
    connect(LonLEs, SIGNAL(textChanged(const QString &)), this,
            SLOT(editedLtLe(const QString &)));
    LonLs = new QLabel(tr("\""), hBox);
    LonLs->setFont(f);
    LonLS = new QLabel(tr("E"), hBox);

    hBox = new QHBox(CoordGB);
    AltL = new QLabel(tr(" Alitude:"), hBox);
    AltLE = new QLineEdit(hBox);
    AltLE->setText("0");
    alt = &gpsData->altitude;
    switch (alt->altUnit)
    {
    case Altitude::None:
        AltL = new QLabel(tr("... "), hBox);
        break;
    case Altitude::Feet:
        AltL = new QLabel(tr(" ft   "), hBox);
        break;
    case Altitude::FL:
        AltL = new QLabel(tr(" FL   "), hBox);
        break;
    case Altitude::Meter:
        AltL = new QLabel(tr(" meter"), hBox);
        break;
    }
    LatiLEm->setText("");
    LatiLEs->setText("");
    LonLEm->setText("");
    LonLEs->setText("");

//    MapCoordEditorForm(gpsData->currPos.posUnit);

    switch (gpsData->currPos.posUnit)
    {
    case Position::Degree:
        DDddB->setChecked(true);
        break;
    case Position::DegMin:
        DDMMmmB->setChecked(true);
        break;
    case Position::DegMinSec:
        DDMMSSssB->setChecked(true);
        break;
    }
}

void MapCoordEditorWidget::editedLtLe(const QString &)
{
    if (LonLEd->edited() || LonLEm->edited() || LonLEs->edited()
        || LatiLEd->edited() || LatiLEm->edited() || LatiLEs->edited())
    {
        if (mapLatLonCB->currentItem() != 0)
        {
            mapLatLonCB->setCurrentItem(0);
        }
    }
}
void MapCoordEditorWidget::toggledDDddB(bool state)
{
    if (state)
    {
        double x;
        x = LatiLEd->text().toDouble();
        x = x + copysign(LatiLEm->text().toDouble() / 60,
                         x) + copysign(LatiLEs->text().toDouble() / 3600, x);
        LatiLEd->setText(tr("%1").arg(x, 0, 'f'));

        x = LonLEd->text().toDouble();
        x = x + copysign(LonLEm->text().toDouble() / 60,
                         x) + copysign(LonLEs->text().toDouble() / 3600, x);
        LonLEd->setText(tr("%1").arg(x, 0, 'f'));

        /*  if ( mapLatLonCB->currentItem() != 0 ) {
           mapLatLonCB->setCurrentItem(0);
           } */

        LatiLEd->setValidator(new QDoubleValidator(-360, 360, 8, LatiLEd));
        LonLEd->setValidator(new QDoubleValidator(-360, 360, 8, LonLEd));
        LatiLEm->setText("");
        LatiLEs->setText("");
        LonLEm->setText("");
        LonLEs->setText("");
        LatiLEm->hide();
        LatiLEs->hide();
        LonLEm->hide();
        LonLEs->hide();
        LatiLm->hide();
        LatiLs->hide();
        LonLm->hide();
        LonLs->hide();
        repaint();
    }
}

void MapCoordEditorWidget::toggledDDMMmmB(bool state)
{
    if (state)
    {
        double x, y;
        x = LatiLEd->text().toDouble();
        x = x + copysign(LatiLEm->text().toDouble() / 60,
                         x) + copysign(LatiLEs->text().toDouble() / 3600, x);
        LatiLEd->setText(tr("%1").arg(int (x)));
        y = fabs(x - int (x)) * 60;
        LatiLEm->setText(tr("%1").arg(y, 0, 'f'));

        x = LonLEd->text().toDouble();
        x = x + copysign(LonLEm->text().toDouble() / 60,
                         x) + copysign(LonLEs->text().toDouble() / 3600, x);
        LonLEd->setText(tr("%1").arg(int (x)));
        y = fabs(x - int (x)) * 60;
        LonLEm->setText(tr("%1").arg(y, 0, 'f'));

        if (mapLatLonCB->currentItem() != 0)
        {
            mapLatLonCB->setCurrentItem(0);
        }
        LatiLEd->setValidator(new QIntValidator(-360, 360, LatiLEd));
        LatiLEm->setValidator(new QDoubleValidator(0, 59.9999, 10, LatiLEm));
        LonLEd->setValidator(new QIntValidator(-360, 360, LonLEd));
        LonLEm->setValidator(new QDoubleValidator(0, 59.9999, 10, LonLEm));
        LatiLEs->setText("");
        LonLEs->setText("");
        LatiLEm->show();
        LatiLEs->hide();
        LonLEm->show();
        LonLEs->hide();
        LatiLm->show();
        LatiLs->hide();
        LonLm->show();
        LonLs->hide();
        repaint();
    }
}

void MapCoordEditorWidget::toggledDDMMSSssB(bool state)
{
    if (state)
    {
        double x, y;
        x = LatiLEd->text().toDouble();
        x = x + copysign(LatiLEm->text().toDouble() / 60,
                         x) + copysign(LatiLEs->text().toDouble() / 3600, x);
        LatiLEd->setText(tr("%1").arg(int (x)));
        y = fabs(x - int (x)) * 60;
        LatiLEm->setText(tr("%1").arg(int (y)));
        x = fabs(y - int (y)) * 60;
        LatiLEs->setText(tr("%1").arg(x, 0, 'f', 3));

        x = LonLEd->text().toDouble();
        x = x + copysign(LonLEm->text().toDouble() / 60,
                         x) + copysign(LonLEs->text().toDouble() / 3600, x);
        LonLEd->setText(tr("%1").arg(int (x)));
        y = fabs(x - int (x)) * 60;
        LonLEm->setText(tr("%1").arg(int (y)));
        x = fabs(y - int (y)) * 60;
        LonLEs->setText(tr("%1").arg(x, 0, 'f', 3));

        if (mapLatLonCB->currentItem() != 0)
        {
            mapLatLonCB->setCurrentItem(0);
        }
        LatiLEd->setValidator(new QIntValidator(-360, 360, LatiLEd));
        LatiLEm->setValidator(new QIntValidator(0, 59, LatiLEm));
        LatiLEs->setValidator(new QDoubleValidator(0, 59.9999, 10, LatiLEs));
        LonLEd->setValidator(new QIntValidator(-360, 360, LonLEd));
        LonLEm->setValidator(new QIntValidator(0, 59, LonLEm));
        LonLEs->setValidator(new QDoubleValidator(0, 59.9999, 10, LonLEs));
        LatiLEm->show();
        LatiLEs->show();
        LonLEm->show();
        LonLEs->show();
        LatiLm->show();
        LatiLs->show();
        LonLm->show();
        LonLs->show();
        repaint();
    }
}

MapCoordEditorWidget::~MapCoordEditorWidget()
{
};

MapPlaceEditorDialog::MapPlaceEditorDialog(QWidget * parent, const char *name,
                                           bool modal,
                                           WFlags f):QDialog(parent, name,
                                                             modal, f)
{
    resize(245, 150);
    mapSrcEditW = new MapPlaceEditorWidget(this, "");
}

MapPlaceEditorDialog::~MapPlaceEditorDialog()
{
};

MapPlaceEditorWidget::MapPlaceEditorWidget(QWidget * parent,
                                           const char *name):QVBox(parent,
                                                                   name)
{
    QHBox *hBox, *vBox;

    resize(parent->geometry().size());
    vBox = new QVBox(this);
    PlaceGB = new QVGroupBox(tr("Place name: "), vBox);
    hBox = new QHBox(PlaceGB);
    NameLEd = new QLineEdit(hBox);
    PlaceGB1 = new QVGroupBox(tr("Comment: "), vBox);
    hBox = new QHBox(PlaceGB1);
    CommentLEd = new QLineEdit(hBox);
}


MapPlaceEditorWidget::~MapPlaceEditorWidget()
{
};

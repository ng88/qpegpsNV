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


#include "route.h"

#include <qvbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qcheckbox.h>

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qfile.h>
#include <qvgroupbox.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qpe/config.h>
#include <qpoint.h>

#include <qframe.h>
#include <qbuttongroup.h>
#include <qwidgetstack.h>
#include <qlistbox.h>
#include <qlayout.h>

#include <qtimer.h>

#include "qpegps.h"
#include "settings.h"


/* added by ng */

RoutePoint::RoutePoint(double lat, double lng, double meters2prec, const QString & comment, int commentPos)
  : _lat(lat), _lng(lng), _meters2prec(meters2prec), _comment(comment), _commentPos(commentPos)
{
}

RoutePoint::RoutePoint(const QString & str)
{
    const QStringList& args = QStringList::split(';', str, true);
    if(args.count() >= 7)
    {
        _lat = MathEx::deg2rad(args[1].toDouble());
        _lng = MathEx::deg2rad(args[2].toDouble());
        //altitude  = args[3].toDouble();
        //time      = args[4].toDouble();
        _meters2prec  = args[5].toDouble();
        _comment = args[6];
        _commentPos = 0;
    }

}



Route::Route()
 :  _closer(0), _routeLost(false), _length(0),
    _distanceBeforeNextComment(0), _routeDone(0),
    _lastCommentNb(0), _lastReachedComment(0)
{
}

Route::~Route()
{
    dispose();
}

void Route::dispose()
{
    RoutePoint *pt = _route.first();

    while(pt)
    {
        delete pt;
        pt = _route.next();
    }

    _route.clear();
    _comments.clear();
    _length = 0;
    _routeLost = false;
    _closer = 0;
    _distanceBeforeNextComment = 0;
    _lastCommentNb = 0;
    _routeDone = 0;
    _lastReachedComment = 0;
    
    
}

void Route::readFromFile(const QString & file)
{
    QFile f(file);
    readFromFile(f);
}

void Route::readFromFile(QFile & file)
{
    dispose();
    
    if(file.open(IO_ReadOnly))
    {
        QTextStream stream(&file);
        QString line;
        int comments = 0;

        while(!stream.atEnd())
        {
            RoutePoint *pt = new RoutePoint(stream.readLine());
            
            _length += pt->meters2prec();

            if(pt->comment().length() > 0)
            {
                pt->setCommentPos(++comments);
                _comments.append(pt);
            }

            _route.append(pt);


        }
        
        
        //TODO _route[0]->setMeters2prec(distance de tolérance de perte de route);

        file.close();
    }
    
    emit routeChanged();
}

/** Routing calculations are also done here for optmization */
void Route::draw(QPainter * painter, MapBase * actmap, int x1, int y1, int mx,
                 int my, GpsData & data)
{
    if(!actmap)
        return;
       


    if(!_route.isEmpty())
    {
    
        /* do we compute for route info? */
        bool computeInfos = !data.ManualPosit;
    
        double myPosLt = MathEx::deg2rad(data.latitudeGps);
        double myPosLg = MathEx::deg2rad(data.longitudeGps);
    
        RoutePoints routeInfos;
        
        double lg, lt, xwp = 0, ywp = 0;
        int fx = 0, fy = 0;
        int xtp, ytp;

        bool precdef = false;
        bool in = false;

        QPen pen = painter->pen();
        pen.setColor(data.routeColor);
        pen.setWidth(data.rtLineThick);
        painter->setPen(pen);
        
        
        
        RoutePoint * oldCloser      = _closer;
        double       d              = 0.0;
        double       closerDist     = 0.0;
        double       calcDist       = 0.0;
        int          lastComment    = 0;
        
        _closer = 0;
        
        RoutePoint *tp = _route.first();
        
        calcDist = -tp->meters2prec();
        
        while(tp)
        {
            calcDist += tp->meters2prec();
            
            lt = tp->latitude();
            lg = tp->longitude();

            actmap->calcxy(&xwp, &ywp, lg, lt);
            xtp = (int) xwp - x1;
            ytp = (int) ywp - y1;
            
            if(computeInfos)
            {
                if(!_routeLost)
                {
                    if(tp->commentPos())
                        lastComment = tp->commentPos();
                        
                    /* we seek the closer next point */
                    d = MathEx::computeDistanceLtLgRadM(lt, lg, myPosLt, myPosLg);
                    if( d <= closerDist || _closer == 0 )
                    {
                        _closer = tp;
                        closerDist = d;
                        _routeDone = calcDist;
                        _lastReachedComment = lastComment;
                    }
                    
                }
            }
            

            if(xtp >= 0 && ytp >= 0 && (int) xwp < mx && (int) ywp <= my)
            {
                if(tp->commentPos()) /* commented point */
                    routeInfos.append(tp);

                in = true;

                if(precdef)
                    painter->drawLine(fx, fy, xtp, ytp);
            }
            else if(in)
            {
                painter->drawLine(fx, fy, xtp, ytp);
                in = false;
            }

            fx = xtp;
            fy = ytp;
            precdef = true;

            tp = _route.next();
        }
        

        
        if(data.rtDisplayIcon)
        {
        
            QFont normalFont = painter->font();
            QFont commentFont(normalFont);
            commentFont.setPointSize(10);
            commentFont.setBold(true);
            painter->setFont(commentFont);
    
            pen.setColor(data.routeIconTxtColor);
            pen.setWidth(1);
            painter->setPen(pen);
            
            painter->setBrush(QBrush(data.routeIconColor));
        
            /* draw commented point */
            QString num;
            
            tp = routeInfos.first();
            while(tp)
            {
                lt = tp->latitude();
                lg = tp->longitude();
    
                actmap->calcxy(&xwp, &ywp, lg, lt);
                xtp = (int) xwp - x1;
                ytp = (int) ywp - y1;
    
                
                num.setNum(tp->commentPos());
                int numDigits = num.length();
    
                painter->drawRect(xtp - (4 + 3 * numDigits), ytp - 6, (8 + 6 * numDigits), 12);
                painter->drawText(xtp - (4 + 3 * numDigits), ytp - 6, (8 + 6 * numDigits), 12, Qt::AlignHCenter, num);
                tp = routeInfos.next();
            }
                    
            painter->setFont(normalFont);
        }

        
        if(computeInfos)
        {
            /* if we are still on the road */
            if(!_routeLost)
            {
//                 if( _closerDist > /*closer->meters2prec()*/ 5  )
//                 { //we left the route    
//                                                                         
//                     emit routeLost();            
//                     printf("ouch we're lost!\n");
//                     
//                     _routeLost = true;
//
//                 }
//                 else
                if( _closer != oldCloser )
                { //we 've moved
 
                    emit positionChanged();
                    /*printf("we've moved to (%f, %f) route done %f, last announced comment %d, last reached %d"
                                        , _closer->latitudeDeg()
                                        , _closer->longitudeDeg()
                                        , _routeDone
                                        , _lastCommentNb
                                        , _lastReachedComment );*/
                    
                    //we seek the next commented point
                    
                    if( _route.findRef(_closer) > -1 )
                    {
                        tp = _closer;
                        _distanceBeforeNextComment = -tp->meters2prec();
                        while(tp)
                        {
                            _distanceBeforeNextComment += tp->meters2prec();
                                
                            if(tp->commentPos() == _lastCommentNb + 1
                                && (_lastReachedComment == _lastCommentNb
                                        || _lastReachedComment == _lastCommentNb + 1) ) 
                            { /* we found the next commented point */
                                //printf("next comment in %e m ", _distanceBeforeNextComment);
                                if(_distanceBeforeNextComment < data.rtPreventValue)
                                { /* we have to prevent the diver */
                                    _lastCommentNb = tp->commentPos();
                                    emit routeInfo(tp->comment(), tp->commentPos());
                                    //printf("\nnew comment (%d): %s", _lastCommentNb, tp->comment().latin1());
                                    break;
                                }
                            }
                            
                            tp = _route.next();
                        }
                    }
                    
                    printf("\n");
                }
            
            }
            
            if(data.rtDrawLine)
            {
                pen.setColor(data.routePosLineColor);
                pen.setWidth(1);
                painter->setPen(pen);
                
                if(_closer)
                {
                    int x, y;
                    
                    
                    lt = _closer->latitude();
                    lg = _closer->longitude();
            
                    actmap->calcxy(&xwp, &ywp, lg, lt);
                    xtp = (int) xwp - x1;
                    ytp = (int) ywp - y1;
                    
                    lt = myPosLt;
                    lg = myPosLg;
            
                    actmap->calcxy(&xwp, &ywp, lg, lt);
                    x = (int) xwp - x1;
                    y = (int) ywp - y1;
                    
                    
                    painter->drawLine(x, y, xtp, ytp);
                
                }
            }

        }
        
    }

}

RouteGUI::RouteGUI(Qpegps * appl, QWidget * parent, const char *name,
                   WFlags fl):QScrollView(parent, name, fl)
{
    /** For data/settings access **/
    _app = appl;


    /** GUI **/
    setHScrollBarMode(AlwaysOff);
    setVScrollBarMode(AlwaysOn);

    QVBox *mainBox = new QVBox(this);
    mainBox->setMaximumWidth(217); //magic number to fix QScrollView bug
    
    addChild(mainBox);

    setResizePolicy(AutoOneFit);

    QVGroupBox *vBox = new QVGroupBox(tr("Route"), mainBox);


    new QLabel(tr("Select a route to use:"), vBox);

    QHBox *hBox = new QHBox(vBox);
    hBox->setSpacing(5);

    _cboRoutes = new QComboBox(hBox);
    _btnOK = new QPushButton(tr("OK"), hBox);
    _btnOK->setMaximumWidth(30);
    _btnOK->setMinimumWidth(30);
    


    _lblCurrent = new QLabel(tr("<b>No route loaded</b>"), vBox);
    _chkDspIcon = new QCheckBox(tr("Display an icon at each turn"), vBox);


    vBox = new QVGroupBox(tr("Directives"), mainBox);

    _chkDspPopup = new QCheckBox(tr("Show directives in a popup"), vBox);


    hBox = new QHBox(vBox);

    _chkPopupAutoClose = new QCheckBox(tr("Auto close popup after:"), hBox);
    _spnAutoCloseTime = new QSpinBox(1, 1000, 1, hBox);
    new QLabel(tr(" s"), hBox);
    
    hBox = new QHBox(vBox);

    _chkTts = new QCheckBox(tr("Read directives with "), hBox);
    _cboTts = new QComboBox(hBox);
    _cboTts->insertStringList(_app->speakers().speakerNames());


    hBox = new QHBox(vBox);
    hBox->setSpacing(2);

    new QLabel(tr("Prevention distance:"), hBox);
    _spnPreventDist = new QSpinBox(30, 30000, 1, hBox);
    new QLabel(tr(" m"), hBox);
    
    
    vBox = new QVGroupBox(tr("Route drawing"), mainBox);
    
    _chkDrawLine = new QCheckBox(tr("Draw a line between current position\n\tand route"), vBox);

    hBox = new QHBox(vBox);
    new QLabel(tr("Line thickness"), hBox);
    _spnLineThick = new QSpinBox(1, 8, 1, hBox);
    
    


//     vBox = new QVGroupBox(tr("Navigation"), mainBox);
//     
//     hBox = new QHBox(vBox);
//     
//     new QCheckBox(tr("Warm when leaving the planed route from"), hBox);
//     new QSpinBox(5, 10000, 5, hBox);
//     new QLabel(tr("m"), hBox);



    /** Signal/slot connect **/

    connect(_app, SIGNAL(currentChanged(QWidget *)), this, SLOT(currentTabChanged(QWidget *)));
    connect(_btnOK, SIGNAL(pressed()), this, SLOT(btnOkPressed()));
    connect(_cboTts, SIGNAL(highlighted(int)), this, SLOT(ttsEngineSelected(int)));
    
    connect(_chkDspIcon, SIGNAL(toggled(bool)), this, SLOT(valuesUpdated()));
    connect(_chkDspPopup, SIGNAL(toggled(bool)), this, SLOT(valuesUpdated()));
    connect(_chkPopupAutoClose, SIGNAL(toggled(bool)), this, SLOT(valuesUpdated()));
    connect(_chkTts, SIGNAL(toggled(bool)), this, SLOT(valuesUpdated()));
    connect(_chkDrawLine, SIGNAL(toggled(bool)), this, SLOT(valuesUpdated()));
    connect(_spnAutoCloseTime, SIGNAL(valueChanged(int)), this, SLOT(valuesUpdated()));
    connect(_spnPreventDist, SIGNAL(valueChanged(int)), this, SLOT(valuesUpdated()));
    connect(_spnLineThick, SIGNAL(valueChanged(int)), this, SLOT(valuesUpdated()));

    /** Settings **/
    loadSetting();
 

}

void RouteGUI::draw(QPainter * painter, MapBase * actmap, int x1, int y1, int mx, int my)
{
    _route.draw(painter, actmap, x1, y1, mx, my, _app->gpsData());
}

void RouteGUI::ttsEngineSelected(int v)
{
    _app->speakers().setCurrent(v);
}


void RouteGUI::currentTabChanged(QWidget * e)
{
    if(e == this)
        updateGUI();
}

void RouteGUI::updateGUI()
{
    /** Route list **/

    _cboRoutes->clear();
    _cboRoutes->insertItem(tr("[no route]"));

    QDir routes(_app->gpsData().trackPathStr, "*.qrt");
    routes.setFilter(QDir::Files | QDir::NoSymLinks);

    const QFileInfoList *list = routes.entryInfoList();
    QFileInfoListIterator it(*list);
    QFileInfo *fi;

    while((fi = it.current()) != 0)
    {
        _cboRoutes->insertItem(fi->fileName());
        ++it;
    }

}

void RouteGUI::btnOkPressed()
{

    if(_cboRoutes->currentItem() > 0)
    {
        _lblCurrent->setText(tr("<b>Current route: %1</b>").
                             arg(_cboRoutes->currentText()));
        _route.readFromFile(_app->gpsData().trackPathStr + "/" +
                            _cboRoutes->currentText());
    }
    else
    {
        _lblCurrent->setText(tr("<b>No route loaded</b>"));
        _route.clear();
    }

}

void RouteGUI::valuesUpdated()
{
    _app->gpsData().rtDisplayIcon =  _chkDspIcon->isChecked();
    _app->gpsData().rtDisplayPopup =  _chkDspPopup->isChecked();
    _app->gpsData().rtPopupAutoClose =  _chkPopupAutoClose->isChecked();
    _app->gpsData().rtAutoCloseTime =  _spnAutoCloseTime->value();
    _app->gpsData().rtUseTts =  _chkTts->isChecked();
    _app->gpsData().rtPreventValue =  _spnPreventDist->value();
    _app->gpsData().rtDrawLine =  _chkDrawLine->isChecked();
    _app->gpsData().rtLineThick =  _spnLineThick->value();
    
}

void RouteGUI::loadSetting()
{
    Config cfg("qpegps");
    cfg.setGroup("route");

    _chkDspIcon->setChecked(_app->gpsData().rtDisplayIcon = cfg.readBoolEntry("displayIcon", true));
    _chkDspPopup->setChecked(_app->gpsData().rtDisplayPopup = cfg.readBoolEntry("displayPopup", true));
    _chkPopupAutoClose->setChecked(_app->gpsData().rtPopupAutoClose = cfg.readBoolEntry("popupAutoClose", true));
    _spnAutoCloseTime->setValue(_app->gpsData().rtAutoCloseTime = cfg.readNumEntry("autoCloseTime", 15));
    _chkTts->setChecked(_app->gpsData().rtUseTts = cfg.readBoolEntry("tts", false));
    _cboTts->setCurrentItem(cfg.readNumEntry("ttsEngine", 0));
    _spnPreventDist->setValue(_app->gpsData().rtPreventValue = cfg.readNumEntry("preventValue", 500));
    _chkDrawLine->setChecked(_app->gpsData().rtDrawLine = cfg.readBoolEntry("drawLine", true));
    _spnLineThick->setValue(_app->gpsData().rtLineThick = cfg.readNumEntry("lineThick", 3));

}

void RouteGUI::saveSetting()
{
    Config cfg("qpegps");
    cfg.setGroup("route");

    cfg.writeEntry("displayIcon", _chkDspIcon->isChecked());
    cfg.writeEntry("displayPopup", _chkDspPopup->isChecked());
    cfg.writeEntry("popupAutoClose", _chkPopupAutoClose->isChecked());
    cfg.writeEntry("autoCloseTime", _spnAutoCloseTime->value());
    cfg.writeEntry("tts", _chkTts->isChecked());
    cfg.writeEntry("ttsEngine", _cboTts->currentItem());
    cfg.writeEntry("preventValue", _spnPreventDist->value());
    cfg.writeEntry("drawLine", _chkDrawLine->isChecked());
    cfg.writeEntry("lineThick", _spnLineThick->value());
    
}


RouteGUI::~RouteGUI()
{
    saveSetting();
}


RouteAlert::RouteAlert(Qpegps * app, QWidget * p, Route * rt)
  : QDialog(p, 0, false, Qt::WType_TopLevel /*| Qt::WType_Dialog*/ | Qt::WStyle_Customize | Qt::WStyle_NoBorder)
    , timeLeft(0)
{

    route = rt;
    
    _app = app;

    QVBoxLayout* frame10Layout;
    QHBoxLayout* layout10;
    QGridLayout* WStackPageLayout;
    QGridLayout* WStackPageLayout_2;
    QHBoxLayout* layout8;
    //QHBoxLayout* layout7;
    QWidget* WStackPage;
    QWidget* WStackPage_2;
    QButtonGroup* btnsTop;
    QPushButton* btnPlay;
    QPushButton* btnPrev;
    QPushButton* btnSuiv;
    QPushButton* btnClose;
    QLabel* lblRouteDone;
    //QLabel* lblRouteBComment;

    QSize s20x20( 20, 20 );
    QColor backColor( 255, 255, 255 );
    
    QGridLayout* RouteAlertBaseLayout = new QGridLayout( this, 1, 1, 2, 2, "RouteAlertBaseLayout"); 

    QFrame * frm = new QFrame( this );
    frm->setBackgroundColor( backColor );
    frm->setFrameShape( QFrame::Box );
    frm->setFrameShadow( QFrame::Plain );
    frame10Layout = new QVBoxLayout( frm, 2, 2, "frame10Layout"); 

    layout10 = new QHBoxLayout( 0, 0, 2, "layout10"); 

    lblInfo = new QLabel( frm, "lblInfo" );
    lblInfo->setBackgroundColor( backColor );
    QFont lblInfo_font(  lblInfo->font() );
    lblInfo_font.setBold( true );
    lblInfo->setFont( lblInfo_font ); 
    layout10->addWidget( lblInfo );

    btnsTop = new QButtonGroup( frm, "btnsTop" );
    btnsTop->setBackgroundColor( backColor );
    btnsTop->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, btnsTop->sizePolicy().hasHeightForWidth() ) );
    btnsTop->setMaximumSize( QSize( 32767, 22 ) );
    btnsTop->setFrameShape( QButtonGroup::NoFrame );

    btnList = new QPushButton( btnsTop, "btnList" );
    btnList->setGeometry( QRect( 2, 1, 20, 20 ) );
    btnList->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, btnList->sizePolicy().hasHeightForWidth() ) );
    btnList->setMinimumSize( s20x20 );
    btnList->setMaximumSize( s20x20 );
    btnList->setFocusPolicy( QPushButton::NoFocus );
    btnList->setToggleButton( true );
    btnList->setOn( false );
    btnList->setAutoDefault( false );

    btnPlay = new QPushButton( btnsTop, "btnPlay" );
    btnPlay->setGeometry( QRect( 24, 1, 20, 20 ) );
    btnPlay->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, btnPlay->sizePolicy().hasHeightForWidth() ) );
    btnPlay->setMinimumSize( s20x20 );
    btnPlay->setMaximumSize( s20x20 );
    btnPlay->setFocusPolicy( QPushButton::NoFocus );
    btnPlay->setAutoDefault( false );

    btnPrev = new QPushButton( btnsTop, "btnPrev" );
    btnPrev->setGeometry( QRect( 46, 1, 20, 20 ) );
    btnPrev->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, btnPrev->sizePolicy().hasHeightForWidth() ) );
    btnPrev->setMinimumSize( s20x20 );
    btnPrev->setMaximumSize( s20x20 );
    btnPrev->setFocusPolicy( QPushButton::NoFocus );
    btnPrev->setAutoDefault( false );

    btnSuiv = new QPushButton( btnsTop, "btnSuiv" );
    btnSuiv->setGeometry( QRect( 68, 1, 20, 20 ) );
    btnSuiv->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, btnSuiv->sizePolicy().hasHeightForWidth() ) );
    btnSuiv->setMinimumSize( s20x20 );
    btnSuiv->setMaximumSize( s20x20 );
    btnSuiv->setFocusPolicy( QPushButton::NoFocus );
    btnSuiv->setAutoDefault( false );

    btnClose = new QPushButton( btnsTop, "btnClose" );
    btnClose->setGeometry( QRect( 90, 1, 20, 20 ) );
    btnClose->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, btnClose->sizePolicy().hasHeightForWidth() ) );
    btnClose->setMinimumSize( s20x20 );
    btnClose->setMaximumSize( s20x20 );
    btnClose->setFocusPolicy( QPushButton::NoFocus );
    btnClose->setAutoDefault( false );
    layout10->addWidget( btnsTop );
    frame10Layout->addLayout( layout10 );
    
    

    QFrame * line = new QFrame( frm );
    line->setFrameShape( QFrame::HLine );
    line->setFrameShadow( QFrame::Plain );
    line->setFrameShape( QFrame::HLine );
    frame10Layout->addWidget( line );

    widgetStack1 = new QWidgetStack( frm, "widgetStack1" );
    widgetStack1->setBackgroundColor( backColor );

    WStackPage = new QWidget( widgetStack1, "WStackPage" );
    WStackPage->setBackgroundColor( backColor );
    WStackPageLayout = new QGridLayout( WStackPage, 1, 1, 2, 2, "WStackPageLayout"); 

    lblComment = new QLabel( WStackPage, "lblComment" );
    lblComment->setBackgroundColor( backColor );
    QFont lblComment_font(  lblComment->font() );
    lblComment_font.setPointSize( 20 );
    lblComment_font.setBold( true );
    lblComment->setFont( lblComment_font ); 
    lblComment->setTextFormat( QLabel::PlainText );
    lblComment->setAlignment( int( QLabel::WordBreak | QLabel::AlignCenter ) );

    WStackPageLayout->addWidget( lblComment, 0, 0 );
    widgetStack1->addWidget( WStackPage, 0 );

    WStackPage_2 = new QWidget( widgetStack1, "WStackPage_2" );
    WStackPage_2->setBackgroundColor( backColor );
    WStackPageLayout_2 = new QGridLayout( WStackPage_2, 1, 1, 2, 2, "WStackPageLayout_2"); 

    lbComments = new QListBox( WStackPage_2, "lbComments" );
    lbComments->setBackgroundColor( backColor );

    WStackPageLayout_2->addWidget( lbComments, 0, 0 );
    widgetStack1->addWidget( WStackPage_2, 1 );
    frame10Layout->addWidget( widgetStack1 );

    line = new QFrame( frm );
    line->setFrameShape( QFrame::HLine );
    line->setFrameShadow( QFrame::Plain );
    line->setFrameShape( QFrame::HLine );
    frame10Layout->addWidget( line );

    layout8 = new QHBoxLayout( 0, 0, 2, "layout8"); 

    lblRouteDone = new QLabel( frm, "lblRouteDone" );
    lblRouteDone->setBackgroundColor( backColor );
    lblRouteDone->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, lblRouteDone->sizePolicy().hasHeightForWidth() ) );
    layout8->addWidget( lblRouteDone );

    lblRouteDoneTotal = new QLabel( frm, "lblRouteDoneTotal" );
    lblRouteDoneTotal->setBackgroundColor( backColor );
    lblRouteDoneTotal->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, lblRouteDoneTotal->sizePolicy().hasHeightForWidth() ) );
    layout8->addWidget( lblRouteDoneTotal );
    frame10Layout->addLayout( layout8 );

    /*layout7 = new QHBoxLayout( 0, 0, 2, "layout7"); 

    lblRouteBComment = new QLabel( frm, "lblRouteBComment" );
    lblRouteBComment->setBackgroundColor( backColor );
    lblRouteBComment->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, lblRouteBComment->sizePolicy().hasHeightForWidth() ) );
    layout7->addWidget( lblRouteBComment );

    lblRouteBeforeComment = new QLabel( frm, "lblRouteBeforeComment" );
    lblRouteBeforeComment->setBackgroundColor( backColor );
    lblRouteBeforeComment->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, lblRouteBeforeComment->sizePolicy().hasHeightForWidth() ) );
    layout7->addWidget( lblRouteBeforeComment );
    frame10Layout->addLayout( layout7 );*/

    autoCloseLine = new QFrame( frm );
    autoCloseLine->setFrameShape( QFrame::HLine );
    autoCloseLine->setFrameShadow( QFrame::Plain );
    autoCloseLine->setFrameShape( QFrame::HLine );
    frame10Layout->addWidget( autoCloseLine );

    chkAutoClose = new QCheckBox( frm, "chkAutoClose" );
    chkAutoClose->setBackgroundColor( backColor );
    chkAutoClose->setFocusPolicy( QCheckBox::NoFocus );
    frame10Layout->addWidget( chkAutoClose );

    RouteAlertBaseLayout->addWidget( frm, 0, 0 );

    //btnsTop->setTitle( QString::null );
    btnList->setPixmap( QPixmap(app->gpsData().iconsPathStr + "/liste.xpm") );
    btnPlay->setPixmap( QPixmap(app->gpsData().iconsPathStr + "/hp.xpm") );
    btnPrev->setPixmap( QPixmap(app->gpsData().iconsPathStr + "/prev.xpm") );
    btnSuiv->setPixmap( QPixmap(app->gpsData().iconsPathStr + "/suiv.xpm") );
    btnClose->setPixmap(QPixmap(app->gpsData().iconsPathStr + "/close.xpm") );
    lblRouteDone->setText( tr( "Route done:" ) );
    //lblRouteBComment->setText( tr( "Route before next info:" ) );

    
    
    //resize( QSize(228, 287).expandedTo(minimumSizeHint()) );
    resize( QSize(220, 260).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
    
    autoCloseTimer = new QTimer(this);

    // signals and slots connections
    //connect( btnList, SIGNAL( stateChanged(int) ), widgetStack1, SLOT( raiseWidget(int) ) );
    connect( btnClose, SIGNAL( clicked() ), this, SLOT( close() ) );
    connect( btnsTop, SIGNAL( pressed(int) ), this, SLOT( btnsTopClicked(int) ) );
    connect( chkAutoClose, SIGNAL( clicked() ), this, SLOT( chkAutoCloseClicked() ) );
    connect( lbComments, SIGNAL( highlighted(int) ), this, SLOT( listSel(int) ) );
    connect( lbComments, SIGNAL( selected(int) ), this, SLOT( listSel(int) ) );
    connect( autoCloseTimer, SIGNAL(timeout()), this, SLOT(timerDone()) );
    connect( route, SIGNAL(positionChanged()), this, SLOT(routeInfoUpdated()) );
    connect( route, SIGNAL(routeChanged()), this, SLOT(routeChanged()) );
    
    widgetStack1->raiseWidget(0);

}


void RouteAlert::routeInfoUpdated()
{
    if(isVisible())
        //lblRouteBeforeComment->setText( Distance::toStringFromKm(route->distanceBeforeNextComment() / 1000) );
        lblRouteDoneTotal->setText(   Distance::toStringFromKm(route->done() / 1000)
                                    + tr(" on ")
                                    + Distance::toStringFromKm(route->length() / 1000) );
}

void RouteAlert::timerDone()
{
    timeLeft--;
    
    if(timeLeft < 0)
    {
        autoCloseTimer->stop();
        close();
    }
    else
        chkAutoClose->setText( tr( "Auto close in %1 s" ).arg(timeLeft) );
}

void RouteAlert::routeChanged()
{
    lbComments->clear();
    
    RoutePoint * pt = route->commentedRoutePoints().first();
    while(pt)
    {
        lbComments->insertItem(pt->comment());
        pt = route->commentedRoutePoints().next();
    }
}

void RouteAlert::show(int commentIndex, bool autoClose, int autoCloseTime )
{

    if(autoClose)
    {
        
        chkAutoClose->show();
        autoCloseLine->show();
        timeLeft = autoCloseTime + 1;
        
        chkAutoClose->setChecked(true);
        autoCloseTimer->start(1000);
        timerDone();
    }
    else
    {
        chkAutoClose->hide();
        autoCloseLine->hide();
        autoCloseTimer->stop();
        timeLeft = 0;
    }
    
    showComment(commentIndex);
    QDialog::show();
    routeInfoUpdated();
    
}

void RouteAlert::btnsTopClicked(int i)
{
    switch(i)
    {
        case 0: /* show list */
            widgetStack1->raiseWidget( (btnList->isOn()) ? 0 : 1 );
            break;
        case 1: /* say comment */
            _app->speakers().current()->sayText( lblComment->text() );
            break;
        case 2: /* < */
            if(currentComment > 1)
                showComment(currentComment - 1);
            break;
        case 3: /* > */
            if(currentComment < lbComments->count() )
                showComment(currentComment + 1);
            break;
    };
}

void RouteAlert::chkAutoCloseClicked()
{
     if(chkAutoClose->isChecked())
        autoCloseTimer->start(1000);
    else
        autoCloseTimer->stop();
}

void RouteAlert::listSel(int i)
{
    showComment(i + 1);
}

void RouteAlert::showComment(int i)
{
    widgetStack1->raiseWidget(0);
    btnList->setOn(false);
    currentComment = i;
    lblComment->setText( lbComments->text(i - 1) );
    lblInfo->setText( tr( "Route info #%1" ).arg(i) );
}


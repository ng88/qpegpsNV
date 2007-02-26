
TEMPLATE        = app
#CONFIG         = qt warn_on debug
CONFIG          = qt warn_on release
HEADERS         = gpsdata.h \
		  client.h \
		  maps.h \
		  mapdisp.h \
		  mapinfo.h \
		  settings.h \
		  fetchmap.h \	  
		  route.h \  
		  dirview.h \
		  dirdialog.h \
		  gpsstatus.h \
          track.h \
		  qpegps.h \
                  colordlg.h \ 
                  datum/datum.h

SOURCES         = gpsdata.cpp \
		  client.cpp \
		  mapdisp.cpp \
		  mapinfo.cpp \
		  maps.cpp \
		  settings.cpp \
		  fetchmap.cpp \
		  route.cpp \
		  dirview.cpp \
		  dirdialog.cpp \
		  gpsstatus.cpp \
		  track.cpp \
		  qpegps.cpp \
                  colordlg.cpp \
                  datum/datum.c \
                  datum/ellipse.c \
                  datum/geocent.c 

INCLUDEPATH     += $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include
LIBS            += -lqpe -L$(QPEDIR)/lib
#REQUIRES        = network large-config
TARGET		= qpegps
DISTFILES	= COPYING gpsrate.c cfgps
TRANSLATIONS    = qpegps_de.ts qpegps_jp.ts

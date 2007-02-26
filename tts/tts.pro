TEMPLATE	= lib
CONFIG		+= qt warn_on release
#CONFIG		+= qt warn_on debug
HEADERS	=	tts.h ttsimpl.h
SOURCES	=	tts.cpp ttsimpl.cpp
TARGET		= ttsapplet
DESTDIR		= $(QPEDIR)/plugins/applets
INCLUDEPATH += $(QPEDIR)/include flite/include flite/lang/cmu_us_kal16
LIBS            += -lqpe -Lflite/lib -lflite_cmu_us_kal16 -lflite_usenglish -lflite_cmulex -lflite -lm 
DISTFILES = opie-tts.control opie-tts.postinst opie-tts.postrm
VERSION		= 1.0.0

 

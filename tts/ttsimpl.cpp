#include "tts.h"
#include "ttsimpl.h"


TtsImpl::TtsImpl()
    : tts(0), ref(0) {
}

TtsImpl::~TtsImpl() {
    delete tts;
}

QWidget *TtsImpl::applet( QWidget *parent ) {
    if ( !tts ) {
	tts = new Tts( parent );
    }
    return tts;
}

int TtsImpl::position() const {
    return 6;
}

QRESULT TtsImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface ) {
    *iface = 0;
    if ( uuid == IID_QUnknown ) {
	*iface = this;
    } else if ( uuid == IID_TaskbarApplet ) {
	*iface = this;
    }

    if ( *iface ) {
	(*iface)->addRef();
    }
    return QS_OK;
}

Q_EXPORT_INTERFACE() {
    Q_CREATE_INSTANCE( TtsImpl )
}

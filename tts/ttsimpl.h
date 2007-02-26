#include <qpe/taskbarappletinterface.h>

class Tts;

class TtsImpl : public TaskbarAppletInterface {
public:
    TtsImpl();
    virtual ~TtsImpl();

    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
    Q_REFCOUNT

    virtual QWidget *applet( QWidget *parent );
    virtual int position() const;

private:
    Tts *tts;
    ulong ref;
};

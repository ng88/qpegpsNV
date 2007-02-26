#ifndef EXAMPLE_H
#define EXAMPLE_H
#include "examplebase.h"

class Example : public ExampleBase
{ 
    Q_OBJECT

public:
    Example( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~Example();

    QStringList listOfMsg;
    unsigned int pos;

private slots:
    void saySomething();
};

#endif // EXAMPLE_H

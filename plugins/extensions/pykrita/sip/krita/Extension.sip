class Extension : QObject
{

%TypeHeaderCode
#include "Extension.h"
%End

public:
    explicit Extension(QObject *parent  /TransferThis/ = 0);
    virtual void setup() = 0;
    virtual void createActions(Window *window) = 0;
};

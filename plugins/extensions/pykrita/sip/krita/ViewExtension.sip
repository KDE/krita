class ViewExtension : QObject
{

%TypeHeaderCode
#include "ViewExtension.h"
%End

public:
    explicit ViewExtension(QObject *parent  /TransferThis/ = 0);
    virtual void setup() = 0;
};

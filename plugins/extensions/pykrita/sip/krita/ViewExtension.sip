%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtWidgets/QtWidgetsmod.sip

class ViewExtension : public QObject
{

%TypeHeaderCode
#include "ViewExtension.h"
%End

public:
    explicit ViewExtension(QObject *parent  /TransferThis/ = 0);
    virtual void setup() = 0;
};

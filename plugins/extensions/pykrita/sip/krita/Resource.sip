%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtWidgets/QtWidgetsmod.sip

class Resource : public QObject
{
%TypeHeaderCode
#include "Resource.h"
%End

public:
    explicit Resource(QObject *parent  /TransferThis/ = 0);
};

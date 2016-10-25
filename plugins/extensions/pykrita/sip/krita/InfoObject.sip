%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtWidgets/QtWidgetsmod.sip

class InfoObject : public QObject
{
%TypeHeaderCode
#include "InfoObject.h"
%End

public:
    explicit InfoObject(QObject *parent  /TransferThis/ = 0);
};

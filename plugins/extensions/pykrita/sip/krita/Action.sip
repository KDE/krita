%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtWidgets/QtWidgetsmod.sip

class Action : public QObject
{
%TypeHeaderCode
#include "Action.h"
%End

public:
    explicit Action(QObject *parent  /TransferThis/ = 0);
};

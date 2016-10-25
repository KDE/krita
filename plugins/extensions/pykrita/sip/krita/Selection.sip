%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtWidgets/QtWidgetsmod.sip

class Selection : public QObject
{
%TypeHeaderCode
#include "Selection.h"
%End

public:
    explicit Selection(QObject *parent  /TransferThis/ = 0);
};

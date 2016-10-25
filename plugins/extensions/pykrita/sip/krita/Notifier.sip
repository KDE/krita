%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtWidgets/QtWidgetsmod.sip

class Notifier : public QObject
{
%TypeHeaderCode
#include "Notifier.h"
%End

public:
    explicit Notifier(QObject *parent  /TransferThis/ = 0);
};

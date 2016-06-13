%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtWidgets/QtWidgetsmod.sip

class Application : public QObject
{
%TypeHeaderCode
#include "application.h"
%End

public:
    explicit Application(QObject *parent  /TransferThis/ = 0);
};

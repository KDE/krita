%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtWidgets/QtWidgetsmod.sip

class Generator : public QObject
{
%TypeHeaderCode
#include "Generator.h"
%End

public:
    explicit Generator(QObject *parent  /TransferThis/ = 0);
};

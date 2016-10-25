%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtWidgets/QtWidgetsmod.sip

class ColorDepth : public QObject
{
%TypeHeaderCode
#include "ColorDepth.h"
%End

public:
    explicit ColorDepth(QObject *parent  /TransferThis/ = 0);
};

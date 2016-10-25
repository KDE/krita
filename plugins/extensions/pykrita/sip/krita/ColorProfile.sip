%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtWidgets/QtWidgetsmod.sip

class ColorProfile : public QObject
{
%TypeHeaderCode
#include "ColorProfile.h"
%End

public:
    explicit ColorProfile(QObject *parent  /TransferThis/ = 0);
};

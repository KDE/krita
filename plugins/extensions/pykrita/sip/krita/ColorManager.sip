%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtWidgets/QtWidgetsmod.sip

class ColorManager : public QObject
{
%TypeHeaderCode
#include "ColorManager.h"
%End

public:
    explicit ColorManager(QObject *parent  /TransferThis/ = 0);
};

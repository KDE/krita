%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtWidgets/QtWidgetsmod.sip

class ColorModel : public QObject
{
%TypeHeaderCode
#include "ColorModel.h"
%End

public:
    explicit ColorModel(QObject *parent  /TransferThis/ = 0);
};

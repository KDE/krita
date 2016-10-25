%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtWidgets/QtWidgetsmod.sip

class Transformation : public QObject
{
%TypeHeaderCode
#include "Transformation.h"
%End

public:
    explicit Transformation(QObject *parent  /TransferThis/ = 0);
};

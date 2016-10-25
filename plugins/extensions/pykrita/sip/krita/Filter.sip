%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtWidgets/QtWidgetsmod.sip

class Filter : public QObject
{
%TypeHeaderCode
#include "Filter.h"
%End

public:
    explicit Filter(QObject *parent  /TransferThis/ = 0);
};

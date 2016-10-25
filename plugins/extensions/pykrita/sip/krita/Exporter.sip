%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtWidgets/QtWidgetsmod.sip

class Exporter : public QObject
{
%TypeHeaderCode
#include "Exporter.h"
%End

public:
    explicit Exporter(QObject *parent  /TransferThis/ = 0);
};

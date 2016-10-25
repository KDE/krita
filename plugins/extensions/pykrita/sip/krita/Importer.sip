%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtWidgets/QtWidgetsmod.sip

class Importer : public QObject
{
%TypeHeaderCode
#include "Importer.h"
%End

public:
    explicit Importer(QObject *parent  /TransferThis/ = 0);
};

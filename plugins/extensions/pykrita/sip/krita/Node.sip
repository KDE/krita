%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtWidgets/QtWidgetsmod.sip

class Node : public QObject
{
%TypeHeaderCode
#include "Node.h"
%End

public:
    explicit Node(QObject *parent  /TransferThis/ = 0);
};

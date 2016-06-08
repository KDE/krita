%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

class Node : public QObject
{

%TypeHeaderCode
#include "node.h"
%End

public:
    explicit Node(QObject *parent  /TransferThis/ = 0);

};

%Module PyKrita4.Node

%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

class Node : public QObject
{

%TypeHeaderCode
#include <../../libkis/node.h>
%End

public:
    explicit Node(QObject *parent  /TransferThis/ = 0);

};

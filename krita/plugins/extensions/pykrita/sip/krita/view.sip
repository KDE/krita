%Module PyKrita4.View

%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

class View : public QObject
{
%TypeHeaderCode
#include <../../libkis/view.h>
%End

public:
    explicit View(void */In/, QObject *parent  /TransferThis/ = 0);

};

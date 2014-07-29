%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

class View : public QObject
{
%TypeHeaderCode
#include "view.h"
%End

public:
    explicit View(QObject *view, QObject *parent  /TransferThis/ = 0);

};

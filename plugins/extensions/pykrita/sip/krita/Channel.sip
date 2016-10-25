%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtWidgets/QtWidgetsmod.sip

class Channel : public QObject
{
%TypeHeaderCode
#include "Channel.h"
%End

public:
    explicit Channel(QObject *parent  /TransferThis/ = 0);
};

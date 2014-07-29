%Module PyKrita4.Application

%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

class Application : public QObject
{
%TypeHeaderCode
#include <../../libkis/application.h>
%End

public:
    explicit Application(QObject *parent  /TransferThis/ = 0);
};

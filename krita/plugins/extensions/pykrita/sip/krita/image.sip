%Module PyKrita4.Image

%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

class Image : public QObject
{
%TypeHeaderCode
#include <../../libkis/image.h>
%End

public:
    explicit Image(void */In/, QObject *parent  /TransferThis/ = 0);

};
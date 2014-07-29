%Module PyKrita4.Document

%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

class Document : public QObject
{
%TypeHeaderCode
#include <../../libkis/document.h>
%End

public:
    explicit Document(void */In/, QObject *parent  /TransferThis/ = 0);

    Image *image();
};

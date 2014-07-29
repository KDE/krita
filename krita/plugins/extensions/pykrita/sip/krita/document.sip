%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

class Document : public QObject
{
%TypeHeaderCode
#include "document.h"
%End

public:
    explicit Document(QObject *document, QObject *parent  /TransferThis/ = 0);

    Image *image();
};

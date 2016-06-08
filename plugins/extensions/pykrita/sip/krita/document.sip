%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

class Document : public QObject /NoDefaultCtors/
{
%TypeHeaderCode
#include "document.h"
%End

public:

    Image *image();
};

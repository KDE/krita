%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

class Image : public QObject
{
%TypeHeaderCode
#include "image.h"
%End

public:
    explicit Image(QObject *image, QObject *parent  /TransferThis/ = 0);

};

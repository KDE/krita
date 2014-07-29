%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

class Krita : public QObject
{
%TypeHeaderCode
#include "krita.h"
%End

public:
    Krita(QObject *parent /TransferThis/ = 0);

    QList<MainWindow*> mainWindows();
    QList<View*> views();
    QList<Document*> documents();
    QList<Image*> images();

};


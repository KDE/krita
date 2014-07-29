%Module PyKrita4.Module

%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

class Module : public QObject
{
%TypeHeaderCode
#include <module.h>
%End

public:
    Module(QObject *parent /TransferThis/ = 0);

    QList<MainWindow*> mainWindows();
    QList<View*> views();
    QList<Document*> documents();
    QList<Image*> images();

};


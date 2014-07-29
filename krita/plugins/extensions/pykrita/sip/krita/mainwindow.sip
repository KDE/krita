%Module PyKrita4.MainWindow

%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip


class MainWindow : public QObject
{
%TypeHeaderCode
#include <mainwindow.h>
%End

public:
    MainWindow(void */In/, QObject *parent  /TransferThis/ = 0);
    QList<View*> views();
};

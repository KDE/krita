%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

class Krita : public QObject
{
%TypeHeaderCode
#include "krita.h"
%End

    Krita(QObject *parent /TransferThis/ = 0);
public:
    static Krita* instance();
    QList<MainWindow*> mainWindows();
    QList<View*> views();
    QList<Document*> documents();
    QList<Image*> images();

    QAction *createAction(const QString &text);

    void addViewExtension(ViewExtension* _viewExtension /TransferThis/);
    void addDockWidgetFactory(DockWidgetFactoryBase* _factory /Transfer, TransferThis/);
};


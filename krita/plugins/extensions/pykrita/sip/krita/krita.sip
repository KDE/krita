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

    void addViewExtension(ViewExtension* _viewExtension /GetWrapper/);
%MethodCode
        Py_BEGIN_ALLOW_THREADS
        sipCpp->addViewExtension(a0);
        Py_END_ALLOW_THREADS
        
        sipTransferTo(a0Wrapper, Py_None);
%End
    void addDockWidgetFactory(DockWidgetFactoryBase* _factory /GetWrapper/);
%MethodCode
        Py_BEGIN_ALLOW_THREADS
        sipCpp->addDockWidgetFactory(a0);
        Py_END_ALLOW_THREADS
        
        sipTransferTo(a0Wrapper, Py_None);
%End
};


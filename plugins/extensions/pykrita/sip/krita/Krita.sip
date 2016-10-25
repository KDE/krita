%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtWidgets/QtWidgetsmod.sip

class Krita : public QObject
{
%TypeHeaderCode
#include "Krita.h"
%End

public:
    explicit Krita(QObject *parent  /TransferThis/ = 0);
    public:
    static Krita* instance();
    QList<Window*> windows();
    QList<View*> views();
    QList<Document*> documents();
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

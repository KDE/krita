class Krita : QObject
{
%TypeHeaderCode
#include "Krita.h"
%End
public:
    Krita(QObject*  parent /TransferThis/ = 0);
    virtual ~Krita();
    Document * activeDocument() const;
    void setActiveDocument(Document*  value);
    bool batchmode() const;
    void setBatchmode(bool value);
    QVariantMap actions() const;
    QVariantList documents() const;
    QVariantList exporters() const;
    QVariantList filters() const;
    QVariantList generators() const;
    QVariantList importers() const;
    Notifier * notifier() const;
    InfoObject * preferences() const;
    void setPreferences(InfoObject*  value);
    QString version() const;
    QVariantList views() const;
    QVariantList windows() const;
    QVariantList resources() const;
    void setResources(QList<Resource *> value);
public Q_SLOTS:
    void addDockWidget(DockWidget*  dockWidget);
    void addAction(Action*  action);
    bool closeApplication();
    Document * createDocument();
    Document * openDocument();
    Window * openWindow();
    QAction * createAction(const QString & text);
    void addViewExtension(ViewExtension*  viewExtension /GetWrapper/);
%MethodCode
        Py_BEGIN_ALLOW_THREADS
        sipCpp->addViewExtension(a0);
        Py_END_ALLOW_THREADS

        sipTransferTo(a0Wrapper, Py_None);
%End
    
    QList<ViewExtension *> viewExtensions();

    void addDockWidgetFactory(DockWidgetFactoryBase*  factory /GetWrapper/);
%MethodCode
        Py_BEGIN_ALLOW_THREADS
        sipCpp->addDockWidgetFactory(a0);
        Py_END_ALLOW_THREADS

        sipTransferTo(a0Wrapper, Py_None);
%End

    static Krita * instance();
    static QObject * fromVariant(const QVariant & v);
private:
    private:
        Krita(const Krita &); // Generated
};

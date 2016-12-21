class Krita : QObject
{
%TypeHeaderCode
#include "Krita.h"
%End
public:
public:
    Krita(QObject*  parent /TransferThis/ = 0);
    virtual ~Krita();
    Document * activeDocument() const;
    void setActiveDocument(Document*  value);
    bool batchmode() const;
    void setBatchmode(bool value);
    QList<Action *> actions() const;
    Action * action(const QString & name) const;
    QList<Document *> documents() const;
    QList<Filter *> filters() const;
    QList<Generator *> generators() const;
    Notifier * notifier() const;
    InfoObject * preferences() const;
    void setPreferences(InfoObject*  value);
    QString version() const;
    QList<View *> views() const /Factory/;
    Window * activeWindow() const /Factory/;
    QList<Window *> windows() const /Factory/;
    QList<Resource *> resources() const;
    void setResources(QList<Resource *> value);
public Q_SLOTS:
    void addDockWidget(DockWidget*  dockWidget);
    void addAction(Action*  action);
    bool closeApplication();
    Document * createDocument(int width, int height, const QString &name, const QString &colorModel, const QString &colorDepth, const QString &profile)  /Factory/;
    Document * openDocument(const QString &filename)  /Factory/;
    Window * openWindow();
    QAction * createAction(const QString & text);

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

    static Krita * instance();
    static QObject * fromVariant(const QVariant & v);
private:
    Krita(const Krita &); // Generated
};

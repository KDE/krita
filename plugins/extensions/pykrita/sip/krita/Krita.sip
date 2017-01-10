class Krita : QObject
{
%TypeHeaderCode
#include "Krita.h"
%End
public:
public:
    Krita(QObject*  parent /TransferThis/ = 0);
    virtual ~Krita();
    Document * activeDocument() const /Factory/;
    void setActiveDocument(Document*  value);
    bool batchmode() const;
    void setBatchmode(bool value);
    QList<Action *> actions() const /Factory/;
    Action * action(const QString & name) const;
    QList<Document *> documents() const /Factory/;
    QStringList filters() const;
    Filter * filter(const QString &name) const /Factory/;
    QStringList generators() const /Factory/;
    Generator * generator(const QString &name) const /Factory/;
    Notifier * notifier() const /Factory/;
    InfoObject * preferences() const /Factory/;
    void setPreferences(InfoObject*  value);
    QString version() const;
    QList<View *> views() const /Factory/;
    Window * activeWindow() const /Factory/;
    QList<Window *> windows() const /Factory/;
    QList<Resource *> resources() const /Factory/;
    void setResources(QList<Resource *> value);
public Q_SLOTS:
    void addDockWidget(DockWidget*  dockWidget);
    bool closeApplication();
    Document * createDocument(int width, int height, const QString &name, const QString &colorModel, const QString &colorDepth, const QString &profile)  /Factory/;
    Document * openDocument(const QString &filename)  /Factory/;
    Window * openWindow();
    Action * createAction(const QString & text);

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


class Krita : QObject
{
%TypeHeaderCode
#include "Krita.h"
%End
public:
    Krita(QObject*  parent /TransferThis/ = 0);
    virtual ~Krita();
public Q_SLOTS:

    Document * activeDocument() const /Factory/;
    void setActiveDocument(Document*  value);
    bool batchmode() const;
    void setBatchmode(bool value);
    QList<QAction *> actions() const;
    QAction *action(const QString & name) const;
    QList<Document *> documents() const /Factory/;
    QList<QDockWidget *> dockers() const;

    QStringList filters() const;
    Filter * filter(const QString &name) const /Factory/;
    QStringList colorModels() const;
    QStringList colorDepths(const QString &colorModel) const;
    QStringList filterStrategies() const;
    QStringList profiles(const QString &colorModel, const QString &ColorDepth) const;
    bool addProfile(const QString &profilePath);
    Notifier * notifier() const;
    QString version() const;
    QList<View *> views() const /Factory/;
    Window * activeWindow() const /Factory/;
    QList<Window *> windows() const /Factory/;
    QMap<QString, Resource *> resources(QString &type) const /Factory/;
    QStringList recentDocuments() const;
    Document * createDocument(int width, int height, const QString &name, const QString &colorModel, const QString &colorDepth, const QString &profile, double resolution)  /Factory/;
    QList<Extension*> extensions() /Factory/;
    Document * openDocument(const QString &filename)  /Factory/;
    Window * openWindow();
    QIcon icon(QString &iconName) const;

    void addExtension(Extension* _extension /GetWrapper/);
%MethodCode
        Py_BEGIN_ALLOW_THREADS
        sipCpp->addExtension(a0);
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

    void writeSetting(const QString &group, const QString &name, const QString &value);
    QString readSetting(const QString &group, const QString &name, const QString &defaultValue);

    static Krita * instance();
    static QObject * fromVariant(const QVariant & v);
    static QString krita_i18n(const QString &text);
    static QString krita_i18nc(const QString &context, const QString &text);
private:
    Krita(const Krita &); // Generated
};

class Notifier : QObject
{
%TypeHeaderCode
#include "Notifier.h"
%End
    Notifier(const Notifier & __0);
public:
    Notifier(QObject*  parent /TransferThis/ = 0);
    virtual ~Notifier();
    bool active() const;
    void setActive(bool value);
Q_SIGNALS:
    void applicationClosing();
    void imageCreated(Document*  image);
    void imageSaved(const QString &filename);
    void imageClosed(const QString &filename);
    void viewCreated(View *view);
    void viewClosed(View *view);
    void windowCreated(Window *window);
    void configurationChanged();
};

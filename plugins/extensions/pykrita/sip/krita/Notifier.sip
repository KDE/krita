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
    void applicationStarted();
    void applicationClosed();
    void imageCreated(Document*  image);
    void imageLoaded(Document*  image);
    void imageSaved(Document*  image);
    void imageClosed(Document*  image);
    void nodeCreated(Document*  node);
private:
};

class Window : QObject
{
%TypeHeaderCode
#include "Window.h"
%End
    Window(const Window & __0);
public:
    QList<View *> views() const /Factory/;
    void setViews(QList<View *> value);
    QString viewMode() const;
    void setViewMode(QString value);
public Q_SLOTS:
    void activate();
    void close();
Q_SIGNALS:
    void windowClosed();
private:
};

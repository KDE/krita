class Window : QObject
{
%TypeHeaderCode
#include "Window.h"
%End
    Window(const Window & __0);
public:
    QList<View *> views() const;
    void setViews(QList<View *> value);
    QString viewMode() const;
    void setViewMode(QString value);
public Q_SLOTS:
    void Close(bool confirm);
private:
};

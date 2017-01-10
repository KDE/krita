class Window : QObject
{
%TypeHeaderCode
#include "Window.h"
%End
    Window(const Window & __0);
public Q_SLOTS:
    QList<View *> views() const /Factory/;
    void addView(Document *document);
    void showView(View *view);
    void activate();
    void close();
Q_SIGNALS:
    void windowClosed();
private:
};

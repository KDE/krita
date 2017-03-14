class Window : QObject
{
%TypeHeaderCode
#include "Window.h"
%End
    Window(const Window & __0);
public:    
    bool operator==(const Window &other) const;
    bool operator!=(const Window &other) const;

public Q_SLOTS:
    QMainWindow *qwindow() const;
    QList<View *> views() const /Factory/;
    View *addView(Document *document)  /Factory/;
    void showView(View *view);
    void activate();
    void close();
Q_SIGNALS:
    void windowClosed();
private:
};

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
    View *activeView() const /Factory/;
    void activate();
    void close();
    QAction *createAction(const QString &id, const QString &text = QString(), const QString &menuLocation = QString("tools/scripts"));
Q_SIGNALS:
    void windowClosed();
private:
};

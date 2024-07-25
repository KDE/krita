class Window : QObject
{
%TypeHeaderCode
#include "Window.h"
%End

%ConvertToSubClassCode
  // Need to add sub class conversion, otherwise a Window passed to a Signal is provided as a QObject instead of
  // a Window object
  // (not sure why, but explicit conversion here fix the problem)
  Window* mc = qobject_cast<Window*>(sipCpp);
  sipType = 0;
  if (mc) {
      sipType = sipType_Window;
  }
%End

    Window(const Window & __0);
public:
    bool operator==(const Window &other) const;
    bool operator!=(const Window &other) const;

public Q_SLOTS:
    QMainWindow *qwindow() const;
    QList<QDockWidget *> dockers() const;
    QList<View *> views() const /Factory/;
    View *addView(Document *document)  /Factory/;
    void showView(View *view);
    View *activeView() const /Factory/;
    void activate();
    void close();
    QAction *createAction(const QString &id, const QString &text = QString(), const QString &menuLocation = QString("tools/scripts"));
Q_SIGNALS:
    void windowClosed();
    void themeChanged();
    void activeViewChanged();
private:
};

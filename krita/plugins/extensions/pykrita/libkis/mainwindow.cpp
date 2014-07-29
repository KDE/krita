#include "mainwindow.h"

#include <KoMainWindow.h>
#include <kis_view2.h>

#include "view.h"

MainWindow::MainWindow(KoMainWindow *mainWin, QObject *parent)
    : QObject(parent)
    , m_mainWindow(mainWin)
{
}

QList<View *> MainWindow::views()
{
    QList<View*> ret;
    KisView2 *view = qobject_cast<KisView2*>(m_mainWindow->rootView());
    if (view) {
        ret << new View(view, this);
    }
    return ret;
}

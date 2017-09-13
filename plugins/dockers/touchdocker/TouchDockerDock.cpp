/*
 *  Copyright (c) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "TouchDockerDock.h"

#include <QtQuickWidgets/QQuickWidget>
#include <QQmlEngine>
#include <QQmlContext>
#include <QAction>
#include <QUrl>

#include <klocalizedstring.h>
#include <kactioncollection.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include <KoDialog.h>
#include <KoResourcePaths.h>
#include <kis_icon.h>
#include <KoCanvasBase.h>
#include <KisViewManager.h>
#include <kis_canvas2.h>
#include <KisMainWindow.h>
#include <KisViewManager.h>
#include <kis_config.h>
#include <KisPart.h>
#include <KisDocument.h>
#include <KisMimeDatabase.h>
#include <kis_action_manager.h>
#include <kis_action.h>

#include <Theme.h>
#include <Settings.h>
#include <DocumentManager.h>
#include <KisSketchView.h>

class TouchDockerDock::Private
{
public:
    Private()
    {
    }

    TouchDockerDock *q;
    bool allowClose {true};
    KisViewManager *viewManager {0};
    KisSketchView *sketchView {0};
    QString currentSketchPage;
    KoDialog *openDialog {0};
    KoDialog *saveAsDialog {0};
};


TouchDockerDock::TouchDockerDock( )
    : QDockWidget(i18n("Touch Docker"))
    , d(new Private())
{
    m_quickWidget = new QQuickWidget(this);
    setWidget(m_quickWidget);
    setEnabled(true);
    m_quickWidget->engine()->rootContext()->setContextProperty("mainWindow", this);

    m_quickWidget->engine()->addImportPath(KoResourcePaths::getApplicationRoot() + "/lib/qml/");
    m_quickWidget->engine()->addImportPath(KoResourcePaths::getApplicationRoot() + "/lib64/qml/");

    Settings *settings = new Settings(this);
    DocumentManager::instance()->setSettingsManager(settings);
    m_quickWidget->engine()->rootContext()->setContextProperty("Settings", settings);

    Theme *theme = Theme::load(KSharedConfig::openConfig()->group("General").readEntry<QString>("theme", "default"),
                               m_quickWidget->engine());
    if (theme) {
        settings->setTheme(theme);
    }

    m_quickWidget->setSource(QUrl("qrc:/touchstrip.qml"));
    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

}

TouchDockerDock::~TouchDockerDock()
{
}

bool TouchDockerDock::allowClose() const
{
    return d->allowClose;
}

void TouchDockerDock::setAllowClose(bool allow)
{
    d->allowClose = allow;
}

QString TouchDockerDock::currentSketchPage() const
{
    return d->currentSketchPage;
}

void TouchDockerDock::setCurrentSketchPage(QString newPage)
{
    d->currentSketchPage = newPage;
    emit currentSketchPageChanged();
}

void TouchDockerDock::closeEvent(QCloseEvent* event)
{
    if (!d->allowClose) {
        event->ignore();
        emit closeRequested();
    } else {
        event->accept();
    }
}

void TouchDockerDock::slotButtonPressed(const QString &id)
{
    if (id == "fileOpenButton") {
        showFileOpenDialog();
    }
    else if (id == "fileSaveButton" && m_canvas && m_canvas->viewManager() && m_canvas->viewManager()->document()) {
        bool batchMode = m_canvas->viewManager()->document()->fileBatchMode();
        m_canvas->viewManager()->document()->setFileBatchMode(true);
        m_canvas->viewManager()->document()->save(true, 0);
        m_canvas->viewManager()->document()->setFileBatchMode(batchMode);
    }
    else if (id == "fileSaveAsButton" && m_canvas && m_canvas->viewManager() && m_canvas->viewManager()->document()) {
        showFileSaveAsDialog();
    }
    else if (m_canvas && m_canvas->viewManager() && m_canvas->viewManager()->actionManager()) {
        QAction *a = action(id);
        if (a) {
            if (a->isCheckable()) {
                a->toggle();
            }
            else {
                a->trigger();
            }
        }
    }
    else if (id == "Key_Shift") {
        // set shift state for the next pointer event, somehow
    }
    else if (id == "Key_Ctrl") {
        // set ctrl state for the next pointer event, somehow
    }
    else if (id == "Key_Alt") {
        // set alt state for the next pointer event, somehow
    }
}

void TouchDockerDock::slotOpenImage(QString path)
{
    if (d->openDialog) {
        d->openDialog->accept();
    }
    KisPart::instance()->currentMainwindow()->openDocument(QUrl::fromLocalFile(path), KisMainWindow::None);
}

void TouchDockerDock::slotSaveAs(QString path, QString mime)
{
    if (d->saveAsDialog) {
        d->saveAsDialog->accept();
    }
    m_canvas->viewManager()->document()->saveAs(QUrl::fromLocalFile(path), mime.toLatin1(), true);
    m_canvas->viewManager()->document()->waitForSavingToComplete();
}

void TouchDockerDock::hideFileOpenDialog()
{
    if (d->openDialog) {
        d->openDialog->accept();
    }
}

void TouchDockerDock::hideFileSaveAsDialog()
{
    if (d->saveAsDialog) {
        d->saveAsDialog->accept();
    }
}

QAction *TouchDockerDock::action(const QString id) const
{
    return m_canvas->viewManager()->actionManager()->actionByName(id);
}

void TouchDockerDock::showFileOpenDialog()
{
    if (!d->openDialog) {
        d->openDialog = createDialog("qrc:/opendialog.qml");
    }

    d->openDialog->exec();
}

void TouchDockerDock::showFileSaveAsDialog()
{
    if (!d->openDialog) {
        d->openDialog = createDialog("qrc:/saveasdialog.qml");
    }
    d->openDialog->exec();
}

KoDialog *TouchDockerDock::createDialog(const QString qml)
{
    KoDialog *dlg = new KoDialog(this);
    dlg->setButtons(KoDialog::None);

    QQuickWidget *quickWidget = new QQuickWidget(this);
    dlg->setMainWidget(quickWidget);

    setEnabled(true);
    quickWidget->engine()->rootContext()->setContextProperty("mainWindow", this);

    quickWidget->engine()->addImportPath(KoResourcePaths::getApplicationRoot() + "/lib/qml/");
    quickWidget->engine()->addImportPath(KoResourcePaths::getApplicationRoot() + "/lib64/qml/");

    Settings *settings = new Settings(this);
    DocumentManager::instance()->setSettingsManager(settings);
    quickWidget->engine()->rootContext()->setContextProperty("Settings", settings);

    Theme *theme = Theme::load(KSharedConfig::openConfig()->group("General").readEntry<QString>("theme", "default"),
                               quickWidget->engine());
    settings->setTheme(theme);

    quickWidget->setSource(QUrl(qml));
    quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    dlg->setMinimumSize(1280, 768);

    return dlg;
}

QObject *TouchDockerDock::sketchKisView() const
{
    return d->sketchView;
}

void TouchDockerDock::setSketchKisView(QObject* newView)
{
    if (d->sketchView) {
        d->sketchView->disconnect(this);
    }

    if (d->sketchView != newView) {
        d->sketchView = qobject_cast<KisSketchView*>(newView);
        emit sketchKisViewChanged();
    }
}

void TouchDockerDock::setCanvas(KoCanvasBase *canvas)
{
    setEnabled(true);

    if (m_canvas == canvas) {
        return;
    }

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
    }

    if (!canvas) {
        m_canvas = 0;
        return;
    }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);

}


void TouchDockerDock::unsetCanvas()
{
    setEnabled(true);
    m_canvas = 0;
}

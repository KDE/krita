/*
 *  SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "TouchDockerDock.h"

#include <QtQuickWidgets/QQuickWidget>
#include <QQmlEngine>
#include <QQmlContext>
#include <QAction>
#include <QUrl>
#include <QKeyEvent>
#include <QApplication>

#include <klocalizedstring.h>
#include <kactioncollection.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include <kis_action_registry.h>
#include <KoDialog.h>
#include <KoResourcePaths.h>
#include <kis_icon.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KisViewManager.h>
#include <kis_canvas2.h>
#include <KisMainWindow.h>
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

#include <QVersionNumber>

namespace
{

bool shouldSetAcceptTouchEvents()
{
    // See https://bugreports.qt.io/browse/QTBUG-66718
    static QVersionNumber qtVersion = QVersionNumber::fromString(qVersion());
    static bool retval = qtVersion > QVersionNumber(5, 9, 3) && qtVersion.normalized() != QVersionNumber(5, 10);
    return retval;
}

} // namespace

class TouchDockerDock::Private
{
public:
    Private()
    {
    }

    TouchDockerDock *q;
    bool allowClose {true};
    KisSketchView *sketchView {0};
    QString currentSketchPage;
    KoDialog *openDialog {0};
    KoDialog *saveAsDialog {0};

    QMap<QString, QString> buttonMapping;

    bool shiftOn {false};
    bool ctrlOn {false};
    bool altOn {false};
};


TouchDockerDock::TouchDockerDock()
    : KDDockWidgets::DockWidget(i18n("Touch Docker"))
    , d(new Private())
{

    QStringList defaultMapping = QStringList() << "decrease_opacity"
                                               << "increase_opacity"
                                               << "make_brush_color_lighter"
                                               << "make_brush_color_darker"
                                               << "decrease_brush_size"
                                               << "increase_brush_size"
                                               << "previous_preset"
                                               << "clear";

    QStringList mapping = KisConfig(true).readEntry<QString>("touchdockermapping", defaultMapping.join(',')).split(',');
    for (int i = 0; i < 8; ++i) {
        if (i < mapping.size()) {
            d->buttonMapping[QString("button%1").arg(i + 1)] = mapping[i];
        }
        else if (i < defaultMapping.size()) {
            d->buttonMapping[QString("button%1").arg(i + 1)] = defaultMapping[i];
        }
    }

    m_quickWidget = new QQuickWidget(this);
    if (shouldSetAcceptTouchEvents()) {
        m_quickWidget->setAttribute(Qt::WA_AcceptTouchEvents);
    }
    setWidget(m_quickWidget);
    setEnabled(true);
    m_quickWidget->engine()->rootContext()->setContextProperty("mainWindow", this);

    m_quickWidget->engine()->addImportPath(KoResourcePaths::getApplicationRoot() + "/lib/qml/");
    m_quickWidget->engine()->addImportPath(KoResourcePaths::getApplicationRoot() + "/lib64/qml/");

    m_quickWidget->engine()->addPluginPath(KoResourcePaths::getApplicationRoot() + "/lib/qml/");
    m_quickWidget->engine()->addPluginPath(KoResourcePaths::getApplicationRoot() + "/lib64/qml/");

    Settings *settings = new Settings(this);
    DocumentManager::instance()->setSettingsManager(settings);
    m_quickWidget->engine()->rootContext()->setContextProperty("Settings", settings);

    Theme *theme = Theme::load(KSharedConfig::openConfig()->group("General").readEntry<QString>("theme", "default"),
                               m_quickWidget->engine());
    if (theme) {
        settings->setTheme(theme);
    }


    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickWidget->setSource(QUrl("qrc:/touchstrip.qml"));


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
        if(m_canvas->viewManager()->document()->path().isEmpty()) {
            showFileSaveAsDialog();
        } else {
            bool batchMode = m_canvas->viewManager()->document()->fileBatchMode();
            m_canvas->viewManager()->document()->setFileBatchMode(true);
            m_canvas->viewManager()->document()->save(true, 0);
            m_canvas->viewManager()->document()->setFileBatchMode(batchMode);
        }
    }
    else if (id == "fileSaveAsButton" && m_canvas && m_canvas->viewManager() && m_canvas->viewManager()->document()) {
        showFileSaveAsDialog();
    }
    else {
        QAction *a = action(id);
        if (a) {
            if (a->isCheckable()) {
                a->toggle();
            }
            else {
                a->trigger();
            }
        }

        else if (id == "shift") {
            // set shift state for the next pointer event, somehow
            QKeyEvent event(d->shiftOn ? QEvent::KeyRelease : QEvent::KeyPress,
                               0,
                               Qt::ShiftModifier);
            QApplication::sendEvent(KisPart::instance()->currentMainwindow(), &event);
            d->shiftOn = !d->shiftOn;
        }
        else if (id == "ctrl") {
            // set ctrl state for the next pointer event, somehow
            QKeyEvent event(d->ctrlOn ? QEvent::KeyRelease : QEvent::KeyPress,
                               0,
                               Qt::ControlModifier);
            QApplication::sendEvent(KisPart::instance()->currentMainwindow(), &event);
            d->ctrlOn = !d->ctrlOn;
        }
        else if (id == "alt") {
            // set alt state for the next pointer event, somehow
            QKeyEvent event(d->altOn ? QEvent::KeyRelease : QEvent::KeyPress,
                               0,
                               Qt::AltModifier);
            QApplication::sendEvent(KisPart::instance()->currentMainwindow(), &event);
            d->altOn = !d->altOn;

        }
    }
}

void TouchDockerDock::slotOpenImage(QString path)
{
    if (d->openDialog) {
        d->openDialog->accept();
    }
    KisPart::instance()->currentMainwindow()->openDocument(path, KisMainWindow::None);
}

void TouchDockerDock::slotSaveAs(QString path, QString mime)
{
    if (d->saveAsDialog) {
        d->saveAsDialog->accept();
    }
    m_canvas->viewManager()->document()->saveAs(path, mime.toLatin1(), true);
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

QString TouchDockerDock::imageForButton(QString id)
{
    if (d->buttonMapping.contains(id)) {
        id = d->buttonMapping[id];
    }
    if (KisActionRegistry::instance()->hasAction(id)) {
        QString a = KisActionRegistry::instance()->getActionProperty(id, "icon");
        if (!a.isEmpty()) {
            return "image://icon/" + a;
        }
    }
    return QString();
}

QString TouchDockerDock::textForButton(QString id)
{
    if (d->buttonMapping.contains(id)) {
        id = d->buttonMapping[id];
    }
    if (KisActionRegistry::instance()->hasAction(id)) {
        QString a = KisActionRegistry::instance()->getActionProperty(id, "iconText");
        if (a.isEmpty()) {
            a = KisActionRegistry::instance()->getActionProperty(id, "text");
        }
        return a;
    }

    return id;
}

QAction *TouchDockerDock::action(QString id) const
{
    if (m_canvas && m_canvas->viewManager()) {
        if (d->buttonMapping.contains(id)) {
            id = d->buttonMapping[id];
        }

        QAction *action = m_canvas->viewManager()->actionManager()->actionByName(id);
        if (!action) {
            return m_canvas->canvasController()->actionCollection()->action(id);
        }

        return action;
    }
    return 0;
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
    if (!d->saveAsDialog) {
        d->saveAsDialog = createDialog("qrc:/saveasdialog.qml");
    }
    d->saveAsDialog->exec();
}

void TouchDockerDock::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::PaletteChange) {
        m_quickWidget->setSource(QUrl("qrc:/touchstrip.qml"));
        event->accept();
    } else {
        event->ignore();
    }
}

void TouchDockerDock::tabletEvent(QTabletEvent *event)
{
#ifdef Q_OS_WIN
    /**
     * On Windows (only in WinInk mode), unless we accept the tablet event,
     * OS will start windows gestures, like click+hold for right click.
     * It will block any mouse events generation.
     *
     * In our own (hacky) implementation, if we accept the event, we block
     * the gesture, but still generate a fake mouse event.
     */
    event->accept();
#else
    QDockWidget::tabletEvent(event);
#endif
}

KoDialog *TouchDockerDock::createDialog(const QString qml)
{
    KoDialog *dlg = new KoDialog(this);
    dlg->setButtons(KoDialog::None);

    QQuickWidget *quickWidget = new QQuickWidget(this);
    if (shouldSetAcceptTouchEvents()) {
        quickWidget->setAttribute(Qt::WA_AcceptTouchEvents);
    }
    dlg->setMainWidget(quickWidget);

    setEnabled(true);
    quickWidget->engine()->rootContext()->setContextProperty("mainWindow", this);

    quickWidget->engine()->addImportPath(KoResourcePaths::getApplicationRoot() + "/lib/qml/");
    quickWidget->engine()->addImportPath(KoResourcePaths::getApplicationRoot() + "/lib64/qml/");

    quickWidget->engine()->addPluginPath(KoResourcePaths::getApplicationRoot() + "/lib/qml/");
    quickWidget->engine()->addPluginPath(KoResourcePaths::getApplicationRoot() + "/lib64/qml/");


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

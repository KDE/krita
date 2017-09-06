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
    settings->setTheme(theme);

    m_quickWidget->setSource(QUrl("qrc:/hello.qml"));
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
    qDebug() << "slotButtonPressed" << sender() << id;
    if (id == "fileOpenButton") {
        showFileDialog();
    }
}


void TouchDockerDock::showFileDialog()
{
    KoDialog dlg;

    QQuickWidget *quickWidget = new QQuickWidget(this);
    dlg.setMainWidget(quickWidget);

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


    quickWidget->setSource(QUrl("qrc:/kritasketch.qml"));
    quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    dlg.setMinimumSize(1280, 768);
    dlg.exec();
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

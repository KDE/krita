/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 * Copyright (C) 2012 KO GmbH. Contact: Boudewijn Rempt <boud@kogmbh.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "MainWindow.h"

#include <QApplication>
#include <QQmlContext>
#include <QQmlEngine>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>

#include <KoDialog.h>
#include <KoZoomController.h>

#include "filter/kis_filter_registry.h"
#include <brushengine/kis_paintop_registry.h>
#include <kis_icon.h>

#include "KisViewManager.h"
#include <kis_canvas2.h>
#include <kis_canvas_controller.h>
#include "kis_config.h"
#include <KisDocument.h>

#include "SketchDeclarativeView.h"
#include "RecentFileManager.h"
#include "DocumentManager.h"
#include "QmlGlobalEngine.h"
#include "Settings.h"

class MainWindow::Private
{
public:
    Private(MainWindow* qq)
        : q(qq)
        , allowClose(true)
        , viewManager(0)
	{
        centerer = new QTimer(q);
        centerer->setInterval(10);
        centerer->setSingleShot(true);
        connect(centerer, SIGNAL(timeout()), q, SLOT(adjustZoomOnDocumentChangedAndStuff()));
	}
	MainWindow* q;
    bool allowClose;
    KisViewManager* viewManager;
    QString currentSketchPage;
	QTimer *centerer;
};

MainWindow::MainWindow(QStringList fileNames, QWidget* parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags ), d( new Private(this))
{
    qApp->setActiveWindow(this);

    setWindowTitle(i18n("Krita Sketch"));
    setWindowIcon(KisIconUtils::loadIcon("kritasketch"));

    // Load filters and other plugins in the gui thread
    Q_UNUSED(KisFilterRegistry::instance());
    Q_UNUSED(KisPaintOpRegistry::instance());

    KisConfig cfg;
    cfg.setNewCursorStyle(CURSOR_STYLE_NO_CURSOR);
    cfg.setNewOutlineStyle(OUTLINE_NONE);
    cfg.setUseOpenGL(true);

    Q_FOREACH (QString fileName, fileNames) {
        DocumentManager::instance()->recentFileManager()->addRecent(fileName);
    }
    connect(DocumentManager::instance(), SIGNAL(documentChanged()), SLOT(resetWindowTitle()));
    connect(DocumentManager::instance(), SIGNAL(documentSaved()), SLOT(resetWindowTitle()));

    QQuickView* view = new SketchDeclarativeView();
    QmlGlobalEngine::instance()->setEngine(view->engine());
    view->engine()->rootContext()->setContextProperty("mainWindow", this);

#ifdef Q_OS_WIN
    QDir appdir(qApp->applicationDirPath());

    // Corrects for mismatched case errors in path (qtdeclarative fails to load)
    wchar_t buffer[1024];
    QString absolute = appdir.absolutePath();
    DWORD rv = ::GetShortPathName((wchar_t*)absolute.utf16(), buffer, 1024);
    rv = ::GetLongPathName(buffer, buffer, 1024);
    QString correctedPath((QChar *)buffer);
    appdir.setPath(correctedPath);

    // for now, the app in bin/ and we still use the env.bat script
    appdir.cdUp();

    // QT5TODO: adapt to QML_IMPORT_PATH usage and install to ${QML_INSTALL_DIR}
    view->engine()->addImportPath(appdir.canonicalPath() + "/lib/calligra/imports");
    view->engine()->addImportPath(appdir.canonicalPath() + "/lib64/calligra/imports");
    QString mainqml = appdir.canonicalPath() + "/share/apps/kritasketch/kritasketch.qml";
#else
    QString mainqml = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kritasketch/kritasketch.qml");
#endif

    Q_ASSERT(QFile::exists(mainqml));
    if (!QFile::exists(mainqml)) {
        QMessageBox::warning(0, i18nc("@title:window", "No QML found"), mainqml + " doesn't exist.");
    }
    QFileInfo fi(mainqml);

    view->setSource(QUrl::fromLocalFile(fi.canonicalFilePath()));
    view->setResizeMode( QQuickView::SizeRootObjectToView );

    if (view->errors().count() > 0) {
        Q_FOREACH (const QQmlError &error, view->errors()) {
            dbgKrita << error.toString();
        }
    }

    QWidget* container = QWidget::createWindowContainer(view);
    setCentralWidget(container);
}

void MainWindow::resetWindowTitle()
{
    QUrl url(DocumentManager::instance()->settingsManager()->currentFile());
    QString fileName = url.fileName();
    if(url.scheme() == "temp")
        fileName = i18n("Untitled");

    KoDialog::CaptionFlags flags = KoDialog::HIGCompliantCaption;
    KisDocument* document = DocumentManager::instance()->document();
    if (document && document->isModified() ) {
        flags |= KoDialog::ModifiedCaption;
    }

    setWindowTitle( KoDialog::makeStandardCaption(fileName, this, flags) );
}

bool MainWindow::allowClose() const
{
    return d->allowClose;
}

void MainWindow::setAllowClose(bool allow)
{
    d->allowClose = allow;
}

QString MainWindow::currentSketchPage() const
{
    return d->currentSketchPage;
}

void MainWindow::setCurrentSketchPage(QString newPage)
{
    d->currentSketchPage = newPage;
    emit currentSketchPageChanged();
}
void MainWindow::adjustZoomOnDocumentChangedAndStuff()
{
    if (d->viewManager) {
        qApp->processEvents();
        d->viewManager->zoomController()->setZoom(KoZoomMode::ZOOM_PAGE, 1.0);
        qApp->processEvents();
        QPoint center = d->viewManager->canvas()->rect().center();
        static_cast<KoCanvasControllerWidget*>(d->viewManager->canvasBase()->canvasController())->zoomRelativeToPoint(center, 0.9);
        qApp->processEvents();
    }
}

QObject* MainWindow::sketchKisView() const
{
    return d->viewManager;
}

void MainWindow::setSketchKisView(QObject* newView)
{
    if (d->viewManager)
        d->viewManager->disconnect(this);
    if (d->viewManager != newView)
    {
        d->viewManager = qobject_cast<KisViewManager*>(newView);
        connect(d->viewManager, SIGNAL(sigLoadingFinished()), d->centerer, SLOT(start()));
        d->centerer->start();
        emit sketchKisViewChanged();
    }
}

void MainWindow::minimize()
{
    setWindowState(windowState() ^ Qt::WindowMinimized);
}

void MainWindow::closeWindow()
{
    //For some reason, close() does not work even if setAllowClose(true) was called just before this method.
    //So instead just completely quit the application, since we are using a single window anyway.
    QApplication::exit();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    // TODO this needs setting somewhere...
//     d->constants->setGridWidth( event->size().width() / d->constants->gridColumns() );
//     d->constants->setGridHeight( event->size().height() / d->constants->gridRows() );
    QWidget::resizeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (!d->allowClose) {
        event->ignore();
        emit closeRequested();
    } else {
        event->accept();
    }
}

MainWindow::~MainWindow()
{
    delete d;
}


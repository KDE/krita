/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 * Copyright (C) 2012 KO GmbH. Contact: Boudewijn Rempt <boud@kogmbh.com>
 * Copyright (C) 2013 Dan Leinir Turthra Jensen <admin@leinir.dk>
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
#include "desktopviewproxy.h"
#include "ViewModeSwitchEvent.h"

#include <QApplication>
#include <QResizeEvent>
#include <QDeclarativeView>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDir>
#include <QFile>
#include <QToolButton>
#include <QFileInfo>
#include <QGLWidget>
#include <QDesktopServices>

#include <QUrl>
#include <KoResourcePaths.h>
#include <kactioncollection.h>
#include <QMessageBox>
#include <kxmlguifactory.h>
#include <KoDialog.h>

#include <KoCanvasBase.h>
#include <KisMainWindow.h>
#include <KoGlobal.h>
#include <KoDocumentInfo.h>
#include <KoAbstractGradient.h>
#include <KoZoomController.h>
#include <KoFileDialog.h>
#include <KisDocumentEntry.h>
#include <KisImportExportManager.h>
#include <KoToolManager.h>
#include <KoIcon.h>
#include <kis_icon.h>

#include "filter/kis_filter.h"
#include "filter/kis_filter_registry.h"
#include <brushengine/kis_paintop.h>
#include <brushengine/kis_paintop_registry.h>

#include <brushengine/kis_paintop_preset.h>
#include <KoPattern.h>
#include <kis_config.h>
#include <kis_factory2.h>
#include <KisDocument.h>
#include <KisViewManager.h>
#include <kis_canvas_resource_provider.h>
#include <kis_canvas_controller.h>

#include "sketch/SketchDeclarativeView.h"
#include "sketch/RecentFileManager.h"
#include "sketch/DocumentManager.h"
#include "sketch/QmlGlobalEngine.h"
#include "sketch/Settings.h"

#ifdef Q_OS_WIN
// Slate mode/docked detection stuff
#include <shellapi.h>
#define SM_CONVERTIBLESLATEMODE 0x2003
#define SM_SYSTEMDOCKED         0x2004
#endif

class MainWindow::Private
{
public:
    Private(MainWindow* qq)
        : q(qq)
        , allowClose(true)
        , sketchView(0)
        , desktopWindow(0)
        , currentView(0)
        , desktopCursorStyle(CURSOR_STYLE_NO_CURSOR)
        , slateMode(false)
        , docked(false)
        , sketchKisView(0)
        , desktopKisView(0)
        , desktopViewProxy(0)
        , forceFullScreen(false)
        , forceDesktop(false)
        , forceSketch(false)
        , temporaryFile(false)
        , syncObject(0)
        , toDesktop(0)
        , toSketch(0)
        , switcher(0)
    {
#ifdef Q_OS_WIN
//         slateMode = (GetSystemMetrics(SM_CONVERTIBLESLATEMODE) == 0);
//         docked = (GetSystemMetrics(SM_SYSTEMDOCKED) != 0);
#endif
        centerer = new QTimer(q);
        centerer->setInterval(10);
        centerer->setSingleShot(true);
        connect(centerer, SIGNAL(timeout()), q, SLOT(adjustZoomOnDocumentChangedAndStuff()));
}
    MainWindow* q;
    bool allowClose;
    SketchDeclarativeView* sketchView;
    KisMainWindow* desktopWindow;
    QObject* currentView;
    CursorStyle desktopCursorStyle;

    bool slateMode;
    bool docked;
    QString currentSketchPage;
    KisView* sketchKisView;
    KisView* desktopKisView;
    DesktopViewProxy* desktopViewProxy;

    bool forceFullScreen;
    bool forceDesktop;
    bool forceSketch;
    bool temporaryFile;
    ViewModeSynchronisationObject* syncObject;
    QTimer* centerer;

    QAction * toDesktop;
    QAction * toSketch;
    QToolButton* switcher;

    void initSketchView(QObject* parent)
    {
        sketchView = new SketchDeclarativeView();
        QmlGlobalEngine::instance()->setEngine(sketchView->engine());
        sketchView->engine()->rootContext()->setContextProperty("mainWindow", parent);

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

        sketchView->engine()->addImportPath(appdir.canonicalPath() + "/lib/calligra/imports");
        sketchView->engine()->addImportPath(appdir.canonicalPath() + "/lib64/calligra/imports");
        QString mainqml = appdir.canonicalPath() + "/share/apps/kritagemini/kritagemini.qml";
#else
        sketchView->engine()->addImportPath(KoResourcePaths::findDirs("lib", "calligra/imports").value(0));
        QString mainqml = KoResourcePaths::findResource("data", "kritagemini/kritagemini.qml");
#endif

        Q_ASSERT(QFile::exists(mainqml));
        if (!QFile::exists(mainqml)) {
            QMessageBox::warning(0, i18nc("@title:window", "Krita: No QML Found"), i18n("%1 doesn't exist.", mainqml));
        }
        QFileInfo fi(mainqml);

        sketchView->setSource(QUrl::fromLocalFile(fi.canonicalFilePath()));
        sketchView->setResizeMode( QDeclarativeView::SizeRootObjectToView );

        if (sketchView->errors().count() > 0) {
            Q_FOREACH (const QDeclarativeError &error, sketchView->errors()) {
                dbgKrita << error.toString();
            }
        }

        toDesktop = new QAction(q);
        toDesktop->setEnabled(false);
        toDesktop->setText(tr("Switch to Desktop"));
        // useful for monkey-testing to crash...
        //toDesktop->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_D);
        //q->addAction(toDesktop);
        //connect(toDesktop, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), q, SLOT(switchDesktopForced()));
        connect(toDesktop, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), q, SLOT(switchToDesktop()));
        sketchView->engine()->rootContext()->setContextProperty("switchToDesktopAction", toDesktop);
    }

    void initDesktopView()
    {
        // Initialize all Calligra directories etc.
        KoGlobal::initialize();

        // The default theme is not what we want for Gemini
        KConfigGroup group(KGlobal::config(), "theme");
        if(group.readEntry("Theme", "no-theme-is-set") == QLatin1String("no-theme-is-set")) {
            group.writeEntry("Theme", "Krita-dark");
        }

        desktopWindow = new KisMainWindow();

        toSketch = new QAction(desktopWindow);
        toSketch->setEnabled(false);
        toSketch->setText(tr("Switch to Sketch"));
        toSketch->setIcon(KisIconUtils::loadIcon("system-reboot"));
        toSketch->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_S);
        //connect(toSketch, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), q, SLOT(switchSketchForced()));
        connect(toSketch, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)), q, SLOT(switchToSketch()));
        desktopWindow->actionCollection()->addAction("SwitchToSketchView", toSketch);
        switcher = new QToolButton();
        switcher->setEnabled(false);
        switcher->setText(tr("Switch to Sketch"));
        switcher->setIcon(KisIconUtils::loadIcon("system-reboot"));
        switcher->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        //connect(switcher, SIGNAL(clicked(bool)), q, SLOT(switchDesktopForced()));
        connect(switcher, SIGNAL(clicked(bool)), q, SLOT(switchToSketch()));
        desktopWindow->menuBar()->setCornerWidget(switcher);

        // DesktopViewProxy connects itself up to everything appropriate on construction,
        // and destroys itself again when the view is removed
        desktopViewProxy = new DesktopViewProxy(q, desktopWindow);
        connect(desktopViewProxy, SIGNAL(documentSaved()), q, SIGNAL(documentSaved()));
        connect(desktopViewProxy, SIGNAL(documentSaved()), q, SLOT(resetWindowTitle()));
    }

    void notifySlateModeChange();
    void notifyDockingModeChange();
    bool queryClose();
};

MainWindow::MainWindow(QStringList fileNames, QWidget* parent, Qt::WindowFlags flags )
    : QMainWindow( parent, flags ), d( new Private(this) )
{
    qApp->setActiveWindow( this );

    setWindowTitle(i18n("Krita Gemini"));
    setWindowIcon(KisIconUtils::loadIcon("kritagemini"));

	// Load filters and other plugins in the gui thread
	Q_UNUSED(KisFilterRegistry::instance());
	Q_UNUSED(KisPaintOpRegistry::instance());

    KisConfig cfg;
    // Store the current setting before we do "things", and heuristic our way to a reasonable
    // default if it's no cursor (that's most likely due to a broken config)
    if (cfg.newCursorStyle() != CURSOR_STYLE_NO_CURSOR)
        d->desktopCursorStyle = cfg.newCursorStyle();
    cfg.setNewCursorStyle(CURSOR_STYLE_NO_CURSOR);
    cfg.setUseOpenGL(true);

    Q_FOREACH (QString fileName, fileNames) {
        DocumentManager::instance()->recentFileManager()->addRecent( QDir::current().absoluteFilePath( fileName ) );
    }

    connect(DocumentManager::instance(), SIGNAL(documentChanged()), SLOT(documentChanged()));
    connect(DocumentManager::instance(), SIGNAL(documentChanged()), SLOT(resetWindowTitle()));
    connect(DocumentManager::instance(), SIGNAL(documentSaved()), SLOT(resetWindowTitle()));

    d->initSketchView(this);

    // Set the initial view to sketch... because reasons.
    // Really, this allows us to show the pleasant welcome screen from Sketch
    switchToSketch();

    if(!fileNames.isEmpty()) {
        //It feels a little hacky, but call a QML function to open files.
        //This saves a lot of hassle required to change state for loading dialogs etc.
        QMetaObject::invokeMethod(d->sketchView->rootObject(), "openFile", Q_ARG(QVariant, fileNames.at(0)));
    }
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

void MainWindow::switchDesktopForced()
{
    if (d->slateMode)
        d->forceDesktop = true;
    d->forceSketch = false;
}

void MainWindow::switchSketchForced()
{
    if (!d->slateMode)
        d->forceSketch = true;
    d->forceDesktop = false;
}

void MainWindow::switchToSketch()
{
    if (d->toSketch)
    {
        d->toSketch->setEnabled(false);
        d->switcher->setEnabled(false);
    }

    d->syncObject = new ViewModeSynchronisationObject;
    KisViewManager* view = 0;

    KisConfig cfg;
    if (d->desktopWindow && centralWidget() == d->desktopWindow) {
        d->desktopCursorStyle = cfg.newCursorStyle();
        view = qobject_cast<KisViewManager*>(d->desktopWindow->activeView());

        //Notify the view we are switching away from that we are about to switch away from it
        //giving it the possibility to set up the synchronisation object.
        ViewModeSwitchEvent aboutToSwitchEvent(ViewModeSwitchEvent::AboutToSwitchViewModeEvent, view, d->sketchView, d->syncObject);
        QApplication::sendEvent(view, &aboutToSwitchEvent);

        d->desktopWindow->setParent(0);
    }

    setCentralWidget(d->sketchView);

    if (d->slateMode) {
        setWindowState(windowState() | Qt::WindowFullScreen);
        if (d->syncObject->initialized)
            QTimer::singleShot(50, this, SLOT(sketchChange()));
    }
    else
        QTimer::singleShot(50, this, SLOT(sketchChange()));

    if (view && view->document()) {
        view->document()->setFileBatchMode(true);
    }
}

void MainWindow::sketchChange()
{
    if (centralWidget() != d->sketchView || !d->syncObject)
        return;

    if (d->desktopWindow)
    {
        if (!d->sketchKisView || !d->sketchView->canvasWidget())
        {
            QTimer::singleShot(100, this, SLOT(sketchChange()));
            return;
        }
        qApp->processEvents();
        KisViewManager* view = qobject_cast<KisViewManager*>(d->desktopWindow->activeView());
        //Notify the new view that we just switched to it, passing our synchronisation object
        //so it can use those values to sync with the old view.
        ViewModeSwitchEvent switchedEvent(ViewModeSwitchEvent::SwitchedToSketchModeEvent, view, d->sketchView, d->syncObject);
        QApplication::sendEvent(d->sketchView, &switchedEvent);
        d->syncObject = 0;
        qApp->processEvents();
        KisConfig cfg;
        cfg.setNewCursorStyle(CURSOR_STYLE_NO_CURSOR);
        emit switchedToSketch();
    }
    if (d->toDesktop)
    {
        qApp->processEvents();
        d->toDesktop->setEnabled(true);
    }
}

void MainWindow::switchToDesktop(bool justLoaded)
{
    if (d->toDesktop)
        d->toDesktop->setEnabled(false);

    ViewModeSynchronisationObject* syncObject = new ViewModeSynchronisationObject;

    KisViewManager* view = 0;
    if (d->desktopWindow) {
        view = qobject_cast<KisViewManager*>(d->desktopWindow->activeView());
    }

    //Notify the view we are switching away from that we are about to switch away from it
    //giving it the possibility to set up the synchronisation object.
    ViewModeSwitchEvent aboutToSwitchEvent(ViewModeSwitchEvent::AboutToSwitchViewModeEvent, d->sketchView, view, syncObject);
    QApplication::sendEvent(d->sketchView, &aboutToSwitchEvent);
    qApp->processEvents();

    if (d->currentSketchPage == "MainPage")
    {
        d->sketchView->setParent(0);
        setCentralWidget(d->desktopWindow);
    }

    if (!d->forceFullScreen) {
        setWindowState(windowState() & ~Qt::WindowFullScreen);
    }

    if (view) {
        //Notify the new view that we just switched to it, passing our synchronisation object
        //so it can use those values to sync with the old view.
        ViewModeSwitchEvent switchedEvent(ViewModeSwitchEvent::SwitchedToDesktopModeEvent, d->sketchView, view, syncObject);
        QApplication::sendEvent(view, &switchedEvent);
        KisConfig cfg;
        cfg.setNewCursorStyle(d->desktopCursorStyle);
    }

    if (d->toSketch && !justLoaded)
    {
        qApp->processEvents();
        d->toSketch->setEnabled(true);
        d->switcher->setEnabled(true);
    }

    if (view && view->document()) {
        view->document()->setFileBatchMode(false);
    }
}

void MainWindow::adjustZoomOnDocumentChangedAndStuff()
{
    if (d->desktopWindow && centralWidget() == d->desktopWindow) {
        KisView* view = qobject_cast<KisView*>(d->desktopWindow->activeView());
        // We have to set the focus on the view here, otherwise the toolmanager is unaware of which
        // canvas should be handled.
        view->canvasController()->setFocus();
        view->setFocus();
        QPoint center = view->rect().center();
        view->canvasController()->zoomRelativeToPoint(center, 0.9);
        qApp->processEvents();
        d->toSketch->setEnabled(true);
        d->switcher->setEnabled(true);
    }
    else if (d->sketchKisView && centralWidget() == d->sketchView) {
        qApp->processEvents();
        d->sketchKisView->zoomController()->setZoom(KoZoomMode::ZOOM_PAGE, 1.0);
        qApp->processEvents();
        QPoint center = d->sketchKisView->rect().center();
        d->sketchKisView->canvasController()->zoomRelativeToPoint(center, 0.9);
        qApp->processEvents();
        d->toDesktop->setEnabled(true);
    }
    // Ensure that we do, in fact, have the brush tool selected on the currently active canvas
    KoToolManager::instance()->switchToolRequested( "InteractionTool" );
    qApp->processEvents();
    KoToolManager::instance()->switchToolRequested( "KritaShape/KisToolBrush" );
}

void MainWindow::documentChanged()
{
    if (d->desktopWindow) {
        d->desktopWindow->setNoCleanup(true);
        d->desktopWindow->deleteLater();
        d->desktopWindow = 0;
    }
    d->initDesktopView();
    //d->desktopWindow->setRootDocument(DocumentManager::instance()->document(), DocumentManager::instance()->part(), false);
    qApp->processEvents();
    d->desktopKisView = qobject_cast<KisView*>(d->desktopWindow->activeView());
    //d->desktopKisView->setQtMainWindow(d->desktopWindow);

    // Define new actions here

    KXMLGUIFactory* factory = d->desktopWindow->factory();
    factory->removeClient(d->desktopWindow);
    factory->addClient(d->desktopWindow);

    d->desktopViewProxy->documentChanged();
    connect(d->desktopKisView, SIGNAL(sigLoadingFinished()), d->centerer, SLOT(start()));
    connect(d->desktopKisView, SIGNAL(sigSavingFinished()), this, SLOT(resetWindowTitle()));
    if (d->desktopKisView && d->desktopKisView->canvasBase() && d->desktopKisView->canvasBase()->resourceManager()) {
        connect(d->desktopKisView->canvasBase()->resourceManager(), SIGNAL(canvasResourceChanged(int, const QVariant&)),
                    this, SLOT(resourceChanged(int, const QVariant&)));
    }
    if (!d->forceSketch && !d->slateMode)
        switchToDesktop(true);
}

bool MainWindow::allowClose() const
{
    return d->allowClose;
}

void MainWindow::setAllowClose(bool allow)
{
    d->allowClose = allow;
}

bool MainWindow::slateMode() const
{
    return d->slateMode;
}

void MainWindow::setSlateMode(bool newValue)
{
    d->slateMode = newValue;
}

QString MainWindow::currentSketchPage() const
{
    return d->currentSketchPage;
}

void MainWindow::setCurrentSketchPage(QString newPage)
{
    d->currentSketchPage = newPage;
    emit currentSketchPageChanged();

    if (newPage == "MainPage")
    {
        if (!d->forceSketch && !d->slateMode)
        {
            // Just loaded to desktop, do nothing
        }
        else
        {
            //QTimer::singleShot(3000, this, SLOT(adjustZoomOnDocumentChangedAndStuff()));
        }
    }
}

bool MainWindow::temporaryFile() const
{
    return d->temporaryFile;
}

void MainWindow::setTemporaryFile(bool newValue)
{
    d->temporaryFile = newValue;
    emit temporaryFileChanged();
}

QString MainWindow::openImage()
{
    KoFileDialog dialog(this, KoFileDialog::OpenFile, "OpenDocument");
    dialog.setCaption(i18n("Open Document"));
    dialog.setDefaultDir(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation));

    KisDocumentEntry entry = KisDocumentEntry::queryByMimeType("application/x-krita");
    KService::Ptr service = entry.service();
    dialog.setMimeTypeFilters(KisImportExportManager::mimeFilter("application/x-krita", KisImportExportManager::Import, service->property("X-KDE-ExtraNativeMimeTypes").toStringList()));

    dialog.setHideNameFilterDetailsOption();
    return dialog.filename();
}

void MainWindow::resourceChanged(int key, const QVariant& v)
{
    Q_UNUSED(key);

    if(centralWidget() == d->sketchView)
        return;
    KisPaintOpPresetSP preset = v.value<KisPaintOpPresetSP>();
    if(preset && d->sketchKisView != 0) {
        KisPaintOpPresetSP clone = preset;
        d->sketchKisView->resourceProvider()->setPaintOpPreset(clone);
    }
}

void MainWindow::resourceChangedSketch(int key, const QVariant& v)
{
    Q_UNUSED(key);

    if(centralWidget() == d->desktopWindow)
        return;
    KisPaintOpPresetSP preset = v.value<KisPaintOpPresetSP>();
    if(preset && d->desktopKisView != 0) {
        KisPaintOpPresetSP clone = preset;
        d->desktopKisView->resourceProvider()->setPaintOpPreset(clone);
    }
}

QObject* MainWindow::sketchKisView() const
{
    return d->sketchKisView;
}

void MainWindow::setSketchKisView(QObject* newView)
{
    if (d->sketchKisView) {
        d->sketchKisView->disconnect(this);
        d->sketchKisView->canvasBase()->resourceManager()->disconnect(this);
    }
    if (d->sketchKisView != newView)
    {
        d->sketchKisView = qobject_cast<KisView*>(newView);
        if(d->sketchKisView) {
            d->sketchView->addActions(d->sketchKisView->actions());
//            d->sketchKisView->setQtMainWindow(this);
            connect(d->sketchKisView, SIGNAL(sigLoadingFinished()), d->centerer, SLOT(start()));
            connect(d->sketchKisView->canvasBase()->resourceManager(), SIGNAL(canvasResourceChanged(int, const QVariant&)),
                this, SLOT(resourceChangedSketch(int, const QVariant&)));
            d->centerer->start();
        }
        emit sketchKisViewChanged();
    }
}

void MainWindow::minimize()
{
    setWindowState(windowState() ^ Qt::WindowMinimized);
}

void MainWindow::closeWindow()
{
    if (d->desktopWindow) {
        // This situation shouldn't occur, but protecting potentially dangerous call
        d->desktopWindow->setNoCleanup(true);
    }

    //For some reason, close() does not work even if setAllowClose(true) was called just before this method.
    //So instead just completely quit the application, since we are using a single window anyway.
    DocumentManager::instance()->closeDocument();

    qApp->processEvents();
    QApplication::instance()->quit();
}

bool MainWindow::Private::queryClose()
{
    if (desktopWindow) {
        // This situation shouldn't occur, but protecting potentially dangerous call
        desktopWindow->setNoCleanup(true);
    }
    if (DocumentManager::instance()->document() == 0)
        return true;

    // main doc + internally stored child documents
    if (DocumentManager::instance()->document()->isModified()) {
        QString name;
        if (DocumentManager::instance()->document()->documentInfo()) {
            name = DocumentManager::instance()->document()->documentInfo()->aboutInfo("title");
        }
        if (name.isEmpty())
            name = DocumentManager::instance()->document()->url().fileName();

        if (name.isEmpty())
            name = i18n("Untitled");

        int res = QMessageBox::warning(q,
                                        i18nc("@title:window", "Krita"),
                  i18n("<p>The document <b>'%1'</b> has been modified.</p><p>Do you want to save it?</p>", name),
                  QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

        switch (res) {
        case QMessageBox::Yes : {
            if (DocumentManager::instance()->isTemporaryFile()) {
                if(!desktopViewProxy->fileSaveAs())
                    return false;
            }
            else if (!DocumentManager::instance()->save()) {
                return false;
            }
            break;
        }
        case QMessageBox::No :
            DocumentManager::instance()->document()->removeAutoSaveFiles();
            DocumentManager::instance()->document()->setModified(false);   // Now when queryClose() is called by closeEvent it won't do anything.
            break;
        default : // case QMessageBox::Cancel :
            return false;
        }
    }
    return true;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (centralWidget() == d->desktopWindow)
    {
        if (DocumentManager::instance()->document()->isLoading()) {
            event->ignore();
            return;
        }
        d->allowClose = d->queryClose();
    }

    if (d->allowClose)
    {
        if (d->desktopWindow)
        {
            d->desktopWindow->setNoCleanup(true);
            d->desktopWindow->close();
        }
        event->accept();
    }
    else
    {
        event->ignore();
        emit closeRequested();
    }
}

MainWindow::~MainWindow()
{
    delete d;
    KisConfig cfg;
    cfg.setNewCursorStyle(d->desktopCursorStyle);
}

#ifdef Q_OS_WIN
bool MainWindow::winEvent( MSG * message, long * result )
{
    if (message && message->message == WM_SETTINGCHANGE && message->lParam)
    {
        if (wcscmp(TEXT("ConvertibleSlateMode"), (TCHAR *) message->lParam) == 0)
            d->notifySlateModeChange();
        else if (wcscmp(TEXT("SystemDockMode"), (TCHAR *) message->lParam) == 0)
            d->notifyDockingModeChange();
        *result = 0;
        return true;
    }
    return false;
}
#endif

void MainWindow::Private::notifySlateModeChange()
{
#ifdef Q_OS_WIN
    bool bSlateMode = (GetSystemMetrics(SM_CONVERTIBLESLATEMODE) == 0);

    if (slateMode != bSlateMode)
    {
        slateMode = bSlateMode;
        emit q->slateModeChanged();
        if (forceSketch || (slateMode && !forceDesktop))
        {
            if (!toSketch || (toSketch && toSketch->isEnabled()))
                q->switchToSketch();
        }
        else
        {
                q->switchToDesktop();
        }
        //dbgKrita << "Slate mode is now" << slateMode;
    } 
#endif
}

void MainWindow::Private::notifyDockingModeChange()
{
#ifdef Q_OS_WIN
    bool bDocked = (GetSystemMetrics(SM_SYSTEMDOCKED) != 0);

    if (docked != bDocked)
    {
        docked = bDocked;
        //dbgKrita << "Docking mode is now" << docked;
    }
#endif
}

void MainWindow::cloneResources(KisCanvasResourceProvider *from, KisCanvasResourceProvider *to)
{
    to->setBGColor(from->bgColor());
    to->setFGColor(from->fgColor());
    to->setHDRExposure(from->HDRExposure());
    to->setHDRGamma(from->HDRGamma());
    to->setCurrentCompositeOp(from->currentCompositeOp());
    to->slotPatternActivated(from->currentPattern());
    to->slotGradientActivated(from->currentGradient());
    to->slotNodeActivated(from->currentNode());
    to->setPaintOpPreset(from->currentPreset());
    to->setOpacity(from->opacity());
    to->setGlobalAlphaLock(from->globalAlphaLock());

}

bool MainWindow::forceFullScreen() {
    return d->forceFullScreen;
}

void MainWindow::forceFullScreen(bool newValue)
{
    d->forceFullScreen = newValue;
}


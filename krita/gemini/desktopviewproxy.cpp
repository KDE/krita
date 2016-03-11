/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "desktopviewproxy.h"

#include <QProcess>
#include <QDir>
#include <QApplication>
#include <QDesktopServices>

#include <klocalizedstring.h>
#include <krecentfilesaction.h>
#include <kactioncollection.h>

#include <boost/config/posix_features.hpp>

#include <KisMainWindow.h>
#include <KisImportExportManager.h>
#include <KoFileDialog.h>
#include <KisDocumentEntry.h>

#include "MainWindow.h"
#include <sketch/DocumentManager.h>
#include <sketch/RecentFileManager.h>
#include <sketch/Settings.h>
#include <kis_config.h>
#include <KisDocument.h>
#include <KisViewManager.h>
#include <kis_action_registry.h>

class DesktopViewProxy::Private
{
public:
    Private(MainWindow* mainWindow, KisMainWindow* desktopView)
        : mainWindow(mainWindow)
        , desktopWindow(desktopView)
        , isImporting(false)
    {}
    MainWindow* mainWindow;
    KisMainWindow* desktopWindow;
    bool isImporting;
};

DesktopViewProxy::DesktopViewProxy(MainWindow* mainWindow, KisMainWindow* parent)
    : QObject(parent)
    , d(new Private(mainWindow, parent))
{
    Q_ASSERT(parent); // "There MUST be a KisMainWindow assigned, otherwise everything will blow up");

    // Hide this one... as it doesn't work at all well and release happens :P
    QAction* closeAction = d->desktopWindow->actionCollection()->action("file_close");
    closeAction->setVisible(false);

    // Concept is simple - simply steal all the actions we require to work differently, and reconnect them to local functions
    QAction* newAction = d->desktopWindow->actionCollection()->action("file_new");
    newAction->disconnect(d->desktopWindow);
    connect(newAction, SIGNAL(triggered(bool)), this, SLOT(fileNew()));
    QAction* openAction = d->desktopWindow->actionCollection()->action("file_open");
    openAction->disconnect(d->desktopWindow);
    connect(openAction, SIGNAL(triggered(bool)), this, SLOT(fileOpen()));
    QAction* saveAction = d->desktopWindow->actionCollection()->action("file_save");
    saveAction->disconnect(d->desktopWindow);
    connect(saveAction, SIGNAL(triggered(bool)), this, SLOT(fileSave()));
    QAction* saveasAction = d->desktopWindow->actionCollection()->action("file_save_as");
    saveasAction->disconnect(d->desktopWindow);
    connect(saveasAction, SIGNAL(triggered(bool)), this, SLOT(fileSaveAs()));
    QAction* reloadAction = d->desktopWindow->actionCollection()->action("file_reload_file");
    reloadAction->disconnect(d->desktopWindow);
    connect(reloadAction, SIGNAL(triggered(bool)), this, SLOT(reload()));
    QAction* loadExistingAsNewAction = d->desktopWindow->actionCollection()->action("file_import_file");
    //Hide the "Load existing as new" action. It serves little purpose and currently
    //does the same as open. We cannot just remove it from the action collection though
    //since that causes a crash in KisMainWindow.
    loadExistingAsNewAction->setVisible(false);

    // Recent files need a touch more work, as they aren't simply an action.
    KRecentFilesAction* recent = qobject_cast<KRecentFilesAction*>(d->desktopWindow->actionCollection()->action("file_open_recent"));
    recent->disconnect(d->desktopWindow);
    connect(recent, SIGNAL(urlSelected(QUrl)), this, SLOT(slotFileOpenRecent(QUrl)));
    recent->clear();
    recent->loadEntries(KGlobal::config()->group("RecentFiles"));

    connect(d->desktopWindow, SIGNAL(documentSaved()), this, SIGNAL(documentSaved()));

    // XXX: Shortcut editor is untested in Gemini since refactoring.
    connect(KisActionRegistry::instance(), SIGNAL(shortcutsUpdated()), this, SLOT(updateShortcuts()));

}

DesktopViewProxy::~DesktopViewProxy()
{
    delete d;
}

void DesktopViewProxy::fileNew()
{
    QProcess::startDetached(qApp->applicationFilePath(), QStringList(), QDir::currentPath());
}

void DesktopViewProxy::fileOpen()
{
    KisDocumentEntry entry = KisDocumentEntry::queryByMimeType(KIS_MIME_TYPE);
    KService::Ptr service = entry.service();
    const QStringList mimeFilter = KisImportExportManager::mimeFilter(KIS_MIME_TYPE,
                                                               KisImportExportManager::Import,
                                                               service->property("X-KDE-ExtraNativeMimeTypes").toStringList());


    KoFileDialog dialog(d->desktopWindow, KoFileDialog::OpenFile, "OpenDocument");
    dialog.setCaption(i18n("Open Document"));
    dialog.setDefaultDir(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation));
    dialog.setMimeTypeFilters(mimeFilter);
    QString filename = dialog.url();
    if (filename.isEmpty()) return;

    DocumentManager::instance()->recentFileManager()->addRecent(filename);

    QProcess::startDetached(qApp->applicationFilePath(), QStringList() << filename, QDir::currentPath());
}

void DesktopViewProxy::fileSave()
{
    if(DocumentManager::instance()->isTemporaryFile()) {
        if(d->desktopWindow->saveDocument(d->desktopWindow->activeView()->document(), true)) {
            DocumentManager::instance()->recentFileManager()->addRecent(DocumentManager::instance()->document()->url().toLocalFile());
            DocumentManager::instance()->settingsManager()->setCurrentFile(DocumentManager::instance()->document()->url().toLocalFile());
            DocumentManager::instance()->setTemporaryFile(false);
            emit documentSaved();
        }
    } else {
        DocumentManager::instance()->save();
        emit documentSaved();
    }
}

bool DesktopViewProxy::fileSaveAs()
{
    if(d->desktopWindow->saveDocument(d->desktopWindow->activeView()->document(), true)) {
        DocumentManager::instance()->recentFileManager()->addRecent(DocumentManager::instance()->document()->url().toLocalFile());
        DocumentManager::instance()->settingsManager()->setCurrentFile(DocumentManager::instance()->document()->url().toLocalFile());
        DocumentManager::instance()->setTemporaryFile(false);
        emit documentSaved();
        return true;
    }

    DocumentManager::instance()->settingsManager()->setCurrentFile(DocumentManager::instance()->document()->url().toLocalFile());
    return false;
}

void DesktopViewProxy::reload()
{
    DocumentManager::instance()->reload();
}

void DesktopViewProxy::loadExistingAsNew()
{
    d->isImporting = true;
    fileOpen();
    d->isImporting = false;
}

void DesktopViewProxy::slotFileOpenRecent(const QUrl& url)
{
    QProcess::startDetached(qApp->applicationFilePath(), QStringList() << url.toLocalFile(), QDir::currentPath());
}

/**
 * @brief Override to allow for full-screen support with Canvas-mode
 *
 * The basic behaviour of the KisViewManager is to check the KoConfig and
 * to adjust the main window appropriately. If "hideTitlebar" is set
 * true, then it switches the window between windowed and full-screen.
 * To prevent it leaving the full-screen mode, we set the mode to false
 *
 * @param toggled
 */
void DesktopViewProxy::toggleShowJustTheCanvas(bool toggled)
{
    KisViewManager *kisViewManager = qobject_cast<KisViewManager*>(d->desktopWindow->activeView());
    if(toggled) {
        kisViewManager->showJustTheCanvas(toggled);
    }
    else {
        KisConfig cfg;
        bool fullScreen = d->mainWindow->forceFullScreen();
        bool hideTitlebar = cfg.hideTitlebarFullscreen();
		
        if (fullScreen) {
            cfg.setHideTitlebarFullscreen(false);
        }

        kisViewManager->showJustTheCanvas(toggled);

        if (fullScreen) {
            cfg.setHideTitlebarFullscreen(hideTitlebar);
        }
    }
}

void DesktopViewProxy::documentChanged()
{
    // Remove existing linking for toggling canvas, in order
    // to over-ride the window state behaviour
    QAction* toggleJustTheCanvasAction = d->desktopWindow->actionCollection()->action("view_show_canvas_only");
    toggleJustTheCanvasAction->disconnect(d->desktopWindow);
    connect(toggleJustTheCanvasAction, SIGNAL(toggled(bool)), this, SLOT(toggleShowJustTheCanvas(bool)));
}

void DesktopViewProxy::updateKeyBindings()
{
    //KisView* view = qobject_cast<KisView*>(d->mainWindow->sketchKisView());
    Q_FOREACH (QAction* action, d->desktopWindow->actions()) {
        QAction* otherAction = d->desktopWindow->action(action->objectName().toLatin1());
        if(otherAction) {
            otherAction->setShortcut(action->shortcut());
        }
        else {
            // That's ok - there are some actions that are not truly actions and don't have shortcuts,
            // so we'll just not bother with those, and we're fine with that :)
        }
    }
}


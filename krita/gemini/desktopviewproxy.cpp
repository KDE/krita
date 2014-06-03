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

#include <KoMainWindow.h>
#include <KoFilterManager.h>
#include <KoFileDialog.h>
#include <KoDocumentEntry.h>

#include "MainWindow.h"
#include <sketch/DocumentManager.h>
#include <sketch/RecentFileManager.h>
#include <sketch/Settings.h>
#include <kis_config.h>
#include <kis_doc2.h>
#include <kis_view2.h>

class DesktopViewProxy::Private
{
public:
    Private(MainWindow* mainWindow, KoMainWindow* desktopView)
        : mainWindow(mainWindow)
        , desktopView(desktopView)
        , isImporting(false)
    {}
    MainWindow* mainWindow;
    KoMainWindow* desktopView;
    bool isImporting;
};

DesktopViewProxy::DesktopViewProxy(MainWindow* mainWindow, KoMainWindow* parent)
    : QObject(parent)
    , d(new Private(mainWindow, parent))
{
    Q_ASSERT(parent); // "There MUST be a KoMainWindow assigned, otherwise everything will blow up");

    // Hide this one... as it doesn't work at all well and release happens :P
    QAction* closeAction = d->desktopView->actionCollection()->action("file_close");
    closeAction->setVisible(false);

    // Concept is simple - simply steal all the actions we require to work differently, and reconnect them to local functions
    QAction* newAction = d->desktopView->actionCollection()->action("file_new");
    newAction->disconnect(d->desktopView);
    connect(newAction, SIGNAL(triggered(bool)), this, SLOT(fileNew()));
    QAction* openAction = d->desktopView->actionCollection()->action("file_open");
    openAction->disconnect(d->desktopView);
    connect(openAction, SIGNAL(triggered(bool)), this, SLOT(fileOpen()));
    QAction* saveAction = d->desktopView->actionCollection()->action("file_save");
    saveAction->disconnect(d->desktopView);
    connect(saveAction, SIGNAL(triggered(bool)), this, SLOT(fileSave()));
    QAction* saveasAction = d->desktopView->actionCollection()->action("file_save_as");
    saveasAction->disconnect(d->desktopView);
    connect(saveasAction, SIGNAL(triggered(bool)), this, SLOT(fileSaveAs()));
    QAction* reloadAction = d->desktopView->actionCollection()->action("file_reload_file");
    reloadAction->disconnect(d->desktopView);
    connect(reloadAction, SIGNAL(triggered(bool)), this, SLOT(reload()));
    QAction* loadExistingAsNewAction = d->desktopView->actionCollection()->action("file_import_file");
    //Hide the "Load existing as new" action. It serves little purpose and currently
    //does the same as open. We cannot just remove it from the action collection though
    //since that causes a crash in KoMainWindow.
    loadExistingAsNewAction->setVisible(false);

    // Recent files need a touch more work, as they aren't simply an action.
    KRecentFilesAction* recent = qobject_cast<KRecentFilesAction*>(d->desktopView->actionCollection()->action("file_open_recent"));
    recent->disconnect(d->desktopView);
    connect(recent, SIGNAL(urlSelected(KUrl)), this, SLOT(slotFileOpenRecent(KUrl)));
    recent->clear();
    recent->loadEntries(KGlobal::config()->group("RecentFiles"));

    connect(d->desktopView, SIGNAL(documentSaved()), this, SIGNAL(documentSaved()));
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
    KoDocumentEntry entry = KoDocumentEntry::queryByMimeType(KIS_MIME_TYPE);
    KService::Ptr service = entry.service();
    const QStringList mimeFilter = KoFilterManager::mimeFilter(KIS_MIME_TYPE,
                                                               KoFilterManager::Import,
                                                               service->property("X-KDE-ExtraNativeMimeTypes").toStringList());


    KoFileDialog dialog(d->desktopView, KoFileDialog::OpenFile, "OpenDocument");
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
        if(d->desktopView->saveDocument(true)) {
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
    if(d->desktopView->saveDocument(true)) {
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

void DesktopViewProxy::slotFileOpenRecent(const KUrl& url)
{
    QProcess::startDetached(qApp->applicationFilePath(), QStringList() << url.toLocalFile(), QDir::currentPath());
}

/**
 * @brief Override to allow for full-screen support with Canvas-mode
 *
 * The basic behaviour of the KisView2 is to check the KoConfig and
 * to adjust the main window appropriately. If "hideTitlebar" is set
 * true, then it switches the window between windowed and full-screen.
 * To prevent it leaving the full-screen mode, we set the mode to false
 *
 * @param toggled
 */
void DesktopViewProxy::toggleShowJustTheCanvas(bool toggled)
{
    KisView2* kisView = qobject_cast<KisView2*>(d->desktopView->rootView());
    if(toggled) {
        kisView->showJustTheCanvas(toggled);
    }
    else {
        KisConfig cfg;
        bool fullScreen = d->mainWindow->forceFullScreen();
        bool hideTitlebar = cfg.hideTitlebarFullscreen();
		
        if (fullScreen) {
            cfg.setHideTitlebarFullscreen(false);
        }

        kisView->showJustTheCanvas(toggled);

        if (fullScreen) {
            cfg.setHideTitlebarFullscreen(hideTitlebar);
        }
    }
}

void DesktopViewProxy::documentChanged()
{
    // Remove existing linking for toggling canvas, in order
    // to over-ride the window state behaviour
    KisView2* view = qobject_cast<KisView2*>(d->desktopView->rootView());
    QAction* toggleJustTheCanvasAction = view->actionCollection()->action("view_show_just_the_canvas");
    toggleJustTheCanvasAction->disconnect(view);
    connect(toggleJustTheCanvasAction, SIGNAL(toggled(bool)), this, SLOT(toggleShowJustTheCanvas(bool)));
}

#include "desktopviewproxy.moc"

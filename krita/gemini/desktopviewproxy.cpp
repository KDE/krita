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
#include <KoFileDialogHelper.h>
#include <KoDocumentEntry.h>

#include "MainWindow.h"
#include <sketch/DocumentManager.h>
#include <kis_doc2.h>

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
    loadExistingAsNewAction->disconnect(d->desktopView);
    connect(loadExistingAsNewAction, SIGNAL(triggered(bool)), this, SLOT(loadExistingAsNew()));

    // Recent files need a touch more work, as they aren't simply an action.
    KRecentFilesAction* recent = qobject_cast<KRecentFilesAction*>(d->desktopView->actionCollection()->action("file_open_recent"));
    recent->disconnect(d->desktopView);
    connect(recent, SIGNAL(urlSelected(KUrl)), this, SLOT(slotFileOpenRecent(KUrl)));
    recent->clear();
    recent->loadEntries(KGlobal::config()->group("RecentFiles"));
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


    QString filename = KoFileDialogHelper::getOpenFileName(d->desktopView,
                                                           i18n("Open Document"),
                                                           QDesktopServices::storageLocation(QDesktopServices::PicturesLocation),
                                                           mimeFilter,
                                                           "",
                                                           "OpenDocument");
    if (filename.isEmpty()) return;

    DocumentManager::instance()->openDocument(filename, d->isImporting);
}

void DesktopViewProxy::fileSave()
{
    DocumentManager::instance()->save();
}

bool DesktopViewProxy::fileSaveAs()
{
    return d->desktopView->saveDocument(true);
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
    DocumentManager::instance()->openDocument(url.toLocalFile());
}

#include "desktopviewproxy.moc"

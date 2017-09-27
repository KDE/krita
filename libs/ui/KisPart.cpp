/* This file is part of the KDE project
 * Copyright (C) 1998-1999 Torben Weis       <weis@kde.org>
 * Copyright (C) 2000-2005 David Faure       <faure@kde.org>
 * Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2010-2012 Boudewijn Rempt   <boud@valdyas.org>
 * Copyright (C) 2011 Inge Wallin            <ingwa@kogmbh.com>
 * Copyright (C) 2015 Michael Abrahams       <miabraha@gmail.com>
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

#include "KisPart.h"

#include "KoProgressProxy.h"
#include <KoCanvasController.h>
#include <KoCanvasControllerWidget.h>
#include <KoColorSpaceEngine.h>
#include <KoCanvasBase.h>
#include <KoToolManager.h>
#include <KoShapeBasedDocumentBase.h>
#include <KoResourceServerProvider.h>
#include <kis_icon.h>

#include "KisApplication.h"
#include "KisDocument.h"
#include "KisView.h"
#include "KisViewManager.h"
#include "KisImportExportManager.h"

#include <kis_debug.h>
#include <KoResourcePaths.h>
#include <KoDialog.h>
#include <kdesktopfile.h>
#include <QMessageBox>
#include <klocalizedstring.h>
#include <kactioncollection.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <QKeySequence>

#include <QDialog>
#include <QApplication>
#include <QDomDocument>
#include <QDomElement>
#include <QGlobalStatic>
#include <KisMimeDatabase.h>

#include "KisView.h"
#include "KisDocument.h"
#include "kis_config.h"
#include "kis_shape_controller.h"
#include "kis_resource_server_provider.h"
#include "kis_animation_cache_populator.h"
#include "kis_idle_watcher.h"
#include "kis_image.h"
#include "KisImportExportManager.h"
#include "KisDocument.h"
#include "KoToolManager.h"
#include "KisViewManager.h"
#include "kis_script_manager.h"
#include "KisOpenPane.h"

#include "kis_color_manager.h"
#include "kis_debug.h"

#include "kis_action.h"
#include "kis_action_registry.h"

Q_GLOBAL_STATIC(KisPart, s_instance)


class Q_DECL_HIDDEN KisPart::Private
{
public:
    Private(KisPart *_part)
        : part(_part)
        , idleWatcher(2500)
        , animationCachePopulator(_part)
    {
    }

    ~Private()
    {
    }

    KisPart *part;

    QList<QPointer<KisView> > views;
    QList<QPointer<KisMainWindow> > mainWindows;
    QList<QPointer<KisDocument> > documents;
    QList<KisAction*> scriptActions;

    KActionCollection *actionCollection{0};

    KisIdleWatcher idleWatcher;
    KisAnimationCachePopulator animationCachePopulator;
};


KisPart* KisPart::instance()
{
    return s_instance;
}


KisPart::KisPart()
    : d(new Private(this))
{
    // Preload all the resources in the background
    Q_UNUSED(KoResourceServerProvider::instance());
    Q_UNUSED(KisResourceServerProvider::instance());
    Q_UNUSED(KisColorManager::instance());

    connect(this, SIGNAL(documentOpened(QString)),
            this, SLOT(updateIdleWatcherConnections()));

    connect(this, SIGNAL(documentClosed(QString)),
            this, SLOT(updateIdleWatcherConnections()));

    connect(KisActionRegistry::instance(), SIGNAL(shortcutsUpdated()),
            this, SLOT(updateShortcuts()));
    connect(&d->idleWatcher, SIGNAL(startedIdleMode()),
            &d->animationCachePopulator, SLOT(slotRequestRegeneration()));

    d->animationCachePopulator.slotRequestRegeneration();
}

KisPart::~KisPart()
{
    while (!d->documents.isEmpty()) {
        delete d->documents.takeFirst();
    }

    while (!d->views.isEmpty()) {
        delete d->views.takeFirst();
    }

    while (!d->mainWindows.isEmpty()) {
        delete d->mainWindows.takeFirst();
    }

    delete d;
}

void KisPart::updateIdleWatcherConnections()
{
    QVector<KisImageSP> images;

    Q_FOREACH (QPointer<KisDocument> document, documents()) {
        if (document->image()) {
            images << document->image();
        }
    }

    d->idleWatcher.setTrackedImages(images);
}

void KisPart::addDocument(KisDocument *document)
{
    //dbgUI << "Adding document to part list" << document;
    Q_ASSERT(document);
    if (!d->documents.contains(document)) {
        d->documents.append(document);
        emit documentOpened('/'+objectName());
        emit sigDocumentAdded(document);
        connect(document, SIGNAL(sigSavingFinished()), SLOT(slotDocumentSaved()));
    }
}

QList<QPointer<KisDocument> > KisPart::documents() const
{
    return d->documents;
}

KisDocument *KisPart::createDocument() const
{
    KisDocument *doc = new KisDocument();
    return doc;
}


int KisPart::documentCount() const
{
    return d->documents.size();
}

void KisPart::removeDocument(KisDocument *document)
{
    d->documents.removeAll(document);
    emit documentClosed('/'+objectName());
    emit sigDocumentRemoved(document->url().toLocalFile());
    document->deleteLater();
}

KisMainWindow *KisPart::createMainWindow()
{
    KisMainWindow *mw = new KisMainWindow();
    Q_FOREACH(KisAction *action, d->scriptActions) {
        mw->viewManager()->scriptManager()->addAction(action);
    }
    dbgUI <<"mainWindow" << (void*)mw << "added to view" << this;
    d->mainWindows.append(mw);
    emit sigWindowAdded(mw);
    return mw;
}

KisView *KisPart::createView(KisDocument *document,
                             KoCanvasResourceManager *resourceManager,
                             KActionCollection *actionCollection,
                             QWidget *parent)
{
    // If creating the canvas fails, record this and disable OpenGL next time
    KisConfig cfg;
    KConfigGroup grp( KSharedConfig::openConfig(), "crashprevention");
    if (grp.readEntry("CreatingCanvas", false)) {
        cfg.setUseOpenGL(false);
    }
    if (cfg.canvasState() == "OPENGL_FAILED") {
        cfg.setUseOpenGL(false);
    }
    grp.writeEntry("CreatingCanvas", true);
    grp.sync();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    KisView *view  = new KisView(document, resourceManager, actionCollection, parent);
    QApplication::restoreOverrideCursor();

    // Record successful canvas creation
    grp.writeEntry("CreatingCanvas", false);
    grp.sync();

    addView(view);

    return view;
}

void KisPart::addView(KisView *view)
{
    if (!view)
        return;

    if (!d->views.contains(view)) {
        d->views.append(view);
    }

    emit sigViewAdded(view);
}

void KisPart::removeView(KisView *view)
{
    if (!view) return;

    /**
     * HACK ALERT: we check here explicitly if the document (or main
     *             window), is saving the stuff. If we close the
     *             document *before* the saving is completed, a crash
     *             will happen.
     */
    KIS_ASSERT_RECOVER_RETURN(!view->mainWindow()->hackIsSaving());

    emit sigViewRemoved(view);

    QPointer<KisDocument> doc = view->document();
    d->views.removeAll(view);

    if (doc) {
        bool found = false;
        Q_FOREACH (QPointer<KisView> view, d->views) {
            if (view && view->document() == doc) {
                found = true;
                break;
            }
        }
        if (!found) {
            removeDocument(doc);
        }
    }
}

QList<QPointer<KisView> > KisPart::views() const
{
    return d->views;
}

int KisPart::viewCount(KisDocument *doc) const
{
    if (!doc) {
        return d->views.count();
    }
    else {
        int count = 0;
        Q_FOREACH (QPointer<KisView> view, d->views) {
            if (view && view->isVisible() && view->document() == doc) {
                count++;
            }
        }
        return count;
    }
}

void KisPart::slotDocumentSaved()
{
    KisDocument *doc = qobject_cast<KisDocument*>(sender());
    emit sigDocumentSaved(doc->url().toLocalFile());
}

void KisPart::removeMainWindow(KisMainWindow *mainWindow)
{
    dbgUI <<"mainWindow" << (void*)mainWindow <<"removed from doc" << this;
    if (mainWindow) {
        d->mainWindows.removeAll(mainWindow);
    }
}

const QList<QPointer<KisMainWindow> > &KisPart::mainWindows() const
{
    return d->mainWindows;
}

int KisPart::mainwindowCount() const
{
    return d->mainWindows.count();
}


KisMainWindow *KisPart::currentMainwindow() const
{
    QWidget *widget = qApp->activeWindow();
    KisMainWindow *mainWindow = qobject_cast<KisMainWindow*>(widget);
    while (!mainWindow && widget) {
        widget = widget->parentWidget();
        mainWindow = qobject_cast<KisMainWindow*>(widget);
    }

    if (!mainWindow && mainWindows().size() > 0) {
        mainWindow = mainWindows().first();
    }
    return mainWindow;

}

void KisPart::addScriptAction(KisAction *action)
{
    d->scriptActions << action;
}

KisIdleWatcher* KisPart::idleWatcher() const
{
    return &d->idleWatcher;
}

KisAnimationCachePopulator* KisPart::cachePopulator() const
{
    return &d->animationCachePopulator;
}

void KisPart::openExistingFile(const QUrl &url)
{
    // TODO: refactor out this method!

    KisMainWindow *mw = currentMainwindow();
    KIS_SAFE_ASSERT_RECOVER_RETURN(mw);

    mw->openDocument(url, KisMainWindow::None);
}

void KisPart::updateShortcuts()
{
    // Update any non-UI actionCollections.  That includes:
    //  - Shortcuts called inside of tools
    //  - Perhaps other things?
    KoToolManager::instance()->updateToolShortcuts();

    // Now update the UI actions.
    Q_FOREACH (KisMainWindow *mainWindow, d->mainWindows) {
        KActionCollection *ac = mainWindow->actionCollection();

        ac->updateShortcuts();

        // Loop through mainWindow->actionCollections() to modify tooltips
        // so that they list shortcuts at the end in parentheses
        Q_FOREACH ( QAction* action, ac->actions())
        {
            // Remove any existing suffixes from the tooltips.
            // Note this regexp starts with a space, e.g. " (Ctrl-a)"
            QString strippedTooltip = action->toolTip().remove(QRegExp("\\s\\(.*\\)"));

            // Now update the tooltips with the new shortcut info.
            if(action->shortcut() == QKeySequence(0))
                action->setToolTip(strippedTooltip);
            else
                action->setToolTip( strippedTooltip + " (" + action->shortcut().toString() + ")");
        }
    }
}

void KisPart::openTemplate(const QUrl &url)
{
    qApp->setOverrideCursor(Qt::BusyCursor);
    KisDocument *document = createDocument();

    bool ok = document->loadNativeFormat(url.toLocalFile());
    document->setModified(false);
    document->undoStack()->clear();

    if (ok) {
        QString mimeType = KisMimeDatabase::mimeTypeForFile(url.toLocalFile());
        // in case this is a open document template remove the -template from the end
        mimeType.remove( QRegExp( "-template$" ) );
        document->setMimeTypeAfterLoading(mimeType);
        document->resetURL();
    }
    else {
        if (document->errorMessage().isEmpty()) {
            QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("Could not create document from template\n%1", document->localFilePath()));
        }
        else {
            QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("Could not create document from template\n%1\nReason: %2", document->localFilePath(), document->errorMessage()));
        }
        delete document;
        return;
    }
    addDocument(document);

    KisMainWindow *mw = currentMainwindow();
    mw->addViewAndNotifyLoadingCompleted(document);

    KisOpenPane *pane = qobject_cast<KisOpenPane*>(sender());
    if (pane) {
        pane->hide();
        pane->deleteLater();

    }

    qApp->restoreOverrideCursor();
}

void KisPart::addRecentURLToAllMainWindows(QUrl url)
{
    // Add to recent actions list in our mainWindows
    Q_FOREACH (KisMainWindow *mainWindow, d->mainWindows) {
        mainWindow->addRecentURL(url);
    }
}


void KisPart::startCustomDocument(KisDocument* doc)
{
    addDocument(doc);
    KisMainWindow *mw = currentMainwindow();
    KisOpenPane *pane = qobject_cast<KisOpenPane*>(sender());
    if (pane) {
        pane->hide();
        pane->deleteLater();
    }
    mw->addViewAndNotifyLoadingCompleted(doc);

}

KisInputManager* KisPart::currentInputManager()
{
    KisMainWindow *mw = currentMainwindow();
    KisViewManager *manager = mw ? mw->viewManager() : 0;
    return manager ? manager->inputManager() : 0;
}


/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 1998-1999 Torben Weis <weis@kde.org>
 * SPDX-FileCopyrightText: 2000-2005 David Faure <faure@kde.org>
 * SPDX-FileCopyrightText: 2007-2008 Thorsten Zachmann <zachmann@kde.org>
 * SPDX-FileCopyrightText: 2010-2012 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2011 Inge Wallin <ingwa@kogmbh.com>
 * SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisPart.h"

#include "KoProgressProxy.h"
#include <KoCanvasController.h>
#include <KoCanvasControllerWidget.h>
#include <KoColorSpaceEngine.h>
#include <KoCanvasBase.h>
#include <KoToolManager.h>
#include <KoShapeControllerBase.h>
#include <KoResourceServerProvider.h>
#include <kis_icon.h>

#include "KisApplication.h"
#include "KisMainWindow.h"
#include "KisDocument.h"
#include "KisView.h"
#include "KisViewManager.h"
#include "KisImportExportManager.h"

#include <kis_debug.h>
#include <KoResourcePaths.h>
#include <KoDialog.h>
#include <QMessageBox>
#include <QMenu>
#include <QMap>

#include <QMenuBar>
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
#include <dialogs/KisSessionManagerDialog.h>

#include <kis_group_layer.h>
#include "kis_config.h"
#include "kis_shape_controller.h"
#include "KisResourceServerProvider.h"
#include "kis_animation_cache_populator.h"
#include "kis_image_animation_interface.h"
#include "kis_time_span.h"
#include "kis_idle_watcher.h"
#include "kis_image.h"
#include "KisOpenPane.h"
#include "KisTranslateLayerNamesVisitor.h"
#include "kis_color_manager.h"

#include "kis_action.h"
#include "kis_action_registry.h"
#include "KisSessionResource.h"
#include "KisBusyWaitBroker.h"
#include "dialogs/kis_delayed_save_dialog.h"

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
    KActionCollection *actionCollection{0};
    KisIdleWatcher idleWatcher;
    KisAnimationCachePopulator animationCachePopulator;

    KisSessionResourceSP currentSession;
    bool closingSession{false};
    QScopedPointer<KisSessionManagerDialog> sessionManager;

    QMap<QUrl, QUrl> pendingAddRecentUrlMap;

    bool queryCloseDocument(KisDocument *document) {
        Q_FOREACH(auto view, views) {
            if (view && view->isVisible() && view->document() == document) {
                return view->queryClose();
            }
        }

        return true;
    }
};


KisPart* KisPart::instance()
{
    return s_instance;
}

namespace {
void busyWaitWithFeedback(KisImageSP image)
{
    const int busyWaitDelay = 1000;
    KisDelayedSaveDialog dialog(image, KisDelayedSaveDialog::ForcedDialog, busyWaitDelay, KisPart::instance()->currentMainwindow());
    dialog.blockIfImageIsBusy();
}
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
    KisBusyWaitBroker::instance()->setFeedbackCallback(&busyWaitWithFeedback);
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

void KisPart::addDocument(KisDocument *document, bool notify)
{
    //dbgUI << "Adding document to part list" << document;
    Q_ASSERT(document);
    if (!d->documents.contains(document)) {
        d->documents.append(document);
        if (notify){
            emit documentOpened('/'+ objectName());
            emit sigDocumentAdded(document);
        }
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

KisDocument *KisPart::createTemporaryDocument() const
{
    KisDocument *doc = new KisDocument(false);
    return doc;
}


int KisPart::documentCount() const
{
    return d->documents.size();
}

void KisPart::removeDocument(KisDocument *document, bool deleteDocument)
{
    if (document) {
        d->documents.removeAll(document);
        emit documentClosed('/' + objectName());
        emit sigDocumentRemoved(document->path());
        if (deleteDocument) {
            document->deleteLater();
        }
    }
}

KisMainWindow *KisPart::createMainWindow(QUuid id)
{
    KisMainWindow *mw = new KisMainWindow(id);
    dbgUI <<"mainWindow" << (void*)mw << "added to view" << this;
    d->mainWindows.append(mw);

    // Add all actions with a menu property to the main window
    Q_FOREACH(QAction *action, mw->actionCollection()->actions()) {
        QString menuLocation = action->property("menulocation").toString();
        if (!menuLocation.isEmpty()) {
            QAction *found = 0;
            QList<QAction *> candidates = mw->menuBar()->actions();
            Q_FOREACH(const QString &name, menuLocation.split("/")) {
                Q_FOREACH(QAction *candidate, candidates) {
                    if (candidate->objectName().toLower() == name.toLower()) {
                        found = candidate;
                        candidates = candidate->menu()->actions();
                        break;
                    }
                }
                if (candidates.isEmpty()) {
                    break;
                }
            }

            if (found && found->menu()) {
                found->menu()->addAction(action);
            }
        }
    }


    return mw;
}

void KisPart::notifyMainWindowIsBeingCreated(KisMainWindow *mainWindow)
{
    emit sigMainWindowIsBeingCreated(mainWindow);
}


KisView *KisPart::createView(KisDocument *document,
                             KisViewManager *viewManager,
                             QWidget *parent)
{
    // If creating the canvas fails, record this and disable OpenGL next time
    KisConfig cfg(false);
    KConfigGroup grp( KSharedConfig::openConfig(), "crashprevention");
    if (grp.readEntry("CreatingCanvas", false)) {
        cfg.disableOpenGL();
    }
    if (cfg.canvasState() == "OPENGL_FAILED") {
        cfg.disableOpenGL();
    }
    grp.writeEntry("CreatingCanvas", true);
    grp.sync();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    KisView *view = new KisView(document,  viewManager, parent);
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

bool KisPart::closingSession() const
{
    return d->closingSession;
}

bool KisPart::exists()
{
    return s_instance.exists();
}

bool KisPart::closeSession(bool keepWindows)
{
    d->closingSession = true;

    Q_FOREACH(auto document, d->documents) {
        if (!d->queryCloseDocument(document.data())) {
            d->closingSession = false;
            return false;
        }
    }

    if (d->currentSession) {
        KisConfig kisCfg(false);
        if (kisCfg.saveSessionOnQuit(false)) {

            d->currentSession->storeCurrentWindows();
            d->currentSession->save();

            KConfigGroup cfg = KSharedConfig::openConfig()->group("session");
            cfg.writeEntry("previousSession", d->currentSession->name());
        }

        d->currentSession = nullptr;
    }

    if (!keepWindows) {
        Q_FOREACH (auto window, d->mainWindows) {
            window->close();
        }

        if (d->sessionManager) {
            d->sessionManager->close();
        }
    }

    d->closingSession = false;
    return true;
}

void KisPart::slotDocumentSaved()
{
    KisDocument *doc = qobject_cast<KisDocument*>(sender());
    emit sigDocumentSaved(doc->path());

    QUrl url = QUrl::fromLocalFile(doc->path());
    if (!d->pendingAddRecentUrlMap.contains(url)) {
        return;
    }
    QUrl oldUrl = d->pendingAddRecentUrlMap.take(url);
    addRecentURLToAllMainWindows(QUrl::fromLocalFile(doc->path()));
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

KisMainWindow * KisPart::windowById(QUuid id) const
{
    Q_FOREACH(QPointer<KisMainWindow> mainWindow, d->mainWindows) {
        if (mainWindow->id() == id) {
            return mainWindow;
        }
    }

    return nullptr;
}

KisIdleWatcher* KisPart::idleWatcher() const
{
    return &d->idleWatcher;
}

KisAnimationCachePopulator* KisPart::cachePopulator() const
{
    return &d->animationCachePopulator;
}

void KisPart::prioritizeFrameForCache(KisImageSP image, int frame) {
    KisImageAnimationInterface* animInterface = image->animationInterface();
    if ( animInterface && animInterface->fullClipRange().contains(frame)) {
        d->animationCachePopulator.requestRegenerationWithPriorityFrame(image, frame);
    }
}

void KisPart::openExistingFile(const QString &path)
{
    // TODO: refactor out this method!

    KisMainWindow *mw = currentMainwindow();
    KIS_SAFE_ASSERT_RECOVER_RETURN(mw);

    mw->openDocument(path, KisMainWindow::None);
}

void KisPart::updateShortcuts()
{
    // Update any non-UI actionCollections.  That includes:
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
            if (action->shortcut() == QKeySequence(0))
                action->setToolTip(strippedTooltip);
            else
                action->setToolTip( strippedTooltip + " (" + action->shortcut().toString(QKeySequence::NativeText) + ")");
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
        document->resetPath();
        document->setReadWrite(true);
    }
    else {
        if (document->errorMessage().isEmpty()) {
            QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Could not create document from template\n%1", document->localFilePath()));
        }
        else {
            QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Could not create document from template\n%1\nReason: %2", document->localFilePath(), document->errorMessage()));
        }
        delete document;
        return;
    }
    QMap<QString, QString> dictionary;
    // XXX: fill the dictionary from the desktop file
    KisTranslateLayerNamesVisitor v(dictionary);
    document->image()->rootLayer()->accept(v);

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

void KisPart::addRecentURLToAllMainWindows(QUrl url, QUrl oldUrl)
{
    // Add to recent actions list in our mainWindows
    Q_FOREACH (KisMainWindow *mainWindow, d->mainWindows) {
        mainWindow->addRecentURL(url, oldUrl);
    }
}

void KisPart::queueAddRecentURLToAllMainWindowsOnFileSaved(QUrl url, QUrl oldUrl)
{
    d->pendingAddRecentUrlMap.insert(url, oldUrl);
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

void KisPart::showSessionManager()
{
    if (d->sessionManager.isNull()) {
        d->sessionManager.reset(new KisSessionManagerDialog());
    }

    d->sessionManager->show();
    d->sessionManager->activateWindow();
}

void KisPart::startBlankSession()
{
    KisMainWindow *window = createMainWindow();
    window->initializeGeometry();
    window->show();

}

bool KisPart::restoreSession(const QString &sessionName)
{
    if (sessionName.isNull()) return false;

    KoResourceServer<KisSessionResource> *rserver = KisResourceServerProvider::instance()->sessionServer();
    KisSessionResourceSP session = rserver->resourceByName(sessionName);
    if (!session || !session->valid()) return false;

    return restoreSession(session);
}

bool KisPart::restoreSession(KisSessionResourceSP session)
{
    session->restore();
    d->currentSession = session;
    return true;
}

void KisPart::setCurrentSession(KisSessionResourceSP session)
{
    d->currentSession = session;
}

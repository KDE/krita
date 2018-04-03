/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000-2005 David Faure  <faure@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2010 Boudewijn Rempt   <boud@valdyas.org>
   Copyright (C) 2015 Michael Abrahams  <miabraha@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KIS_PART_H
#define KIS_PART_H

#include <QList>
#include <QPointer>
#include <QUrl>
#include <QUuid>

#include "kritaui_export.h"
#include <KConfigCore/kconfiggroup.h>
#include <KoConfig.h>
#include <KisMainWindow.h>

namespace KIO {
}

class KisAction;
class KisDocument;
class KisView;
class KisDocument;
class KisIdleWatcher;
class KisAnimationCachePopulator;
class KisSessionResource;

/**
 * KisPart is the Great Deku Tree of Krita.
 *
 * It is a singleton class which provides the main entry point to the application.
 * Krita supports multiple documents, multiple main windows, and multiple
 * components.  KisPart manages these resources and provides them to the rest of
 * Krita.  It manages lists of Actions and shortcuts as well.
 *
 * The terminology comes from KParts, which is a system allowing one KDE app
 * to be run from inside another, like pressing F4 inside dophin to run konsole.
 *
 * Needless to say, KisPart hasn't got much to do with KParts anymore.
 */
class KRITAUI_EXPORT KisPart : public QObject
{
    Q_OBJECT

public:

    static KisPart *instance();

    /**
     * Constructor.
     *
     * @param parent may be another KisDocument, or anything else.
     *        Usually passed by KPluginFactory::create.
     */
    explicit KisPart();

    /**
     *  Destructor.
     *
     * The destructor does not delete any attached KisView objects and it does not
     * delete the attached widget as returned by widget().
     */
    ~KisPart() override;

    // ----------------- Document management -----------------

    /**
     * create an empty document. The document is not automatically registered with the part.
     */
    KisDocument *createDocument() const;

    /**
     * Add the specified document to the list of documents this KisPart manages.
     */
    void addDocument(KisDocument *document);

    /**
     * @return a list of all documents this part manages
     */
    QList<QPointer<KisDocument> > documents() const;

    /**
     * @return number of documents this part manages.
     */
    int documentCount() const;

    void removeDocument(KisDocument *document);

    // ----------------- MainWindow management -----------------


    /**
     * Create a new main window.
     */
    KisMainWindow *createMainWindow(QUuid id = QUuid());

    /**
     * Removes a main window from the list of managed windows.
     *
     * This is called by the MainWindow after it finishes its shutdown routine.
     */
    void removeMainWindow(KisMainWindow *mainWindow);

    /**
     * @return the list of main windows.
     */
    const QList<QPointer<KisMainWindow> >& mainWindows() const;

    /**
     * @return the number of shells for the main window
     */
    int mainwindowCount() const;

    void addRecentURLToAllMainWindows(QUrl url);

    /**
     * @return the currently active main window.
     */
    KisMainWindow *currentMainwindow() const;

    KisMainWindow *windowById(QUuid id) const;

    /**
     * @return the application-wide KisIdleWatcher.
     */
    KisIdleWatcher *idleWatcher() const;

    /**
     * @return the application-wide AnimationCachePopulator.
     */
    KisAnimationCachePopulator *cachePopulator() const;

public Q_SLOTS:

    /**
     * This slot loads an existing file.
     * @param url the file to load
     */
    void openExistingFile(const QUrl &url);

    /**
     * This slot loads a template and deletes the sender.
     * @param url the template to load
     */
    void openTemplate(const QUrl &url);


    /**
     * @brief startCustomDocument adds the given document to the document list and deletes the sender()
     * @param doc
     */
    void startCustomDocument(KisDocument *doc);

private Q_SLOTS:

    void updateIdleWatcherConnections();

    void updateShortcuts();

Q_SIGNALS:
    /**
     * emitted when a new document is opened. (for the idle watcher)
     */
    void documentOpened(const QString &ref);

    /**
     * emitted when an old document is closed. (for the idle watcher)
     */
    void documentClosed(const QString &ref);

    // These signals are for libkis or sketch
    void sigViewAdded(KisView *view);
    void sigViewRemoved(KisView *view);
    void sigDocumentAdded(KisDocument *document);
    void sigDocumentSaved(const QString &url);
    void sigDocumentRemoved(const QString &filename);
    void sigWindowAdded(KisMainWindow *window);

public:

    KisInputManager *currentInputManager();

    //------------------ View management ------------------

    /**
     * Create a new view for the document. The view is added to the list of
     * views, and if the document wasn't known yet, it's registered as well.
     */
    KisView *createView(KisDocument *document,
                        KoCanvasResourceManager *resourceManager,
                        KActionCollection *actionCollection,
                        QWidget *parent);

    /**
     * Adds a view to the document. If the part doesn't know yet about
     * the document, it is registered.
     *
     * This calls KisView::updateReadWrite to tell the new view
     * whether the document is readonly or not.
     */
    void addView(KisView *view);

    /**
     * Removes a view of the document.
     */
    void removeView(KisView *view);

    /**
     * @return a list of views this document is displayed in
     */
    QList<QPointer<KisView> > views() const;

    /**
     * @return number of views this document is displayed in
     */
    int viewCount(KisDocument *doc) const;

    //------------------ Session management ------------------

    void showSessionManager();

    void startBlankSession();

    /**
     * Restores a saved session by name
     */
    bool restoreSession(const QString &sessionName);

    void setCurrentSession(KisSessionResource *session);

    /**
     * Attempts to save the session and close all windows.
     * This may involve asking the user to save open files.
     * @return false, if closing was cancelled by the user
     */
    bool closeSession(bool keepWindows = false);

    /**
     * Are we in the process of closing the application through closeSession().
     */
    bool closingSession() const;

private Q_SLOTS:

    void slotDocumentSaved();

private:

    Q_DISABLE_COPY(KisPart)

    class Private;
    Private *const d;

};

#endif

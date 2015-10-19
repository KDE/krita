/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000-2005 David Faure <faure@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2010 Boudewijn Rempt <boud@kogmbh.com>

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

#include "kritaui_export.h"

#include <KisMainWindow.h>

namespace KIO {
}

class KisDocument;
class KisView;
class QGraphicsItem;
class KisDocument;
class KisIdleWatcher;
class KisAnimationCachePopulator;


/**
 * KisPart is the Great Deku Tree of Krita.
 *
 * It is a singleton class which provides the main entry point to the application.
 * Krita supports multiple documents, multiple main windows, and multiple
 * components.  KisPart manages these resources and provides them to the rest of
 * Krita.  It manages lists QActions and shortcuts, as well.
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
    ~KisPart();

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
    KisMainWindow *createMainWindow();

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
     * This slot loads an existing file and deletes the start up widget.
     * @param url the file to load
     */
    void openExistingFile(const QUrl &url);

    /**
     * @brief configureShortcuts opens the shortcut configuration dialog.
     * @param parent the parent widget for the dialog
     *
     * After the user closes the dialog, all actioncollections will be updated
     * with the new shortcuts.
     */
    void configureShortcuts();

protected Q_SLOTS:

    /**
     * This slot loads a template and deletes the start up widget.
     * @param url the template to load
     */
    void openTemplate(const QUrl &url);

private Q_SLOTS:

    void viewDestroyed();

    void startCustomDocument(KisDocument *doc);

    void updateIdleWatcherConnections();

Q_SIGNALS:
    /**
     * emitted when a new document is opened.
     */
    void documentOpened(const QString &ref);

    /**
     * emitted when an old document is closed.
     */
    void documentClosed(const QString &ref);

    void sigViewAdded(KisView *view);
    void sigViewRemoved(KisView *view);

public:

    static KisInputManager *currentInputManager();

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

    /**
     * @return a QGraphicsItem canvas displaying this document. There is only one QGraphicsItem canvas that can
     * be shown by many QGraphicsView subclasses (those should reimplement KoCanvasController
     * as well).
     *
     * @param create if true, a new canvas item is created if there wasn't one.
     */
    QGraphicsItem *canvasItem(KisDocument *document, bool create = true);

    // ------- Startup/openpane etc ---------------

    /**
     * Template resource path used. This is used by the start up widget to show
     * the correct templates.
     */
    QString templatesResourcePath() const;


    /**
     * Creates and shows the start up widget.
     * @param parent the KisMainWindow used as parent for the widget.
     * @param alwaysShow always show the widget even if the user has configured it to not show.
     */
    void showStartUpWidget(KisMainWindow *parent, bool alwaysShow = false);

protected:

    /**
     * Set the templates resource path used. This is used by the start up widget to show
     * the correct templates.
     */
    void setTemplatesResourcePath(const QString &templatesResourcePath);

    /**
     * Struct used in the list created by createCustomDocumentWidgets()
     */
    struct CustomDocumentWidgetItem {
        /// Pointer to the custom document widget
        QWidget *widget;
        /// title used in the sidebar. If left empty it will be displayed as "Custom Document"
        QString title;
        /// icon used in the sidebar. If left empty it will use the unknown icon
        QString icon;
    };

    /**
     * This generates widgets for the startup dialog. It populates the dialog
     * with widgets providing different ways to load new documents.
     *
     * (For example, "blank document", "create from clipboard", "open file")
     *
     * Each widget returned from this function should follow a certain format.
     * The returned widget should provide its own button (preferably 'Create')
     * and implement the logic to implement the document instance correctly.
     * The widget should use the signal 'documentSelected(KisDocument*)' to
     * notify that the startup dialog should close and KisPart should load the
     * new document.
     *
     * @see KisPart::showStartUpWidget()
     * @see KisDocument::createCustomDocumentWidget()
     * @see KisOpenPane::createCustomDocumentWidget()
     *
     * @param parent the parent of the to be created widget.
     * @return a list of KisDocument::CustomDocumentWidgetItem.
     */
    QList<CustomDocumentWidgetItem> createCustomDocumentWidgets(QWidget *parent);

    /**
     * Override this to create a QGraphicsItem that does not rely
     * on proxying a KoCanvasController.
     */
    QGraphicsItem *createCanvasItem(KisDocument *document);


private:

    Q_DISABLE_COPY(KisPart)

    class Private;
    Private *const d;


};

#endif

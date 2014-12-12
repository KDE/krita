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

#ifndef KOPART_H
#define KOPART_H

#include <QList>
#include <QPointer>

#include <kcomponentdata.h>
#include <kurl.h>
#include <kxmlguiclient.h>

#include "krita_export.h"

#include <KisMainWindow.h>

class KJob;
namespace KIO {
  class Job;
}

class KisDocument;
class KisView;
class KisOpenPane;
class QGraphicsItem;
class KisDocument;
class KisAnimationDoc;

/**
 * Override this class in your application. It's the main entry point that
 * should provide the document, the view and the component data to the calligra
 * system.
 *
 * There is/will be a single KisPart instance for an application that will manage
 * the list of documents, views and mainwindows.
 *
 * It hasn't got much to do with kparts anymore.
 */
class KRITAUI_EXPORT KisPart : public QObject
{
    Q_OBJECT

public:

    static KisPart *instance();

private:

    /**
     * Constructor.
     *
     * @param parent may be another KisDocument, or anything else.
     *        Usually passed by KPluginFactory::create.
     */
    explicit KisPart();

public:
    /**
     *  Destructor.
     *
     * The destructor does not delete any attached KisView objects and it does not
     * delete the attached widget as returned by widget().
     */
    ~KisPart();

    // ----------------- mainwindow management -----------------

    /**
     * create an empty document. The document is not automatically registered with the part.
     */
    KisDocument *createDocument() const;
    KisAnimationDoc *createAnimationDoc() const;

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

    // ----------------- mainwindow management -----------------

    /**
     * Create a new main window, but does not add it to the current set of managed main windows.
     */
    KisMainWindow *createMainWindow();

    /**
     * Appends the mainwindow to the list of mainwindows which this part manages.
     */
    void addMainWindow(KisMainWindow *mainWindow);

    /**
     * Removes the mainwindow from the list.
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

    void addRecentURLToAllMainWindows(KUrl url);

    KisMainWindow *currentMainwindow() const;

public slots:

    /**
     * This slot loads an existing file and deletes the start up widget.
     * @param url the file to load
     */
    void openExistingFile(const KUrl& url);

protected slots:

    /**
     * This slot loads a template and deletes the start up widget.
     * @param url the template to load
     */
    void openTemplate(const KUrl& url);

private slots:

    void viewDestroyed();

    void startCustomDocument(KisDocument *doc);

signals:

    void sigViewAdded(KisView *view);
    void sigViewRemoved(KisView *view);

public:

    //------------------ view management ------------------

    /**
     * Create a new view for the document. The view is added to the list of
     * views, and if the document wasn't known yet, it's registered as well.
     */
    KisView *createView(KisDocument *document, KisMainWindow *parent);

    /**
     * Adds a view to the document. If the part doesn't know yet about
     * the document, it is registered.
     *
     * This calls KisView::updateReadWrite to tell the new view
     * whether the document is readonly or not.
     */
    void addView(KisView *view, KisDocument *document);

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

    // ------- startup/openpane etc ---------------

    /**
     * Template type used. This is used by the start up widget to show
     * the correct templates.
     */
    QString templateType() const;


    /**
     * Creates and shows the start up widget.
     * @param parent the KisMainWindow used as parent for the widget.
     * @param alwaysShow always show the widget even if the user has configured it to not show.
     */
    void showStartUpWidget(KisMainWindow *parent, bool alwaysShow = false);

protected:

    /**
     * Set the template type used. This is used by the start up widget to show
     * the correct templates.
     */
    void setTemplateType(const QString& _templateType);

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
     * Override this method in your derived class to show a widget in the startup 'dialog'.
     * This widget should allow the user to set settings for a custom document (i.e. one
     * not based on a template).
     * The returned widget should provide its own button (preferably 'Create') and
     * implement the logic to implement the document instance correctly.
     * After initializing the widget should emit a signal called 'documentSelected(KisDocument*)' which
     * will remove the startupWidget and show the document.
     * @param parent the parent of the to be created widget.
     * @return a list of KisDocument::CustomDocumentWidgetItem.
     */
    QList<CustomDocumentWidgetItem> createCustomDocumentWidgets(QWidget *parent);

    /**
     * Override this to create a QGraphicsItem that does not rely
     * on proxying a KoCanvasController.
     */
    QGraphicsItem *createCanvasItem(KisDocument *document);

protected slots:

    /// Quits Krita with error message from m_errorMessage.
    void showErrorAndDie();

private:

    Q_DISABLE_COPY(KisPart)

    class Private;
    Private *const d;

protected:
    QString m_errorMessage;
    bool m_dieOnError;

};

#endif

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

#include <kservice.h>
#include <kcomponentdata.h>
#include <kurl.h>
#include <kxmlguiclient.h>

#include "komain_export.h"

#include <KoMainWindow.h>

class KJob;
namespace KIO {
  class Job;
}

class KoDocument;
class KoView;
class KoView;
class KoOpenPane;
class QGraphicsItem;

/**
 * Override this class in your application. It's the main entry point that
 * should provide the document, the view and the component data to the calligra
 * system.
 *
 * There is/will be a single KoPart instance for an application that will manage
 * the list of documents, views and mainwindows.
 *
 * It hasn't got much to do with kparts anymore.
 */
class KOMAIN_EXPORT KoPart : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param parent may be another KoDocument, or anything else.
     *        Usually passed by KPluginFactory::create.
     */
    explicit KoPart(QObject *parent);

    /**
     *  Destructor.
     *
     * The destructor does not delete any attached KoView objects and it does not
     * delete the attached widget as returned by widget().
     */
    virtual ~KoPart();

    /**
     * @return The componentData ( KComponentData ) for this GUI client. You set the componentdata
     * in your subclass: setComponentData(AppFactory::componentData()); in the constructor
     */
    KComponentData componentData() const;

    /**
     * @param document the document this part manages
     */
    void setDocument(KoDocument *document);

    /**
     * @return the document this part loads and saves to and makes views for
     */
    KoDocument *document() const;

    // ----------------- mainwindow management -----------------

    /**
     * Create a new main window, but does not add it to the current set of managed main windows.
     */
    virtual KoMainWindow *createMainWindow() = 0;

    /**
     * Appends the mainwindow to the list of mainwindows which show this
     * document as their root document.
     *
     * This method is automatically called from KoMainWindow::setRootDocument,
     * so you do not need to call it.
     */
    virtual void addMainWindow(KoMainWindow *mainWindow);

    /**
     * Removes the mainwindow from the list.
     */
    virtual void removeMainWindow(KoMainWindow *mainWindow);

    /**
     * @return the list of main windows.
     */
    const QList<KoMainWindow*>& mainWindows() const;

    /**
     * @return the number of shells for the main window
     */
    int mainwindowCount() const;

    void addRecentURLToAllMainWindows(KUrl url);

    KoMainWindow *currentMainwindow() const;

protected slots:

    /**
     * This slot loads an existing file and deletes the start up widget.
     * @param url the file to load
     */
    virtual void openExistingFile(const KUrl& url);

    /**
     * This slot loads a template and deletes the start up widget.
     * @param url the template to load
     */
    virtual void openTemplate(const KUrl& url);

signals:

    void closeEmbedInitDialog();

private slots:

    void startCustomDocument();


public:

    //------------------ view management ------------------

    /**
     *  Create a new view for the document.
     */
    KoView *createView(KoDocument *document, QWidget *parent = 0);

    /**
     * Adds a view to the document. If the part doesn't know yet about
     * the document, it is registered.
     *
     * This calls KoView::updateReadWrite to tell the new view
     * whether the document is readonly or not.
     */
    virtual void addView(KoView *view, KoDocument *document);

    /**
     * Removes a view of the document.
     */
    virtual void removeView(KoView *view);

    /**
     * @return a list of views this document is displayed in
     */
    QList<KoView*> views() const;

    /**
     * @return number of views this document is displayed in
     */
    int viewCount() const;

    /**
     * @return a QGraphicsItem canvas displaying this document. There is only one QGraphicsItem canvas that can
     * be shown by many QGraphicsView subclasses (those should reimplement KoCanvasController
     * as well).
     *
     * @param create if true, a new canvas item is created if there wasn't one.
     */
    QGraphicsItem *canvasItem(KoDocument *document, bool create = true);

    // ------- startup/openpane etc ---------------

    /**
     * Template type used. This is used by the start up widget to show
     * the correct templates.
     */
    QString templateType() const;


    /**
     * Creates and shows the start up widget.
     * @param parent the KoMainWindow used as parent for the widget.
     * @param alwaysShow always show the widget even if the user has configured it to not show.
     */
    virtual void showStartUpWidget(KoMainWindow *parent, bool alwaysShow = false);

    /**
     * Removes the startupWidget shown at application start up.
     */
    void deleteOpenPane(bool closing = false);

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
     * After initializing the widget should emit a signal called 'documentSelected()' which
     * will remove the startupWidget and show the document.
     * @param parent the parent of the to be created widget.
     * @return a list of KoDocument::CustomDocumentWidgetItem.
     */
    virtual QList<CustomDocumentWidgetItem> createCustomDocumentWidgets(QWidget *parent);

    /**
     * Creates the open widget showed at application start up.
     * @param parent the parent widget
     * @param instance the KComponentData to be used for KConfig data
     * @param templateType the template-type (group) that should be selected on creation.
     */
    KoOpenPane *createOpenPane(QWidget *parent, const KComponentData &instance,
                               const QString& templateType = QString());

    virtual KoView *createViewInstance(KoDocument *document, QWidget *parent) = 0;

    /**
     * Override this to create a QGraphicsItem that does not rely
     * on proxying a KoCanvasController.
     */
    virtual QGraphicsItem *createCanvasItem(KoDocument *document);

protected:

    /// Call in the constructor of the subclass: setComponentData(AppFactory::componentData());
    virtual void setComponentData(const KComponentData &componentData);

private:

    Q_DISABLE_COPY(KoPart)

    class Private;
    Private *const d;

};

class MockPart : public KoPart
{
public:
    MockPart()
    : KoPart( 0 )
    {}
    KoView *createViewInstance(KoDocument */*document*/, QWidget * /* parent */ ) { return 0; }
    virtual KoMainWindow *createMainWindow() { return 0; }
protected:
    virtual QGraphicsItem *createCanvasItem(KoDocument */*document*/) { return 0; }
};

#endif

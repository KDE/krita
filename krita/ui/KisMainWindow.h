/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000-2004 David Faure <faure@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef KIS_MAIN_WINDOW_H
#define KIS_MAIN_WINDOW_H

#include "kritaui_export.h"

#include <QPointer>
#include <QPrinter>

#include <kxmlguiwindow.h>
#include <kurl.h>
#include <KoCanvasObserverBase.h>
#include <KoCanvasSupervisor.h>

#include "KisView.h"

class QCloseEvent;
class QMoveEvent;

struct KoPageLayout;
class KoCanvasResourceManager;

class KisDocument;
class KisView;
class KisPrintJob;
class KoDockFactoryBase;
class QDockWidget;
class KisView;
class KisViewManager;


// Calligra class but not in main module
class KisDockerManager;

/**
 * @brief Main window for Krita
 *
 * This class is used to represent a main window
 * of a Krita. Each main window contains
 * a menubar and some toolbars.
 */
class KRITAUI_EXPORT KisMainWindow : public KXmlGuiWindow, public KoCanvasSupervisor
{
    Q_OBJECT

public:

    /**
     *  Constructor.
     *
     *  Initializes a Calligra main window (with its basic GUI etc.).
     */
    explicit KisMainWindow();

    /**
     *  Destructor.
     */
    virtual ~KisMainWindow();

    /**
     * Update caption from document info - call when document info
     * (title in the about page) changes.
     */
    void updateCaption();


    // If noCleanup is set, KisMainWindow will not delete the root document
    // or part manager on destruction.
    void setNoCleanup(bool noCleanup);

    /**
     * @brief showView shows the given view. Override this if you want to show
     * the view in a different way than by making it the central widget, for instance
     * as an QMdiSubWindow
     */
    virtual void showView(KisView *view);

    /**
     * @returns the currently active view
     */
    KisView *activeView() const;

    /**
     * Sets the maximum number of recent documents entries.
     */
    void setMaxRecentItems(uint _number);

    /**
     * The document opened a URL -> store into recent documents list.
     */
    void addRecentURL(const KUrl& url);

    /**
     * Load the desired document and show it.
     * @param url the URL to open
     *
     * @return TRUE on success.
     */
    bool openDocument(const KUrl & url);

    void setReadWrite(bool readwrite);


    /// Return the list of dock widgets belonging to this main window.
    QList<QDockWidget*> dockWidgets() const;

    QList<KoCanvasObserverBase*> canvasObservers() const;

    /**
     * @return the KisDockerManager which is assigned
     * WARNING: this could be 0, if no docker have been assigned yet. In that case create one
      * and assign it.
     * @ref setDockerManager to assign it.
     */
    KisDockerManager * dockerManager() const;

    KoCanvasResourceManager *resourceManager() const;

    int viewCount() const;

    /**
     * A wrapper around restoreState
     * @param state the saved state
     * @return TRUE on success
     */
    bool restoreWorkspace(const QByteArray &state);

    KisViewManager *viewManager() const;

    void addViewAndNotifyLoadingCompleted(KisDocument *document);

Q_SIGNALS:

    /**
     * This signal is emitted if the document has been saved successfully.
     */
    void documentSaved();

    /// This signal is emitted when this windows has finished loading of a
    /// document. The document may be opened in another window in the end.
    /// In this case, the signal means there is no link between the window
    /// and the document anymore.
    void loadCompleted();

    /// This signal is emitted right after the docker states have been succefully restored from config
    void restoringDone();

    /// This signal is emitted when the color theme changes
    void themeChanged();

    /// This signal is emitted when the shortcut key configuration has changed
    void keyBindingsChanged();

    void guiLoadingFinished();

public Q_SLOTS:

    /**
     * Slot for eMailing the document using KMail
     *
     * This is a very simple extension that will allow any document
     * that is currently being edited to be emailed using KMail.
     */
    void slotEmailFile();

    /**
     *  Slot for opening a new document.
     *
     *  If the current document is empty, the new document replaces it.
     *  If not, a new mainwindow will be opened for showing the document.
     */
    void slotFileNew();

    /**
     *  Slot for opening a saved file.
     *
     *  If the current document is empty, the opened document replaces it.
     *  If not a new mainwindow will be opened for showing the opened file.
     */
    void slotFileOpen();

    /**
     *  Slot for opening a file among the recently opened files.
     *
     *  If the current document is empty, the opened document replaces it.
     *  If not a new mainwindow will be opened for showing the opened file.
     */
    void slotFileOpenRecent(const QUrl &);

    /**
     * @brief slotPreferences open the preferences dialog
     */
    void slotPreferences();

    /**
     *  Saves the current document with the current name.
     */
    void slotFileSave();

    KisPrintJob* exportToPdf(const QString &pdfFileName = QString());

    void slotProgress(int value);

    /**
     * Saves the document, asking for a filename if necessary.
     *
     * @param saveas if set to TRUE the user is always prompted for a filename
     *
     * @param silent if set to TRUE rootDocument()->setTitleModified will not be called.
     *
     * @param specialOutputFlag set to enums defined in KisDocument if save to special output format
     *
     * @return TRUE on success, false on error or cancel
     *         (don't display anything in this case, the error dialog box is also implemented here
     *         but restore the original URL in slotFileSaveAs)
     */
    bool saveDocument(KisDocument *document, bool saveas = false, bool silent = false, int specialOutputFlag = 0);

    /**
     * Update the option widgets to the argument ones, removing the currently set widgets.
     */
    void newOptionWidgets(const QList<QPointer<QWidget> > & optionWidgetList);


private Q_SLOTS:
    /**
     * Save the list of recent files.
     */
    void saveRecentFiles();

    void slotLoadCompleted();
    void slotLoadCanceled(const QString &);
    void slotSaveCompleted();
    void slotSaveCanceled(const QString &);
    void forceDockTabFonts();


    /**
     * @internal
     */
    void slotDocumentTitleModified(const QString &caption, bool mod);

    /**
     *  Prints the actual document.
     */
    void slotFilePrint();

    /**
     *  Saves the current document with a new name.
     */
    void slotFileSaveAs();

    void slotFilePrintPreview();

    KisPrintJob* exportToPdf(KoPageLayout pageLayout, QString pdfFileName = QString());

    void exportAnimation();

    /**
     * Show a dialog with author and document information.
     */
    void slotDocumentInfo();

    /**
     * Closes all open documents.
     */
    bool slotFileCloseAll();

    /**
     * @brief showAboutApplication show the about box
     */
    virtual void showAboutApplication();

    /**
     *  Closes the mainwindow.
     */
    void slotFileQuit();

    /**
     *  Configure key bindings.
     */
    void slotConfigureKeys();

    /**
     *  Configure toolbars.
     */
    void slotConfigureToolbars();

    /**
     *  Post toolbar config.
     * (Plug action lists back in, etc.)
     */
    void slotNewToolbarConfig();

    /**
     *  Shows or hides a toolbar
     */
    void slotToolbarToggled(bool toggle);

    /**
     * Toggle full screen on/off.
     */
    void viewFullscreen(bool fullScreen);

    /**
     * Toggle docker titlebars on/off.
     */
    void showDockerTitleBars(bool show);

    /**
     * Reload file
     */
    void slotReloadFile();

    /**
     * File --> Import
     *
     * This will call slotFileOpen().  To differentiate this from an ordinary
     * call to slotFileOpen() call @ref isImporting().
     */
    void slotImportFile();

    /**
     * File --> Export
     *
     * This will call slotFileSaveAs().  To differentiate this from an ordinary
     * call to slotFileSaveAs() call @ref isExporting().
     */
    void slotExportFile();

    /**
     * Hide the dockers
     */
    void toggleDockersVisibility(bool visible);

    /**
     * Handle theme changes from theme manager
     */
    void slotThemeChanged();

    void undo();
    void redo();
    void subWindowActivated();
    void updateWindowMenu();
    void setActiveSubWindow(QWidget *window);
    void configChanged();
    void newView(QObject *document);
    void newWindow();
    void closeCurrentWindow();
    void checkSanity();
    /// Quits Krita with error message from m_errorMessage.
    void showErrorAndDie();

protected:

    void closeEvent(QCloseEvent * e);
    void resizeEvent(QResizeEvent * e);

    /// Set the active view, this will update the undo/redo actions
    virtual void setActiveView(KisView *view);

    // QWidget overrides
    virtual void dragEnterEvent(QDragEnterEvent * event);
    virtual void dropEvent(QDropEvent * event);

    void setToolbarList(QList<QAction*> toolbarList);

private:
    /**
     * Add a the given view to the list of views of this mainwindow.
     * This is a private implementation. For public usage please use
     * newView() and addViewAndNotifyLoadingCompleted().
     */
    void addView(KisView *view);


    friend class KisApplication;


    /**
     * Returns the dockwidget specified by the @p factory. If the dock widget doesn't exist yet it's created.
     * Add a "view_palette_action_menu" action to your view menu if you want to use closable dock widgets.
     * @param factory the factory used to create the dock widget if needed
     * @return the dock widget specified by @p factory (may be 0)
     */
    QDockWidget* createDockWidget(KoDockFactoryBase* factory);

    bool openDocumentInternal(const KUrl &url, KisDocument *newdoc = 0);

    /**
     * Returns whether or not the current slotFileSave[As]() or saveDocument()
     * call is actually an export operation (like File --> Export).
     *
     * If this is true, you must call KisDocument::export() instead of
     * KisDocument::save() or KisDocument::saveAs(), in any reimplementation of
     * saveDocument().
     */
    bool isExporting() const;

    /**
     * Returns whether or not the current slotFileOpen() or openDocument()
     * call is actually an import operation (like File --> Import).
     *
     * If this is true, you must call KisDocument::import() instead of
     * KisDocument::openUrl(), in any reimplementation of openDocument() or
     * openDocumentInternal().
     */
    bool isImporting() const;

    /**
     * Reloads the recent documents list.
     */
    void reloadRecentFileList();

    /**
     * Updates the window caption based on the document info and path.
     */
    void updateCaption(const QString & caption, bool mod);
    void updateReloadFileAction(KisDocument *doc);

    void saveWindowSettings();

    QPointer<KisView>activeKisView();

    void applyDefaultSettings(QPrinter &printer);

    bool exportConfirmation(const QByteArray &outputFormat);

    void createActions();

    void applyToolBarLayout();

protected:

    void moveEvent(QMoveEvent *e);

private slots:
    void initializeGeometry();
    void showManual();

private:
    class Private;
    Private * const d;

    QString m_errorMessage;
    bool m_dieOnError;

};

#endif

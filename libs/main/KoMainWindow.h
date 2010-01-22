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

#ifndef KOMAINWINDOW_H
#define KOMAINWINDOW_H

#include "komain_export.h"

#include <kparts/mainwindow.h>

#include <KoCanvasObserverBase.h>
#include <KoCanvasSupervisor.h>

class KoMainWindowPrivate;
class KoDocument;
class KoView;
class KoPrintJob;
class KoDockFactoryBase;
class KRecentFilesAction;
class QDockWidget;
class QLabel;

// KOffice class but not in main module
class KoDockerManager;

namespace KParts
{
class PartManager;
}

/**
 * @brief Main window for a KOffice application
 *
 * This class is used to represent a main window
 * of a KOffice component. Each main window contains
 * a menubar and some toolbars.
 *
 * @note This class does NOT need to be subclassed in your application.
 */
class KOMAIN_EXPORT KoMainWindow : public KParts::MainWindow, public KoCanvasSupervisor
{
    Q_OBJECT
public:

    /**
     *  Constructor.
     *
     *  Initializes a KOffice main window (with its basic GUI etc.).
     */
    explicit KoMainWindow(const KComponentData &instance);

    /**
     *  Destructor.
     */
    virtual ~KoMainWindow();

    /**
     * Called when a document is assigned to this mainwindow.
     * This creates a view for this document, makes it the active part, etc.
     */
    virtual void setRootDocument(KoDocument *doc);

    /**
     * This is used to handle the document used at start up before it actually
     * added as root document.
     */
    void setDocToOpen(KoDocument *doc);

    /**
     * Update caption from document info - call when document info
     * (title in the about page) changes.
     */
    virtual void updateCaption();

    /**
     *  Retrieves the document that is displayed in the mainwindow.
     */
    virtual KoDocument* rootDocument() const;

    virtual KoView *rootView() const;

    virtual KParts::PartManager *partManager();

    /**
     * The application should call this to show or hide a toolbar.
     * It also takes care of the corresponding action in the settings menu.
     */
    void showToolbar(const char * tbName, bool shown);

    /**
     * @return TRUE if the toolbar @p tbName is visible
     */
    bool toolbarIsVisible(const char *tbName);

    /**
     * Get hold of the label in the statusbar, to write messages to it.
     * You can also insert other items in the status bar by using QStatusBar::addWidget.
     */
    QLabel * statusBarLabel();

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
    virtual bool openDocument(const KUrl & url);

    /**
     * Load the URL into this document (and make it root doc after loading)
     *
     * Special method for KoApplication::start, don't use.
     */
    bool openDocument(KoDocument *newdoc, const KUrl & url);

//     virtual DCOPObject * dcopObject();

    /**
     * Reloads the recent documents list.
     */
    void reloadRecentFileList();

    /**
     * Updates the window caption based on the document info and path.
     */
    virtual void updateCaption(const QString & caption, bool mod);
    void updateReloadFileAction(KoDocument *doc);
    void updateVersionsFileAction(KoDocument *doc);

    void setReadWrite(bool readwrite);

    /**
     * Returns the dockwidget specified by the @p factory. If the dock widget doesn't exist yet it's created.
     * Add a "view_palette_action_menu" action to your view menu if you want to use closable dock widgets.
     * @param factory the factory used to create the dock widget if needed
     * @return the dock widget specified by @p factory (may be 0)
     */
    QDockWidget* createDockWidget(KoDockFactoryBase* factory);

    /// Return the list of dock widgets belonging to this main window.
    QList<QDockWidget*> dockWidgets();

    QList<KoCanvasObserverBase*> canvasObservers();

    /**
     * @return the KoDockerManager which is assigned
     * WARNING: this could be 0, if no docker have been assigned yet. In that case create one
      * and assign it.
     * Note This should only be called by KoView
     * @ref setDockerManager to assign it.
     */
    KoDockerManager * dockerManager() const;

    /**
     * use this to assign a KoDockerManager.
     * Note this should only be called by KoView
     * @ref dockerManager to retrieve it.
     */
    void setDockerManager(KoDockerManager *);


signals:
    /**
     * This signal is emitted if the document has been saved successfully.
     */
    void documentSaved();
    /// This signals is emmitted before the save dialog is shown
    void saveDialogShown();

    /// This signal is emitted right after the docker states have been succefully restored from config
    void restoringDone();

public slots:

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
     *  If not, a new shell will be opened for showing the document.
     */
    virtual void slotFileNew();

    /**
     *  Slot for opening a saved file.
     *
     *  If the current document is empty, the opened document replaces it.
     *  If not a new shell will be opened for showing the opened file.
     */
    virtual void slotFileOpen();

    /**
     *  Slot for opening a file among the recently opened files.
     *
     *  If the current document is empty, the opened document replaces it.
     *  If not a new shell will be opened for showing the opened file.
     */
    virtual void slotFileOpenRecent(const KUrl &);

    /**
     *  Saves the current document with the current name.
     */
    virtual void slotFileSave();

    /**
     *  Saves the current document with a new name.
     */
    virtual void slotFileSaveAs();

    /**
     *  Prints the actual document.
     */
    virtual void slotFilePrint();
    virtual void slotFilePrintPreview();

    KoPrintJob* exportToPdf(QString fileName = QString());

    /**
     * Show a dialog with author and document information.
     */
    virtual void slotDocumentInfo();

    /**
     *  Closes the document.
     */
    virtual void slotFileClose();

    /**
     *  Closes the shell.
     */
    virtual void slotFileQuit();

    /**
     *  Configure key bindings.
     */
    virtual void slotConfigureKeys();

    /**
     *  Configure toolbars.
     */
    virtual void slotConfigureToolbars();

    /**
     *  Post toolbar config.
     * (Plug action lists back in, etc.)
     */
    virtual void slotNewToolbarConfig();

    /**
     *  Shows or hides a toolbar
     */
    virtual void slotToolbarToggled(bool toggle);

    /**
     * View splitting stuff
     */
    virtual void slotSplitView();
    virtual void slotRemoveView();
    virtual void slotSetOrientation();

    /**
     * Close all views
     */
    virtual void slotCloseAllViews();

    /**
     * Toggle full screen on/off.
     */
    virtual void viewFullscreen(bool fullScreen);

    /**
     * Reload file
     */
    void slotReloadFile();

    /**
     * This will call a dialogbox to add version to list of files
     */
    void slotVersionsFile();

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

    void slotProgress(int value);


private:

    /**
     * This setting indicates who is calling chooseNewDocument.
     * Usually the app will want to
     * - show the template dialog with 'everything' if InitDocAppStarting, InitDocFileClose or InitDocEmbedded
     * - show the template dialog with 'templates only' if InitDocFileNew
     * - create an empty document with default settings if InitDocEmpty
     */
    enum InitDocFlags { /*InitDocAppStarting, */ InitDocFileNew, InitDocFileClose /*, InitDocEmbedded, InitDocEmpty*/ };

    /// Helper method for slotFileNew and slotFileClose
    void chooseNewDocument(InitDocFlags initDocFlags);

    /**
     * Create a new empty document.
     */
    virtual KoDocument* createDoc() const;

    /**
     * Saves the document, asking for a filename if necessary.
     *
     * @param saveas if set to TRUE the user is always prompted for a filename
     *
     * @param silent if set to TRUE rootDocument()->setTitleModified will not be called.
     *
     * @return TRUE on success, false on error or cancel
     *         (don't display anything in this case, the error dialog box is also implemented here
     *         but restore the original URL in slotFileSaveAs)
     */
    virtual bool saveDocument(bool saveas = false, bool silent = false);

    virtual void closeEvent(QCloseEvent * e);
    virtual void resizeEvent(QResizeEvent * e);

    /**
     * Ask user about saving changes to the document upon exit.
     */
    virtual bool queryClose();

    virtual bool openDocumentInternal(const KUrl &url, KoDocument *newdoc = 0);

    /**
     * Returns whether or not the current slotFileSave[As]() or saveDocument()
     * call is actually an export operation (like File --> Export).
     *
     * If this is true, you must call KoDocument::export() instead of
     * KoDocument::save() or KoDocument::saveAs(), in any reimplementation of
     * saveDocument().
     */
    bool isExporting() const;

    /**
     * Returns whether or not the current slotFileOpen() or openDocument()
     * call is actually an import operation (like File --> Import).
     *
     * If this is true, you must call KoDocument::import() instead of
     * KoDocument::openUrl(), in any reimplementation of openDocument() or
     * openDocumentInternal().
     */
    bool isImporting() const;

    /**
     * Save the list of recent files.
     */
    void saveRecentFiles();

    KRecentFilesAction *recentAction() const;

protected slots:

    virtual void slotActivePartChanged(KParts::Part *newPart);

private slots:

    void slotLoadCompleted();
    void slotLoadCanceled(const QString &);
    void slotSaveCompleted();
    void slotSaveCanceled(const QString &);
    void forceDockTabFonts();

private:
    /**
     * Asks the user if they really want to save the document.
     * Called only if outputFormat != nativeFormat.
     *
     * @return true if the document should be saved
     */
    bool exportConfirmation(const QByteArray &outputFormat);

    void saveWindowSettings();

    // retrieve the current KoView
    KoView* currentView() const;

private:

    KoMainWindowPrivate * const d;
};

#endif

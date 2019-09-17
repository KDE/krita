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
#include <QUuid>
#include <QUrl>

#include <xmlgui/kxmlguiwindow.h>
#include <KoCanvasObserverBase.h>
#include <KoCanvasSupervisor.h>
#include "KisView.h"

class QCloseEvent;
class QMoveEvent;

struct KoPageLayout;
class KoCanvasResourceProvider;

class KisDocument;
class KisPrintJob;
class KoDockFactoryBase;
class QDockWidget;
class KisView;
class KisViewManager;
class KoCanvasController;
class KisWorkspaceResource;

/**
 * @brief Main window for Krita
 *
 * This class is used to represent a main window within a Krita session. Each
 * main window contains a menubar and some toolbars, and potentially several
 * views of several canvases.
 *
 */
class KRITAUI_EXPORT KisMainWindow : public KXmlGuiWindow, public KoCanvasSupervisor
{
    Q_OBJECT

public:
    enum OpenFlag {
        None = 0,
        Import = 0x1,
        BatchMode = 0x2,
        RecoveryFile = 0x4
    };
    Q_DECLARE_FLAGS(OpenFlags, OpenFlag)

public:

    /**
     *  Constructor.
     *
     *  Initializes a Calligra main window (with its basic GUI etc.).
     */
    explicit KisMainWindow(QUuid id = QUuid());

    /**
     *  Destructor.
     */
    ~KisMainWindow() override;

    QUuid id() const;

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
    void addRecentURL(const QUrl &url);

    /**
     * get list of URL strings for recent files
     */
    QList<QUrl> recentFilesUrls();

    /**
     * clears the list of the recent files
     */
    void clearRecentFiles();

    /**
     * removes the given url from the list of recent files
     */
    void removeRecentUrl(const QUrl &url);


    /**
     * Load the desired document and show it.
     * @param url the URL to open
     *
     * @return TRUE on success.
     */
    bool openDocument(const QUrl &url, OpenFlags flags);

    /**
     * Activate a view containing the document in this window, creating one if needed.
     */
    void showDocument(KisDocument *document);


    /**
     * Toggles between showing the welcome screen and the MDI area
     *
     *  hack: There seems to be a bug that prevents events happening to the MDI area if it
     *  isn't actively displayed (set in the widgetStack). This can cause things like the title bar
     *  not to update correctly Before doing any actions related to opening or creating documents,
     *  make sure to switch this first to make sure everything can communicate to the MDI area correctly
     */
    void showWelcomeScreen(bool show);

    /**
     * Saves the document, asking for a filename if necessary.
     *
     * @param saveas if set to TRUE the user is always prompted for a filename
     * @param silent if set to TRUE rootDocument()->setTitleModified will not be called.
     *
     * @return TRUE on success, false on error or cancel
     *         (don't display anything in this case, the error dialog box is also implemented here
     *         but restore the original URL in slotFileSaveAs)
     */
    bool saveDocument(KisDocument *document, bool saveas, bool isExporting);

    void setReadWrite(bool readwrite);

    /// Return the list of dock widgets belonging to this main window.
    QList<QDockWidget*> dockWidgets() const;

    QDockWidget* dockWidget(const QString &id);

    QList<KoCanvasObserverBase*> canvasObservers() const override;

    KoCanvasResourceProvider *resourceManager() const;

    int viewCount() const;

    void saveWindowState(bool restoreNormalState =false);

    const KConfigGroup &windowStateConfig() const;

    /**
     * A wrapper around restoreState
     * @param state the saved state
     * @return TRUE on success
     */
    bool restoreWorkspace(KisWorkspaceResource *workspace);
    bool restoreWorkspaceState(const QByteArray &state);

    static void swapWorkspaces(KisMainWindow *a, KisMainWindow *b);

    KisViewManager *viewManager() const;

    KisView *addViewAndNotifyLoadingCompleted(KisDocument *document);

    QStringList showOpenFileDialog(bool isImporting);

    /**
     * The top-level window used for a detached canvas.
     */
    QWidget *canvasWindow() const;
    bool canvasDetached() const;

    /**
     * Shows if the main window is saving anything right now. If the
     * user presses Ctrl+W too fast, then the document can be close
     * before the saving is completed. I'm not sure if it is fixable
     * in any way without avoiding using porcessEvents()
     * everywhere (DK)
     *
     * Don't use it unless you have no option.
     */
    bool hackIsSaving() const;

    /// Copy the given file into the bundle directory.
    bool installBundle(const QString &fileName) const;

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

    /// emitted when the window is migrated among different screens
    void screenChanged();

public Q_SLOTS:

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
    void slotFileOpen(bool isImporting = false);

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
     * Update caption from document info - call when document info
     * (title in the about page) changes.
     */
    void updateCaption();

    /**
     *  Saves the current document with the current name.
     */
    void slotFileSave();


    void slotShowSessionManager();

    // XXX: disabled
    KisPrintJob* exportToPdf(QString pdfFileName = QString());

    /**
     * Update the option widgets to the argument ones, removing the currently set widgets.
     */
    void newOptionWidgets(KoCanvasController *controller, const QList<QPointer<QWidget> > & optionWidgetList);

    KisView *newView(QObject *document);

    void notifyChildViewDestroyed(KisView *view);

    /// Set the active view, this will update the undo/redo actions
    void setActiveView(KisView *view);

    void subWindowActivated();

    void windowFocused();

    /**
     * Reloads the recent documents list.
     */
    void reloadRecentFileList();

    /**
     * Detach canvas onto a separate window, or restore it back to to main window.
     */
    void setCanvasDetached(bool detached);

    void slotFileSelected(QString path);
    void slotEmptyFilePath();
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
    void slotDocumentTitleModified();

    /**
     *  Prints the actual document.
     */
    void slotFilePrint();

    /**
     *  Saves the current document with a new name.
     */
    void slotFileSaveAs();

    void slotFilePrintPreview();

    void importAnimation();

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
     * Reload file
     */
    void slotReloadFile();


    /**
     * File --> Import
     *
     * This will call slotFileOpen().
     */
    void slotImportFile();

    /**
     * File --> Export
     *
     * This will call slotFileSaveAs().
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
    void updateWindowMenu();
    void setActiveSubWindow(QWidget *window);
    void configChanged();

    void newWindow();
    void closeCurrentWindow();
    void checkSanity();

    /// Quits Krita with error message from m_errorMessage.
    void showErrorAndDie();

    void initializeGeometry();
    void showManual();
    void switchTab(int index);

    void windowScreenChanged(QScreen *screen);

    void slotXmlGuiMakingChanges(bool finished);

protected:

    void closeEvent(QCloseEvent * e) override;
    void resizeEvent(QResizeEvent * e) override;

    // QWidget overrides

private:

    friend class KisWelcomePageWidget;
    void dragMove(QDragMoveEvent *event);
    void dragLeave();

private:

    /**
     * Add a the given view to the list of views of this mainwindow.
     * This is a private implementation. For public usage please use
     * newView() and addViewAndNotifyLoadingCompleted().
     */
    void addView(KisView *view);

    friend class KisPart;


    /**
     * Returns the dockwidget specified by the @p factory. If the dock widget doesn't exist yet it's created.
     * Add a "view_palette_action_menu" action to your view menu if you want to use closable dock widgets.
     * @param factory the factory used to create the dock widget if needed
     * @return the dock widget specified by @p factory (may be 0)
     */
    QDockWidget* createDockWidget(KoDockFactoryBase* factory);

    bool openDocumentInternal(const QUrl &url, KisMainWindow::OpenFlags flags = 0);


    /**
     * Updates the window caption based on the document info and path.
     */
    void updateCaption(const QString & caption, bool modified);
    void updateReloadFileAction(KisDocument *doc);

    void saveWindowSettings();

    QPointer<KisView> activeKisView();

    void applyDefaultSettings(QPrinter &printer);

    void createActions();

    void applyToolBarLayout();

    QByteArray borrowWorkspace(KisMainWindow *borrower);


private:

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

    class Private;
    Private * const d;

    QString m_errorMessage;
    bool m_dieOnError;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KisMainWindow::OpenFlags)

#endif

/* This file is part of the KDE project
   SPDX-FileCopyrightText: 1998, 1999 Torben Weis <weis@kde.org>
   SPDX-FileCopyrightText: 2000-2004 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
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
#include <kis_workspace_resource.h>
#include <KoResource.h>

class QCloseEvent;
class QMoveEvent;

class KoCanvasResourceProvider;

class KisDocument;
class KoDockFactoryBase;
class QDockWidget;
class KisView;
class KisViewManager;
class KoCanvasController;


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
     *  Initializes a Krita main window (with its basic GUI etc.).
     */
    explicit KisMainWindow(QUuid id = QUuid());

    /**
     *  Destructor.
     */
    ~KisMainWindow() override;

    QMenu *createPopupMenu() override;


    QUuid id() const;

    /**
     * @brief showView shows the given view, in @p subWindow if not
     * null, in a new tab otherwise.
     */
    virtual void showView(KisView *view, QMdiSubWindow *subWindow = 0);

    /**
     * @returns the currently active view
     */
    KisView *activeView() const;

    /**
     * Load the desired document and show it.
     * @param url the URL to open
     *
     * @return TRUE on success.
     */
    bool openDocument(const QString &path, OpenFlags flags);

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
    bool saveDocument(KisDocument *document, bool saveas, bool isExporting, bool isAdvancedExporting = false);

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
    bool restoreWorkspace(KoResourceSP res);
    bool restoreWorkspaceState(const QByteArray &state);

    static void swapWorkspaces(KisMainWindow *a, KisMainWindow *b);

    KisViewManager *viewManager() const;

    KisView *addViewAndNotifyLoadingCompleted(KisDocument *document,
                                              QMdiSubWindow *subWindow = 0);

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
     * in any way without avoiding using processEvents()
     * everywhere (DK)
     *
     * Don't use it unless you have no option.
     */
    bool hackIsSaving() const;

    /// Copy the given file into the bundle directory.
    bool installBundle(const QString &fileName) const;

    /**
     * @brief layoutThumbnail
     * @return image for the workspaces.
     */
    QImage layoutThumbnail();

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

    /// This signal is emitted right after the docker states have been successfully restored from config
    void restoringDone();

    /// This signal is emitted when the color theme changes
    void themeChanged();

    /// This signal is emitted when the shortcut key configuration has changed
    void keyBindingsChanged();

    void guiLoadingFinished();

    /// emitted when the current view has changed
    void activeViewChanged();

#ifdef Q_OS_ANDROID
    void sigFullscreenOnShow(bool fullScreen);
#endif

public Q_SLOTS:


    /**
     * clears the list of the recent files
     */
    void clearRecentFiles();

    /**
     * remove one file from the list of the recent files
     */
    void removeRecentFile(QString url);


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
     *  Saves the current document with the current name.
     */
    void slotFileSave();


    void slotShowSessionManager();

    /**
     * Update the option widgets to the argument ones, removing the currently set widgets.
     */
    void newOptionWidgets(KoCanvasController *controller, const QList<QPointer<QWidget> > & optionWidgetList);

    KisView *newView(QObject *document, QMdiSubWindow *subWindow = 0);

    void notifyChildViewDestroyed(KisView *view);

    /// Set the active view, this will update the undo/redo actions
    void setActiveView(KisView *view);
    void unsetActiveView();

    void subWindowActivated();

    void windowFocused();

    /**
     * Detach canvas onto a separate window, or restore it back to to main window.
     */
    void setCanvasDetached(bool detached);

    /**
     * Toggle full screen on/off.
     */
    void viewFullscreen(bool fullScreen);

    /**
     * @brief checkActiveStorages checks whether there is at least one bundle available and
     * at least one paintop preset.
     */
    bool checkActiveBundlesAvailable();
    bool checkPaintOpAvailable();


private Q_SLOTS:
    void slotLoadCompleted();
    void slotLoadCanceled(const QString &);
    void slotSaveCompleted();
    void slotSaveCanceled(const QString &);
    void forceDockTabFonts();

    void slotUpdateWidgetStyle();

    void slotUpdateSaveActionTitle(const QString &documentPath);
    void slotUpdateReadWriteMode(bool readWrite);

    /**
     *  Saves the current document with a new name.
     */
    void slotFileSaveAs();

    void importAnimation();
    
    void importVideoAnimation();

    void renderAnimation();

    void renderAnimationAgain();

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
     * Reset User Configurations.
     */
    void slotResetConfigurations();

    /**
     *  Shows or hides a toolbar
     */
    void slotToolbarToggled(bool toggle);

    /**
     * Toggle docker titlebars on/off.
     */
    void showDockerTitleBars(bool show);

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

    void slotExportAdvance();

    /**
     * Hide the dockers
     */
    void toggleDockersVisibility(bool visible, bool onWelcomePage = false);

    /**
     * Handle theme changes from theme manager
     */
    void updateTheme();
    void slotThemeChanged();

    void undo();
    void redo();
    void updateWindowMenu();
    void updateSubwindowFlags();
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

    void slotXmlGuiMakingChanges(bool finished);

    void orientationChanged();

    void restoreWorkspace();

    void openCommandBar();

    void slotStoragesWarning(const QString &location = QString());

protected:

    void closeEvent(QCloseEvent * e) override;
    void resizeEvent(QResizeEvent * e) override;
    void showEvent(QShowEvent *event) override;

    // QWidget overrides
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;

    bool windowsLayoutSavingAllowed() const override;

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
    void addView(KisView *view, QMdiSubWindow *subWindow = 0);

    friend class KisPart;


    /**
     * Returns the dockwidget specified by the @p factory. If the dock widget doesn't exist yet it's created.
     * Add a "view_palette_action_menu" action to your view menu if you want to use closable dock widgets.
     * @param factory the factory used to create the dock widget if needed
     * @return the dock widget specified by @p factory (may be 0)
     */
    QDockWidget* createDockWidget(KoDockFactoryBase* factory);

    bool openDocumentInternal(const QString &path, KisMainWindow::OpenFlags f = KisMainWindow::OpenFlags());

    void saveWindowSettings();

    QPointer<KisView> activeKisView();

    void createActions();

    void applyToolBarLayout();

    QByteArray borrowWorkspace(KisMainWindow *borrower);

    void customizeTabBar();

    void setMainWindowLayoutForCurrentMainWidget(int widgetIndex, bool widgetIndexChanged);
    void adjustLayoutForWelcomePage();

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

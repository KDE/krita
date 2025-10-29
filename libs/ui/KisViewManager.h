/*
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GUI_CLIENT_H
#define KIS_GUI_CLIENT_H

#include <QDockWidget>
#include <QQueue>
#include <QPointer>

#include <KoToolManager.h>

#include <kritaui_export.h>
#include <kis_types.h>

#include "kis_floating_message.h"

class QPoint;
class KisView;

class KisCanvas2;
class KisCanvasResourceProvider;
class KisDocument;
class KisFilterManager;
class KisGridManager;
class KisGuidesManager;
class KisImageManager;
class KisNodeManager;
class KisDecorationsManager;
class KisPaintopBox;
class KisSelectionManager;
class KisStatusBar;
class KisUndoAdapter;
class KisZoomManager;
class KisPaintopBox;
class KisActionManager;
class KisInputManager;
class KoUpdater;
class KoProgressUpdater;
class KisMainWindow;
class KoZoomController;
class KoCanvasResourceProvider;
class KisIdleTasksManager;

/**
 * KisViewManager manages the collection of views shown in a single mainwindow.
 */
class KRITAUI_EXPORT KisViewManager : public QObject
{

    Q_OBJECT

public:
    /**
     * Construct a new view on the krita document.
     * @param parent   a parent widget we show ourselves in.
     * @param actionCollection an action collection.
     */
    KisViewManager(QWidget *parent, KisKActionCollection *actionCollection);
    ~KisViewManager() override;

    /**
     * Retrieves the entire action collection.
     */
    virtual KisKActionCollection* actionCollection() const;

public:  // Krita specific interfaces

    void setCurrentView(KisView *view);

    /// Return the image this view is displaying
    KisImageWSP image() const;

    KoZoomController *zoomController() const;

    /// The resource provider contains all per-view settings, such as
    /// current color, current paint op etc.
    KisCanvasResourceProvider *canvasResourceProvider();

    /// Return the canvas base class
    KisCanvas2 *canvasBase() const;

    /// Return the actual widget that is displaying the current image
    QWidget* canvas() const;

    /// Return the wrapper class around the statusbar
    KisStatusBar *statusBar() const;

    KisPaintopBox* paintOpBox() const;

    /// create a new progress updater
    QPointer<KoUpdater> createUnthreadedUpdater(const QString &name);
    QPointer<KoUpdater> createThreadedUpdater(const QString &name);

    /// The selection manager handles everything action related to
    /// selections.
    KisSelectionManager *selectionManager();

    /// The node manager handles everything about nodes
    KisNodeManager *nodeManager() const;

    KisActionManager *actionManager() const;

    /**
     * Convenience method to get at the active node, which may be
     * a layer or a mask or a selection
     */
    KisNodeSP activeNode();

    /// Convenience method to get at the active layer
    KisLayerSP activeLayer();

    /// Convenience method to get at the active paint device
    KisPaintDeviceSP activeDevice();

    /// The filtermanager handles everything action-related to filters
    KisFilterManager *filterManager();

    /// The image manager handles everything action-related to the
    /// current image
    KisImageManager *imageManager();

    /// Filters events and sends them to canvas actions
    KisInputManager *inputManager() const;

    KisIdleTasksManager *idleTasksManager();

    /// Convenience method to get at the active selection (the
    /// selection of the current layer, or, if that does not exist,
    /// the global selection.
    KisSelectionSP selection();

    /// Checks if the current global or local selection is editable
    bool selectionEditable();

    /// The undo adapter is used to add commands to the undo stack
    KisUndoAdapter *undoAdapter();

    KisDocument *document() const;

    int viewCount() const;

    /**
     * @brief blockUntilOperationsFinished blocks the GUI of the application until execution
     *        of actions on \p image is finished
     * @param image the image which we should wait for
     * @return true if the image has finished execution of the actions, false if
     *         the user cancelled operation
     */
    bool blockUntilOperationsFinished(KisImageSP image);


    /**
     * @brief blockUntilOperationsFinished blocks the GUI of the application until execution
     *        of actions on \p image is finished. Does *not* provide a "Cancel" button. So the
     *        user is forced to wait.
     * @param image the image which we should wait for
     */
    void blockUntilOperationsFinishedForced(KisImageSP image);

public:

    KisGridManager * gridManager() const;
    KisGuidesManager * guidesManager() const;

    /// disable and enable toolbar controls. used for disabling them during painting.
    void enableControls();
    void disableControls();


    /// shows a floating message in the top right corner of the canvas
    void showFloatingMessage(const QString &message, const QIcon& icon, int timeout = 4500,
                             KisFloatingMessage::Priority priority = KisFloatingMessage::Medium,
                             int alignment = Qt::AlignCenter | Qt::TextWordWrap);

    /// @return the KisMainWindow this view is in, or 0
    KisMainWindow *mainWindow() const;

    /**
     * Gets the KisMainWindow as a QWidget, useful when you just need it to
     * be used as a parent to a dialog or window without needing to include
     * `KisMainWindow.h`.
     */
    QWidget *mainWindowAsQWidget() const;

    /// The QMainWindow associated with this view. This is most likely going to be shell(), but
    /// when running as Gemini or Sketch, this will be set to the applications' own QMainWindow.
    /// This can be checked by qobject_casting to KisMainWindow to check the difference.
    QMainWindow* qtMainWindow() const;

    /// The mainWindow function will return the shell() value, unless this function is called
    /// with a non-null value. To make it return shell() again, simply pass null to this function.
    void setQtMainWindow(QMainWindow* newMainWindow);

    static void initializeResourceManager(KoCanvasResourceProvider *resourceManager);

public Q_SLOTS:

    void switchCanvasOnly(bool toggled);
    void setShowFloatingMessage(bool show);
    void showHideScrollbars();

    /// Visit all managers to update gui elements, e.g. enable / disable actions.
    /// This is heavy-duty call, so it uses a compressor.
    void updateGUI();

    /// Update the style of all the icons
    void updateIcons();

    void slotViewAdded(KisView *view);
    void slotViewRemoved(KisView *view);
    
    void slotActivateTransformTool();

    // Change and update author
    void changeAuthorProfile(const QString &profileName);
    void slotUpdateAuthorProfileActions();

    void slotToggleFgBg();
    void slotResetFgBg();

    // Enable the last set brush outline, or disable it if already enabled
    void slotToggleBrushOutline();

    void updatePrintSizeAction(bool canvasMappingMode);
    
Q_SIGNALS:

    void floatingMessageRequested(const QString &message, const QString &iconName);
    /**
     * @brief viewChanged
     * sent out when the view has changed.
     */
    void viewChanged();

    void brushOutlineToggled();

private Q_SLOTS:

    void slotCreateTemplate();
    void slotCreateCopy();
    void slotDocumentSaved();
    void slotSaveIncremental();
    void slotSaveIncrementalBackup();
    void showStatusBar(bool toggled);
    void toggleTabletLogger();
    void openResourcesDirectory();
    void guiUpdateTimeout();
    void slotUpdatePixelGridAction();
    void slotSaveShowRulersState(bool value);
    void slotSaveRulersTrackMouseState(bool value);
    void slotResetRotation();
    void slotResetDisplay();
private:
    void createActions();
    void setupManagers();

    QString canonicalPath();

    /// The zoommanager handles everything action-related to zooming
    KisZoomManager * zoomManager();

private:
    class KisViewManagerPrivate;
    KisViewManagerPrivate * const d;
};

#endif

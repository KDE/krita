/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_GUI_CLIENT_H
#define KIS_GUI_CLIENT_H

#include <QDockWidget>
#include <QQueue>
#include <QPointer>

#include <KisMainWindow.h>
#include <KisView.h>
#include <KoProgressUpdater.h>
#include <KoToolManager.h>

#include <krita_export.h>
#include <kis_types.h>

#include "kis_floating_message.h"
#include "KisView.h"

class KisAction;
class QPoint;


class KisCanvas2;
class KisCanvasResourceProvider;
class KisDocument;
class KisFilterManager;
class KisGridManager;
class KisImage;
class KisImageManager;
class KisNodeManager;
class KisPaintingAssistantsManager;
class KisPaintopBox;
class KisPerspectiveGridManager;
class KisSelectionManager;
class KisStatusBar;
class KisUndoAdapter;
class KisZoomManager;
class KisPaintopBox;
class KisActionManager;
class KisInputManager;

/**
 * Krita view class
 *
 * Following the broad model-view-controller idea this class shows you one view on the document.
 * There can be multiple views of the same document each in with independent settings for viewMode and zoom etc.
 */
class KRITAUI_EXPORT KisViewManager : public QObject
{

    Q_OBJECT

public:
    /**
     * Construct a new view on the krita document.
     * @param document   the document we show.
     * @param parent   a parent widget we show ourselves in.
     */
    KisViewManager(QWidget *parent, KActionCollection *actionCollection);
    virtual ~KisViewManager();

    /**
     * Retrieves the entire action collection.
     */
    virtual KActionCollection* actionCollection() const;

public:  // Krita specific interfaces

    void setCurrentView(KisView *view);

    /// Return the image this view is displaying
    KisImageWSP image() const;

    KoZoomController *zoomController() const;

    /// The resource provider contains all per-view settings, such as
    /// current color, current paint op etc.
    KisCanvasResourceProvider * resourceProvider();

    /// Return the canvasbase class
    KisCanvas2 * canvasBase() const;

    /// Return the actual widget that is displaying the current image
    QWidget* canvas() const;

    /// Return the wrapper class around the statusbar
    KisStatusBar * statusBar() const;

    /**
      * This adds a widget to the statusbar for this view.
      * If you use this method instead of using statusBar() directly,
      * KisView will take care of removing the items when the view GUI is deactivated
      * and readding them when it is reactivated.
      * The parameters are the same as QStatusBar::addWidget().
      *
      * Note that you can't use KStatusBar methods (inserting text items by id).
      * But you can create a KStatusBarLabel with a dummy id instead, and use
      * it directly, to get the same look and feel.
      */
    void addStatusBarItem(QWidget * widget, int stretch = 0, bool permanent = false);

    /**
      * Remove a widget from the statusbar for this view.
      */
    void removeStatusBarItem(QWidget * widget);

    KisPaintopBox* paintOpBox() const;

    /// create a new progress updater
    KoProgressUpdater *createProgressUpdater(KoProgressUpdater::Mode mode = KoProgressUpdater::Threaded);

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

public:

    KisGridManager * gridManager() const;
    KisPerspectiveGridManager* perspectiveGridManager() const;
    KisPaintingAssistantsManager* paintingAssistantsManager() const;

    /// disable and enable toolbar controls. used for disabling them during painting.
    void enableControls();
    void disableControls();


    /// shows a floating message in the top right corner of the canvas
    void showFloatingMessage(const QString message, const QIcon& icon, int timeout = 4500,
                             KisFloatingMessage::Priority priority = KisFloatingMessage::Medium,
                             int alignment = Qt::AlignCenter | Qt::TextWordWrap);

    /// @return the KoMaindow this view is in, or 0
    KisMainWindow *mainWindow() const;

    /// The QMainWindow associated with this view. This is most likely going to be shell(), but
    /// when running as Gemini or Sketch, this will be set to the applications' own QMainWindow.
    /// This can be checked by qobject_casting to KisMainWindow to check the difference.
    QMainWindow* qtMainWindow() const;

    /// The mainWindow function will return the shell() value, unless this function is called
    /// with a non-null value. To make it return shell() again, simply pass null to this function.
    void setQtMainWindow(QMainWindow* newMainWindow);

public Q_SLOTS:

    void showJustTheCanvas(bool toggled);
    void setShowFloatingMessage(bool show);
    void showHideScrollbars();

    /// Go to all managers and enable or disable all actions and other
    /// gui elements
    void updateGUI();

    /// Update the style of all the icons
    void updateIcons();

    void slotViewAdded(KisView *view);
    void slotViewRemoved(KisView *view);

Q_SIGNALS:

    void floatingMessageRequested(QString message, QString iconName);

private Q_SLOTS:

    void slotBlacklistCleanup();
    void slotCreateTemplate();
    void slotDocumentSaved();
    void slotSaveIncremental();
    void slotSaveIncrementalBackup();
    void showStatusBar(bool toggled);
    void toggleTabletLogger();
    void openResourcesDirectory();
    void makeStatusBarVisible();
    void guiUpdateTimeout();

private:
    void createActions();
    void createManagers();

    /// The zoommanager handles everything action-related to zooming
    KisZoomManager * zoomManager();

private:
    class KisViewManagerPrivate;
    KisViewManagerPrivate * const d;
};

#endif

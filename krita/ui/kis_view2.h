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

#ifndef KIS_VIEW_2
#define KIS_VIEW_2


#include <QDockWidget>
#include <QQueue>
#include <KoView.h>
#include <KoProgressUpdater.h>
#include <KoToolManager.h>
#include <krita_export.h>
#include <kis_types.h>

class QDragEnterEvent;
class QDropEvent;
class QPoint;

class KisPaintOpPreset;

class KoCanvasController;

class KisCanvas2;
class KisDoc2;
class KisFilterManager;
class KisImage;
class KisLayerManager;
class KisCanvasResourceProvider;
class KisSelectionManager;
class KisStatusBar;
class KisUndoAdapter;
class KisZoomManager;
class KisImageManager;
class KisNodeManager;
class KisMaskManager;
class KisPerspectiveGridManager;
class KisPaintingAssistantsManager;
class KisGridManager;
class KoFavoriteResourceManager;
class KisPaintopBox;

/**
 * Krita view class
 *
 * Following the broad model-view-controller idea this class shows you one view on the document.
 * There can be multiple views of the same document each in with independent settings for viewMode and zoom etc.
 */
class KRITAUI_EXPORT KisView2 : public KoView
{

    Q_OBJECT

public:
    /**
     * Construct a new view on the krita document.
     * @param document   the document we show.
     * @param parent   a parent widget we show ourselves in.
     */
    KisView2(KisDoc2 * document, QWidget * parent);
    virtual ~KisView2();

public:

    // QWidget overrides
    virtual void dragEnterEvent(QDragEnterEvent * event);
    virtual void dropEvent(QDropEvent * event);

    // KoView implementation
    virtual void updateReadWrite(bool readwrite) {
        Q_UNUSED(readwrite);
    }

    // KoView implementation
    virtual KoZoomController *zoomController() const;

public:  // Krita specific interfaces

    /// Return the image this view is displaying
    KisImageWSP image();

    /// The resource provider contains all per-view settings, such as
    /// current color, current paint op etc.
    KisCanvasResourceProvider * resourceProvider();

    /// Return the canvasbase class
    KisCanvas2 * canvasBase() const;

    /// Return the actual widget that is displaying the current image
    QWidget* canvas() const;

    /// Return the wrapper class around the statusbar
    KisStatusBar * statusBar() const;

    /// create a new progress updater
    KoProgressUpdater* createProgressUpdater(KoProgressUpdater::Mode mode = KoProgressUpdater::Threaded);

    /// The selection manager handles everything action related to
    /// selections.
    KisSelectionManager * selectionManager();

    /// The CanvasController decorates the canvas with scrollbars
    /// and knows where to start painting on the canvas widget, i.e.,
    /// the document offset.
    KoCanvasController * canvasController();

    /// The layer manager handles everything action related to
    /// layers
    KisLayerManager * layerManager();

    /// The mask manager handles everything action-related to masks
    KisMaskManager * maskManager();

    /// The node manager handles everything about nodes
    KisNodeManager * nodeManager();

    /**
     * Convenience method to get at the active node, which may be
     * a layer or a mask or a selection
     */
    KisNodeSP activeNode();

    /// Convenience method to get at the active layer
    KisLayerSP activeLayer();

    /// Convenience method to get at the active paint device
    KisPaintDeviceSP activeDevice();

    /// The zoommanager handles everything action-related to zooming
    KisZoomManager * zoomManager();

    /// The filtermanager handles everything action-related to filters
    KisFilterManager * filterManager();

    /// The image manager handles everything action-related to the
    /// current image
    KisImageManager * imageManager();

    /// Convenience method to get at the active selection (the
    /// selection of the current layer, or, if that does not exist,
    /// the global selection.
    KisSelectionSP selection();

    /// The undo adapter is used to add commands to the undo stack
    KisUndoAdapter * undoAdapter();

    /// Go to all managers and enable or disable all actions and other
    /// gui elements
    void updateGUI();

    KisDoc2* document() const;

    /// Connects the signals from the current image to the various
    /// slots of the various managers
    void connectCurrentImage();

    /// Disconnect the current image (for instance, before connecting
    /// another image) from the slots in the various managers
    void disconnectCurrentImage();

    virtual KoPrintJob * createPrintJob();

    KisGridManager * gridManager();
    KisPerspectiveGridManager* perspectiveGridManager();
    KisPaintingAssistantsManager* paintingAssistantManager();
    void setFavoriteResourceManager (KisPaintopBox*);
    KoFavoriteResourceManager* favoriteResourceManager();

signals:

    void sigLoadingFinished();
    void favoritePaletteCalled(const QPoint&);

protected:

    void resizeEvent ( QResizeEvent * event );

private slots:

    void slotLoadingFinished();
    void slotUpdateFullScreen(bool);
    void slotPreferences();
    void slotEditPalette();
    void slotImageSizeChanged();
    void slotTotalRefresh();
    void toggleDockers(bool toggle);

public slots:
    void slotCanvasDestroyed(QWidget*);

private:


    void createGUI();
    void createActions();
    void createManagers();

    void loadPlugins();

private:
    class KisView2Private;
    KisView2Private * const m_d;
};

#endif

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

#include <KoView.h>
#include <KoToolBox.h>
#include <KoToolManager.h>

#include <kis_types.h>

class QDragEnterEvent;
class QDropEvent;

class KoCanvasController;
class KoViewChield;

class KisCanvas2;
class KisDoc2;
class KisFilterManager;
class KisImage;
class KisLayerManager;
class KisOpenGLCanvas2;
class KisQPainterCanvas;
class KisResourceProvider;
class KisSelectionManager;
class KisStatusBar;
class KisUndoAdapter;
class KisZoomManager;


class KisView2 : public KoView {

Q_OBJECT

public:

    KisView2(KisDoc2 * doc, QWidget * parent);
    virtual ~KisView2();

public:

    // QWidget overrides
    virtual void dragEnterEvent ( QDragEnterEvent * event );
    virtual void dropEvent ( QDropEvent * event );

    // KoView implementation
    virtual void updateReadWrite( bool readwrite ) { Q_UNUSED(readwrite); }
    void slotChildActivated(bool a);
    void canvasAddChild( KoViewChild * child );

public:  // Krita specific interfaces

    /// Return the image this view is displaying
    KisImageSP image();

    /// The resource provider contains all per-view settings, such as
    /// current color, current paint op etc.
    KisResourceProvider * resourceProvider();

    /// Return the canvasbase class
    KoCanvasBase * canvasBase() const;

    /// Return the actual widget that is displaying the current image
    QWidget* canvas() const;

    /// Return the wrapper class around the statusbar
    KisStatusBar * statusBar() const;

    /// The selection manager handles everything action related to
    /// selections.
    KisSelectionManager * selectionManager();

    /// The layer manager handles everything action related to
    /// layers
    KisLayerManager * layerManager();

    /// The zoommaanger handles everything action-related to zooming
    KisZoomManager * zoomManager();

    /// The filtermanager handles everything action-related to filters
    KisFilterManager * filterManager();

    /// The undo adapter is used to add commands to the undo stack
    KisUndoAdapter * undoAdapter();

    /// Go to all managers and enable or disable all actions and other
    /// gui elements
    void updateGUI();

private slots:

    void slotLoadingFinished();
    void slotUpdateFullScreen(bool);
    void slotPreferences();

private:

    /// Connects the signals from the current image to the various
    /// slots of the various managers
    void connectCurrentImage();

    /// Disconnect the current image (for instance, before connecting
    /// another image) from the slots in the various managers
    void disconnectCurrentImage();

    void createGUI();
    void createActions();
    void createManagers();

    void loadPlugins();


private:
    class KisView2Private;
    KisView2Private * m_d;
};

#endif

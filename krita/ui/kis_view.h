/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *                1999 Michael Koch    <koch@kde.org>
 *                1999 Carsten Pfeiffer <pfeiffer@kde.org>
 *                2002 Patrick Julien <freak@codepimps.org>
 *                2004 Clarence Dang <dang@kde.org>
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

#ifndef KIS_VIEW_H_
#define KIS_VIEW_H_

#include <config.h>
#include <config-krita.h>

#include <list>

#include <QDateTime>
#include <QPixmap>
#include <QStringList>
#include <QTimer>

#include <QWheelEvent>
#include <QPaintEvent>
#include <QEvent>
#include <QKeyEvent>
#include <QDropEvent>
#include <QLabel>
#include <QShowEvent>
#include <QResizeEvent>
#include <QTabletEvent>
#include <QDragEnterEvent>
#include <QMouseEvent>

#include <kmenu.h>
#include <ksqueezedtextlabel.h>
#include <kdebug.h>
#include <kxmlguibuilder.h>
#include <kxmlguiclient.h>
#include <KoView.h>

#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"
#include "kis_global.h"
#include "kis_debug_areas.h"
#include "kis_types.h"
#include "KoColorProfile.h"
#include "kis_opengl_image_context.h"
#include "KoID.h"
#include "krita_export.h"
#include "KoColor.h"
#include "kis_input_device.h"
#include "ui_wdgpalettechooser.h"

class QLabel;
class QPaintEvent;
class QScrollBar;
class QWidget;
class QPopup;

class KAction;
class KActionMenu;
class KPrinter;
class KToggleAction;
class KToolBar;
class KMenu;

class KoPartSelectAction;
class KoDocumentEntry;
class KoIconItem;
class KoTabBar;
class KoPaletteManager;
class KoGrayWidget;
class KoHSVWidget;
class KoRGBWidget;
class KoUniColorChooser;
class KoDocumentSectionView;

class KisBirdEyeBox;
class KisBrush;
class KisButtonPressEvent;
class KisButtonReleaseEvent;
class KisCanvas;
class KisCanvasObserver;
class KoCompositeOp;
class KisControlFrame;
class KisDoc;
class KisDoubleClickEvent;
class KisFilterManager;
class KisFilterStrategy;
class KisGradient;
class KisGridManager;
class KisLabelProgress;
class KisLayerBox;
class KisMoveEvent;
class KisPaletteWidget;
class KisPattern;
class KisPoint;
class KisRect;
class KisResource;
class KisResourceMediator;
class KisRuler;
class KisSelectionManager;
class OldToolBox;
class KisToolControllerInterface;
class KisToolManager;
class KisUndoAdapter;
class KisFilterConfiguration;
class KisPartLayerHandler;
class KisPaintOpSettings;

class KRITAUI_EXPORT KisView
    : public KoView,
      public KisCanvasSubject,
      public KXMLGUIBuilder,
      private KisCanvasController
{

    Q_OBJECT

    typedef KoView super;

    typedef std::list<KisCanvasObserver*> vKisCanvasObserver;
    typedef vKisCanvasObserver::iterator vKisCanvasObserver_it;
    typedef vKisCanvasObserver::const_iterator vKisCanvasObserver_cit;

public:
    KisView(KisDoc *doc, KisUndoAdapter *adapter, QWidget *parent = 0);
    virtual ~KisView();

public: // KXMLGUIBuilder implementation

    virtual QWidget *createContainer( QWidget *parent, int index, const QDomElement &element, int &id );
    virtual void removeContainer( QWidget *container, QWidget *parent, QDomElement &element, int id );

public: // KoView implementation
    virtual bool eventFilter(QObject *o, QEvent *e);

    virtual void print(KPrinter &printer);
    virtual void setupPrinter(KPrinter &printer);

    virtual void updateReadWrite(bool readwrite);
    virtual void guiActivateEvent(KParts::GUIActivateEvent *event);

    virtual int leftBorder() const;
    virtual int rightBorder() const;
    virtual int topBorder() const;
    virtual int bottomBorder() const;

    qint32 docWidth() const;
    qint32 docHeight() const;

    void updateStatusBarSelectionLabel();

    virtual QPoint applyViewTransformations(const QPoint& p) const;
    virtual QPoint reverseViewTransformations( const QPoint& p) const;
    virtual void canvasAddChild(KoViewChild *child);

signals:

    void brushChanged(KisBrush * brush);
    void gradientChanged(KisGradient * gradient);
    void patternChanged(KisPattern * pattern);
    void paintopChanged(KoID paintop, const KisPaintOpSettings *paintopSettings);
    /**
     * Indicates when the current layer changed so that the current colorspace could have
     * changed.
     **/
    void currentColorSpaceChanged(KoColorSpace* cs);
    void cursorPosition(qint32 xpos, qint32 ypos);

    void sigFGColorChanged(const KoColor &);
    void sigBGColorChanged(const KoColor &);

    void sigInputDeviceChanged(const KisInputDevice& inputDevice);

    /*
     * Emitted whenever the zoom or scroll values change.
     */
    void viewTransformationsChanged();

public slots:

    void slotSetFGColor(const KoColor& c);
    void slotSetBGColor(const KoColor& c);

    void rotateLayer180();
    void rotateLayerLeft90();
    void rotateLayerRight90();
    void mirrorLayerX();
    void mirrorLayerY();
    void scaleLayer(double sx, double sy, KisFilterStrategy *filterStrategy);
    void rotateLayer(double angle);
    void shearLayer(double angleX, double angleY);

    void brushActivated(KisResource *brush);
    void patternActivated(KisResource *pattern);
    void gradientActivated(KisResource *gradient);
    void paintopActivated(const KoID & paintop, const KisPaintOpSettings *paintopSettings);


public:
    virtual void mouseMoveEvent(QMouseEvent *e);

    void resizeCurrentImage(qint32 w, qint32 h, bool cropLayers = false);
    void scaleCurrentImage(double sx, double sy, KisFilterStrategy *filterStrategy);
    void rotateCurrentImage(double angle);
    void shearCurrentImage(double angleX, double angleY);

    void insertPart(const QRect& viewRect, const KoDocumentEntry& entry,
                    KisGroupLayerSP parent, KisLayerSP above);


protected:

    virtual void resizeEvent(QResizeEvent*); // From QWidget
    virtual void styleChange(QStyle& oldStyle); // From QWidget
    virtual void paletteChange(const QPalette& oldPalette); // From QWidget
    virtual void showEvent(QShowEvent *);

protected slots:
    virtual void slotChildActivated(bool a); // from KoView

// -------------------------------------------------------------------------//
//                    KisCanvasSubject implementation
// -------------------------------------------------------------------------//
public:

    KisCanvasSubject * canvasSubject() { return this; };

private:

    virtual KisImageSP currentImg() const;

    virtual void attach(KisCanvasObserver *observer);
    virtual void detach(KisCanvasObserver *observer);
    virtual void notifyObservers();

    virtual KoColor bgColor() const;
    virtual void setBGColor(const KoColor& c);

    virtual KoColor fgColor() const;
    virtual void setFGColor(const KoColor& c);

    float HDRExposure() const;
    void setHDRExposure(float exposure);

    virtual KisBrush *currentBrush() const;
    virtual KisPattern *currentPattern() const;
    virtual KisGradient *currentGradient() const;
    virtual KoID currentPaintop() const;
    virtual const KisPaintOpSettings *currentPaintopSettings() const;

    virtual double zoomFactor() const;

    virtual KisUndoAdapter *undoAdapter() const;

    virtual KisCanvasController *canvasController() const;
    virtual KisToolControllerInterface *toolController() const;

    virtual KisProgressDisplayInterface *progressDisplay() const;

    virtual KisDoc * document() const;

    inline KisGridManager * gridManager() { return m_gridManager; }

    inline KisSelectionManager * selectionManager() { return m_selectionManager; }

    KoPaletteManager * paletteManager();

    KoColorProfile *  monitorProfile();

// -------------------------------------------------------------------------//
//                    KisCanvasController implementation
// -------------------------------------------------------------------------//

public:

    KisCanvasController * getCanvasController() { return this; };


private slots:
    virtual void updateCanvas();

    void updateStatusBarZoomLabel();
    void updateStatusBarProfileLabel();

private:
    virtual KisCanvas *kiscanvas() const;

    virtual qint32 horzValue() const;
    virtual qint32 vertValue() const;

    virtual void scrollTo(qint32 x, qint32 y);

    virtual void updateCanvas(qint32 x, qint32 y, qint32 w, qint32 h);
    virtual void updateCanvas(const QRect& imageRect);

    virtual void zoomIn();
    virtual void zoomIn(qint32 x, qint32 y);

    virtual void zoomOut();
    virtual void zoomOut(qint32 x, qint32 y);

    virtual void zoomTo(qint32 x, qint32 y, qint32 w, qint32 h);
    virtual void zoomTo(const QRect& r);
    virtual void zoomTo(const KisRect& r);
    virtual void zoomAroundPoint(double x, double y, double zf);

    virtual QPoint viewToWindow(const QPoint& pt);
    virtual QPoint viewToWindow(const QPoint& pt) const;
    virtual KisPoint viewToWindow(const KisPoint& pt);
    virtual QRect viewToWindow(const QRect& rc);
    virtual KisRect viewToWindow(const KisRect& rc);
    virtual void viewToWindow(qint32 *x, qint32 *y);

    virtual QPoint windowToView(const QPoint& pt);
    virtual QPoint windowToView(const QPoint& pt) const;
    virtual KisPoint windowToView(const KisPoint& pt);
    virtual QRect windowToView(const QRect& rc);
    virtual KisRect windowToView(const KisRect& rc);
    virtual void windowToView(qint32 *x, qint32 *y);

    virtual QCursor setCanvasCursor(const QCursor & cursor);

    void setInputDevice(KisInputDevice inputDevice);
    KisInputDevice currentInputDevice() const;

// -------------------------------------------------------------------------//
//                      KisView internals
// -------------------------------------------------------------------------//

private:

    void connectCurrentImg();
    void disconnectCurrentImg();
//    void eraseGuides();
//    void paintGuides();
//    void updateGuides();
//    void viewGuideLines();

    void imgUpdateGUI();

    void layerUpdateGUI(bool enable);
    void createLayerBox();
    void createDockers();

    void paintToolOverlay(const QRegion& region);

    void paintQPaintDeviceView(const QRegion& canvasRegion);
    void paintOpenGLView(const QRect& canvasRect);

    void updateQPaintDeviceCanvas(const QRect& imageRect);
    void updateOpenGLCanvas(const QRect& imageRect);

    /**
     * Update the whole of the KisCanvas, including areas outside the image.
     */
    void refreshKisCanvas();

    void selectionDisplayToggled(bool displaySelection);

    bool activeLayerHasSelection();

    /**
     * Reset the monitor profile to the new settings.
     */
    void resetMonitorProfile();

    void setupActions();
    void setupCanvas();
    void setupRulers();
    void setupScrollBars();
    void setupStatusBar();

    /**
     * Import an image as a layer. If there is more than
     * one layer in the image, import all of them as separate
     * layers.
     *
     * @param url the url to the image file
     * @return the number of layers added
     */
    qint32 importImage(const KUrl& url = KUrl());
    KisFilterManager * filterManager() { return m_filterManager; }
    void setCurrentImage(KisImageSP image);

    /**
     * Returns the next zoom level when zooming in from the current level.
     */
    double nextZoomInLevel() const;

    /**
     * Returns the next zoom level when zooming out from the current level.
     */
    double nextZoomOutLevel() const;

    /**
     * Returns the next zoom level when zooming out from the given level.
     */
    double nextZoomOutLevel(double zoomLevel) const;

    /**
     * Returns the zoom level that fits the image to the canvas.
     */
    double fitToCanvasZoomLevel() const;

    /**
     * Set the zoom level on first creating the view.
     */
    void setInitialZoomLevel();

    void startInitialZoomTimerIfReady();

private slots:
    void layersUpdated(); // Used in the channel separation to notify the view that we have added a few layers.

    void imgUpdated(QRect rc);
    void slotOpenGLImageUpdated(QRect rc);

    void imgResizeToActiveLayer();

    void canvasGotMoveEvent(KisMoveEvent *e);
    void canvasGotButtonPressEvent(KisButtonPressEvent *e);
    void canvasGotButtonReleaseEvent(KisButtonReleaseEvent *e);
    void canvasGotDoubleClickEvent(KisDoubleClickEvent *e);
    void canvasGotPaintEvent(QPaintEvent *e);
    void canvasGotMouseWheelEvent(QWheelEvent *e);
    void canvasGotKeyPressEvent(QKeyEvent*);
    void canvasGotKeyReleaseEvent(QKeyEvent*);
    void canvasGotDragEnterEvent(QDragEnterEvent*);
    void canvasGotDropEvent(QDropEvent*);

    void reconnectAfterPartInsert();

    QPoint mapToScreen(const QPoint& pt);
    void slotImageProperties();

    void layerCompositeOp(const KoCompositeOp* compositeOp);
    void layerOpacity(int opacity, bool dontundo);
    void layerOpacityFinishedChanging(int previous, int opacity);

    void layerToggleVisible();
    void layerToggleLocked();
    void actLayerVisChanged(int show);
    void layerProperties();
    void showLayerProperties(KisLayerSP layer);
    void layerAdd();
    void addLayer(KisGroupLayerSP parent, KisLayerSP above);
    void addGroupLayer(KisGroupLayerSP parent, KisLayerSP above);
    void addPartLayer();
    void addPartLayer(KisGroupLayerSP parent, KisLayerSP above, const KoDocumentEntry& entry);
    void addAdjustmentLayer();
    void addAdjustmentLayer(KisGroupLayerSP parent, KisLayerSP above);
    void addAdjustmentLayer(KisGroupLayerSP parent, KisLayerSP above, const QString & name, KisFilterConfiguration * filter, KisSelectionSP selection);
    void layerRemove();
    void layerDuplicate();
    void layerRaise();
    void layerLower();
    void layerFront();
    void layerBack();
    void flattenImage();
    void mergeLayer();
    void saveLayerAsImage();

    void slotUpdateFullScreen(bool toggle);
    void showRuler();

    void slotZoomIn();
    void slotZoomOut();
    void slotActualPixels();
    void slotActualSize();
    void slotFitToCanvas();

    void slotImageSizeChanged(qint32 w, qint32 h);

    void scrollH(int value);
    void scrollV(int value);

    void slotInsertImageAsLayer();
    void profileChanged(KoColorProfile *  profile);

    void slotAddPalette();
    void slotEditPalette();

    void preferences();

    void slotAutoScroll(const QPoint &p);

    void handlePartLayerAdded(KisLayerSP layer);

    /// Is called when the file is loaded
    void slotLoadingFinished();

    void slotInitialZoomTimeout();

private:

    bool m_panning;

    KisTool * m_oldTool;

    KisDoc *m_doc;
    KisCanvas *m_canvas;
    KMenu *m_popup;
    KisPartLayerHandler* m_partHandler;

    KisGridManager * m_gridManager;
    KisSelectionManager * m_selectionManager;
    KisFilterManager * m_filterManager;
    KoPaletteManager * m_paletteManager;
    KisToolManager * m_toolManager;
    bool m_actLayerVis;

    // Fringe benefits
    KisRuler *m_hRuler;
    KisRuler *m_vRuler;
    qint32 m_rulerThickness;
    qint32 m_vScrollBarExtent;
    qint32 m_hScrollBarExtent;

    // Actions
    KAction *m_imgFlatten;
    KAction *m_imgMergeLayer;
    KAction *m_imgRename;
    KAction *m_imgResizeToLayer;
    KAction *m_imgScan;

    KoPartSelectAction * m_actionPartLayer;
    KAction * m_actionAdjustmentLayer;
    KAction *m_layerAdd;
    KAction *m_layerBottom;
    KAction *m_layerDup;
    KAction *m_layerHide;
    KAction *m_layerLower;
    KAction *m_layerProperties;
    KAction *m_layerRaise;
    KAction *m_layerRm;
    KAction *m_layerSaveAs;
    KAction *m_layerTop;

    KAction *m_zoomIn;
    KAction *m_zoomOut;
    KAction *m_actualPixels;
    KAction *m_actualSize;
    KAction *m_fitToCanvas;

    KAction *m_fullScreen;
    KAction *m_imgProperties;

    KToggleAction *m_RulerAction;
    KToggleAction *m_guideAction;

    // Widgets
    QScrollBar *m_hScroll; // XXX: the sizing of the scrollthumbs
    QScrollBar *m_vScroll; // is not right yet.
    int m_scrollX;
    int m_scrollY;
    int m_canvasXOffset;
    int m_canvasYOffset;

    bool m_paintViewEnabled;
    bool m_guiActivateEventReceived;
    bool m_showEventReceived;
    bool m_imageLoaded;

    QTimer m_initialZoomTimer;


//    KisGuideSP m_currentGuide;
//    QPoint m_lastGuidePoint;
    KisUndoAdapter *m_adapter;
    vKisCanvasObserver m_observers;
    QLabel *m_statusBarZoomLabel;
    KSqueezedTextLabel *m_statusBarSelectionLabel;
    KSqueezedTextLabel *m_statusBarProfileLabel;
    KisLabelProgress *m_progress;


    KisLayerBox *m_layerBox;
    OldToolBox * m_toolBox;
    KisControlFrame * m_brushesAndStuffToolBar;

    // Current colours, brushes, patterns etc.

    KoColor m_fg;
    KoColor m_bg;

    KisBrush *m_brush;
    KisPattern *m_pattern;
    KisGradient *m_gradient;

    KoID m_paintop;
    const KisPaintOpSettings *m_paintopSettings;

    QTime m_tabletEventTimer;
    QTabletEvent::TabletDevice m_lastTabletEventDevice;

    QPixmap m_canvasPixmap;
    bool m_toolIsPainting;

#ifdef HAVE_OPENGL
    // OpenGL context for the current image, containing textures
    // shared between multiple views.
    KisOpenGLImageContextSP m_OpenGLImageContext;
#endif

    // Monitorprofile for this view
    KoColorProfile *  m_monitorProfile;

    float m_HDRExposure;

    // Currently active input device (mouse, stylus, eraser...)
    KisInputDevice m_inputDevice;

    KisBirdEyeBox * m_birdEyeBox;
    KoUniColorChooser *m_colorchooser;
    KoHSVWidget *m_hsvwidget;
    KoRGBWidget *m_rgbwidget;
    KoGrayWidget *m_graywidget;
    KisPaletteWidget *m_palettewidget;
    KoID m_currentColorChooserDisplay;

private:
    KisImageSP m_image;

protected:

    friend class KisSelectionManager;
    friend class KisFilterManager;
    friend class KisGridManager;
};

class KisPaletteChooser : public QDialog, public Ui::KisPaletteChooser
{
    Q_OBJECT

    public:
        KisPaletteChooser(QWidget *parent) : QDialog(parent) { setupUi(this); }
};

#endif // KIS_VIEW_H_

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

#include <qdatetime.h>
#include <qpixmap.h>
#include <qstringlist.h>
#include <list>

#include <kdebug.h>
#include <kxmlguibuilder.h>
#include <kxmlguiclient.h>
#include <koView.h>

#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_scale_visitor.h"
#include "kis_profile.h"
#include "kis_opengl_image_context.h"
#include "kis_id.h"
#include "koffice_export.h"
#include "kis_color.h"

class QButton;
class QLabel;
class QPaintEvent;
class QScrollBar;
class QWidget;

class DCOPObject;
class KAction;
class KActionMenu;
class KPrinter;
class KToggleAction;
class KToolBar;

class KoPartSelectAction;
class KoIconItem;
class KoTabBar;
class KoPaletteManager;

class KisBirdEyeBox;
class KisBrush;
class KisButtonPressEvent;
class KisButtonReleaseEvent;
class KisCanvas;
class KisCanvasObserver;
class KisCompositeOp;
class KisControlFrame;
class KisDoc;
class KisDoubleClickEvent;
class KisFilterManager;
class KisFilterStrategy;
class KisGradient;
class KisGrayWidget;
class KisHSVWidget;
class KisLabelProgress;
class KisLayerBox;
class KisMoveEvent;
class KisPaletteWidget;
class KisPattern;
class KisPoint;
class KisRect;
class KisResource;
class KisResourceMediator;
class KisRGBWidget;
class KisRuler;
class KisSelectionManager;
class KoToolBox;
class KisToolControllerInterface;
class KisToolManager;
class KisUndoAdapter;


class KRITA_EXPORT KisView
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
    KisView(KisDoc *doc, KisUndoAdapter *adapter, QWidget *parent = 0, const char *name = 0);
    virtual ~KisView();

public: // KXMLGUIBuilder implementation

    virtual QWidget *createContainer( QWidget *parent, int index, const QDomElement &element, int &id );
    virtual void removeContainer( QWidget *container, QWidget *parent, QDomElement &element, int id );

public: // KoView implementation
    virtual bool eventFilter(QObject *o, QEvent *e);

    virtual DCOPObject* dcopObject();

    virtual void print(KPrinter &printer);
    virtual void setupPrinter(KPrinter &printer);

    virtual void updateReadWrite(bool readwrite);
    virtual void guiActivateEvent(KParts::GUIActivateEvent *event);

    Q_INT32 docWidth() const;
    Q_INT32 docHeight() const;

    void updateStatusBarSelectionLabel();

signals:

    void brushChanged(KisBrush * brush);
    void gradientChanged(KisGradient * gradient);
    void patternChanged(KisPattern * pattern);
    void paintopChanged(KisID paintop);
    void cursorPosition(Q_INT32 xpos, Q_INT32 ypos);

public slots:

    void slotSetFGColor(const KisColor& c);
    void slotSetBGColor(const KisColor& c);

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
    void paintopActivated(const KisID & paintop);


public:

    void resizeCurrentImage(Q_INT32 w, Q_INT32 h, bool cropLayers = false);
    void scaleCurrentImage(double sx, double sy, KisFilterStrategy *filterStrategy);
    void rotateCurrentImage(double angle);
    void shearCurrentImage(double angleX, double angleY);


protected:

    virtual void resizeEvent(QResizeEvent*); // From QWidget


// -------------------------------------------------------------------------//
//                    KisCanvasSubject implementation
// -------------------------------------------------------------------------//
public:

    KisCanvasSubject * getCanvasSubject() { return this; };

private:

    virtual KisImageSP currentImg() const;

    virtual void attach(KisCanvasObserver *observer);
    virtual void detach(KisCanvasObserver *observer);
    virtual void notifyObservers();

    virtual KisColor bgColor() const;
    virtual void setBGColor(const KisColor& c);

    virtual KisColor fgColor() const;
    virtual void setFGColor(const KisColor& c);

    float HDRExposure() const;
    void setHDRExposure(float exposure);

    virtual KisBrush *currentBrush() const;
    virtual KisPattern *currentPattern() const;
    virtual KisGradient *currentGradient() const;
    virtual KisID currentPaintop() const;

    virtual double zoomFactor() const;

    virtual KisUndoAdapter *undoAdapter() const;

    virtual KisCanvasController *canvasController() const;
    virtual KisToolControllerInterface *toolController() const;

    virtual KisProgressDisplayInterface *progressDisplay() const;

    virtual KisDoc * document() const;

    KisSelectionManager * selectionManager() { return m_selectionManager; }

    KoPaletteManager * paletteManager();

    KisProfile *  monitorProfile();

// -------------------------------------------------------------------------//
//                    KisCanvasController implementation
// -------------------------------------------------------------------------//

public:

    KisCanvasController * getCanvasController() { return this; };


private:
    virtual KisCanvas *canvas() const;
    
    virtual Q_INT32 horzValue() const;
    virtual Q_INT32 vertValue() const;

    virtual void scrollTo(Q_INT32 x, Q_INT32 y);

    virtual void updateCanvas();
    virtual void updateCanvas(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);
    virtual void updateCanvas(const QRect& rc);

    virtual void zoomIn();
    virtual void zoomIn(Q_INT32 x, Q_INT32 y);

    virtual void zoomOut();
    virtual void zoomOut(Q_INT32 x, Q_INT32 y);

    virtual void zoomTo(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);
    virtual void zoomTo(const QRect& r);
    virtual void zoomTo(const KisRect& r);
    virtual void zoomAroundPoint(double x, double y, double zf);

    virtual QPoint viewToWindow(const QPoint& pt);
    virtual KisPoint viewToWindow(const KisPoint& pt);
    virtual QRect viewToWindow(const QRect& rc);
    virtual KisRect viewToWindow(const KisRect& rc);
    virtual void viewToWindow(Q_INT32 *x, Q_INT32 *y);

    virtual QPoint windowToView(const QPoint& pt);
    virtual KisPoint windowToView(const KisPoint& pt);
    virtual QRect windowToView(const QRect& rc);
    virtual KisRect windowToView(const KisRect& rc);
    virtual void windowToView(Q_INT32 *x, Q_INT32 *y);

    virtual QCursor setCanvasCursor(const QCursor & cursor);

    void setInputDevice(enumInputDevice inputDevice);
    enumInputDevice currentInputDevice() const;

// -------------------------------------------------------------------------//
//                      KisView internals
// -------------------------------------------------------------------------//

private:

    void clearCanvas(const QRect& rc);
    void connectCurrentImg();
    void disconnectCurrentImg();
//    void eraseGuides();
//    void paintGuides();
//    void updateGuides();
    void imgUpdateGUI();

    void layerUpdateGUI(bool enable);
    void createLayerBox();
    void createDockers();

    void paintView(const KisRect& rc);
    void paintOpenGLView(const KisRect& rc);

    /**
     * Reset the monitor profile to the new settings.
     */
    void resetMonitorProfile();

    void setupActions();
    void setupCanvas();
    void setupRulers();
    void setupScrollBars();
    void setupStatusBar();

    void updateStatusBarZoomLabel();
    void updateStatusBarProfileLabel();

    Q_INT32 importImage(const KURL& url = KURL());
    virtual void updateCanvas(const KisRect& rc);
    KisFilterManager * filterManager() { return m_filterManager; }
    void layersUpdated(); // Used in the channel separation to notify the view that we have added a few layers.
    void setCurrentImage(KisImageSP image);

private slots:

    void imgUpdated(KisImageSP img, const QRect& rc);
    void imgResizeToActiveLayer();

    void canvasGotMoveEvent(KisMoveEvent *e);
    void canvasGotButtonPressEvent(KisButtonPressEvent *e);
    void canvasGotButtonReleaseEvent(KisButtonReleaseEvent *e);
    void canvasGotDoubleClickEvent(KisDoubleClickEvent *e);
    void canvasGotPaintEvent(QPaintEvent *e);
    void canvasGotEnterEvent(QEvent *e);
    void canvasGotLeaveEvent(QEvent *e);
    void canvasGotMouseWheelEvent(QWheelEvent *e);
    void canvasRefresh();
    void canvasGotKeyPressEvent(QKeyEvent*);
    void canvasGotKeyReleaseEvent(QKeyEvent*);
    void canvasGotDragEnterEvent(QDragEnterEvent*);
    void canvasGotDropEvent(QDropEvent*);

    void docImageListUpdate();
    void layersUpdated(KisImageSP img);

    QPoint mapToScreen(const QPoint& pt);
    void slotImageProperties();

    void layerCompositeOp(const KisCompositeOp& compositeOp);
    void layerOpacity(int opacity);

    void layerToggleVisible();
    void layerSelected(int n);
    void layerToggleLinked();
    void layerToggleLocked();
    void layerProperties();
    void layerAdd();
    void addPartLayer();
    void layerRemove();
    void layerDuplicate();
    void layerRaise();
    void layerLower();
    void layerFront();
    void layerBack();
    void flattenImage();
    void mergeVisibleLayers();
    void mergeLayer();
    void mergeLinkedLayers();
    void saveLayerAsImage();

    void slotUpdateFullScreen(bool toggle);
    void showRuler();

    void slotZoomIn();
    void slotZoomOut();
    void slotActualPixels();
    void slotActualSize();
    void slotImageSizeChanged(KisImageSP img, Q_INT32 w, Q_INT32 h);

    void scrollH(int value);
    void scrollV(int value);

    void slotInsertImageAsLayer();
    void profileChanged(KisProfile *  profile);

    void preferences();

private:

    KisDoc *m_doc;
    KisCanvas *m_canvas;

    KisSelectionManager * m_selectionManager;
    KisFilterManager * m_filterManager;
    KoPaletteManager * m_paletteManager;
    KisToolManager * m_toolManager;

    // Fringe benefits
    KisRuler *m_hRuler;
    KisRuler *m_vRuler;

    // Actions
    KAction *m_imgFlatten;
    KAction *m_imgMergeLinked;
    KAction *m_imgMergeVisible;
    KAction *m_imgMergeLayer;
    KAction *m_imgRename;
    KAction *m_imgResizeToLayer;
    KAction *m_imgScan;

    KoPartSelectAction * m_actionPartLayer;
    KAction *m_layerAdd;
    KAction *m_layerBottom;
    KAction *m_layerDup;
    KAction *m_layerHide;
    KAction *m_layerLink;
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

    KAction *m_fullScreen;
    KAction *m_imgProperties;

    KToggleAction *m_RulerAction;

    DCOPObject *m_dcop;

    // Widgets
    QScrollBar *m_hScroll; // XXX: the sizing of the scrollthumbs
    QScrollBar *m_vScroll; // is not right yet.
    int m_scrollX;
    int m_scrollY;
//    KisGuideSP m_currentGuide;
//    QPoint m_lastGuidePoint;
    KisUndoAdapter *m_adapter;
    vKisCanvasObserver m_observers;
    QLabel *m_statusBarZoomLabel;
    QLabel *m_statusBarSelectionLabel;
    QLabel *m_statusBarProfileLabel;
    KisLabelProgress *m_progress;

    KisLayerBox *m_layerBox;
    KoToolBox * m_toolBox;
    KisControlFrame * m_brushesAndStuffToolBar;

    // Current colours, brushes, patterns etc.

    KisColor m_fg;
    KisColor m_bg;

    KisBrush *m_brush;
    KisPattern *m_pattern;
    KisGradient *m_gradient;

    KisID m_paintop;

    QTime m_tabletEventTimer;
    QTabletEvent::TabletDevice m_lastTabletEventDevice;

    QPixmap m_canvasPixmap;

    // OpenGL context for the current image, containing textures
    // shared between multiple views.
    KisOpenGLImageContextSP m_OpenGLImageContext;

    // Monitorprofile for this view
    KisProfile *  m_monitorProfile;

    float m_HDRExposure;

    // Currently active input device (mouse, stylus, eraser...)
    enumInputDevice m_inputDevice;

    KisBirdEyeBox * m_birdEyeBox;
    KisHSVWidget *m_hsvwidget;
    KisRGBWidget *m_rgbwidget;
    KisGrayWidget *m_graywidget;
    KisPaletteWidget *m_palettewidget;

private:
    KisImageSP m_current;

protected:

    friend class KisSelectionManager;
    friend class KisFilterManager;

};

#endif // KIS_VIEW_H_

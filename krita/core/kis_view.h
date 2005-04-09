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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef KIS_VIEW_H_
#define KIS_VIEW_H_

#include <qdatetime.h>
#include <qpixmap.h>

#include <list>
#include <map>

#include <qcolor.h>
#include <koView.h>
#include <kdebug.h>

#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"
#include "kis_global.h"
#include "kis_tool_controller.h"
#include "kis_types.h"
#include "kis_scale_visitor.h"
#include "kis_profile.h"

#include "kis_id.h"

class QButton;
class QLabel;
class QPaintEvent;
class QScrollBar;
class QWidget;

class DCOPObject;
class KAction;
class KPrinter;
class KToggleAction;

class KoIconItem;
class KoTabBar;

class KisCanvasObserver;
class KisRuler;
class KisBrush;
class KisCanvas;
class KisLabelProgress;
class KisDoc;
class KisGradient;
class KisPattern;
class KisResource;
class KisUndoAdapter;
class KisRect;
class KisPoint;
class KisButtonPressEvent;
class KisButtonReleaseEvent;
class KisMoveEvent;
class KisSelectionManager;
class KisDockerManager;
class KisToolRegistry;
class KisFilterRegistry;


class KisView
	: public KoView,
	  public KisCanvasSubject,
	  private KisCanvasControllerInterface,
	  private KisToolControllerInterface
{

	Q_OBJECT

	typedef KoView super;

	typedef std::list<KisCanvasObserver*> vKisCanvasObserver;
	typedef vKisCanvasObserver::iterator vKisCanvasObserver_it;
	typedef vKisCanvasObserver::const_iterator vKisCanvasObserver_cit;

	typedef std::map<enumInputDevice, KisTool *> InputDeviceToolMap;
	typedef std::map<enumInputDevice, vKisTool> InputDeviceToolSetMap;


public:
	KisView(KisDoc *doc, KisUndoAdapter *adapter, QWidget *parent = 0, const char *name = 0);
	virtual ~KisView();


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

public: // Plugin access API. XXX: This needs redesign.

	Q_INT32 importImage(bool createLayer, bool modal = false, const KURL& url = KURL());

	virtual KisImageSP currentImg() const;

	virtual void updateCanvas();
	virtual void updateCanvas(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);
	virtual void updateCanvas(const QRect& rc);
	virtual void updateCanvas(const KisRect& rc);

	virtual KisProgressDisplayInterface *progressDisplay() const;

	void layersUpdated();

	KisDoc * getDocument() { return m_doc; }

	KisSelectionManager * selectionManager() { return m_selectionManager; }
	KisDockerManager * dockerManager() { return m_dockerManager; |

signals:
	void bgColorChanged(const QColor& c);
	void fgColorChanged(const QColor& c);

	void brushChanged(KisBrush * brush);
	void gradientChanged(KisGradient * gradient);
	void patternChanged(KisPattern * pattern);

	void currentLayerChanged(int layer);

	void cursorPosition(Q_INT32 xpos, Q_INT32 ypos);
	void cursorEnter();
	void cursorLeave();


public slots:
	void slotSetFGColor(const QColor& c);
	void slotSetBGColor(const QColor& c);

	void next_layer();
	void previous_layer();

	// image action slots
	// XXX: Rename to make all names consistent with slotDoX() pattern
	void slotImportImage();
	void export_image();
	void slotImageProperties();
        void imgResizeToActiveLayer();
	void add_new_image_tab();
	void remove_current_image_tab();
	void resizeCurrentImage(Q_INT32 w, Q_INT32 h, bool cropLayers = false);
	void scaleCurrentImage(double sx, double sy, enumFilterType ftype = MITCHELL_FILTER);
        void rotateCurrentImage(double angle);
        void shearCurrentImage(double angleX, double angleY);

	// Layer action slots
	void rotateLayer180();
	void rotateLayerLeft90();
	void rotateLayerRight90();
	void mirrorLayerX();
	void mirrorLayerY();
	void scaleLayer(double sx, double sy, enumFilterType ftype = MITCHELL_FILTER);
	void rotateLayer(double angle);
	void shearLayer(double angleX, double angleY);
	/// Crop the current layer to the specified dimensions, do not move it the image origin.
	void cropLayer(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);
	// settings action slots
	void preferences();
	void layerCompositeOp(int compositeOp);
	void layerOpacity(int opacity);

protected:

	virtual void resizeEvent(QResizeEvent*);

public:
	KisCanvasSubject * getCanvasSubject() { return this; };

private:
	// Implement KisCanvasSubject
	virtual void attach(KisCanvasObserver *observer);
	virtual void detach(KisCanvasObserver *observer);
	virtual void notify();
	virtual QString currentImgName() const;
	virtual QColor bgColor() const;
	virtual void setBGColor(const QColor& c);
	virtual QColor fgColor() const;
	virtual void setFGColor(const QColor& c);
	virtual KisBrush *currentBrush() const;
	virtual KisPattern *currentPattern() const;
	virtual KisGradient *currentGradient() const;
	virtual double zoomFactor() const;
	virtual KisUndoAdapter *undoAdapter() const;
	virtual KisCanvasControllerInterface *canvasController() const;
	virtual KisToolControllerInterface *toolController() const;
	virtual KoDocument *document() const;
	// Sets the specified cursor; returns the previous cursor
	virtual QCursor setCanvasCursor(const QCursor & cursor);

public:

	KisCanvasControllerInterface * getCanvasController() { return this; };


private:
	// Implement KisCanvasControllerInterface
	virtual QWidget *canvas() const;
	virtual Q_INT32 horzValue() const;
	virtual Q_INT32 vertValue() const;
	virtual void scrollTo(Q_INT32 x, Q_INT32 y);
	virtual void zoomIn();
	virtual void zoomIn(Q_INT32 x, Q_INT32 y);
	virtual void zoomOut();
	virtual void zoomOut(Q_INT32 x, Q_INT32 y);
	virtual void zoomTo(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);
	virtual void zoomTo(const QRect& r);
	virtual void zoomTo(const KisRect& r);
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

private:
	// Implement KisToolControllerInterface
	virtual void setCurrentTool(KisTool *tool);
	virtual KisTool *currentTool() const;

private:
	void clearCanvas(const QRect& rc);
	void connectCurrentImg() const;
	void disconnectCurrentImg() const;
	void eraseGuides();
	void paintGuides();
	void updateGuides();
	void imgUpdateGUI();

	void layerUpdateGUI(bool enable);
	void paintView(const KisRect& rc);

	/**
	 * Get the profile that this view uses to display itself on
	 * he monitor.
	 */
	KisProfileSP monitorProfile();

	/**
	 * Reset the monitor profile to the new settings.
	 */
	void resetMonitorProfile();


	bool selectColor(QColor& result);
	void selectImage(KisImageSP img);

	void setupActions();
	void setupCanvas();
	void setupRulers();
	void setupScrollBars();
	void setupTabBar();
	void setupStatusBar();
	void setupTools();

        void updateStatusBarZoomLabel();
	void updateStatusBarProfileLabel();

	void zoomUpdateGUI(Q_INT32 x, Q_INT32 y, double zf);

	void setInputDevice(enumInputDevice inputDevice);
	enumInputDevice currentInputDevice() const;

	KisTool *findTool(QString toolName, enumInputDevice inputDevice = INPUT_DEVICE_UNKNOWN) const;

public slots:

	void layerToggleVisible();
	void layerSelected(int n);
	void layerToggleLinked();
	void layerToggleLocked();
	void layerProperties();

	void layerToImage();
	void layerAdd();
	void layerRemove();
	void layerDuplicate();
	void layerAddMask(int n);
	void layerRmMask(int n);
	void layerRaise();
	void layerLower();
	void layerFront();
	void layerBack();
	void layerLevel(int n);
	void flattenImage();
	void mergeVisibleLayers();
	void mergeLayer();
	void mergeLinkedLayers();
	void saveLayerAsImage();
	void currentImageUpdated(KisImageSP img);
	void selectFGColor();
	void selectBGColor();
	void reverseFGAndBGColors();
	void selectImage(const QString&);
	void brushActivated(KisResource *brush);
	void patternActivated(KisResource *pattern);
	void gradientActivated(KisResource *gradient);
	void scrollH(int value);
	void scrollV(int value);
	void slotEmbedImage(const QString& filename);
	void slotInsertImageAsLayer();
	void imgUpdated(KisImageSP img, const QRect& rc);
	void imgUpdated(KisImageSP img);
	void profileChanged(KisProfileSP profile);
	void slotZoomIn();
	void slotZoomOut();
	void slotImageSizeChanged(KisImageSP img, Q_INT32 w, Q_INT32 h);

	void slotUpdateFullScreen(bool toggle);
	void updateTabBar();
	void showRuler();

	void duplicateCurrentImg();



private slots:

	void popupTabBarMenu( const QPoint& );
	void moveImage( unsigned, unsigned );
	void slotRename();

	void canvasGotMoveEvent(KisMoveEvent *e);
	void canvasGotButtonPressEvent(KisButtonPressEvent *e);
	void canvasGotButtonReleaseEvent(KisButtonReleaseEvent *e);
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
private:
	KisDoc *m_doc;
	KisCanvas *m_canvas;

	KisSelectionManager * m_selectionManager;
	KisDockerManager * m_dockerManager;

        // Fringe benefits
	KoTabBar *m_tabBar;
	QButton *m_tabFirst;
	QButton *m_tabLeft;
	QButton *m_tabRight;
	QButton *m_tabLast;

	KisRuler *m_hRuler;
	KisRuler *m_vRuler;

        // Actions
	KAction *m_imgDup;
	KAction *m_imgExport;
	KAction *m_imgImport;
	KAction *m_imgFlatten;
	KAction *m_imgMergeLinked;
	KAction *m_imgMergeVisible;
	KAction *m_imgMergeLayer;
	KAction *m_imgRename;
	KAction *m_imgResizeToLayer;
	KAction *m_imgRm;
	KAction *m_imgScan;

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
	KAction *m_layerToImage;
	KAction *m_layerTop;

	KAction *m_zoomIn;
	KAction *m_zoomOut;

	KAction *m_fullScreen;
	KAction *m_imgProperties;

	KToggleAction *m_RulerAction;

	DCOPObject *m_dcop;

        // Widgets
	QScrollBar *m_hScroll; // XXX: the sizing of the scrollthumbs
	QScrollBar *m_vScroll; // is not right yet.
	int m_scrollX;
	int m_scrollY;
	KisGuideSP m_currentGuide;
	QPoint m_lastGuidePoint;
	KisUndoAdapter *m_adapter;
	vKisCanvasObserver m_observers;
	QLabel *m_statusBarZoomLabel;
	QLabel *m_statusBarSelectionLabel;
	QLabel *m_statusBarProfileLabel;
	KisLabelProgress *m_progress;

	// Current colours, brushes, patterns etc.

	QColor m_fg;
	QColor m_bg;

	KisBrush *m_brush;
	KisPattern *m_pattern;
	KisGradient *m_gradient;

	enumInputDevice m_inputDevice;
	InputDeviceToolMap m_inputDeviceToolMap;
	InputDeviceToolSetMap m_inputDeviceToolSetMap;

	QTime m_tabletEventTimer;
	QTabletEvent::TabletDevice m_lastTabletEventDevice;
	QPixmap m_canvasPixmap;

	// Monitorprofile for this view
	KisProfileSP m_monitorProfile;

private:
	mutable KisImageSP m_current;

	// XXX: Temporary re-instatement of old way to load filters and tools
public:
	KisToolRegistry * toolRegistry() const;
	KisFilterRegistry * filterRegistry() const;

	virtual KisFilterSP filterGet(const KisID& id);
	virtual KisIDList filterList();

private:
	KisFilterRegistry * m_filterRegistry;
	KisToolRegistry * m_toolRegistry;

protected:

	friend class KisSelectionManager;
	friend class KisDockerManager;

};

#endif // KIS_VIEW_H_


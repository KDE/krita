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

#if !defined KIS_VIEW_H_
#define KIS_VIEW_H_

#include <list>
#include <koColor.h>
#include <koView.h>
#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"
#include "kis_global.h"
#include "kis_tool_controller.h"
#include "kis_types.h"

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
class KisBuilderMonitor;
class KisCanvas;
class KisChannelView;
class KisLabelBuilderProgress;
class KisDoc;
class KisGradient;
class KisListBox;
class KisPattern;
class KisResource;
class KisResourceMediator;
class KisSideBar;
class DockFrameDocker;
class ColorDocker;
class ControlFrame;
class KisUndoAdapter;
class KisRect;
class KisPoint;

class KisView : public KoView,
		private KisCanvasSubject,
		private KisCanvasControllerInterface,
		private KisToolControllerInterface {
	Q_OBJECT
	typedef KoView super;
	typedef std::list<KisCanvasObserver*> vKisCanvasObserver;
	typedef vKisCanvasObserver::iterator vKisCanvasObserver_it;
	typedef vKisCanvasObserver::const_iterator vKisCanvasObserver_cit;

public:
	KisView(KisDoc *doc, KisUndoAdapter *adapter, QWidget *parent = 0, const char *name = 0);
	virtual ~KisView();

public:
	virtual bool eventFilter(QObject *o, QEvent *e);
	virtual int canvasXOffset() const;
	virtual int canvasYOffset() const;
	virtual DCOPObject* dcopObject();
	virtual void print(KPrinter &printer);
	virtual void setupPrinter(KPrinter &printer);
	virtual void updateReadWrite(bool readwrite);
	virtual void guiActivateEvent(KParts::GUIActivateEvent *event);

public:
	Q_INT32 docWidth() const;
	Q_INT32 docHeight() const;
	Q_INT32 importImage(bool createLayer, bool modal = false, const QString& filename = QString::null);

signals:
	void bgColorChanged(const KoColor& c);
	void fgColorChanged(const KoColor& c);
	void cursorPosition(Q_INT32 xpos, Q_INT32 ypos);
	void cursorEnter();
	void cursorLeave();

public slots:
	void dialog_gradient();
	void slotSetFGColor(const KoColor& c);
	void slotSetBGColor(const KoColor& c);

	void next_layer();
	void previous_layer();

	// image action slots
	void slotImportImage();
	void export_image();
	void imgResizeToActiveLayer();
	void add_new_image_tab();
	void remove_current_image_tab();
	void imageResize();

	// tool action slots
	void tool_properties();

	// settings action slots
	void preferences();
    void copy();
	void cut();
	void removeSelection();
	void paste();
	void copySelectionToNewLayer();
	void layer_rotate180();
	void layer_rotateleft90();
	void layer_rotateright90();
	void layer_rotate_custom();
	void layer_mirrorX();
	void layer_mirrorY();
	void selectAll();
	void unSelectAll();
protected:
	virtual void resizeEvent(QResizeEvent*);

private:
	// Implement KisCanvasSubject
	virtual void attach(KisCanvasObserver *observer);
	virtual void detach(KisCanvasObserver *observer);
	virtual void notify();
	virtual KisImageSP currentImg() const;
	virtual QString currentImgName() const;
	virtual KoColor bgColor() const;
	virtual void setBGColor(const KoColor& c);
	virtual KoColor fgColor() const;
	virtual void setFGColor(const KoColor& c);
	virtual KisBrush *currentBrush() const;
	virtual KisPattern *currentPattern() const;
	virtual KisGradient *currentGradient() const;
	virtual double zoomFactor() const;
	virtual KisUndoAdapter *undoAdapter() const;
	virtual KisCanvasControllerInterface *canvasController() const;
	virtual KisToolControllerInterface *toolController() const;
	virtual KoDocument *document() const;

private:
	// Implement KisCanvasControllerInterface
	virtual QWidget *canvas() const;
	virtual Q_INT32 horzValue() const;
	virtual Q_INT32 vertValue() const;
	virtual void updateCanvas();
	virtual void updateCanvas(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);
	virtual void updateCanvas(const QRect& rc);
	virtual void updateCanvas(const KisRect& rc);
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
	void fillSelection(const KoColor& c, QUANTUM opacity);
	void layerUpdateGUI(bool enable);
	void paintView(const KisRect& rc);
	void selectionUpdateGUI(bool enable);
	bool selectColor(KoColor& result);
	void selectImage(KisImageSP img);
	void setupActions();
	void setupCanvas();
	void setupClipboard();
	void setupRulers();
	void setupScrollBars();
	void setupDockers();
	void setupTabBar();
        void updateStatusBarZoomLabel();
	void setupStatusBar();
	void setupTools();
	void zoomUpdateGUI(Q_INT32 x, Q_INT32 y, double zf);

private slots:
	void clipboardDataChanged();
	void duplicateCurrentImg();
	void fillSelectionBg();
	void fillSelectionFg();
	void paste_into();
    void popupTabBarMenu( const QPoint& );
    void moveImage( unsigned, unsigned );
    void slotRename();


	void canvasGotMousePressEvent(QMouseEvent *e);
	void canvasGotMouseMoveEvent(QMouseEvent *e);
	void canvasGotMouseReleaseEvent(QMouseEvent *e);
	void canvasGotTabletEvent(QTabletEvent *e);
	void canvasGotPaintEvent(QPaintEvent *e);
	void canvasGotEnterEvent(QEvent *e);
	void canvasGotLeaveEvent(QEvent *e);
	void canvasGotMouseWheelEvent(QWheelEvent *e);
	void canvasRefresh();
	void canvasGotKeyPressEvent(QKeyEvent*);
	void canvasGotKeyReleaseEvent(QKeyEvent*);
	void docImageListUpdate();
	void floatSidebar();
	void imgSelectionChanged(KisImageSP img);
	void layerToggleVisible();
	void layerSelected(int n);
	void layerToggleLinked();
	void layerProperties();
	void layerResize();
	void layerResizeToImage();
	void layerToImage();
	void layerTransform();
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
	void layersUpdated();
	void layersUpdated(KisImageSP img);
	void layerTransform(bool smooth);

	void placeSidebarLeft();
	QPoint mapToScreen(const QPoint& pt);
	void merge_all_layers();
	void merge_visible_layers();
	void merge_linked_layers();
	void nBuilders(Q_INT32 size);
	void save_layer_as_image();
	void projectionUpdated(KisImageSP img);
	void selectFGColor();
	void selectBGColor();
	void reverseFGAndBGColors();
	void reset();
	void selectImage(const QString&);
	void brushActivated(KisResource *brush);
	void patternActivated(KisResource *pattern);
	void setPaintOffset();
	void scrollH(int value);
	void scrollTo(Q_INT32 x, Q_INT32 y);
	void scrollV(int value);
	void slotEmbedImage(const QString& filename);
	void showSidebar();
	void slotInsertImageAsLayer();
	void imgUpdated(KisImageSP img, const QRect& rc);
	void slotZoomIn();
	void slotZoomOut();
	void viewColorDocker();
	void viewControlDocker();
	void viewLayerChannelDocker();
	void viewResourceDocker();
	void updateTabBar();

private:
	KisDoc *m_doc;
	KisCanvas *m_canvas;

        // Fringe benefits
	KoTabBar *m_tabBar;
	QButton *m_tabFirst;
	QButton *m_tabLeft;
	QButton *m_tabRight;
	QButton *m_tabLast;

	KisRuler *m_hRuler;
	KisRuler *m_vRuler;

        // Actions
	KAction *m_zoomIn;
	KAction *m_zoomOut;
	KAction *m_imgRm;
	KAction *m_imgResize;
	KAction *m_imgDup;
	KAction *m_imgImport;
	KAction *m_imgExport;
	KAction *m_imgScan;
	KAction *m_imgResizeToLayer;
	KAction *m_imgMergeAll;
	KAction *m_imgMergeVisible;
	KAction *m_imgMergeLinked;
	KAction *m_layerAdd;
	KAction *m_layerRm;
	KAction *m_layerDup;
	KAction *m_layerLink;
	KAction *m_layerHide;
	KAction *m_layerProperties;
	KAction *m_layerSaveAs;
	KAction *m_layerResize;
	KAction *m_layerResizeToImage;
	KAction *m_layerToImage;
	KAction *m_layerTransform;
	KAction *m_layerRaise;
	KAction *m_layerLower;
	KAction *m_layerTop;
	KAction *m_layerBottom;
	KAction *m_selectionCut;
	KAction *m_selectionCopy;
	KAction *m_selectionPaste;
	KAction *m_selectionPasteInto;
	KAction *m_selectionToNewLayer;
	KAction *m_selectionFillBg;
	KAction *m_selectionFillFg;
	KAction *m_selectionRm;
	KAction *m_selectionSelectAll;
	KAction *m_selectionSelectNone;
    KAction *m_imgRename;
	DCOPObject *m_dcop;

        // Widgets
	QScrollBar *m_hScroll; // XXX: the sizing of the scrollthumbs
	QScrollBar *m_vScroll; // is not right yet.

        //Dockers
	DockFrameDocker *m_layerchanneldocker;
	DockFrameDocker *m_resourcedocker;
	DockFrameDocker *m_toolcontroldocker;
	ColorDocker *m_colordocker;

	QWidget *m_paletteChooser;
	QWidget *m_gradientChooser;
	QWidget *m_imageChooser;

	ControlFrame *m_controlWidget;
	KisChannelView *m_channelView;
	QWidget *m_pathView;
	KisBuilderMonitor *m_imgBuilderMgr;
	KisLabelBuilderProgress *m_buildProgress;
	KisResourceMediator *m_brushMediator;
	KisResourceMediator *m_patternMediator;

        // Current colours, brushes, patterns etc.
        Q_INT32 m_xoff;
        Q_INT32 m_yoff;
	KoColor m_fg;
	KoColor m_bg;
	KisBrush *m_brush;
	KisPattern *m_pattern;
	KisGradient *m_gradient;
	KisListBox *m_layerBox;
	KisTool *m_tool;
	bool m_clipboardHasImage;
	KisGuideSP m_currentGuide;
	QPoint m_lastGuidePoint;
	KisUndoAdapter *m_adapter;
	vKisCanvasObserver m_observers;

        QLabel *m_statusBarZoomLabel;

private:
	mutable KisImageSP m_current;
};

#endif // KIS_VIEW_H_


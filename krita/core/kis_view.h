/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *                1999 Michael Koch    <koch@kde.org>
 *                1999 Carsten Pfeiffer <pfeiffer@kde.org>
 *                2002 Patrick Julien <freak@codepimps.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#if !defined KIS_VIEW_H_
#define KIS_VIEW_H_

#include <koColor.h>
#include <koView.h>
#include "kis_global.h"
#include "kis_types.h"

class QButton;
class QPaintEvent;
class QScrollBar;
class QWidget;

class DCOPObject;
class KPrinter;
class KRuler;
class KToggleAction;

class KisCanvas;
class KisDoc;
class KisGradient;
class KisGridViewFrame;
class KisPainter;
class KisSideBar;

class KisBrushChooser;
class KisPatternChooser;
class KisKrayonChooser;

class KisKrayon;
class KisBrush;
class KisPattern;
class KisTabBar;
class KisChannelView;
class KisListBoxView;

class KisView : public KoView {
	Q_OBJECT
	typedef KoView super;

public:
	KisView(KisDoc *doc, QWidget *parent = 0, const char *name = 0);
	virtual ~KisView();

public:
	// Overide KoView
	virtual QWidget *canvas();
	virtual int canvasXOffset() const;
	virtual int canvasYOffset() const;
	virtual DCOPObject* dcopObject();
	virtual void print(KPrinter &printer);
	virtual void setupPrinter(KPrinter &printer);
	virtual void updateReadWrite(bool readwrite);

public:  
	Q_INT32 docWidth() const;
	Q_INT32 docHeight() const;
	Q_INT32 importImage(bool createLayer, const QString& filename = QString::null);
	void updateCanvas();
	void updateCanvas(const QRect& rc);
//	void zoom(int x, int y, float zf);
	void zoom_in(int x, int y);
	void zoom_out(int x, int y);

//	void setActiveTool(KisTool *tool);
//	KisTool* getActiveTool();

	void setupPainter();
	void setupDialogs();
	void setupTools();


	KoColor fgColor();
	KoColor bgColor();

	KisPainter  *kisPainter();
	KisPattern  *currentPattern();
	KisBrush    *currentBrush();

//	void activateTool(KisTool*);
	void layerScale(bool smooth);
        
//	Q_INT32 xScrollOffset();
//	Q_INT32 yScrollOffset();

	void scrollTo(QPoint p);

	void save_layer_image(bool mergeLayers);


	void setSetFGColor(const KoColor& c);
	void setSetBGColor(const KoColor& c);

	void setCanvasCursor(const QCursor& cursor);

signals:
	void bgColorChanged(const KoColor&);
	void fgColorChanged(const KoColor&);     

public slots:
	void slotRefreshPainter();
	void slotUpdateImage();

	void slotDocUpdated();
	void slotDocUpdated(const QRect&);

	void slotSetKrayon(KisKrayon *);
	void slotSetBrush(KisBrush *);
	void slotSetPattern(KisPattern *);
	void slotSetFGColor(const KoColor& c);
	void slotSetBGColor(const KoColor& c);

	void slotTabSelected(const QString& name);

	void slotHalt(); // for the faint of heart
    
	void zoom_in();
	void zoom_out();
     
	// edit action slots
	void copy();
	void cut();
	void removeSelection();
	void paste();
	void crop();
	void selectAll();
	void unSelectAll();

	// dialog action slots
	void dialog_gradient();
	void dialog_colors();
	void dialog_crayons();
	void dialog_brushes();
	void dialog_patterns();
	void dialog_layers();
	void dialog_channels();

	// layer action slots
	void insert_layer();
	void remove_layer();
	void link_layer();
	void hide_layer();
	void next_layer();
	void previous_layer();

	void layer_properties(); 

	void slotInsertImageAsLayer();
	void save_layer_as_image();
	void slotEmbeddImage(const QString& filename);

	void layer_scale_smooth();
	void layer_scale_rough();
	void layer_rotate180();
	void layer_rotateleft90();
	void layer_rotateright90();
	void layer_rotate_custom();
	void layer_mirrorX();
	void layer_mirrorY();

	// image action slots
	void slotImportImage();
	void export_image();
	void add_new_image_tab();
	void remove_current_image_tab();
	void merge_all_layers();
	void merge_visible_layers();
	void merge_linked_layers();

	// tool action slots
	void tool_properties();

	// settings action slots
	void showMenubar();
	void showToolbar();
	void showStatusbar();
	void floatSidebar();
	void showSidebar();
	void leftSidebar();
	void saveOptions();
	void preferences();

protected:
	virtual void resizeEvent(QResizeEvent*);

private:
	void clearCanvas(const QRect& rc);
	Q_INT32 horzValue() const;
	void paintView(const QRect& rc);
	void setupActions();
	void setupCanvas();
	void setupRulers();
	void setupScrollBars();
	void setupSideBar();
	void setupTabBar();
	Q_INT32 vertValue() const;

private slots:
	void canvasGotMousePressEvent(QMouseEvent *);
	void canvasGotMouseMoveEvent(QMouseEvent *);
	void canvasGotMouseReleaseEvent(QMouseEvent *);
	void canvasGotPaintEvent(QPaintEvent *);
	void canvasGotEnterEvent(QEvent *);
	void canvasGotLeaveEvent(QEvent *);
	void canvasGotMouseWheelEvent(QWheelEvent *);
	void docImageListUpdate();
	void layerToggleVisible(int n);
	void layerSelected(int n);
	void layerToggleLinked(int n);
	void layerProperties(int n);
	void layerAdd();
	void layerRemove(int n);
	void layerAddMask(int n);
	void layerRmMask(int n);
	void layerRaise(int n);
	void layerLower(int n);
	void layerFront(int n);
	void layerBack(int n);
	void layerLevel(int n);
	void layersUpdated();
	void setPaintOffset();
	void scrollH(int);
	void scrollV(int);

private:
	KisDoc *m_doc;
	KisCanvas *m_canvas;
	KisTabBar *m_tabBar;
	QButton *m_tabFirst; 
	QButton *m_tabLeft; 
	QButton *m_tabRight; 
	QButton *m_tabLast;
	KisSideBar *m_sideBar;
	KRuler *m_hRuler;
	KRuler *m_vRuler;
	KAction *m_zoomIn;
	KAction *m_zoomOut;
	KToggleAction *m_sidebarToggle; 
	KToggleAction *m_floatsidebarToggle; 
	KToggleAction *m_lsidebarToggle;
	KToggleAction *m_dlgColorsToggle;
       	KToggleAction *m_dlgCrayonToggle; 
	KToggleAction *m_dlgBrushToggle;
	KToggleAction *m_dlgPatternToggle; 
	KToggleAction *m_dlgLayersToggle; 
	KToggleAction *m_dlgChannelsToggle;
	QScrollBar *m_hScroll; 
	QScrollBar *m_vScroll;
	DCOPObject *m_dcop;
	Q_INT32 m_xoff;
	Q_INT32 m_yoff;    

#if 0
	// krayon box (sidebar)

	KisDoc *m_doc;  // always needed
	KisToolSP m_pTool; // current active tool
	KisToolSP m_paste; // The special paste tool

	// krayon objects - all can be krayons
	KisKrayon     *m_pKrayon;   // current krayon for this view   
	KisBrush      *m_pBrush;    // current brush for this view
	KisPattern    *m_pPattern;  // current pattern for this view 
	KisGradient   *m_pGradient; // current gradient   
	KisImage      *m_pImage;    // current image for this view

	// sidebar dock widgets
	KisKrayonChooser     *m_pKrayonChooser;    
	KisBrushChooser      *m_pBrushChooser;
	KisPatternChooser    *m_pPatternChooser;
	QWidget              *m_pPaletteChooser;    
	QWidget              *m_pGradientChooser;
	QWidget              *m_pImageChooser;
	KisChannelView	 *m_pChannelView;

	// krayon and kde objects
	KisPainter          *m_pPainter;
	KoColor            m_fg;
	KoColor m_bg;

	// normal variables
	float	    m_zoomFactor;

#endif
//	KisListBoxView *m_layerView;

};

#endif // KIS_VIEW_H_


/*
 *  kis_view.h - part of Krayon
 *
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *                1999 Michael Koch    <koch@kde.org>
 *                1999 Carsten Pfeiffer <pfeiffer@kde.org>
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

#ifndef __kis_view_h__
#define __kis_view_h__

#include <qscrollbar.h>
#include <qwidget.h>

#include <koView.h>

#include "kis_global.h"
#include "kis_color.h"
#include "kis_tool.h"

class QButton;
class QPaintEvent;
class QScrollBar;

class KAction;
class KRuler;
class KToggleAction;
class KHelpMenu;
class DCOPObject;

class KisDoc;
class KisImage;
class KisCanvas;
class KisPainter;
class KisTabBar;
class KisSideBar;
class KisGradient;

class KisBrushChooser;
class KisPatternChooser;
class KisKrayonChooser;

class KisLayerView;
class KisChannelView;

class PasteTool;
class KisKrayon;
class KisBrush;
class KisPattern;
class KisTool;

class KisView : public KoView {
	Q_OBJECT
	typedef KoView super;

public:  
	KisView(KisDoc *doc, QWidget *parent = 0, const char *name = 0);
	virtual ~KisView();

	inline void setActiveTool(KisTool *tool);
	inline KisTool* getActiveTool();
	inline KisDoc* getKisDocument();

	void setupPixmap();
	void setupPainter();
	void setupCanvas();
	void setupSideBar();
	void setupScrollBars();
	void setupRulers();
	void setupDialogs();
	void setupActions();
	void setupTabBar();
	void setupTools();

	virtual DCOPObject* dcopObject();
	virtual void updateReadWrite(bool readwrite);
	virtual void setupPrinter(KPrinter &printer);
	virtual void print(KPrinter &printer);

	KisColor& fgColor() { return m_fg; }
	KisColor& bgColor() { return m_bg; }

	KisCanvas   *kisCanvas()        { return m_pCanvas; }
	KisPainter  *kisPainter()       { return m_pPainter; }
	KisPattern  *currentPattern()   { return m_pPattern; }
	KisBrush    *currentBrush()     { return m_pBrush; }

	void activateTool(KisTool*);
	void updateCanvas(const QRect& rc);
	void layerScale(bool smooth);
        
	int docWidth();
	int docHeight();

	int xPaintOffset();
	int yPaintOffset();
     
	int xScrollOffset() { return m_pHorz->value(); }
	int yScrollOffset() { return m_pVert->value(); }
	void scrollTo(QPoint p);

	int insert_layer_image(bool newLayer, const QString &filename = QString::null);
	void save_layer_image(bool mergeLayers);

	void zoom(int x, int y, float zf);
	void zoom_in(int x, int y);
	void zoom_out(int x, int y);
	float zoomFactor() const;
	void setZoomFactor(float zf);

signals:
	void bgColorChanged(const KisColor &);
	void fgColorChanged(const KisColor &);     

public slots:
	void slotRefreshPainter();
	void slotUpdateImage();

	void slotDocUpdated();
	void slotDocUpdated(const QRect&);

	void slotSetKrayon(KisKrayon *);
	void slotSetBrush(KisBrush *);
	void slotSetPattern(KisPattern *);
	void slotSetFGColor(const KisColor&);
	void slotSetBGColor(const KisColor&);
	void slotSetPaintOffset();

	void slotTabSelected(const QString& name);

	void slotHalt(); // for the faint of heart
	void slotGimp(); // for the weak of mind
    
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

	void insert_image_as_layer();
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
	void import_image();
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

protected slots:
	// scrollbar slots
	void scrollH(int);
	void scrollV(int);

	void canvasGotMousePressEvent(QMouseEvent *);
	void canvasGotMouseMoveEvent (QMouseEvent *);
	void canvasGotMouseReleaseEvent (QMouseEvent *);
	void canvasGotPaintEvent(QPaintEvent *);
	void canvasGotEnterEvent(QEvent *);
	void canvasGotLeaveEvent(QEvent *);
	void canvasGotMouseWheelEvent(QWheelEvent *);

protected:
	virtual void resizeEvent(QResizeEvent*);

	void appendToDocImgList(QImage& loadedImg, KURL& u);
	void addHasNewLayer(QImage& loadedImg, KURL& u);
	void paintView(const QRect& rc);
	void clearCanvas(const QRect& rc);
             
protected:
   	// krayon box floating dialog actions
	ktvector m_tools;

	KToggleAction *m_dialog_colors, *m_dialog_krayons, *m_dialog_brushes,
                 *m_dialog_patterns, *m_dialog_layers, *m_dialog_channels;

	// krayon box (sidebar)
	KToggleAction *m_side_bar, *m_float_side_bar, *m_lsidebar;

	KisDoc *m_doc;  // always needed
	KisTool *m_pTool; // current active tool
	KisTool *m_paste; // The special paste tool

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
	KisLayerView         *m_pLayerView;
	KisChannelView       *m_pChannelView;

	// krayon and kde objects
	KisCanvas           *m_pCanvas;
	KisPainter          *m_pPainter;
	KisSideBar          *m_pSideBar;
	QScrollBar          *m_pHorz, *m_pVert;
	KRuler              *m_pHRuler, *m_pVRuler;
	KisColor            m_fg, m_bg;
	KisTabBar           *m_pTabBar;
	QButton             *m_pTabFirst, *m_pTabLeft, 
	*m_pTabRight, *m_pTabLast;

	// what's this doing here?
	KHelpMenu            *m_helpMenu;

	// normal variables
	float	    m_zoomFactor;
	int         m_xPaintOffset;
	int         m_yPaintOffset;    

	DCOPObject *m_dcop;
};

void KisView::setActiveTool(KisTool *tool)
{
	//assert(tool == 0 || (tool && m_pTools.find(tool) != -1));
	m_pTool = tool;
}

KisTool* KisView::getActiveTool()
{
	return m_pTool;
}

KisDoc* KisView::getKisDocument()
{
	return m_doc;
}

#endif

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
class KAction;
class KPrinter;
class KRuler;
class KToggleAction;
class KoIconItem;
class KisBrush;
class KisCanvas;
class KisChannelView;
class KisCrayon;
class KisDoc;
class KisGradient;
class KisItemChooser;
class KisListBox;
class KisPattern;
class KisSideBar;
class KisTabBar;

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
	KoColor bgColor();
	KoColor fgColor();
	KisBrush *currentBrush();
	KisImageSP currentImg() const;
	KisPattern *currentPattern();
	QString currentImgName() const;
	Q_INT32 docWidth() const;
	Q_INT32 docHeight() const;
	Q_INT32 importImage(bool createLayer, const QString& filename = QString::null);
	Q_INT32 exportImage(bool mergeLayers,  const QString& filename = QString::null);
	void updateCanvas();
	void updateCanvas(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);
	void updateCanvas(const QRect& rc);
	void setBGColor(const KoColor& c);
	void setFGColor(const KoColor& c);
	void setCanvasCursor(const QCursor& cursor);
	void zoomIn(Q_INT32 x, Q_INT32 y);
	void zoomOut(Q_INT32 x, Q_INT32 y);

signals:
	void bgColorChanged(const KoColor& c);
	void fgColorChanged(const KoColor& c);     

public slots:
	void dialog_gradient();
	void dialog_colors();
	void dialog_crayons();
	void dialog_brushes();
	void dialog_patterns();
	void dialog_layers();
	void dialog_channels();
	void slotSetBGColor(const KoColor& c);
	void slotSetFGColor(const KoColor& c);
	void zoomIn();
	void zoomOut();
     
	void next_layer();
	void previous_layer();


	void layer_scale_smooth();
	void layer_scale_rough();
	void layerScale(bool smooth);
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

	// tool action slots
	void tool_properties();

	// settings action slots
	void saveOptions();
	void preferences();

protected:
	virtual void resizeEvent(QResizeEvent*);

private:
	void clearCanvas(const QRect& rc);
	Q_INT32 horzValue() const;
	void layerUpdateGUI(bool enable);
	void paintView(const QRect& rc);
	void selectImage(KisImageSP img);
	void setupActions();
	void setupCanvas();
	void setupRulers();
	void setupScrollBars();
	void setupSideBar();
	void setupTabBar();
	void setupTools();
	Q_INT32 vertValue() const;
	void zoomUpdateGUI(Q_INT32 x, Q_INT32 y, double zf);

private slots:
	void copy();
	void cut();
	void removeSelection();
	void paste();
	void crop();
	void selectAll();
	void unSelectAll();
	void canvasGotMousePressEvent(QMouseEvent *e);
	void canvasGotMouseMoveEvent(QMouseEvent *e);
	void canvasGotMouseReleaseEvent(QMouseEvent *e);
	void canvasGotPaintEvent(QPaintEvent *e);
	void canvasGotEnterEvent(QEvent *e);
	void canvasGotLeaveEvent(QEvent *e);
	void canvasGotMouseWheelEvent(QWheelEvent *e);
	void canvasRefresh();
	void docImageListUpdate();
	void floatSidebar();
	void layerToggleVisible(int n);
	void layerSelected(int n);
	void layerToggleLinked();
	void layerProperties();
	void layerAdd();
	void layerRemove();
	void layerAddMask(int n);
	void layerRmMask(int n);
	void layerRaise();
	void layerLower();
	void layerFront();
	void layerBack();
	void layerLevel(int n);
	void layersUpdated();
	void placeSidebarLeft();
	void merge_all_layers();
	void merge_visible_layers();
	void merge_linked_layers();
	void save_layer_as_image();
	void reset();
	void selectImage(const QString&);
	void setActiveBrush(KoIconItem *brush);
	void setActiveCrayon(KoIconItem *);
	void setActivePattern(KoIconItem *pattern);
	void setPaintOffset();
	void scrollH(int value);
	void scrollTo(Q_INT32 x, Q_INT32 y);
	void scrollV(int value);
	void slotEmbedImage(const QString& filename);
	void showMenubar();
	void showSidebar();
	void showStatusbar();
	void showToolbar();
	void slotInsertImageAsLayer();

private:
	KisDoc *m_doc;
	KisCanvas *m_canvas;
	KisTabBar *m_tabBar;
	QButton *m_tabFirst; 
	QButton *m_tabLeft; 
	QButton *m_tabRight; 
	QButton *m_tabLast;
	KRuler *m_hRuler;
	KRuler *m_vRuler;
	KAction *m_zoomIn;
	KAction *m_zoomOut;
	KAction *m_layerRm;
	KAction *m_layerLink;
	KAction *m_layerHide;
	KAction *m_layerProperties;
	KAction *m_layerNext;
	KAction *m_layerPrev;
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
	KoColor m_fg;
	KoColor m_bg;
	KisSideBar *m_sideBar;
	KisItemChooser *m_brushChooser;
	KisItemChooser *m_crayonChooser;    
	KisItemChooser *m_patternChooser;
	QWidget *m_paletteChooser;    
	QWidget *m_gradientChooser;
	QWidget *m_imageChooser;
	KisBrush *m_brush;
	KisCrayon *m_crayon;
	KisPattern *m_pattern;
	KisGradient *m_gradient;
	KisListBox *m_layerBox;
	KisChannelView *m_channelView;

private:
	mutable KisImageSP m_current;
};

#endif // KIS_VIEW_H_


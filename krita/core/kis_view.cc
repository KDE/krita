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
// Qt
#include <qapplication.h>
#include <qbutton.h>
#include <qevent.h>
#include <qpainter.h>
#include <qscrollbar.h>
#include <qspinbox.h>
#include <qlineedit.h>

// KDE
#include <dcopobject.h>
#include <kaction.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knotifyclient.h>
#include <kpushbutton.h>
#include <kruler.h>
#include <kstdaction.h>

// KOffice
#include <koColor.h>
#include <koView.h>

// Local
#include "kis_doc.h"
#include "kis_canvas.h"
#include "kis_channelview.h"
#include "kis_dlg_new_layer.h"
#include "kis_dlg_paintoffset.h"
#include "kis_image_builder.h"
#include "kis_itemchooser.h"
#include "kis_factory.h"
#include "kis_painter.h"
#include "kis_layer.h"
#include "kis_listbox.h"
#include "kis_paint_device.h"
#include "kis_dlg_paint_properties.h"
#include "kis_resourceserver.h"
#include "kis_sidebar.h"
#include "kis_tabbar.h"
#include "kis_tool.h"
#include "kis_tool_factory.h"
#include "kis_view.h"
#include "kis_util.h"

#define KISVIEW_MIN_ZOOM (1.0 / 16.0)
#define KISVIEW_MAX_ZOOM 16.0

KisView::KisView(KisDoc *doc, QWidget *parent, const char *name) : super(doc, parent, name)
{
	if (!doc -> isReadWrite())
		setXMLFile("krita_readonly.rc");
	else
		setXMLFile("krita.rc");

	m_doc = doc;
	m_canvas = 0;
	m_tabBar = 0;
	m_tabFirst = 0;
	m_tabLeft = 0;
	m_tabRight = 0;
	m_tabLast = 0;
	m_hRuler = 0;
	m_vRuler = 0;
	m_zoomIn = 0;
	m_zoomOut = 0;
	m_layerRm = 0;
	m_layerLink = 0;
	m_layerHide = 0;
	m_layerProperties = 0;
	m_layerNext = 0;
	m_layerPrev = 0;
	m_sidebarToggle = 0;
	m_floatsidebarToggle  = 0;
	m_lsidebarToggle = 0;
	m_dlgColorsToggle = 0;
       	m_dlgCrayonToggle = 0;
	m_dlgBrushToggle = 0;
	m_dlgPatternToggle = 0;
	m_dlgLayersToggle = 0;
	m_dlgChannelsToggle = 0;
	m_hScroll = 0;
	m_vScroll = 0;
	m_dcop = 0;
	m_xoff = 0;
	m_yoff = 0;
	m_fg = KoColor::black();
	m_bg = KoColor::white();
	m_sideBar = 0;
	m_brushChooser = 0;
	m_crayonChooser = 0;
	m_patternChooser = 0;
	m_paletteChooser = 0;
	m_gradientChooser = 0;
	m_imageChooser = 0;
	m_brush = 0;
	m_crayon = 0;
	m_pattern = 0;
	m_gradient = 0;
	m_layerBox = 0;
	setInstance(KisFactory::global());
	setupActions();
	setupCanvas();
	setupRulers();
	setupScrollBars();
	setupSideBar();
	setupTabBar();
	dcopObject();
	connect(m_doc, SIGNAL(imageListUpdated()), SLOT(docImageListUpdate()));
	connect(m_doc, SIGNAL(layersUpdated(KisImageSP)), SLOT(layersUpdated(KisImageSP)));
	connect(m_doc, SIGNAL(projectionUpdated(KisImageSP)), SLOT(projectionUpdated(KisImageSP)));
	m_toolSet = toolFactory(this, doc);
}

KisView::~KisView()
{
	delete m_dcop;
}

DCOPObject* KisView::dcopObject()
{
#if 0
	if (!m_dcop)
		m_dcop = new KRayonViewIface(this);

	return m_dcop;
#endif
	return 0;
}

void KisView::setupSideBar()
{
	m_sideBar = new KisSideBar(this, "kis_sidebar");

	m_crayonChooser = new KisItemChooser(KisFactory::rServer() -> brushes(), false, m_sideBar -> dockFrame(), "crayon_chooser");
	m_crayonChooser -> addItem(KisFactory::rServer() -> patterns());
//	m_pKrayon = m_crayonChooser -> currentKrayon();
	QObject::connect(m_crayonChooser, SIGNAL(selected(KoIconItem*)), this, SLOT(setActiveCrayon(KoIconItem*)));
	m_crayonChooser -> setCaption(i18n("Crayons"));
	m_sideBar -> plug(m_crayonChooser);

	m_brushChooser = new KisItemChooser(KisFactory::rServer() -> brushes(), true, m_sideBar -> dockFrame(), "brush_chooser");
	m_brush = dynamic_cast<KisBrush*>(m_brushChooser -> currentItem());
	QObject::connect(m_brushChooser, SIGNAL(selected(KoIconItem*)), this, SLOT(setActiveBrush(KoIconItem*)));
	m_brushChooser -> setCaption(i18n("Brushes"));
	m_sideBar -> plug(m_brushChooser);

	m_patternChooser = new KisItemChooser(KisFactory::rServer() -> patterns(), true, m_sideBar -> dockFrame(), "pattern_chooser");
//	m_pPattern = m_patternChooser -> currentPattern();
	QObject::connect(m_patternChooser, SIGNAL(selected(KoIconItem*)), this, SLOT(setActivePattern(KoIconItem*)));
	m_patternChooser -> setCaption(i18n("Patterns"));
	m_sideBar -> plug(m_patternChooser);

	m_gradientChooser = new QWidget(this);
//	m_gradient = new KisGradient;
	m_gradientChooser -> setCaption(i18n("Gradients"));
	m_sideBar -> plug(m_gradientChooser);

	m_imageChooser = new QWidget(this);
	m_imageChooser -> setCaption(i18n("Images"));
	m_sideBar -> plug(m_imageChooser);

	m_paletteChooser = new QWidget(this);
	m_paletteChooser -> setCaption(i18n("Palettes"));
	m_sideBar -> plug(m_paletteChooser);

	m_layerBox = new KisListBox(i18n("layer"), KisListBox::SHOWALL, m_sideBar);
	m_layerBox -> setCaption(i18n("Layers"));

	connect(m_layerBox, SIGNAL(itemToggleVisible()), SLOT(layerToggleVisible()));
	connect(m_layerBox, SIGNAL(itemSelected(int)), SLOT(layerSelected(int)));
	connect(m_layerBox, SIGNAL(itemToggleLinked()), SLOT(layerToggleLinked()));
	connect(m_layerBox, SIGNAL(itemProperties()), SLOT(layerProperties()));
	connect(m_layerBox, SIGNAL(itemAdd()), SLOT(layerAdd()));
	connect(m_layerBox, SIGNAL(itemRemove()), SLOT(layerRemove()));
	connect(m_layerBox, SIGNAL(itemAddMask(int)), SLOT(layerAddMask(int)));
	connect(m_layerBox, SIGNAL(itemRmMask(int)), SLOT(layerRmMask(int)));
	connect(m_layerBox, SIGNAL(itemRaise()), SLOT(layerRaise()));
	connect(m_layerBox, SIGNAL(itemLower()), SLOT(layerLower()));
	connect(m_layerBox, SIGNAL(itemFront()), SLOT(layerFront()));
	connect(m_layerBox, SIGNAL(itemBack()), SLOT(layerBack()));
	connect(m_layerBox, SIGNAL(itemLevel(int)), SLOT(layerLevel(int)));

	m_sideBar -> plug(m_layerBox);
	layersUpdated();

	m_channelView = new KisChannelView(m_doc, this);
	m_channelView -> setCaption(i18n("Channels"));
	m_sideBar -> plug(m_channelView);

	m_sideBar -> slotActivateTab(i18n("Brushes"));
//	m_sideBar -> setActiveBrush(*m_pBrush);
	m_sideBar -> slotSetBGColor(m_bg);
	m_sideBar -> slotSetFGColor(m_fg);
	connect(m_sideBar, SIGNAL(fgColorChanged(const KoColor&)), this, SLOT(slotSetFGColor(const KoColor&)));
	connect(m_sideBar, SIGNAL(bgColorChanged(const KoColor&)), this, SLOT(slotSetBGColor(const KoColor&)));
	connect(this, SIGNAL(fgColorChanged(const KoColor&)), m_sideBar, SLOT(slotSetFGColor(const KoColor&)));
	connect(this, SIGNAL(bgColorChanged(const KoColor&)), m_sideBar, SLOT(slotSetBGColor(const KoColor&)));
	m_sidebarToggle -> setChecked(true);
}

void KisView::setupScrollBars()
{
	m_vScroll = new QScrollBar(QScrollBar::Vertical, this);
	m_hScroll = new QScrollBar(QScrollBar::Horizontal, this);
	m_vScroll -> setGeometry(width() - 16, 20, 16, height() - 36);
	m_hScroll -> setGeometry(20, height() - 16, width() - 36, 16);
	m_hScroll -> setValue(0);
	m_vScroll -> setValue(0);
	QObject::connect(m_vScroll, SIGNAL(valueChanged(int)), this, SLOT(scrollV(int)));
	QObject::connect(m_hScroll, SIGNAL(valueChanged(int)), this, SLOT(scrollH(int)));
}

void KisView::setupRulers()
{
	m_hRuler = new KRuler(Qt::Horizontal, this);
	m_vRuler = new KRuler(Qt::Vertical, this);
	m_hRuler -> setGeometry(20, 0, width() - 20, 20);
	m_vRuler -> setGeometry(0, 20, 20, height() - 20);
	m_vRuler -> setRulerMetricStyle(KRuler::Pixel);
	m_hRuler -> setRulerMetricStyle(KRuler::Pixel);
	m_vRuler -> setFrameStyle(QFrame::Panel | QFrame::Raised);
	m_hRuler -> setFrameStyle(QFrame::Panel | QFrame::Raised);
	m_hRuler -> setLineWidth(1);
	m_vRuler -> setLineWidth(1);
	m_hRuler -> setShowEndLabel(true);
	m_vRuler -> setShowEndLabel(true);
	m_hRuler -> setShowPointer(true);
	m_vRuler -> setShowPointer(true);
}

void KisView::setupTabBar()
{
	m_tabBar = new KisTabBar(this, m_doc);
	m_tabBar -> slotImageListUpdated();
	connect(m_tabBar, SIGNAL(tabSelected(const QString&)), SLOT(selectImage(const QString&)));
	QObject::connect(m_doc, SIGNAL(imageListUpdated()), m_tabBar, SLOT(slotImageListUpdated()));
	m_tabFirst = new KPushButton(this);
	m_tabLeft = new KPushButton(this);
	m_tabRight = new KPushButton(this);
	m_tabLast = new KPushButton(this);
	m_tabFirst -> setPixmap(QPixmap(BarIcon("tab_first")));
	m_tabLeft -> setPixmap(QPixmap(BarIcon("tab_left")));
	m_tabRight -> setPixmap(QPixmap(BarIcon("tab_right")));
	m_tabLast -> setPixmap(QPixmap(BarIcon("tab_last")));
	QObject::connect(m_tabFirst, SIGNAL(clicked()), m_tabBar, SLOT(slotScrollFirst()));
	QObject::connect(m_tabLeft, SIGNAL(clicked()), m_tabBar, SLOT(slotScrollLeft()));
	QObject::connect(m_tabRight, SIGNAL(clicked()), m_tabBar, SLOT(slotScrollRight()));
	QObject::connect(m_tabLast, SIGNAL(clicked()), m_tabBar, SLOT(slotScrollLast()));
}

void KisView::setupActions()
{
	// navigation actions
	(void)new KAction(i18n("Refresh Canvas"), "reload", 0, this, SLOT(canvasRefresh()), actionCollection(), "refresh_canvas");
	(void)new KAction(i18n("Reset Button"), "stop", 0, this, SLOT(reset()), actionCollection(), "panic_button");

	// selection actions
	(void)KStdAction::cut(this, SLOT(cut()), actionCollection(), "cut");
	(void)KStdAction::copy(this, SLOT(copy()), actionCollection(), "copy");
	(void)KStdAction::paste(this, SLOT(paste()), actionCollection(), "paste_special");
	(void)new KAction(i18n("Remove Selection"), "remove", 0, this, SLOT(removeSelection()), actionCollection(), "remove");
	(void)new KAction(i18n("Copy Selection to New Layer"), "crop", 0,  this, SLOT(crop()), actionCollection(), "crop");
	(void)KStdAction::selectAll(this, SLOT(selectAll()), actionCollection(), "select_all");
	(void)new KAction(i18n("Select None"), 0, this, SLOT(unSelectAll()), actionCollection(), "select_none");

	// import/export actions
	(void)new KAction(i18n("Import Image..."), "wizard", 0, this, SLOT(slotImportImage()), actionCollection(), "import_image");
	(void)new KAction(i18n("Export Image..."), "wizard", 0, this, SLOT(export_image()), actionCollection(), "export_image");

	// view actions
	m_zoomIn = new KAction(i18n("Zoom &In"), "viewmag+", 0, this, SLOT(zoomIn()), actionCollection(), "zoom_in");
	m_zoomOut = new KAction(i18n("Zoom &Out"), "viewmag-", 0, this, SLOT(zoomOut()), actionCollection(), "zoom_out");

	// tool settings actions
	(void)new KAction(i18n("&Gradient Dialog..."), "blend", 0, this, SLOT(dialog_gradient()), actionCollection(), "dialog_gradient");

	// tool actions
	(void)new KAction(i18n("&Current Tool Properties..."), "configure", 0, this, SLOT(tool_properties()), actionCollection(), "current_tool_properties");

	// layer actions
	(void)new KAction(i18n("&Add Layer..."), 0, this, SLOT(layerAdd()), actionCollection(), "insert_layer");
	m_layerRm = new KAction(i18n("&Remove Layer"), 0, this, SLOT(layerRemove()), actionCollection(), "remove_layer");
	m_layerLink = new KAction(i18n("&Link/Unlink Layer"), 0, this, SLOT(layerToggleLinked()), actionCollection(), "link_layer");
	m_layerHide = new KAction(i18n("&Hide/Show Layer"), 0, this, SLOT(layerToggleVisible()), actionCollection(), "hide_layer");
	m_layerNext = new KAction(i18n("&Next Layer"), "forward", 0, this, SLOT(next_layer()), actionCollection(), "next_layer");
	m_layerPrev = new KAction(i18n("&Previous Layer"), "back", 0, this, SLOT(previous_layer()), actionCollection(), "previous_layer");
	m_layerProperties = new KAction(i18n("Layer Properties..."), 0, this, SLOT(layerProperties()), actionCollection(), "layer_properties");
	(void)new KAction(i18n("I&nsert Image as Layer..."), 0, this, SLOT(slotInsertImageAsLayer()), actionCollection(), "insert_image_as_layer");
	(void)new KAction(i18n("Save Layer as Image..."), 0, this, SLOT(save_layer_as_image()), actionCollection(), "save_layer_as_image");

	// layer transformations - should be generic, for selection too
	(void)new KAction(i18n("Scale Layer Smoothly"), 0, this, SLOT(layer_scale_smooth()), actionCollection(), "layer_scale_smooth");
	(void)new KAction(i18n("Scale Layer - Keep Palette"), 0, this, SLOT(layer_scale_rough()), actionCollection(), "layer_scale_rough");
	(void)new KAction(i18n("Rotate &180"), 0, this, SLOT(layer_rotate180()), actionCollection(), "layer_rotate180");
	(void)new KAction(i18n("Rotate &270"), 0, this, SLOT(layer_rotateleft90()), actionCollection(), "layer_rotateleft90");
	(void)new KAction(i18n("Rotate &90"), 0, this, SLOT(layer_rotateright90()), actionCollection(), "layer_rotateright90");
	(void)new KAction(i18n("Rotate &Custom"), 0, this, SLOT(layer_rotate_custom()), actionCollection(), "layer_rotate_custom");
	(void)new KAction(i18n("Mirror &X"), 0, this, SLOT(layer_mirrorX()), actionCollection(), "layer_mirrorX");
	(void)new KAction(i18n("Mirror &Y"), 0, this, SLOT(layer_mirrorY()), actionCollection(), "layer_mirrorY");

	// image actions
	(void)new KAction(i18n("Add New Image..."), 0, this, SLOT(add_new_image_tab()), actionCollection(), "add_new_image_tab");
	(void)new KAction(i18n("Remove Current Image"), 0, this, SLOT(remove_current_image_tab()), actionCollection(), "remove_current_image_tab");
	(void)new KAction(i18n("Merge &All Layers"), 0, this, SLOT(merge_all_layers()), actionCollection(), "merge_all_layers");
	(void)new KAction(i18n("Merge &Visible Layers"), 0, this, SLOT(merge_visible_layers()), actionCollection(), "merge_visible_layers");
	(void)new KAction(i18n("Merge &Linked Layers"), 0, this, SLOT(merge_linked_layers()), actionCollection(), "merge_linked_layers");

	// setting actions
	(void)new KAction(i18n("Paint Offset..."), "paint_offet", this, SLOT(setPaintOffset()), actionCollection(), "paint_offset");
	m_sidebarToggle = new KToggleAction(i18n("Show/Hide Sidebar"), "ok", 0, this, SLOT(showSidebar()), actionCollection(), "show_sidebar");
	m_floatsidebarToggle = new KToggleAction(i18n("Dock/Undock Sidebar"), "attach", 0, this, SLOT(floatSidebar()), actionCollection(), "float_sidebar");
	m_lsidebarToggle = new KToggleAction(i18n("Left/Right Sidebar"), "view_right", 0, this, SLOT(placeSidebarLeft()), actionCollection(), "left_sidebar");
	(void)KStdAction::saveOptions(this, SLOT(saveOptions()), actionCollection(), "save_options");
	(void)new KAction(i18n("Krita Preferences..."), "edit", 0, this, SLOT(preferences()), actionCollection(), "preferences");

	// crayon box toolbar actions - these will be used only
	// to dock and undock wideget in the crayon box
	m_dlgColorsToggle = new KToggleAction(i18n("&Colors"), "color_dialog", 0, this, SLOT(dialog_colors()), actionCollection(), "colors_dialog");
	m_dlgCrayonToggle = new KToggleAction(i18n("C&rayons"), "krayon_box", 0, this, SLOT(dialog_crayons()), actionCollection(), "crayons_dialog");
	m_dlgBrushToggle = new KToggleAction(i18n("Brushes"), "brush_dialog", 0, this, SLOT(dialog_brushes()), actionCollection(), "brushes_dialog");
	m_dlgPatternToggle = new KToggleAction(i18n("Patterns"), "pattern_dialog", 0, this, SLOT(dialog_patterns()), actionCollection(), "patterns_dialog");
	m_dlgLayersToggle = new KToggleAction(i18n("Layers"), "layer_dialog", 0, this, SLOT(dialog_layers()), actionCollection(), "layers_dialog");
	m_dlgChannelsToggle = new KToggleAction(i18n("Channels"), "channel_dialog", 0, this, SLOT(dialog_channels()), actionCollection(), "channels_dialog");

	m_dlgBrushToggle -> setChecked(true);
	m_dlgPatternToggle -> setChecked(true);
	m_dlgLayersToggle -> setChecked(true);
	m_dlgChannelsToggle -> setChecked(true);
}

void KisView::reset()
{
	zoomUpdateGUI(0, 0, 1.0);
}

void KisView::resizeEvent(QResizeEvent *)
{
	Q_INT32 rsideW = 0;
	Q_INT32 lsideW = 0;
	Q_INT32 ruler = 20;
	Q_INT32 tbarOffset = 64;
	Q_INT32 tbarBtnH = 16;
	Q_INT32 tbarBtnW = 16;
	Q_INT32 drawH;
	Q_INT32 drawW;
	Q_INT32 docW;
	Q_INT32 docH;

	if (m_sideBar && m_sidebarToggle -> isChecked() && !m_floatsidebarToggle -> isChecked()) {
		if (m_lsidebarToggle -> isChecked()) {
			lsideW = m_sideBar -> width();
			m_sideBar -> setGeometry(0, 0, lsideW, height());
		} else {
			rsideW = m_sideBar -> width();
			m_sideBar -> setGeometry(width() - rsideW, 0, rsideW, height());
		}

		m_sideBar -> show();
	}

	m_hRuler -> setGeometry(ruler + lsideW, 0, width() - ruler - rsideW - lsideW, ruler);
	m_vRuler -> setGeometry(0 + lsideW, ruler, ruler, height() - (ruler + tbarBtnH));
	m_tabFirst -> setGeometry(0 + lsideW, height() - tbarBtnH, tbarBtnW, tbarBtnH);
	m_tabLeft -> setGeometry(tbarBtnW + lsideW, height() - tbarBtnH, tbarBtnW, tbarBtnH);
	m_tabRight -> setGeometry(2 * tbarBtnW + lsideW, height() - tbarBtnH, tbarBtnW, tbarBtnH);
	m_tabLast -> setGeometry(3 * tbarBtnW + lsideW, height() - tbarBtnH, tbarBtnW, tbarBtnH);
	m_tabFirst -> show();
	m_tabLeft -> show();
	m_tabRight -> show();
	m_tabLast -> show();
	drawH = height() - ruler - tbarBtnH - canvasYOffset();
	drawW = width() - ruler - lsideW - rsideW - canvasXOffset();
	docW = docWidth();
	docH = docHeight();
	docW = static_cast<Q_INT32>((zoom()) * docW);
	docH = static_cast<Q_INT32>((zoom()) * docH);
	m_vScroll -> setEnabled(docH > drawH);
	m_hScroll -> setEnabled(docW > drawW);

	if (docH <= drawH && docW <= drawW) {
		// we need no scrollbars
		m_vScroll -> hide();
		m_hScroll -> hide();
		m_vScroll -> setValue(0);
		m_hScroll -> setValue(0);
		m_canvas -> setGeometry(ruler + lsideW, ruler, drawW, drawH);
		m_tabBar -> setGeometry(tbarOffset + lsideW, height() - tbarBtnH, width() - rsideW - lsideW - tbarOffset, tbarBtnH);
		m_canvas -> show();
		m_tabBar -> show();
	} else if (docH <= drawH) {
		// we need a horizontal scrollbar only
		m_vScroll -> hide();
		m_vScroll -> setValue(0);
		m_hScroll -> setRange(0, static_cast<int>((docW - drawW) / zoom()));
		m_hScroll -> setGeometry(tbarOffset + lsideW + (width() - rsideW -lsideW - tbarOffset) / 2,
				height() - tbarBtnH,
				(width() - rsideW -lsideW - tbarOffset) / 2,
				tbarBtnH);
		m_canvas -> setGeometry(ruler + lsideW, ruler, drawW, drawH);
		m_tabBar -> setGeometry(tbarOffset + lsideW, height() - tbarBtnH, (width() - rsideW - lsideW - tbarOffset) / 2, tbarBtnH);
		m_hScroll -> show();
		m_canvas -> show();
		m_tabBar -> show();
	} else if(docW <= drawW) {
		// we need a vertical scrollbar only
		m_hScroll -> hide();
		m_hScroll -> setValue(0);
		m_vScroll -> setRange(0, static_cast<int>((docH - drawH) / zoom()));
		m_vScroll -> setGeometry(width() - tbarBtnW - rsideW, ruler, tbarBtnW, height() - (ruler + tbarBtnH));
		m_canvas -> setGeometry(ruler + lsideW, ruler, drawW - tbarBtnW, drawH);
		m_tabBar -> setGeometry(tbarOffset + lsideW, height() - tbarBtnH, width() - rsideW -lsideW - tbarOffset, tbarBtnH);
		m_vScroll -> show();
		m_canvas -> show();
		m_tabBar -> show();
	} else {
		// we need both scrollbars
		m_vScroll -> setRange(0, static_cast<int>((docH - drawH) / zoom()));
		m_vScroll -> setGeometry(width() - tbarBtnW - rsideW, ruler, tbarBtnW, height() - (ruler + tbarBtnH));
		m_hScroll -> setRange(0, static_cast<int>((docW - drawW) / zoom()));
		m_hScroll -> setGeometry(tbarOffset + lsideW + (width() - rsideW -lsideW - tbarOffset) / 2,
				height() - tbarBtnH,
				(width() - rsideW -lsideW - tbarOffset) / 2,
				tbarBtnH);
		m_canvas -> setGeometry(ruler + lsideW, ruler, drawW - tbarBtnW, drawH);
		m_tabBar -> setGeometry(tbarOffset + lsideW, height() - tbarBtnH, (width() - rsideW -lsideW - tbarOffset)/2, tbarBtnH);
		m_vScroll -> show();
		m_hScroll -> show();
		m_canvas -> show();
		m_tabBar -> show();
	}

	m_vRuler -> setRange(0, docH + static_cast<Q_INT32>(100 * zoom()));
	m_hRuler -> setRange(0, docW + static_cast<Q_INT32>(100 * zoom()));
	m_vScroll -> setLineStep(static_cast<Q_INT32>(100 * zoom()));
	m_hScroll -> setLineStep(static_cast<Q_INT32>(100 * zoom()));

	if (m_vScroll -> isVisible())
		m_vRuler -> setOffset(m_vScroll -> value());
	else
		m_vRuler -> setOffset(-canvasYOffset());

	if (m_hScroll -> isVisible())
		m_hRuler -> setOffset(m_hScroll -> value());
	else
		m_hRuler -> setOffset(-canvasXOffset());

	m_hRuler -> show();
	m_vRuler -> show();
}

/*
    updateReadWrite - for the functionally illiterate
*/
void KisView::updateReadWrite(bool /*readwrite*/)
{
}

inline
void KisView::clearCanvas(const QRect& rc)
{
	QPainter gc(m_canvas);

	gc.eraseRect(rc);
}

void KisView::activateTool(KisToolSP tool)
{
	if (tool && qFind(m_toolSet.begin(), m_toolSet.end(), tool) != m_toolSet.end()) {
		m_tool = tool;
		m_tool -> cursor(m_canvas);
	}
}

KisImageSP KisView::currentImg() const
{
	if (m_current && m_doc -> contains(m_current))
		return m_current;

	m_current = m_doc -> imageNum(0);
	return m_current;
}

QString KisView::currentImgName() const
{
	if (m_current)
		return m_current -> name();

	return QString::null;
}

inline
Q_INT32 KisView::horzValue() const
{
	return m_hScroll -> value();
}

inline
Q_INT32 KisView::vertValue() const
{
	return m_vScroll -> value();
}

void KisView::paintView(const QRect& rc)
{
	KisImageSP img = currentImg();

	if (img) {
		QPainter gc(m_canvas);
		QRect ur = rc;
		Q_INT32 xt;
		Q_INT32 yt;

		if (canvasXOffset())
			gc.eraseRect(0, 0, static_cast<Q_INT32>(canvasXOffset() * zoom()), height());

		if (canvasYOffset())
			gc.eraseRect(static_cast<Q_INT32>(canvasXOffset() * zoom()), 0, width(), static_cast<Q_INT32>(canvasYOffset() * zoom()));

		gc.eraseRect(static_cast<Q_INT32>(canvasXOffset() * zoom()), static_cast<Q_INT32>(docHeight() * zoom()) - canvasYOffset(), width(), height());
		gc.eraseRect(static_cast<Q_INT32>(docWidth() * zoom()) - canvasXOffset(), static_cast<Q_INT32>(canvasYOffset() * zoom()), width(), height());
		xt = -canvasXOffset() + horzValue();
		yt = -canvasYOffset() + vertValue();
		ur.moveBy(xt, yt);

		if (img -> width() * zoom() < ur.right())
			ur.setWidth(img -> width());

		if (img -> height() * zoom() < ur.bottom())
			ur.setHeight(img -> height());

		Q_ASSERT(ur.width() <= docWidth());
		Q_ASSERT(ur.height() <= docHeight());
		ur.setBottom(ur.bottom() + 1);
		ur.setRight(ur.right() + 1);
		xt = canvasXOffset() - horzValue();
		yt = canvasYOffset() - vertValue();

		if (zoom() < 1.0 || zoom() > 1.0)
			gc.setViewport(0, 0, static_cast<Q_INT32>(m_canvas -> width() * zoom()), static_cast<Q_INT32>(m_canvas -> height() * zoom()));

		if (xt || yt)
			gc.translate(xt, yt);

		m_doc -> setProjection(img);
		m_doc -> paintContent(gc, ur, false, 1.0, 1.0);
	} else {
		clearCanvas(rc);
	}
}

inline
void KisView::updateCanvas()
{
	QRect rc(0, 0, m_canvas -> width(), m_canvas -> height());

	updateCanvas(rc);
}

inline
void KisView::updateCanvas(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
	QRect rc(x, y, w, h);

	updateCanvas(rc);
}

void KisView::updateCanvas(const QRect& rc)
{
	QRect ur = rc;

	if (zoom() > 1.0 || zoom() < 1.0) {
		Q_INT32 urL = ur.left();
		Q_INT32 urT = ur.top();
		Q_INT32 urW = ur.width();
		Q_INT32 urH = ur.height();

		urL = static_cast<int>(static_cast<double>(urL) / zoom());
		urT = static_cast<int>(static_cast<double>(urT) / zoom());
		urW = static_cast<int>(static_cast<double>(urW) / zoom());
		urH = static_cast<int>(static_cast<double>(urH) / zoom());
		ur.setLeft(urL);
		ur.setTop(urT);
		ur.setWidth(urW);
		ur.setHeight(urH);
	}

	paintView(ur);
}

#if 0
void KisView::activateTool(KisTool* t)
{
#if 0
	if (!t)
		return;

	// remove the selection outline, if any
	// prevent old tool from receiving events from canvas
	if (m_pTool) {
		m_pTool -> clearOld();
		m_pTool -> setChecked(false);
	}

	m_pTool = t;
	m_pTool -> setChecked(true);
	m_pTool -> setBrush(m_pBrush);
	m_pTool -> setPattern(m_pPattern);

	if (m_canvas)
		m_canvas -> setCursor(m_pTool -> cursor());
#endif
}
#endif

/*
    tool_properties invokes the optionsDialog() method for the
    currentImg active tool.  There should be an options dialog for
    each tool, but these can also be consolidated into a master
    options dialog by reparenting the widgets for each tool to a
    tabbed properties dialog, each tool getting a tab  - later
*/

void KisView::tool_properties()
{
#if 0
	Q_ASSERT(m_pTool);
	m_pTool -> optionsDialog();
#endif
}

void KisView::layerUpdateGUI(bool enable)
{
	m_layerRm -> setEnabled(enable);
	m_layerLink -> setEnabled(enable);
	m_layerHide -> setEnabled(enable);
	m_layerProperties -> setEnabled(enable);
	m_layerNext -> setEnabled(enable);
	m_layerPrev -> setEnabled(enable);
}

/*---------------------------------
    edit selection action slots
----------------------------------*/

/*
    copy - copy selection contents to global kapp->clipboard()
*/

void KisView::copy()
{
#if 0
	if (!m_doc -> setClipImage())
		kdDebug() << "m_doc->setClipImage() failed" << endl;

	if (m_doc -> getClipImage()) {
		QImage cImage = *m_doc -> getClipImage();
		kapp -> clipboard() -> setImage(cImage);
       	}
#endif
}

/*
    cut - move selection contents to global kapp->clipboard()
*/

void KisView::cut()
{
#if 0
	copy();
	removeSelection();
#endif
}

/*
    same as cut but don't move selection contents to clipboard
*/

void KisView::removeSelection()
{
#if 0
	// remove selection in place
	if (!m_doc -> getSelection() -> erase())
		kdDebug() << "m_doc->m_Selection.erase() failed" << endl;

	// clear old selection outline
	m_pTool -> clearOld();
#endif
}

/*
    paste - from the global kapp->clipboard(). The image
    in the clipboard (if any) is copied to the past tool clip
    image so it can be used like a brush or stamp tool to paint
    with, or it can just be moved into place and pasted in.
*/

void KisView::paste()
{
#if 0
	if (m_doc -> getClipImage()) {
		m_paste -> setClip();
		activateTool(m_paste);
	}
	else
		KMessageBox::sorry(0, i18n("Nothing to paste!"), "", false);
#endif
}

/*
    create a new layer from the selection, same size as the
    selection (in the case of a non-rectangular selection, find
    and use the bounding rectangle for selected pixels)
*/

void KisView::crop()
{
#if 0
	if (!m_doc -> hasSelection()) {
		KMessageBox::sorry(NULL, i18n("No selection to crop!"), "", FALSE);
		return;
	}

	// copy contents of the currentImg selection to a QImage
	if (!m_doc->setClipImage()) {
		kdDebug() << "m_doc->setClipImage() failed" << endl;
		return;
	}

	// contents of currentImg selection - make sure it's good
	if (!m_doc -> getClipImage()) {
		kdDebug() << "m_doc->getClipImage() failed" << endl;
		return;
	}

	QImage cImage = *m_doc -> getClipImage();

	// add new layer same size as selection rectangle,
	// then paste cropped image into it at 0,0 offset
	// keep old image - user can remove it later if he wants
	// by removing its layer or may want to keep the original.

	KisImageSP img = currentImg();

	if (!img)
		return;

	QRect layerRect(0, 0, cImage.width(), cImage.height());
	QString name = i18n("layer %1").arg(img->layerList().size());

	img->addLayer(layerRect, white, true, name);
	uint indx = img->layerList().size() - 1;
	img->setCurrentLayer(indx);
	img->setFrontLayer(indx);

	layersUpdated();

	// copy the image into the layer - this should now
	// be handled by the framebuffer object, not the doc

	if (!m_doc->QtImageToLayer(&cImage, this)) {
		kdDebug(0) << "crop: can't load image into layer." << endl;
	}
	else {
	}

	/* remove the currentImg clip image which now belongs to the
	   previous layer - selection also is removed. To crop again,
	   you must first make another selection in the currentImg layer.*/

	m_doc->removeClipImage();
	m_doc->clearSelection();
#endif
}

/*
    selectAll - use the bounding rectangle of the layer itself
*/

void KisView::selectAll()
{
#if 0
	KisImageSP img = currentImg();

	if (img) {
		QRect rc = img -> getCurrentLayer() -> imageExtents();
		m_doc -> setSelection(rc);
	}
#endif
}

/*
    unSelectAll - clear the selection, if any
*/

void KisView::unSelectAll()
{
//	m_doc -> clearSelection();
}

void KisView::zoomUpdateGUI(Q_INT32 x, Q_INT32 y, double zf)
{
	Q_ASSERT(m_zoomIn);
	Q_ASSERT(m_zoomOut);
	setZoom(zf);
	m_zoomIn -> setEnabled(zf <= KISVIEW_MAX_ZOOM);
	m_zoomOut -> setEnabled(zf >= KISVIEW_MIN_ZOOM);

	if (zf > 3.0) {
		m_hRuler -> setPixelPerMark(static_cast<Q_INT32>(zf * 1.0));
		m_vRuler -> setPixelPerMark(static_cast<Q_INT32>(zf * 1.0));
	} else {
		Q_INT32 mark = static_cast<Q_INT32>(zf * 10.0);

		if (!mark)
			mark = 1;

		m_hRuler -> setPixelPerMark(mark);
		m_vRuler -> setPixelPerMark(mark);
	}

	m_hRuler -> setShowTinyMarks(zf <= 0.4);
	m_vRuler -> setShowTinyMarks(zf <= 0.4);
	m_hRuler -> setShowLittleMarks(zf > 0.3);
	m_vRuler -> setShowLittleMarks(zf > 0.3);
	m_hRuler -> setShowMediumMarks(zf > 0.2);
	m_vRuler -> setShowMediumMarks(zf > 0.2);

	if (x < 0 || y < 0) {
		resizeEvent(0);
		updateCanvas();
	} else {
		x = static_cast<Q_INT32>(x * zf - width() / 2);
		y = static_cast<Q_INT32>(y * zf - height() / 2);

		if (x < 0)
			x = 0;

		if (y < 0)
			y = 0;

		scrollTo(x, y);
	}
}

void KisView::zoomIn(Q_INT32 x, Q_INT32 y)
{
	zoomUpdateGUI(x, y, zoom() * 2);
}

void KisView::zoomOut(Q_INT32 x, Q_INT32 y)
{
	zoomUpdateGUI(x, y, zoom() / 2);
}

void KisView::zoomIn()
{
	zoomUpdateGUI(-1, -1, zoom() * 2);
}

void KisView::zoomOut()
{
	zoomUpdateGUI(-1, -1, zoom() / 2);
}

/*
    dialog_gradient - invokes a GradientDialog which is
    now an options dialog.  Gradients can be used by many tools
    and are not a tool in themselves.
*/

void KisView::dialog_gradient()
{
#if 0
	GradientDialog *pGradientDialog = new GradientDialog(m_pGradient);
	pGradientDialog->exec();

	if (pGradientDialog->result() == QDialog::Accepted) {
		/* set m_pGradient here and update gradientwidget in sidebar
		   to show sample of gradient selected. Also update effect for
		   the framebuffer's gradient, which is used in painting */

		int type = pGradientDialog->gradientTab()->gradientType();
		m_pGradient->setEffect(static_cast<KImageEffect::GradientType>(type));

		KisFrameBuffer *fb = m_doc->frameBuffer();
		fb->setGradientEffect(static_cast<KImageEffect::GradientType>(type));

		kdDebug() << "gradient type is " << type << endl;
	}
#endif
}

void KisView::dialog_colors()
{
}

void KisView::dialog_crayons()
{
}

void KisView::dialog_brushes()
{
	m_brushChooser -> setDocked(m_dlgBrushToggle -> isChecked());
}

void KisView::dialog_patterns()
{
#if 0
	if (m_dialog_patterns -> isChecked())
		m_sideBar -> plug(m_pPatternChooser);
	else
		m_sideBar -> unplug(m_pPatternChooser);
#endif
}

void KisView::dialog_layers()
{
#if 0
	if(m_dialog_layers -> isChecked())
		m_sideBar -> plug(m_layerBox);
	else
		m_sideBar -> unplug(m_layerBox);
#endif
}

void KisView::dialog_channels()
{
#if 0
	if (m_dialog_channels -> isChecked())
		m_sideBar -> plug(m_pChannelView);
	else
		m_sideBar -> unplug(m_pChannelView);
#endif
}

void KisView::next_layer()
{
	KisImageSP img = currentImg();
	KisLayerSP layer;

	if (!img)
		return;

	layer = img -> activeLayer();

	if (layer) {
		m_doc -> layerNext(img, layer);
		resizeEvent(0);
		updateCanvas();
	}
}

void KisView::previous_layer()
{
	KisImageSP img = currentImg();
	KisLayerSP layer;

	if (!img)
		return;

	layer = img -> activeLayer();

	if (layer) {
		m_doc -> layerPrev(img, layer);
		resizeEvent(0);
		updateCanvas();
	}
}

void KisView::slotImportImage()
{
	if (importImage(false) > 0)
		m_doc -> setModified(true);
}

void KisView::export_image()
{
//	exportImage(true);
}

void KisView::slotInsertImageAsLayer()
{
	if (importImage(true) > 0)
		m_doc -> setModified(true);
}

void KisView::save_layer_as_image()
{
//	exportImage(false);
}

void KisView::slotEmbedImage(const QString &)
{
//	importImage(false, filename);
}

Q_INT32 KisView::importImage(bool createLayer, const QString& filename)
{
	KURL url(filename);

	if (filename.isEmpty())
		url = KFileDialog::getOpenURL(QString::null, KisUtil::readFilters(), 0, i18n("Image File for Layer"));

	KisImageBuilder ib(m_doc, url);
	KisImageSP img;

	switch (ib.buildImage()) {
	case KisImageBuilder_RESULT_NO_URI:
	case KisImageBuilder_RESULT_NOT_LOCAL:
		KNotifyClient::event("cannotopenfile");
		return 0;
	case KisImageBuilder_RESULT_NOT_EXIST:
		KMessageBox::error(this, i18n("File does not exist."), i18n("Error Loading File"));
		KNotifyClient::event("cannotopenfile");
		return 0;
	case KisImageBuilder_RESULT_BAD_FETCH:
		KMessageBox::error(this, i18n("Unable to download file."), i18n("Error Loading File"));
		KNotifyClient::event("cannotopenfile");
		return 0;
	case KisImageBuilder_RESULT_EMPTY:
		KMessageBox::error(this, i18n("Empty file."), i18n("Error Loading File"));
		KNotifyClient::event("cannotopenfile");
		break;
	case KisImageBuilder_RESULT_FAILURE:
		KMessageBox::error(this, i18n("Error Loading File."), i18n("Error Loading File"));
		KNotifyClient::event("cannotopenfile");
		break;
	case KisImageBuilder_RESULT_OK:
		break;
	}

	img = ib.image();

	if (createLayer && currentImg()) {
		vKisLayerSP v = img -> layers();
		KisImageSP current = currentImg();
		Q_INT32 rc = v.size();

		for (vKisLayerSP_it it = v.begin(); it != v.end(); it++) {
			KisLayerSP layer = *it;

			layer -> setImage(current);
			layer -> setName(current -> nextLayerName());
			current -> add(layer, -1);
			current -> top(layer);
			m_layerBox -> setCurrentItem(img -> index(layer));
		}

		layersUpdated();
		current -> invalidate();
		resizeEvent(0);
		updateCanvas();
		return rc;
	}

	m_doc -> addImage(img);
	selectImage(img);
	return 1;
}

Q_INT32 KisView::exportImage(bool,  const QString&)
{
#if 0
	KURL url = KFileDialog::getSaveURL(QString::null, KisUtil::writeFilters(), 0, i18n("Image File for Layer"));

	if (!url.isEmpty()) {
		if (mergeLayers) {
			/* merge should not always remove layers -
			   merged into another but should have an option
			   for keeping old layers and merging into a new
			   one created for that purpose with a Yes/No dialog
			   to confirm, at least. */
			merge_all_layers();
		}

		//  save as standard image file (jpg, png, xpm, ppm,
		//  bmp, tiff, but NO gif due to patent restrictions)
		if (!m_doc -> saveAsQtImage(url.path(), mergeLayers))
			kdDebug(0) << "Can't save doc as image" << endl;
	}
#endif
	return 0;
}

void KisView::layer_scale_smooth()
{
//    layerScale(true);
}


void KisView::layer_scale_rough()
{
//    layerScale(false);
}


void KisView::layerScale(bool )
{
#if 0
    KisImageSP img = m_doc->currentImg();
    if (!img)  return;

    KisLayerSP lay = img->getCurrentLayer();
    if (!lay)  return;

    KisFrameBuffer *fb = m_doc->frameBuffer();
    if (!fb)  return;

    NewLayerDialog *pNewLayerDialog = new NewLayerDialog();
    pNewLayerDialog->exec();
    if(!pNewLayerDialog->result() == QDialog::Accepted)
        return;

    QRect srcR(lay->imageExtents());

    // only get the part of the layer which is inside the
    // image boundaries - layer can be bigger or can overlap
    srcR = srcR.intersect(img->imageExtents());

    bool ok;

    if(smooth)
        ok = fb->scaleSmooth(srcR,
            pNewLayerDialog->width(), pNewLayerDialog->height());
    else
        ok = fb->scaleRough(srcR,
            pNewLayerDialog->width(), pNewLayerDialog->height());

    if(!ok)
    {
        kdDebug() << "layer_scale() failed" << endl;
    }
    else
    {
        // bring new scaled layer to front
        uint indx = img->layerList().size() - 1;
        img->setCurrentLayer(indx);
        img->markDirty(img->getCurrentLayer()->imageExtents());
	layerSelected(indx);
	layersUpdated();

        m_doc->setModified(true);
    }
#endif
}

void KisView::layer_rotate180()
{

}

void KisView::layer_rotateleft90()
{

}

void KisView::layer_rotateright90()
{

}

void KisView::layer_rotate_custom()
{

}

void KisView::layer_mirrorX()
{
}

void KisView::layer_mirrorY()
{
}

void KisView::add_new_image_tab()
{
	if (m_doc -> slotNewImage()) {
		m_doc -> setModified(true);
	}
}

void KisView::remove_current_image_tab()
{
	KisImageSP current = currentImg();

	if (current) {
		m_current = 0;
		m_doc -> removeImage(current);
	}
}

void KisView::merge_all_layers()
{
#if 0
	KisImageSP img = currentImg();

	if (img) {
		img -> mergeAllLayers();
		m_doc -> setModified(true);
	}
#endif
}


void KisView::merge_visible_layers()
{
#if 0
    if (currentImg())
    {
        currentImg()->mergeVisibleLayers();

        m_doc->setModified(true);
    }
#endif
}


void KisView::merge_linked_layers()
{
#if 0
    if (currentImg())
    {
        currentImg()->mergeLinkedLayers();

        m_doc->setModified(true);
    }
#endif
}

void KisView::showMenubar()
{
}


void KisView::showToolbar()
{
}


void KisView::showStatusbar()
{
}


void KisView::showSidebar()
{
	if (m_sidebarToggle -> isChecked())
		m_sideBar -> show();
	else
		m_sideBar -> hide();

	resizeEvent(0);
}

void KisView::floatSidebar()
{
	m_sideBar -> setDocked(!m_floatsidebarToggle -> isChecked());
	resizeEvent(0);
}

void KisView::placeSidebarLeft()
{
	resizeEvent(0);
}

/*
    saveOptions - here we need to write entries to a congig
    file.
*/
void KisView::saveOptions()
{
}


/*
    preferences - the main Krayon preferences dialog - modeled
    after the konqueror prefs dialog - quite nice compound dialog
*/
void KisView::preferences()
{
 //   PreferencesDialog::editPreferences();
}

Q_INT32 KisView::docWidth() const
{
	return currentImg() ? currentImg() -> width() : 0;
}

Q_INT32 KisView::docHeight() const
{
	return currentImg() ? currentImg() -> height() : 0;
}

void KisView::scrollTo(Q_INT32 x, Q_INT32 y)
{
	kdDebug() << "scroll to " << x << "," << y << endl;

    // this needs to update the scrollbar values and
    // let resizeEvent() handle the repositioning
    // with showScollBars()
}


void KisView::setActiveBrush(KoIconItem *brush)
{
	m_brush = dynamic_cast<KisBrush*>(brush);
#if 0
	Q_ASSERT(b);
	m_pBrush = b;

	if (m_pTool) {
		m_pTool -> setBrush(b);
		m_pTool -> setCursor();
	}
#endif
}

void KisView::setActiveCrayon(KoIconItem *)
{
#if 0
	m_pKrayon = k;
	m_sideBar -> setActiveCrayon(*k);
#endif
}

void KisView::setActivePattern(KoIconItem *)
{
#if 0
	// set currentImg pattern for this view
	m_pPattern = p;

	// set pattern for other things that use patterns
	Q_ASSERT(m_sideBar);
	Q_ASSERT(m_doc);
	m_sideBar -> setActivePattern(*p);
	m_doc -> frameBuffer() -> setPattern(p);
#endif
}

void KisView::setBGColor(const KoColor& c)
{
	emit bgColorChanged(c);
	m_bg = c;
}

void KisView::setFGColor(const KoColor& c)
{
	emit fgColorChanged(c);
	m_fg = c;
}

void KisView::slotSetFGColor(const KoColor& c)
{
	m_fg = c;
}

void KisView::slotSetBGColor(const KoColor& c)
{
	m_bg = c;
}

void KisView::setupPrinter(KPrinter& )
{
#if 0
    printer.setPageSelection(KPrinter::ApplicationSide);

    int count = 0;
    QStringList imageList = m_doc->images();
    for (QStringList::Iterator it = imageList.begin(); it != imageList.end(); ++it) {
        if (*it == currentImg()->name())
            break;
        ++count;
    }

    printer.setCurrentPage(1 + count);
    printer.setMinMax(1, m_doc->images().count());
    printer.setPageSize(KPrinter::A4);
    printer.setOrientation(KPrinter::Portrait);
#endif
}

void KisView::print(KPrinter &)
{
#if 0
    printer.setFullPage(true);
    QPainter paint;
    paint.begin(&printer);
    paint.setClipping(false);
    QValueList<int> imageList;
    int from = printer.fromPage();
    int to = printer.toPage();
    if(!from && !to)
    {
        from = printer.minPage();
        to = printer.maxPage();
    }
    for (int i = from; i <= to; i++)
        imageList.append(i);
    QString tmp_currentImageName = m_doc->currentImgName();
    QValueList<int>::Iterator it = imageList.begin();
    for (; it != imageList.end(); ++it)
    {
        int imageNumber = *it - 1;
        if (it != imageList.begin())
            printer.newPage();

        m_doc->setImage(*m_doc->images().at(imageNumber));
        m_doc->paintContent(paint, m_doc->getImageRect());
    }
    paint.end ();
    m_doc->setImage(tmp_currentImageName);
#endif
}

void KisView::setupTools()
{
#if 0
	ktvector tools;

	tools = m_doc -> getTools();

	if (tools.empty()) {
		tools = ::toolFactory(m_canvas, m_pBrush, m_pPattern, m_doc);
		m_paste = new PasteTool(m_doc, m_canvas);
		tools.push_back(m_paste);
	}

	for (ktvector_size_type i = 0; i < tools.size(); i++) {
		KisTool *p = tools[i];

		Q_ASSERT(p);
		p -> setupAction(actionCollection());
	}

	if (m_doc -> viewCount() < 1)
		m_doc -> setTools(tools);

	tools[0] -> toolSelect();
	activateTool(tools[0]);
#endif
}

void KisView::setCanvasCursor(const QCursor& )
{
#if 0
	KisCanvas *canvas = kisCanvas();

	Q_ASSERT(canvas);
	canvas -> setCursor(cursor);
#endif
}

void KisView::canvasGotPaintEvent(QPaintEvent *event)
{
	QRect ur = event -> rect();

	if (zoom() > 1.0 || zoom() < 1.0) {
		Q_INT32 urL = ur.left();
		Q_INT32 urT = ur.top();
		Q_INT32 urW = ur.width();
		Q_INT32 urH = ur.height();

		urL = static_cast<Q_INT32>(static_cast<double>(urL) / zoom());
		urT = static_cast<Q_INT32>(static_cast<double>(urT) / zoom());
		urW = static_cast<Q_INT32>(static_cast<double>(urW) / zoom());
		urH = static_cast<Q_INT32>(static_cast<double>(urH) / zoom());
		ur.setLeft(urL);
		ur.setTop(urT);
		ur.setWidth(urW);
		ur.setHeight(urH);
	}

	paintView(ur);
}

void KisView::canvasGotMousePressEvent(QMouseEvent *e)
{
	if (m_tool) {
		Q_INT32 x = static_cast<Q_INT32>((e -> pos().x() - canvasXOffset() + horzValue() * zoom()) / zoom());
		Q_INT32 y = static_cast<Q_INT32>((e -> pos().y() - canvasYOffset() + vertValue() * zoom()) / zoom());
		QMouseEvent ev(QEvent::MouseButtonPress, QPoint(x, y), e -> globalPos(), e -> button(), e -> state());

		m_tool -> mousePress(&ev);
	}
}

void KisView::canvasGotMouseMoveEvent(QMouseEvent *e)
{
	if (m_tool) {
		Q_INT32 x = static_cast<Q_INT32>((e -> pos().x() - canvasXOffset() + horzValue() * zoom()) / zoom());
		Q_INT32 y = static_cast<Q_INT32>((e -> pos().y() - canvasYOffset() + vertValue() * zoom()) / zoom());
		QMouseEvent ev(QEvent::MouseButtonPress, QPoint(x, y), e -> globalPos(), e -> button(), e -> state());

		if (zoom() >= 1.0 / 4.0) {
			m_hRuler -> setValue(e -> pos().x() - canvasXOffset());
			m_vRuler -> setValue(e -> pos().y() - canvasYOffset());
		}

		m_tool -> mouseMove(&ev);
	}
}

void KisView::canvasGotMouseReleaseEvent(QMouseEvent *e)
{
	if (m_tool) {
		Q_INT32 x = static_cast<Q_INT32>((e -> pos().x() - canvasXOffset() + horzValue() * zoom()) / zoom());
		Q_INT32 y = static_cast<Q_INT32>((e -> pos().y() - canvasYOffset() + vertValue() * zoom()) / zoom());
		QMouseEvent ev(QEvent::MouseButtonPress, QPoint(x, y), e -> globalPos(), e -> button(), e -> state());

		m_tool -> mouseRelease(&ev);
	}
}

void KisView::canvasGotEnterEvent(QEvent *e)
{
	if (m_tool) {
		QEvent ev(*e);

		m_tool -> enter(&ev);
	}
}

void KisView::canvasGotLeaveEvent (QEvent *e)
{
	if (m_tool) {
		QEvent ev(*e);

		m_tool -> leave(&ev);
	}
}

void KisView::canvasGotMouseWheelEvent(QWheelEvent *event)
{
	QApplication::sendEvent(m_vScroll, event);
}

void KisView::canvasRefresh()
{
	updateCanvas();
}

void KisView::layerToggleVisible()
{
	KisImageSP img = currentImg();

	if (img) {
		KisLayerSP layer = img -> activeLayer();

		if (layer) {
			layer -> visible(!layer -> visible());
			img -> invalidate(vertValue(), horzValue(), width(), height());
			m_doc -> setModified(true);
			resizeEvent(0);
			updateCanvas();
			layersUpdated();
		}
	}
}

void KisView::layerSelected(int n)
{
	KisImageSP img = currentImg();

	layerUpdateGUI(img -> activateLayer(n));
}

void KisView::docImageListUpdate()
{
	m_current = 0;
	zoomUpdateGUI(0, 0, 1.0);
	resizeEvent(0);
	updateCanvas();
}

void KisView::layerToggleLinked()
{
	KisImageSP img = currentImg();

	if (img) {
		KisLayerSP layer = img -> activeLayer();

		if (layer) {
			layer -> linked(!layer -> linked());
			m_doc -> setModified(true);
			layersUpdated();
		}
	}
}

void KisView::layerProperties()
{
	KisImageSP img = currentImg();

	if (img) {
		KisLayerSP layer = img -> activeLayer();

		if (layer) {
			KisPaintPropertyDlg dlg(layer -> name(), layer -> opacity());

			if (dlg.exec() == QDialog::Accepted && (layer -> name() != dlg.getName() || layer -> opacity() != dlg.getOpacity()))
				m_doc -> layerProperties(img, layer, dlg.getOpacity(), dlg.getName());
		}
	}
}

void KisView::layerAdd()
{
	KisImageSP img = currentImg();

	if (img) {
		NewLayerDialog dlg(this);

		dlg.exec();

		if (dlg.result() == QDialog::Accepted) {
			KisLayerSP layer = m_doc -> layerAdd(img, dlg.width(), dlg.height(), img -> nextLayerName(), OPACITY_OPAQUE);

			if (layer) {
				m_layerBox -> setCurrentItem(img -> index(layer));
				resizeEvent(0);
				updateCanvas(layer -> x(), layer -> y(), layer -> width(), layer -> height());
			} else {
				KMessageBox::error(this, i18n("Could not add layer to image."), i18n("Layer Error"));
			}
		}
	}
}

void KisView::layerRemove()
{
	KisImageSP img = currentImg();

	if (img) {
		KisLayerSP layer = img -> activeLayer();

		if (layer) {
			Q_INT32 n = img -> index(layer);

			m_doc -> layerRemove(img, layer);
			m_layerBox -> setCurrentItem(n - 1);
			resizeEvent(0);
			updateCanvas();
			layerUpdateGUI(img -> activeLayer() != 0);
		}
	}
}

void KisView::layerAddMask(int /*n*/)
{
}

void KisView::layerRmMask(int /*n*/)
{
}

void KisView::layerRaise()
{
	KisImageSP img = currentImg();
	KisLayerSP layer;

	if (!img)
		return;

	layer = img -> activeLayer();

	if (layer) {
		m_doc -> layerRaise(img, layer);
		m_layerBox -> setCurrentItem(img -> index(layer));
		resizeEvent(0);
		updateCanvas();
	}
}

void KisView::layerLower()
{
	KisImageSP img = currentImg();
	KisLayerSP layer;

	if (!img)
		return;

	layer = img -> activeLayer();

	if (layer) {
		m_doc -> layerLower(img, layer);
		m_layerBox -> setCurrentItem(img -> index(layer));
		resizeEvent(0);
		updateCanvas();
	}
}

void KisView::layerFront()
{
	KisImageSP img = currentImg();

	if (img && img -> activeLayer())
		img -> top(img -> activeLayer());
}

void KisView::layerBack()
{
	KisImageSP img = currentImg();

	if (img && img -> activeLayer())
		img -> bottom(img -> activeLayer());
}

void KisView::layerLevel(int /*n*/)
{
}

void KisView::layersUpdated()
{
	KisImageSP img = currentImg();

	layerUpdateGUI(img && img -> activeLayer());

	if (img) {
		vKisLayerSP l = img -> layers();
		Q_INT32 current = m_layerBox -> getCurrentItem();

		m_layerBox -> setUpdatesEnabled(false);
		m_layerBox -> clear();

		for (vKisLayerSP_it it = l.begin(); it != l.end(); it++)
			m_layerBox -> insertItem((*it) -> name(), (*it) -> visible(), (*it) -> linked());

		m_layerBox -> setUpdatesEnabled(true);
		m_layerBox -> repaint();
		m_layerBox -> setCurrentItem(current);
		m_doc -> setModified(true);
	}
}

void KisView::layersUpdated(KisImageSP img)
{
	if (img == currentImg())
		layersUpdated();
}

void KisView::selectImage(const QString& name)
{
	m_current = m_doc -> findImage(name);
	layersUpdated();
	resizeEvent(0);
	updateCanvas();
}

void KisView::selectImage(KisImageSP img)
{
	m_current = img;
	layersUpdated();
	resizeEvent(0);
	updateCanvas();
	m_tabBar -> slotImageListUpdated();
}

void KisView::setPaintOffset()
{
	KisDlgPaintOffset dlg(m_xoff, m_yoff, this, "dlg_paint_offset");

	dlg.exec();

	if (dlg.result() == QDialog::Accepted && (dlg.xoff() != m_xoff || dlg.yoff() != m_yoff)) {
		m_xoff = dlg.xoff();
		m_yoff = dlg.yoff();
		resizeEvent(0);
	}
}

void KisView::scrollH(int value)
{
	m_hRuler -> setOffset(value);
	m_canvas -> repaint();
}

void KisView::scrollV(int value)
{
	m_vRuler -> setOffset(value);
	m_canvas -> repaint();
}

QWidget *KisView::canvas()
{
	return m_canvas;
}

int KisView::canvasXOffset() const
{
	return static_cast<Q_INT32>(static_cast<double>(m_xoff) * zoom());
}

int KisView::canvasYOffset() const
{
	return static_cast<Q_INT32>(static_cast<double>(m_yoff) * zoom());
}

void KisView::setupCanvas()
{
	m_canvas = new KisCanvas(this, "kis_canvas");

	QObject::connect(m_canvas, SIGNAL(mousePressed(QMouseEvent*)), this, SLOT(canvasGotMousePressEvent(QMouseEvent*)));
	QObject::connect(m_canvas, SIGNAL(mouseMoved(QMouseEvent*)), this, SLOT(canvasGotMouseMoveEvent(QMouseEvent*)));
	QObject::connect(m_canvas, SIGNAL(mouseReleased(QMouseEvent*)), this, SLOT(canvasGotMouseReleaseEvent(QMouseEvent*)));
	QObject::connect(m_canvas, SIGNAL(gotPaintEvent(QPaintEvent*)), this, SLOT(canvasGotPaintEvent(QPaintEvent*)));
	QObject::connect(m_canvas, SIGNAL(gotEnterEvent(QEvent*)), this, SLOT(canvasGotEnterEvent(QEvent*)));
	QObject::connect(m_canvas, SIGNAL(gotLeaveEvent(QEvent*)), this, SLOT(canvasGotLeaveEvent(QEvent*)));
	QObject::connect(m_canvas, SIGNAL(mouseWheelEvent(QWheelEvent*)), this, SLOT(canvasGotMouseWheelEvent(QWheelEvent*)));
}

void KisView::projectionUpdated(KisImageSP img)
{
	if (img == currentImg())
		updateCanvas();
}

#include "kis_view.moc"


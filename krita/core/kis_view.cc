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

// Local
#include "kis_doc.h"
#include "kis_canvas.h"
#include "kis_dlg_paintoffset.h"
#include "kis_image_builder.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_sidebar.h"
#include "kis_tabbar.h"
#include "kis_view.h"
#include "kis_util.h"

#if 0
// Qt
#include <qclipboard.h>
#include <qlistbox.h>
#include <qsize.h>
#include <qstringlist.h>
#include <qvaluelist.h>

// KDE
#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <khelpmenu.h>
#include <klistview.h>
#include <kmimetype.h>
#include <kprinter.h>

// KOffice
#include <koColor.h>
#include <koView.h>

// core classes
#include "kis_brush.h"
#include "kis_factory.h"
#include "kis_framebuffer.h"
#include "kis_gradient.h"
#include "kis_krayon.h"
#include "kis_painter.h"
#include "kis_pattern.h"
#include "kis_pluginserver.h"
#include "kis_magick.h"
#include "kis_selection.h"
#include "kis_tool.h"
#include "KRayonViewIface.h"

// ui
#include "kis_brushchooser.h"
#include "kis_patternchooser.h"
#include "kis_krayonchooser.h"
#include "kis_layerview.h"
#include "kis_channelview.h"
#include "kis_listbox.h"
#include "kis_dlg_paint_properties.h"

// dialogs
#include "kis_dlg_gradient.h"
#include "kis_dlg_preferences.h"
#include "kis_dlg_new.h"
#include "kis_dlg_new_layer.h"

// tools
#include "kis_tool_factory.h"
#include "kis_tool_paste.h"
#endif

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
	m_sideBar = 0;
	m_tabBar = 0;
	m_tabFirst = 0;
	m_tabLeft = 0;
	m_tabRight = 0;
	m_tabLast = 0;
	m_hRuler = 0;
	m_vRuler = 0;
	m_zoomIn = 0;
	m_zoomOut = 0;
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

	connect(m_doc, SIGNAL(imageListUpdated()), SLOT(docImageListUpdate()));

#if 0
	m_zoomFactor = 1.0;
	setInstance(KisFactory::global());
	m_pTool = 0;
	dcopObject();

	QObject::connect(m_doc, SIGNAL(docUpdated()), this, SLOT(slotDocUpdated()));
	QObject::connect(m_doc, SIGNAL(docUpdated(const QRect&)), this, SLOT(slotDocUpdated(const QRect&)));
	QObject::connect(this, SIGNAL(embeddImage(const QString&)), this, SLOT(slotEmbeddImage(const QString&)));
	connect(m_doc, SIGNAL(layersUpdated()), SLOT(layersUpdated()));

	m_fg = KoColor::black();
	m_bg = KoColor::white();

	m_xoff = 0;
	m_yoff = 0;

	m_pTool     = 0;
	m_pBrush    = 0;
	m_pPattern  = 0;
	m_pGradient = 0;
	m_pPatternChooser = 0;

	setupPainter();
	slotSetBrush(m_pBrush);
	slotSetPattern(m_pPattern);
#endif
	setupActions();
	setupCanvas();
	setupRulers();
	setupScrollBars();
	setupSideBar();
	setupTabBar();
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

/*
    Set up painter object for use of QPainter methods to draw
    into KisLayer memory.  Note:  The painter object should be
    attached to the image, not view.  It needs to be moved.
    The reason for this is that the painter paints on the image,
    and has nothing to do with the canvas or the view. If there
    are multiple views then the same painter paints images which
    are shown in each view.

    Even better, the currentImg image should be set by the view and not
    by the document.  Each view could show a different currentImg
    image.  Curently each view shows the same image, which is very
    limiting, although it's more compliant with koffice standards.
    The best solution is to have a painter for each image, which would
    take more memory, but detaches it from either document or view.
*/

void KisView::setupPainter()
{
//	m_pPainter = new KisPainter(m_doc, this);
}


/*
    SideBar - has tabs for brushes, layers, channels, etc.
    Nonstandard, but who sets the standards?  Unrelieved
    uniformity is the mark of small minds.  Note:  Some of these
    sidebar widgets are unfinished and just have placeholders
    so far.  Note that the sidebar itself, and each widget in it,
    can be detached or floated if the user wants them to be
    separate windows.
*/

void KisView::setupSideBar()
{
#if 0
	m_sideBar = new KisSideBar(this, "kis_sidebar");
	// krayon chooser
	m_pKrayonChooser = new KisKrayonChooser(this);
	m_pKrayon = m_pKrayonChooser -> currentKrayon();

	QObject::connect(m_pKrayonChooser, SIGNAL(selected(KisKrayon *)), this, SLOT(slotSetKrayon(KisKrayon*)));

	m_pKrayonChooser -> setCaption(i18n("Krayons"));
	m_sideBar -> plug(m_pKrayonChooser);

	// brush chooser
	m_pBrushChooser = new KisBrushChooser(m_sideBar -> dockFrame());
	m_pBrush = m_pBrushChooser -> currentBrush();
	QObject::connect(m_pBrushChooser, SIGNAL(selected(KisBrush *)), this, SLOT(slotSetBrush(KisBrush*)));
	m_pBrushChooser -> setCaption(i18n("Brushes"));
	m_sideBar -> plug(m_pBrushChooser);

	// pattern chooser
	m_pPatternChooser = new KisPatternChooser(this);
	m_pPattern = m_pPatternChooser -> currentPattern();
	QObject::connect(m_pPatternChooser, SIGNAL(selected(KisPattern *)), this, SLOT(slotSetPattern(KisPattern*)));
	m_pPatternChooser -> setCaption(i18n("Patterns"));
	m_sideBar -> plug(m_pPatternChooser);

	// gradient chooser
	m_pGradientChooser = new QWidget(this);
	m_pGradient = new KisGradient;

	/*
	   m_pGradient = m_pGradientChooser->currentGradient();
	   QObject::connect(m_pGradientChooser,
	   SIGNAL(selected(KisGradient *)),
	   this, SLOT(slotSetGradient(KisGradient*)));
	 */
	m_pGradientChooser -> setCaption(i18n("Gradients"));
	m_sideBar -> plug(m_pGradientChooser);

	// image file chooser
	m_pImageChooser = new QWidget(this);
	/*
	   m_pImage = m_pImageFileChooser->currentImageFile();
	   QObject::connect(m_pImageFileChooser,
	   SIGNAL(selected(KisImageFile *)),
	   this, SLOT(slotSetImageFile(KisImageFile*)));
	 */
	m_pImageChooser -> setCaption(i18n("Images"));
	m_sideBar -> plug(m_pImageChooser);

	// palette chooser
	m_pPaletteChooser = new QWidget(this);
	/*
	   m_pPalette = m_pPaletteChooser->currentPattern();
	   QObject::connect(m_pPaletteChooser,
	   SIGNAL(selected(KisPalette *)),
	   this, SLOT(slotSetPalette(KisPalette *)));
	 */
	m_pPaletteChooser -> setCaption(i18n("Palettes"));
	m_sideBar -> plug(m_pPaletteChooser);

	m_layerView = new KisListBoxView("Layer", KisListBoxView::SHOWALL, m_sideBar);
	m_layerView -> setCaption(i18n("Layers"));

	connect(m_layerView, SIGNAL(itemToggleVisible(int)), SLOT(layerToggleVisible(int)));
	connect(m_layerView, SIGNAL(itemSelected(int)), SLOT(layerSelected(int)));
	connect(m_layerView, SIGNAL(itemToggleLinked(int)), SLOT(layerToggleLinked(int)));
	connect(m_layerView, SIGNAL(itemProperties(int)), SLOT(layerProperties(int)));
	connect(m_layerView, SIGNAL(itemAdd()), SLOT(layerAdd()));
	connect(m_layerView, SIGNAL(itemRemove(int)), SLOT(layerRemove(int)));
	connect(m_layerView, SIGNAL(itemAddMask(int)), SLOT(layerAddMask(int)));
	connect(m_layerView, SIGNAL(itemRmMask(int)), SLOT(layerRmMask(int)));
	connect(m_layerView, SIGNAL(itemRaise(int)), SLOT(layerRaise(int)));
	connect(m_layerView, SIGNAL(itemLower(int)), SLOT(layerLower(int)));
	connect(m_layerView, SIGNAL(itemFront(int)), SLOT(layerFront(int)));
	connect(m_layerView, SIGNAL(itemBack(int)), SLOT(layerBack(int)));
	connect(m_layerView, SIGNAL(itemLevel(int)), SLOT(layerLevel(int)));

	m_sideBar -> plug(m_layerView);
	layersUpdated();

	// channel view
	m_pChannelView = new KisChannelView(m_doc, this);
	m_pChannelView -> setCaption(i18n("Channels"));
	m_sideBar -> plug(m_pChannelView);

	// activate brushes tab
	m_sideBar -> slotActivateTab(i18n("Brushes"));

	// init sidebar
	m_sideBar -> slotSetBrush(*m_pBrush);
	m_sideBar -> slotSetFGColor(m_fg);
	m_sideBar -> slotSetBGColor(m_bg);

	connect(m_sideBar, SIGNAL(fgColorChanged(const KoColor&)), this, SLOT(slotSetFGColor(const KoColor&)));
	connect(m_sideBar, SIGNAL(bgColorChanged(const KoColor&)), this, SLOT(slotSetBGColor(const KoColor&)));

	connect(this, SIGNAL(fgColorChanged(const KoColor&)), m_sideBar, SLOT(slotSetFGColor(const KoColor&)));
	connect(this, SIGNAL(bgColorChanged(const KoColor&)), m_sideBar, SLOT(slotSetBGColor(const KoColor&)));

	m_side_bar -> setChecked(true);
#endif
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
	m_hRuler -> setGeometry(20, 0, width()-20, 20);
	m_vRuler -> setGeometry(0, 20, 20, height()-20);
	m_vRuler -> setRulerMetricStyle(KRuler::Pixel);
	m_hRuler -> setRulerMetricStyle(KRuler::Pixel);
	m_vRuler -> setFrameStyle(QFrame::Panel | QFrame::Raised);
	m_hRuler -> setFrameStyle(QFrame::Panel | QFrame::Raised);
	m_hRuler -> setLineWidth(1);
	m_vRuler -> setLineWidth(1);
	//m_hRuler->setShowEndLabel(true);
	//m_vRuler->setShowEndLabel(true);
}

void KisView::setupTabBar()
{
	m_tabBar = new KisTabBar(this, m_doc);
	m_tabBar -> slotImageListUpdated();
	QObject::connect(m_tabBar, SIGNAL(tabSelected(const QString&)), m_doc, SLOT(setCurrentImage(const QString&)));
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
	(void)new KAction(i18n("Refresh Canvas"), "reload", 0, this, SLOT(slotDocUpdated()), actionCollection(), "refresh_canvas");
	(void)new KAction(i18n("Panic Button"), "stop", 0, this, SLOT(slotHalt()), actionCollection(), "panic_button");

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
	m_zoomIn = new KAction(i18n("Zoom &In"), "viewmag+", 0, this, SLOT(zoom_in()), actionCollection(), "zoom_in");
	m_zoomOut = new KAction(i18n("Zoom &Out"), "viewmag-", 0, this, SLOT(zoom_out()), actionCollection(), "zoom_out");

	// tool settings actions
	(void)new KAction(i18n("&Gradient Dialog..."), "blend", 0, this, SLOT(dialog_gradient()), actionCollection(), "dialog_gradient");

	// tool actions
	(void)new KAction(i18n("&Current Tool Properties..."), "configure", 0, this, SLOT(tool_properties()), actionCollection(), "current_tool_properties");

	// layer actions
	(void)new KAction(i18n("&Add Layer..."), 0, this, SLOT(layerAdd()), actionCollection(), "insert_layer");
	(void)new KAction(i18n("&Remove Layer"), 0, this, SLOT(remove_layer()), actionCollection(), "remove_layer");
	(void)new KAction(i18n("&Link/Unlink Layer"), 0, this, SLOT(link_layer()), actionCollection(), "link_layer");
	(void)new KAction(i18n("&Hide/Show Layer"), 0, this, SLOT(hide_layer()), actionCollection(), "hide_layer");
	(void)new KAction(i18n("&Next Layer"), "forward", 0, this, SLOT(next_layer()), actionCollection(), "next_layer");
	(void)new KAction(i18n("&Previous Layer"), "back", 0, this, SLOT(previous_layer()), actionCollection(), "previous_layer");
	(void)new KAction(i18n("Layer Properties..."), 0, this, SLOT(layer_properties()), actionCollection(), "layer_properties");
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
	m_lsidebarToggle = new KToggleAction(i18n("Left/Right Sidebar"), "view_right", 0, this, SLOT(leftSidebar()), actionCollection(), "left_sidebar");
	(void)KStdAction::saveOptions(this, SLOT(saveOptions()), actionCollection(), "save_options");
	(void)new KAction(i18n("Krayon Preferences..."), "edit", 0, this, SLOT(preferences()), actionCollection(), "preferences");

	// krayon box toolbar actions - these will be used only
	// to dock and undock wideget in the krayon box
	m_dlgColorsToggle = new KToggleAction(i18n("&Colors"), "color_dialog", 0, this, SLOT(dialog_colors()), actionCollection(), "colors_dialog");
	m_dlgCrayonToggle = new KToggleAction(i18n("&Krayons"), "krayon_box", 0, this, SLOT(dialog_crayons()), actionCollection(), "krayons_dialog");
	m_dlgBrushToggle = new KToggleAction(i18n("Brushes"), "brush_dialog", 0, this, SLOT(dialog_brushes()), actionCollection(), "brushes_dialog");
	m_dlgPatternToggle = new KToggleAction(i18n("Patterns"), "pattern_dialog", 0, this, SLOT(dialog_patterns()), actionCollection(), "patterns_dialog");
	m_dlgLayersToggle = new KToggleAction(i18n("Layers"), "layer_dialog", 0, this, SLOT(dialog_layers()), actionCollection(), "layers_dialog");
	m_dlgChannelsToggle = new KToggleAction(i18n("Channels"), "channel_dialog", 0, this, SLOT(dialog_channels()), actionCollection(), "channels_dialog");

	m_dlgBrushToggle -> setChecked(true);
	m_dlgPatternToggle -> setChecked(true);
	m_dlgLayersToggle -> setChecked(true);
	m_dlgChannelsToggle -> setChecked(true);
}

/*
    slotHalt - try to restore reasonable defaults for a user
    who may have pushed krayon beyond its limits or the
    limits of his/her hardware and system memory!  This can happen
    when someone sets a ridiculously high zoom factor which
    requires a supercomputer for all the floating point
    calculatons.  Krayon is not idiot proof!
*/

void KisView::slotHalt()
{
#if 0
    KMessageBox::error(NULL,
        "STOP! In the name of Love ...", "System Error", FALSE);

    zoom(0, 0, 1.0);
    slotUpdateImage();
    slotRefreshPainter();
#endif
}

/*
    slotTabSelected - these refer to the tabs for images. Currently
    this is the only way to change the currentImg image.  There should
    be other ways, also.
*/

void KisView::slotTabSelected(const QString& )
{
#if 0
	m_doc->setCurrentImage(name);
	slotRefreshPainter();
#endif
}

/*
    refreshPainter - refresh and resize the painter device
    whenever the image or layer is changed
*/

void KisView::slotRefreshPainter()
{
#if 0
	KisImageSP img = m_doc -> currentImg();

	if(img) {
		KisLayerSP lay = img -> getCurrentLayer();

		if(lay) {
			QRect extents(lay -> imageExtents());

			m_pPainter -> resize(extents.left() + extents.width(), extents.top() + extents.height());
		}

		m_pPainter -> clearAll();
	}
#endif
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
		if(m_lsidebarToggle -> isChecked()) {
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
		//m_vScroll->setRange(0, docH - drawH);
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
//	m_hScroll -> setLineStep(1 + setLineStep<Q_INT32>(10 * zoom()));

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

/*
    slotUpdateImage - a cheap hack to mark the entire image
    dirty to force a repaint AND to send a fake resize event
    to force the view to show the scrollbars
*/
void KisView::slotUpdateImage()
{
#if 0
	KisImageSP img = m_doc -> currentImg();

	if (img) {
		QRect updateRect(0, 0, img -> width(), img -> height());
		img -> markDirty(updateRect);
	}
#endif
}

/*
    slotDocUpdated - response to a signal from the document
    that there is a new or different currentImg image for the
    document - setCurrentImage() in kis_doc.cc
*/

void KisView::slotDocUpdated()
{
#if 0
	QPainter p;
	QRect ur(0, 0, m_canvas -> width(), m_canvas -> height());

	p.begin(m_canvas);
	p.eraseRect(ur);
	p.end();

	m_canvas -> repaint();
	slotRefreshPainter();
	//kdDebug() << "KisView::slotDocUpdated\n";
#endif
}

/*
    slotDocUpdated - response to a signal from
    the document that content has changed and that we
    need to update the canvas -  a definite update area
    is given, so only update that rectangle's contents.
*/

void KisView::slotDocUpdated(const QRect& )
{
#if 0
	KisImageSP img = m_doc -> currentImg();
	QRect ur = rc;
	QPainter p;
	float zF = zoom();

	if (!img)
		return;

	p.begin(m_canvas);
	ur.moveBy(static_cast<int>((canvasXOffset() + m_hScroll -> value()) * zF),  static_cast<int>(((canvasYOffset() + m_vScroll -> value()) * zF)));
	ur = ur.intersect(img -> imageExtents());
	ur.setBottom(ur.bottom() + 1);
	ur.setRight(ur.right() + 1);

	int xt = canvasXOffset() - m_hScroll -> value();
	int yt = canvasYOffset() - m_vScroll -> value();

	p.translate(xt, yt);
	p.scale(zF, zF);
	m_doc -> paintContent(p, ur);
	p.end();

	if (m_pTool && !m_pTool -> willModify()) {
		QPaintEvent ev(ur, false);

		m_pTool -> paintEvent(&ev);
	}
#endif
}

inline
void KisView::clearCanvas(const QRect& rc)
{
	QPainter gc(m_canvas);

	gc.eraseRect(rc);
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
	KisImageSP img = m_doc -> currentImg();

	if (img) {
		QPainter gc(m_canvas);
		QRect ur = rc;
		Q_INT32 xt;
		Q_INT32 yt; 

		if (canvasXOffset())
			gc.eraseRect(0, 0, canvasXOffset(), height());

		if (canvasYOffset())
			gc.eraseRect(canvasXOffset(), 0, width(), canvasYOffset());

		gc.eraseRect(canvasXOffset(), canvasYOffset() + static_cast<Q_INT32>(docHeight() * zoom()), width(), height());
		gc.eraseRect(canvasXOffset() + static_cast<Q_INT32>(docWidth() * zoom()), canvasYOffset(), width(), height());
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

		m_doc -> paintContent(gc, ur, false, 1.0, 1.0);
	} else {
		clearCanvas(rc);
	}
}

void KisView::updateCanvas()
{
	QRect rc(0, 0, docWidth(), docHeight());

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
	slotUpdateImage();
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
		slotUpdateImage();
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

	KisImageSP img = m_doc -> currentImg();

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
		slotUpdateImage();
		slotRefreshPainter();
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
	KisImageSP img = m_doc -> currentImg();

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

/*--------------------------
       Zooming
---------------------------*/


#if 0
void KisView;::zoom(int , int , float )
{
	/* Avoid divide by zero errors by disallowing a zoom
	   factor of zero, which is impossible anyway, as it would
	   make the image infinitely small in size. */
	if (zf == 0) zf = 1;

	/* Set a reasonable lower limit for a zoom factor of 1/8.
	   At this level a 1600x1600 image would be 200 x 200 in size.
	   Extremely low zooms, like extremely high ones, can be very
	   expensive in terms of processor cycles and degrade performace,
	   although not nearly as much as extrmely high zooms.*/
	if (zf < 0.15) zf = 1.0/8.0;

	/* Set a reasonable upper limit for a zoom factor of 16. At this
	   level each pixel in the layer is shown as a 16x16 rectangle.
	   Zoom levels higher than this serve no useful purpose and are
	 *VERY* expensive.  It's possible to accidentally set a very high
	 zoom by continuing to click on the image with the zoom tool
	 without this limit. */
	else if(zf > 16.0) zf = 16.0;

	setZoomFactor(zf);

	// clear everything
	QPainter p;
	p.begin(m_canvas);
	p.eraseRect(0, 0, width(), height());
	p.end();

	// adjust scaling of rulers to zoom factor
	if(zf > 3.0)
	{
		// 8 / 16 pixels per mark at 8.0 / 16.0 zoom factors
		m_hRuler->setPixelPerMark((int)(zf * 1.0));
		m_vRuler->setPixelPerMark((int)(zf * 1.0));
	}
	else
	{
		// to pixels per mark at zoom factor of 1.0
		m_hRuler->setPixelPerMark((int)(zf * 10.0));
		m_vRuler->setPixelPerMark((int)(zf * 10.0));
	}

	// Kruler - lacks sane builtin limits at tiny sizes
	// this causes hangups - avoid tiny rulers

	if(zf > 3.0)
	{
		m_hRuler->setValuePerLittleMark(1);
		m_vRuler->setValuePerLittleMark(1);
	}
	else
	{
		m_hRuler->setValuePerLittleMark(10);
		m_vRuler->setValuePerLittleMark(10);
	}

	// zoom factor of 1/4
	if(zf < 0.30)
	{
		m_hRuler->setShowLittleMarks(false);
		m_vRuler->setShowLittleMarks(false);
	}
	// zoom factor of 1/2 or greater
	else
	{
		m_hRuler->setShowLittleMarks(true);
		m_vRuler->setShowLittleMarks(true);
	}

	// zoom factor of 1/8 - lowest possible
	if(zf < 0.20)
	{
		m_hRuler->setShowMediumMarks(false);
		m_vRuler->setShowMediumMarks(false);
	}
	// zoom factor of 1/4 or greater
	else
	{
		m_hRuler->setShowMediumMarks(true);
		m_vRuler->setShowMediumMarks(true);
	}

	/* scroll to the point clicked on and update the canvas.
	   Currently scrollTo() doesn't do anything but the zoomed view
	   does have the same offset as the prior view so it
	   approximately works */

	int x = static_cast<int> (_x * zf - docWidth() / 2);
	int y = static_cast<int> (_y * zf - docHeight() / 2);

	if (x < 0) x = 0;
	if (y < 0) y = 0;

	scrollTo(QPoint(x, y));

	m_canvas->update();

	/* at low zoom levels mark everything dirty and redraw the
	   entire image to insure that the previous image is erased
	   completely from border areas.  Otherwise screen artificats
	   can be seen.*/

	if(zf < 1.0) slotUpdateImage();
}
#endif

void KisView::zoom_in(int , int )
{
#if 0
	float zf = zoom() * 2;

	zoom(x, y, zf);
#endif
}

void KisView::zoom_out(int , int )
{
#if 0
	float zf = zoom() / 2;

	zoom(x, y, zf);
#endif
}

void KisView::zoom_in()
{
	Q_ASSERT(m_zoomIn);
	Q_ASSERT(m_zoomOut);
	setZoom(zoom() * 2);
	m_zoomIn -> setEnabled(zoom() <= KISVIEW_MAX_ZOOM);
	m_zoomOut -> setEnabled(zoom() >= KISVIEW_MIN_ZOOM);
	resizeEvent(0);
	updateCanvas();
}

void KisView::zoom_out()
{
	Q_ASSERT(m_zoomIn);
	Q_ASSERT(m_zoomOut);
	setZoom(zoom() / 2);
	m_zoomIn -> setEnabled(zoom() <= KISVIEW_MAX_ZOOM);
	m_zoomOut -> setEnabled(zoom() >= KISVIEW_MIN_ZOOM);
	resizeEvent(0);
	updateCanvas();
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
#if 0
	KFloatingDialog *f = static_cast<KFloatingDialog *>(m_pBrushChooser);

	f -> setDocked(m_dialog_brushes -> isChecked());
#endif
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
		m_sideBar -> plug(m_layerView);
	else
		m_sideBar -> unplug(m_layerView);
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

/*-------------------------------
    layer action slots
--------------------------------*/

/*
    Properties dialog for the currentImg layer.
    Only for changing name and opacity so far.
*/

void KisView::layer_properties()
{
//	layerProperties(0);
}

void KisView::insert_layer()
{
//	layerAdd();
}

/*
    remove currentImg layer - to remove other layers, a user must
    access the layers tableview in a dialog or sidebar widget
*/

void KisView::remove_layer()
{
#if 0
	KisImageSP img = m_doc -> currentImg();
	int i;

	if (img && (i = img -> getCurrentLayerIndex()) != -1)
		layerRemove(i);
#endif
}

/*
    hide/show the currentImg layer - to hide other layers, a user must
    access the layers tableview in a dialog or sidebar widget
*/

void KisView::hide_layer()
{
#if 0
	KisImageSP img = m_doc -> currentImg();
	int i;

	if (img && (i = img -> getCurrentLayerIndex()) != -1)
		layerToggleVisible(i);
#endif
}

/*
    link/unlink the currentImg layer - to link other layers, a user must
    access the layers tableview in a dialog or sidebar widget
*/

void KisView::link_layer()
{
#if 0
	KisImageSP img = m_doc -> currentImg();
	int i;

	if (img && (i = img -> getCurrentLayerIndex()) != -1)
		layerToggleLinked(i);
#endif
}

/*
    make the next layer in the layers list the active one and
    bring it to the front of the view
*/

void KisView::next_layer()
{
#if 0
	KisImageSP img = m_doc -> currentImg();

	if (!img)
		return;

	int indx = img -> getCurrentLayerIndex();

	if (indx == -1)
		return;

	KisLayerSPLst layers = img -> layerList();

	if (static_cast<uint>(indx) < layers.size() - 1) {
		LayerTable *tbl = m_pLayerView -> layerTable();

		// make the next layer the currentImg one, select it,
		// and make sure it's visible
		indx++;
		img -> setCurrentLayer(indx);
		tbl -> selectLayer(indx);
		layers[indx] -> setVisible(true);

		// hide all layers on top of this one so this
		// one is clearly visible and can be painted on!

		while (++static_cast<uint>(indx) <= img -> layerList().size() - 1)
			layers[indx] -> setVisible(false);

		img->markDirty(img->getCurrentLayer()->imageExtents());
		m_pLayerView->layerTable()->updateTable();
		m_pLayerView->layerTable()->updateAllCells();
		slotRefreshPainter();
		m_doc -> setModified(true);
	}
#endif
}


/*
    make the previous layer in the layers list the active one and
    bring it to the front of the view
*/
void KisView::previous_layer()
{
#if 0
	KisImageSP img = m_doc -> currentImg();

	if (!img)
		return;

	int indx = img -> getCurrentLayerIndex();

	if (indx > 0) {
		// make the previous layer the currentImg one, select it,
		// and make sure it's visible
		--indx;
		img->setCurrentLayer(indx);
		m_pLayerView->layerTable()->selectLayer(indx);
		img->layerList()[indx]->setVisible(true);

		// hide all layers beyond this one so this
		// one is clearly visible and can be painted on!
		while(++static_cast<uint>(indx) <= img->layerList().size() - 1) {
			img->layerList()[indx]->setVisible(false);
		}

		img->markDirty(img->getCurrentLayer()->imageExtents());

		m_pLayerView->layerTable()->updateTable();
		m_pLayerView->layerTable()->updateAllCells();
		slotRefreshPainter();

		m_doc->setModified(true);
	}
#endif
}

void KisView::slotImportImage()
{	
	if (importImage(false) > 0) {
		m_doc -> setModified(true);
		resizeEvent(0);
	}
}

void KisView::export_image()
{
//	save_layer_image(true);
}

void KisView::slotInsertImageAsLayer()
{
	if (importImage(true) > 0) {
		m_doc -> setModified(true);
		resizeEvent(0);
	}
}

void KisView::save_layer_as_image()
{
//	save_layer_image(false);
}

void KisView::slotEmbeddImage(const QString &)
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
	case KisImageBuilder_RESULT_OK:
		break;
	}

	img = ib.image();

	if (createLayer) {
		vKisLayerSP v = img -> layers();
		KisImageSP current = m_doc -> currentImg();
		Q_INT32 rc = v.size();

		for (vKisLayerSP_it it = v.begin(); it != v.end(); it++) {
			KisLayerSP layer = *it;

			layer -> setImage(current);
			current -> add(layer, -1);
		}

		return rc;
	}

	m_doc -> addImage(img);
	return 1;
}


/*
    save_layer_image - export the currentImg image after merging
    layers or just export the currentImg layer -  like the above
    method, the body of this, after a valid url is obtained,
    belongs in the doc, not the view and eventually needs to be
    moved from the doc to koffice/filters.
*/

void KisView::save_layer_image(bool )
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
        slotRefreshPainter();

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

/*--------------------------
    image action slots
--------------------------*/

void KisView::add_new_image_tab()
{
#if 0
    if(m_doc->slotNewImage())
    {
        slotUpdateImage();
        slotRefreshPainter();

        m_doc->setModified(true);
    }
#endif
}


void KisView::remove_current_image_tab()
{
#if 0
    if (m_doc->currentImg())
    {
        m_doc->removeImage(m_doc->currentImg());
        slotUpdateImage();
        slotRefreshPainter();

        m_doc->setModified(true);
    }
#endif
}


void KisView::merge_all_layers()
{
#if 0
	KisImageSP img = m_doc -> currentImg();

	if (img) {
		img -> mergeAllLayers();
		slotUpdateImage();
		slotRefreshPainter();
		m_doc -> setModified(true);
	}
#endif
}


void KisView::merge_visible_layers()
{
#if 0
    if (m_doc->currentImg())
    {
        m_doc->currentImg()->mergeVisibleLayers();
        slotUpdateImage();
        slotRefreshPainter();

        m_doc->setModified(true);
    }
#endif
}


void KisView::merge_linked_layers()
{
#if 0
    if (m_doc->currentImg())
    {
        m_doc->currentImg()->mergeLinkedLayers();
        slotUpdateImage();
        slotRefreshPainter();

        m_doc->setModified(true);
    }
#endif
}


/*------------------------------------
  Preferences and configuration slots
---------------------------------------*/
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
#if 0
    if (m_side_bar->isChecked())
    {
        m_sideBar->show();
    }
    else
    {
        m_sideBar->hide();
    }

    // force resize to show scrollbars, etc
    resizeEvent(0L);
#endif
}


void KisView::floatSidebar()
{
#if 0
    KFloatingDialog *f = (KFloatingDialog *)m_sideBar;
    f->setDocked(!m_float_side_bar->isChecked());

    // force resize to show scrollbars, etc
    resizeEvent(0L);
#endif
}

/*
    leftSidebar -this does nothing except force a resize to show scrollbars
    Repositioning of sidebar is handled by resizeEvent() entirley
*/
void KisView::leftSidebar()
{
//    resizeEvent(0L);
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
	if (m_doc -> currentImg()) 
		return m_doc -> currentImg() -> width();

	return 0;
}

Q_INT32 KisView::docHeight() const
{
	if (m_doc -> currentImg()) 
		return m_doc -> currentImg() -> height();

	return 0;
}

void KisView::scrollTo(QPoint )
{
//    kdDebug() << "scroll to " << pt.x() << "," << pt.y() << endl;

    // this needs to update the scrollbar values and
    // let resizeEvent() handle the repositioning
    // with showScollBars()
}


void KisView::slotSetBrush(KisBrush*  )
{
#if 0
	Q_ASSERT(b);
	m_pBrush = b;

	if (m_pTool) {
		m_pTool -> setBrush(b);
		m_pTool -> setCursor();
	}
#endif
}

void KisView::slotSetKrayon(KisKrayon*  )
{
#if 0
	m_pKrayon = k;
	m_sideBar -> slotSetKrayon(*k);
#endif
}

void KisView::slotSetPattern(KisPattern*  )
{
#if 0
	// set currentImg pattern for this view
	m_pPattern = p;

	// set pattern for other things that use patterns
	Q_ASSERT(m_sideBar);
	Q_ASSERT(m_doc);
	m_sideBar -> slotSetPattern(*p);
	m_doc -> frameBuffer() -> setPattern(p);
#endif
}


void KisView::setSetFGColor(const KoColor&)
{
#if 0
	emit fgColorChanged(c);
	m_fg = c;
#endif
}

void KisView::setSetBGColor(const KoColor&)
{
#if 0
	emit bgColorChanged(c);
	m_bg = c;
#endif
}

/*
    The new foreground color should show up in the color selector
    via signal sent to colorselector
*/
void KisView::slotSetFGColor(const KoColor&)
{
//	m_fg = c;
}

/*
    The new background color should show up in the color selector
    via signal sent to colorselector
*/
void KisView::slotSetBGColor(const KoColor&)
{
//	m_bg = c;
}

void KisView::setupPrinter(KPrinter& )
{
#if 0
    printer.setPageSelection(KPrinter::ApplicationSide);

    int count = 0;
    QStringList imageList = m_doc->images();
    for (QStringList::Iterator it = imageList.begin(); it != imageList.end(); ++it) {
        if (*it == m_doc->currentImg()->name())
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

#if 0
void KisView::appendToDocImgList(const QSize& , const KURL& )
{
	QRect layerRect(0, 0, size.width(), size.height());
	QString layerName(u.fileName());
	KisImageSP newimg = m_doc -> newImage(m_doc -> nextImageName(), layerRect.width(), layerRect.height());

	// add background for layer - should this always be white?
	bgMode bg = bm_White; // TODO bm_Transparent, bm_ForegroundColor, bm_BackgroundColor
	KoColor clr;

	if (bg == bm_White)
		clr = KoColor::white();
	else if (bg == bm_Transparent)
		clr = KoColor::white();
	else if (bg == bm_ForegroundColor)
		clr = m_fg;
	else if (bg == bm_BackgroundColor)
		clr = m_bg;

	newimg -> addLayer(QRect(0, 0, newimg -> width(), newimg -> height()), clr, false, i18n("background"));
	newimg -> markDirty(QRect(0, 0, newimg -> width(), newimg -> height()));
	m_doc -> setCurrentImage(newimg);
	layersUpdated();
}
#endif

#if 0
void KisView::addHasNewLayer(const QSize& , const KURL& )
{
	KisImageSP img = m_doc -> currentImg();
	QRect layerRect(0, 0, size.width(), size.height());
	QString layerName(i18n("Layer %1").arg(img -> nLayers()));
	uint indx;

	img -> addLayer(layerRect, white, false, layerName);
	indx = img -> layerList().size() - 1;
	img -> setCurrentLayer(indx);
	img -> setFrontLayer(indx);
	layerSelected(indx);
	layersUpdated();
}
#endif

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

void KisView::canvasGotMousePressEvent(QMouseEvent *)
{
	kdDebug() << "KisView::canvasGotMousePressEvent\n";
#if 0
	if (m_pTool) {
		int x = e -> pos().x() - canvasXOffset() + (int)(zoom() * m_hScroll -> value());
		int y = e -> pos().y() - canvasYOffset() + (int)(zoom() * m_vScroll -> value());
		QMouseEvent ev(QEvent::MouseButtonPress, QPoint(x, y), e -> globalPos(), e -> button(), e -> state());

		m_pTool -> mousePress(&ev);

		if (e -> button() == Qt::LeftButton && m_pTool && m_pTool -> willModify())
			m_doc -> setModified(true);
	}
#endif
}

void KisView::canvasGotMouseMoveEvent (QMouseEvent *)
{
//	kdDebug() << "KisView::canvasGotMouseMoveEvent\n";
#if 0
	if (m_pTool) {
		int x = e -> pos().x() - canvasXOffset() + (int)(zoom() * m_hScroll -> value());
		int y = e -> pos().y() - canvasYOffset() + (int)(zoom() * m_vScroll -> value());
		QMouseEvent ev(QEvent::MouseMove, QPoint(x, y), e->globalPos(), e->button(), e->state());

		// set ruler pointers
		if (zoom() >= 1.0 / 4.0) {
			m_hRuler -> setValue(e -> pos().x() - canvasXOffset());
			m_vRuler -> setValue(e -> pos().y() - canvasYOffset());
		}

		m_pTool -> mouseMove(&ev);
	}
#endif
}

void KisView::canvasGotMouseReleaseEvent (QMouseEvent *)
{
	kdDebug() << "KisView::canvasGotMouseReleaseEvent\n";
#if 0
	if (m_pTool) {
		int x = e -> pos().x() - canvasXOffset() + (int)(zoom() * m_hScroll -> value());
		int y = e -> pos().y() - canvasYOffset() + (int)(zoom() * m_vScroll -> value());
		QMouseEvent ev(QEvent::MouseButtonRelease, QPoint(x, y), e -> globalPos(), e -> button(), e -> state());

		m_pTool -> mouseRelease(&ev);
	}
#endif
}

void KisView::canvasGotEnterEvent (QEvent *)
{
	kdDebug() << "KisView::canvasGotEnterEvent\n";
#if 0
	if (m_pTool) {
		QEvent ev(*e);
		m_pTool -> enterEvent(&ev);
	}
#endif
}

void KisView::canvasGotLeaveEvent (QEvent *)
{
	kdDebug() << "KisView::canvasGotLeaveEvent\n";
#if 0
	if (m_pTool) {
		// clear artifacts from tools which paint on canvas
		// this does not affect the image or layer
		if (m_pTool -> shouldRepaint())
			m_canvas -> repaint();

		QEvent ev(*e);

		m_pTool -> leaveEvent(&ev);
	}
#endif
}

void KisView::canvasGotMouseWheelEvent(QWheelEvent *event)
{
	QApplication::sendEvent(m_vScroll, event);
}

void KisView::layerToggleVisible(int )
{
#if 0
	KisImageSP img = m_doc -> currentImg();
	KisLayerSPLst l = img -> layerList();
	KisLayerSP lay = l[n];

	lay -> setVisible(!lay -> visible());
	img -> markDirty(lay -> imageExtents());
	m_doc -> setModified(true);
#endif
}

void KisView::layerSelected(int )
{	
#if 0
	KisImage *img = m_doc -> currentImg();

	img -> setCurrentLayer(n);
#endif
}

void KisView::docImageListUpdate()
{
	resizeEvent(0);
	updateCanvas();
}

void KisView::layerToggleLinked(int)
{
#if 0
	KisImageSP img = m_doc -> currentImg();
	KisLayerSPLst l = img -> layerList();
	KisLayerSP lay = l[n];

	lay -> setLinked(!lay -> linked());
	m_doc -> setModified(true);
#endif
}

void KisView::layerProperties(int /*n*/)
{
#if 0
	KisImageSP img = m_doc -> currentImg();
	KisLayerSP lay = img -> getCurrentLayer();
	KisPaintPropertyDlg dlg(lay -> name(), lay -> opacity());

	if (dlg.exec() == QDialog::Accepted) {
		lay -> setName(dlg.getName());
		lay -> setOpacity(dlg.getOpacity());
		layersUpdated();
		slotUpdateImage();
	}
#endif
}

void KisView::layerAdd()
{
#if 0
	KisImageSP img = m_doc -> currentImg();
	int indx;

	if (!img)
		return;

	NewLayerDialog dlg;
	int n = img -> getHishestLayerEver();

	dlg.exec();

	if (dlg.result() == QDialog::Accepted) {
		QRect layerRect(0, 0, dlg.width(), dlg.height());
		QString name = i18n("layer %1").arg(n);

		img -> addLayer(layerRect, white, true, name);
		indx = n - 1;
		img -> setCurrentLayer(indx);
		m_layerView -> setCurrentItem(indx);

		slotUpdateImage();
		slotRefreshPainter();
		m_doc -> setModified(true);
	}
#endif
}

void KisView::layerRemove(int )
{
#if 0
	KisImageSP img = m_doc -> currentImg();

	if (img) {
		img -> removeLayer(n);
		m_doc -> setModified(true);
		slotUpdateImage();
		slotRefreshPainter();
		m_layerView -> setSelected(n - 1);
	}
#endif
}

void KisView::layerAddMask(int /*n*/)
{
}

void KisView::layerRmMask(int /*n*/)
{
}

void KisView::layerRaise(int )
{
#if 0
	KisImageSP img = m_doc -> currentImg();
	int npos;

	if (img) {
		npos = m_layerView -> getCurrentItem() - 1;
		img -> upperLayer(n);
		m_layerView -> setSelected(npos);
	}
#endif
}

void KisView::layerLower(int )
{
#if 0
	KisImageSP img = m_doc -> currentImg();
	int npos;

	if (img) {
		npos = m_layerView -> getCurrentItem() + 1;
		img -> lowerLayer(n);
		m_layerView -> setSelected(npos);
	}
#endif
}

void KisView::layerFront(int )
{
#if 0
	KisImageSP img = m_doc -> currentImg();

	if (img)
		img -> setFrontLayer(n);
#endif

}

void KisView::layerBack(int )
{
#if 0
	KisImageSP img = m_doc -> currentImg();

	if (img)
		img -> setBackgroundLayer(n);
#endif
}

void KisView::layerLevel(int /*n*/)
{
}

void KisView::layersUpdated()
{
#if 0
	KisImageSP img = m_doc -> currentImg();

	if (img) {
		KisLayerSPLst l = img -> layerList();

		m_layerView -> setUpdatesEnabled(false);
		m_layerView -> clear();

		for (KisLayerSPLstConstIterator it = l.begin(); it != l.end(); it++)
			m_layerView -> insertItem((*it) -> name(), (*it) -> visible(), (*it) -> linked());

		m_layerView -> setUpdatesEnabled(true);
		m_layerView -> repaint();
		m_doc -> setModified(true);
	}
#endif
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

void KisView::scrollH(int)
{
	m_hRuler -> setOffset(m_hScroll -> value());
	m_canvas -> repaint();
}

void KisView::scrollV(int)
{
	m_vRuler -> setOffset(m_vScroll -> value());
	m_canvas -> repaint();
}

QWidget *KisView::canvas()
{
	return m_canvas;
}

int KisView::canvasXOffset() const
{
	return static_cast<Q_INT32>(m_xoff * zoom());
}

int KisView::canvasYOffset() const
{
	return static_cast<Q_INT32>(m_yoff * zoom());
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


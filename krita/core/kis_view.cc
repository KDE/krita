/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *                1999 Michael Koch    <koch@kde.org>
 *                1999 Carsten Pfeiffer <pfeiffer@kde.org>
 *                2002 Patrick Julien <freak@codepimps.org>
 *                2003-2004 Boudewijn Rempt <boud@valdyas.org>
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

#include <algorithm>
#include <cmath>

// Qt
#include <qapplication.h>
#include <qbutton.h>
#include <qclipboard.h>
#include <qcursor.h>
#include <qevent.h>
#include <qpainter.h>
#include <qscrollbar.h>
#include <qspinbox.h>
#include <qdockarea.h>
#include <qstringlist.h>
#include <qstyle.h>
#include <qpopupmenu.h>

// KDE
#include <dcopobject.h>
#include <kaction.h>
#include <kcolordialog.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knotifyclient.h>
#include <kprinter.h>
#include <kpushbutton.h>
#include <kstatusbar.h>
#include <kstdaction.h>
#include <klineeditdlg.h>

// KOffice
#include <koColor.h>
#include <koMainWindow.h>
#include <koView.h>
#include "kotabbar.h"

// Local
#include "builder/kis_builder_monitor.h"
#include "builder/kis_builder_subject.h"
#include "kis_brush.h"
#include "kis_canvas.h"
#include "kis_channelview.h"
#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_factory.h"
#include "kis_guide.h"
#include "kis_gradient.h"
#include "kis_icon_item.h"
#include "kis_image_magick_converter.h"
#include "kis_imagepipe_brush.h"
#include "kis_layer.h"
#include "kis_listbox.h"
#include "kis_paint_device.h"
#include "kis_paint_device_visitor.h"
#include "kis_painter.h"
#include "kis_resource_mediator.h"
#include "kis_resourceserver.h"
#include "kis_ruler.h"
#include "kis_floatingselection.h"
#include "kis_controlframe.h"
#include "kis_dockframedocker.h"
#include "kis_colordocker.h"
#include "kis_tool.h"
#include "kis_tool_factory.h"
#include "kis_tool_non_paint.h"
#include "kis_types.h"
#include "kis_undo_adapter.h"
#include "kis_util.h"
#include "kis_view.h"
#include "kis_rect.h"
#include "KRayonViewIface.h"
#include "labels/kis_label_builder_progress.h"
#include "labels/kis_label_cursor_pos.h"
#include "labels/kis_label_io_progress.h"
#include "strategy/kis_strategy_move.h"
#include "visitors/kis_flatten.h"
#include "visitors/kis_merge.h"

// Dialog boxes
#include "kis_dlg_builder_progress.h"
#include "kis_dlg_dimension.h"
#include "kis_dlg_gradient.h"
#include "kis_dlg_new_layer.h"
#include "kis_dlg_paint_properties.h"
#include "kis_dlg_paintoffset.h"
#include "kis_dlg_transform.h"
#include "kis_dlg_preferences.h"

#define KISVIEW_MIN_ZOOM (1.0 / 16.0)
#define KISVIEW_MAX_ZOOM 16.0

KisView::KisView(KisDoc *doc, KisUndoAdapter *adapter, QWidget *parent, const char *name) : super(doc, parent, name)
{
	if (!doc -> isReadWrite())
		setXMLFile("krita_readonly.rc");
	else
		setXMLFile("krita.rc");

	m_tool = 0;
	m_doc = doc;
	m_adapter = adapter;
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
	m_layerAdd = 0;
	m_layerRm = 0;
	m_layerDup = 0;
	m_layerLink = 0;
	m_layerHide = 0;
	m_layerProperties = 0;
	m_layerSaveAs = 0;
	m_layerResize = 0;
	m_layerResizeToImage = 0;
	m_layerToImage = 0;
	m_layerTransform = 0;
	m_layerRaise = 0;
	m_layerLower = 0;
	m_layerTop = 0;
	m_layerBottom = 0;
	m_selectionCut = 0;
	m_selectionCopy = 0;
	m_selectionPaste = 0;
	m_selectionPasteInto = 0;
	m_selectionToNewLayer = 0;
	m_selectionFillBg = 0;
	m_selectionFillFg = 0;
	m_selectionRm = 0;
	m_selectionSelectAll = 0;
	m_selectionSelectNone = 0;
	m_imgRm = 0;
	m_imgResize = 0;
	m_imgDup = 0;
	m_imgImport = 0;
	m_imgExport = 0;
	m_imgScan = 0;
	m_imgResizeToLayer = 0;
	m_imgMergeAll = 0;
	m_imgMergeVisible = 0;
	m_imgMergeLinked = 0;
	m_hScroll = 0;
	m_vScroll = 0;
	m_dcop = 0;
	m_xoff = 0;
	m_yoff = 0;
	m_fg = KoColor::black();
	m_bg = KoColor::white();

	m_layerchanneldocker = 0;
	m_resourcedocker = 0;
	m_toolcontroldocker = 0;
	m_colordocker = 0;
	
	m_paletteChooser = 0;
	m_gradientChooser = 0;
	m_imageChooser = 0;
	m_brush = 0;
	m_pattern = 0;
	m_gradient = 0;
	m_layerBox = 0;
	m_currentGuide = 0;
	m_brushMediator = 0;
	m_imgBuilderMgr = new KisBuilderMonitor(this);
	m_buildProgress = 0;
        m_statusBarZoomLabel = 0;

	setInstance(KisFactory::global());
	setupActions();
	setupCanvas();
	setupRulers();
	setupScrollBars();
	setupDockers();
	setupTabBar();
	setupStatusBar();
	dcopObject();
	connect(m_doc, SIGNAL(imageListUpdated()), SLOT(docImageListUpdate()));
	connect(m_doc, SIGNAL(layersUpdated(KisImageSP)), SLOT(layersUpdated(KisImageSP)));
	connect(m_doc, SIGNAL(projectionUpdated(KisImageSP)), SLOT(projectionUpdated(KisImageSP)));
	connect(this, SIGNAL(embeddImage(const QString&)), SLOT(slotEmbedImage(const QString&)));
	connect(m_imgBuilderMgr, SIGNAL(size(Q_INT32)), SLOT(nBuilders(Q_INT32)));
	setupTools();
	selectionUpdateGUI(false);
	setupClipboard();
	clipboardDataChanged();
}

KisView::~KisView()
{
	delete m_dcop;
}

DCOPObject* KisView::dcopObject()
{
	if (!m_dcop)
		m_dcop = new KRayonViewIface(this);

	return m_dcop;
}

void KisView::setupDockers()
{

	KisResourceServer *rserver = KisFactory::rServer();

	m_layerchanneldocker = new DockFrameDocker(this);
	m_layerchanneldocker -> setCaption(i18n("Layers/Channels/Paths"));

	m_resourcedocker = new DockFrameDocker(this);
	m_resourcedocker -> setCaption(i18n("Brushes/Patterns/Gradients"));

	m_toolcontroldocker = new DockFrameDocker(this);
	m_toolcontroldocker -> setCaption(i18n("Navigator/Info/Options"));

	m_colordocker = new ColorDocker(this);
	m_colordocker -> setCaption(i18n("Color Manager"));

	m_historydocker = new DockFrameDocker(this);
	m_historydocker -> setCaption(i18n("History/Actions"));

	m_brushMediator = new KisResourceMediator(MEDIATE_BRUSHES, rserver, i18n("Brushes"),
						  m_resourcedocker, "brush_chooser", this);
	m_brush = dynamic_cast<KisBrush*>(m_brushMediator -> currentResource());
	m_resourcedocker -> plug(m_brushMediator -> chooserWidget());
	connect(m_brushMediator, SIGNAL(activatedResource(KisResource*)), this, SLOT(brushActivated(KisResource*)));

	m_patternMediator = new KisResourceMediator(MEDIATE_PATTERNS, rserver, i18n("Patterns"),
						    m_resourcedocker, "pattern chooser", this);
	m_pattern = dynamic_cast<KisPattern*>(m_patternMediator -> currentResource());
	m_resourcedocker -> plug(m_patternMediator -> chooserWidget());
	connect(m_patternMediator, SIGNAL(activatedResource(KisResource*)), this, SLOT(patternActivated(KisResource*)));

	m_layerBox = new KisListBox(i18n("layer"), KisListBox::SHOWALL, m_layerchanneldocker);
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

	m_layerchanneldocker -> plug(m_layerBox);
	layersUpdated();

	m_channelView = new KisChannelView(m_doc, this);
	m_channelView -> setCaption(i18n("Channels"));
	m_layerchanneldocker -> plug(m_channelView);

	m_controlWidget = new ControlFrame(m_toolcontroldocker);
	m_controlWidget -> setCaption(i18n("General"));
	m_toolcontroldocker -> plug(m_controlWidget);

	m_controlWidget -> slotSetBGColor(m_bg);
	m_controlWidget -> slotSetFGColor(m_fg);
	connect(m_controlWidget, SIGNAL(fgColorChanged(const KoColor&)), SLOT(slotSetFGColor(const KoColor&)));
	connect(m_controlWidget, SIGNAL(bgColorChanged(const KoColor&)), SLOT(slotSetBGColor(const KoColor&)));
	connect(this, SIGNAL(fgColorChanged(const KoColor&)), m_controlWidget, SLOT(slotSetFGColor(const KoColor&)));
	connect(this, SIGNAL(bgColorChanged(const KoColor&)), m_controlWidget, SLOT(slotSetBGColor(const KoColor&)));
	m_colordocker -> slotSetBGColor(m_bg);
	m_colordocker -> slotSetFGColor(m_fg);
	connect(this , SIGNAL(fgColorChanged(const KoColor&)), m_colordocker, SLOT(slotSetFGColor(const KoColor&)));
	connect(this , SIGNAL(bgColorChanged(const KoColor&)), m_colordocker, SLOT(slotSetBGColor(const KoColor&)));
	connect(m_colordocker, SIGNAL(fgColorChanged(const KoColor&)), this, SLOT(slotSetFGColor(const KoColor&)));
	connect(m_colordocker, SIGNAL(bgColorChanged(const KoColor&)), this, SLOT(slotSetBGColor(const KoColor&)));

	rserver -> loadBrushes();
	rserver -> loadpipeBrushes();
	rserver -> loadPatterns();

	// TODO Here should be a better check
	if ( mainWindow() -> isDockEnabled( DockBottom))
	{
		viewControlDocker();
		viewLayerChannelDocker();
		viewResourceDocker();
		viewColorDocker();
		// Frankly, I'm not really satisfied with the
		// behaviour of the dockers. I wish they would dock
		// onto each other, instead of onto the main window.
		mainWindow() -> setDockEnabled( DockBottom, false);
		mainWindow() -> setDockEnabled( DockTop, false);
	}

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
	m_hRuler = new KisRuler(Qt::Horizontal, this);
	m_vRuler = new KisRuler(Qt::Vertical, this);
	m_hRuler -> setGeometry(20, 0, width() - 20, 20);
	m_vRuler -> setGeometry(0, 20, 20, height() - 20);

	if (statusBar()) {
		m_hRuler -> installEventFilter(this);
		m_vRuler -> installEventFilter(this);
	}
}

void KisView::setupTabBar()
{
	KStatusBar *sb = statusBar();

	if (sb) {
		m_tabBar = new KoTabBar(this);
		updateTabBar();
		connect(m_tabBar, SIGNAL(tabChanged(const QString&)), SLOT(selectImage(const QString&)));
		connect(m_doc, SIGNAL(imageListUpdated()), SLOT(updateTabBar()));
                connect( m_tabBar, SIGNAL( doubleClicked() ), this, SLOT( slotRename() ) );
                connect( m_tabBar, SIGNAL( tabMoved( unsigned, unsigned ) ), this, SLOT( moveImage( unsigned, unsigned ) ) );
                connect( m_tabBar, SIGNAL( contextMenu( const QPoint& ) ), this, SLOT( popupTabBarMenu( const QPoint& ) ) );
	}
}

void KisView::popupTabBarMenu( const QPoint& _point )
{
	if ( !m_doc->isReadWrite() || !factory() )
		return;
	static_cast<QPopupMenu*>(factory()->container("menuimage_popup",this))->popup(_point);
}

void KisView::moveImage( unsigned img , unsigned target)
{
	//todo
#if 0  //code from kspread adapt it.
        QStringList vs = d->workbook->visibleSheets();

	if( target >= vs.count() )
		d->workbook->moveTable( vs[ img ], vs[ vs.count()-1 ], false );
	else
		d->workbook->moveTable( vs[ img ], vs[ target ], true );
#endif
	m_tabBar->moveTab( img, target );
}

void KisView::slotRename()
{
	bool ok;
	QString activeName = currentImgName();
	QString newName = KLineEditDlg::getText( i18n("Image Name"), i18n("Enter name:"), activeName, &ok, this );

	// Have a different name ?
	if ( ok ) // User pushed an OK button.
	{
		if ( (newName.stripWhiteSpace()).isEmpty() ) // Image name is empty.
		{
			KNotifyClient::beep();
			KMessageBox::information( this, i18n("Image name cannot be empty."), i18n("Change Image Name") );
			// Recursion
			slotRename();
			return;
		}
		else if ( newName != activeName ) // Image name changed.
		{
			if ( m_doc->findImage(newName) )
			{
				KNotifyClient::beep();
				KMessageBox::information( this, i18n("This name is already used."), i18n("Change Image Name") );
				// Recursion
				slotRename();
				return;
			}
			m_doc->renameImage(activeName, newName);
			m_doc->setModified( true );
		}
	}
}

void KisView::updateStatusBarZoomLabel ()
{
	if (zoom () >= 1)
		m_statusBarZoomLabel->setText(i18n ("Zoom %1:1").arg (zoom ()));
	else if (zoom () > 0)
		m_statusBarZoomLabel->setText(i18n ("Zoom 1:%1").arg (1 / zoom ()));
	else
	{
		kdError () << "KisView::updateStatusBarZoomLabel() with 0 zoom" << endl;
		m_statusBarZoomLabel->setText(QString::null);
	}
}

void KisView::setupStatusBar()
{
	KStatusBar *sb = statusBar();

	if (sb) {
		KisLabelIOProgress *l;
		QLabel *lbl;

		lbl = new KisLabelCursorPos(sb);
		connect(this, SIGNAL(cursorPosition(Q_INT32, Q_INT32)), lbl, SLOT(updatePos(Q_INT32, Q_INT32)));
		connect(this, SIGNAL(cursorEnter()), lbl, SLOT(enter()));
		connect(this, SIGNAL(cursorLeave()), lbl, SLOT(leave()));
		addStatusBarItem(lbl, 0);
		m_statusBarZoomLabel = new QLabel(sb);
		addStatusBarItem(m_statusBarZoomLabel, 1);
                updateStatusBarZoomLabel ();
		m_buildProgress = new KisLabelBuilderProgress(this);
		m_buildProgress -> setMaximumWidth(225);
		m_buildProgress -> setMaximumHeight(sb -> height());
		addStatusBarItem(m_buildProgress, 2);
		m_buildProgress -> hide();
		l = new KisLabelIOProgress(this);
		connect(m_doc, SIGNAL(ioProgress(Q_INT8)), l, SLOT(update(Q_INT8)));
		connect(m_doc, SIGNAL(ioSteps(Q_INT32)), l, SLOT(steps(Q_INT32)));
		connect(m_doc, SIGNAL(ioCompletedStep()), l, SLOT(completedStep()));
		connect(m_doc, SIGNAL(ioDone()), l, SLOT(done()));
		l -> setMaximumWidth(200);
		l -> setMaximumHeight(sb -> height());
		addStatusBarItem(l, 3);
		l -> hide();
	}
}

void KisView::setupActions()
{
	// navigation actions
	KStdAction::redisplay(this, SLOT(canvasRefresh()), actionCollection(), "refresh_canvas");
	(void)new KAction(i18n("Reset Button"), "stop", 0, this, SLOT(reset()), actionCollection(), "panic_button");

	// selection actions
	m_selectionCut = KStdAction::cut(this, SLOT(cut()), actionCollection(), "cut");
	m_selectionCopy = KStdAction::copy(this, SLOT(copy()), actionCollection(), "copy");
	m_selectionPaste = KStdAction::paste(this, SLOT(paste()), actionCollection(), "paste");
	m_selectionPasteInto = new KAction(i18n("Paste Into"), "paste_into", 0, this, SLOT(paste_into()), actionCollection(), "paste_into");

	m_selectionRm = new KAction(i18n("Remove Selection"), "remove", 0, this, SLOT(removeSelection()), actionCollection(), "remove");
	m_selectionToNewLayer = new KAction(i18n("Copy Selection to New Layer"), "copy_selection_to_new_layer", 0,  this, SLOT(copySelectionToNewLayer()), actionCollection(), "copy to layer");
	m_selectionSelectAll = KStdAction::selectAll(this, SLOT(selectAll()), actionCollection(), "select_all");
	m_selectionSelectNone = KStdAction::deselect(this, SLOT(unSelectAll()), actionCollection(), "select_none");
	m_selectionFillFg = new KAction(i18n("Fill with Foreground Color"), 0, this, SLOT(fillSelectionFg()), actionCollection(), "fill_fgcolor");
	m_selectionFillBg = new KAction(i18n("Fill with Background Color"), 0, this, 
					SLOT(fillSelectionBg()), actionCollection(), 
					"fill_bgcolor");

	// import/export actions
	m_imgImport = new KAction(i18n("Import Image..."), "wizard", 0, this, SLOT(slotImportImage()), actionCollection(), "import_image");
	m_imgExport = new KAction(i18n("Export Image..."), "wizard", 0, this, SLOT(export_image()), actionCollection(), "export_image");
	m_imgScan = 0; // How the hell do I get a KAction to the scan plug-in?!?
	m_imgResizeToLayer = new KAction(i18n("Resize Image to Current Layer"), 0, this, SLOT(imgResizeToActiveLayer()), actionCollection(), "resizeimgtolayer");

	// view actions
	m_zoomIn = KStdAction::zoomIn(this, SLOT(slotZoomIn()), actionCollection(), "zoom_in");
	m_zoomOut = KStdAction::zoomOut(this, SLOT(slotZoomOut()), actionCollection(), "zoom_out");

	// tool settings actions
	(void)new KAction(i18n("&Gradient Dialog..."), "blend", 0, this, SLOT(dialog_gradient()), actionCollection(), "dialog_gradient");

	// tool actions
	(void)new KAction(i18n("&Current Tool Properties..."), "configure", 0, this, SLOT(tool_properties()), actionCollection(), "current_tool_properties");

	// layer actions
	m_layerAdd = new KAction(i18n("&Add Layer..."), 0, this, SLOT(layerAdd()), actionCollection(), "insert_layer");
	m_layerRm = new KAction(i18n("&Remove Layer"), 0, this, SLOT(layerRemove()), actionCollection(), "remove_layer");
	m_layerDup = new KAction(i18n("Duplicate Layer"), 0, this, SLOT(layerDuplicate()), actionCollection(), "duplicate_layer");
	m_layerLink = new KAction(i18n("&Link/Unlink Layer"), 0, this, SLOT(layerToggleLinked()), actionCollection(), "link_layer");
	m_layerHide = new KAction(i18n("&Hide/Show Layer"), 0, this, SLOT(layerToggleVisible()), actionCollection(), "hide_layer");
	m_layerRaise = new KAction(i18n("Raise Layer"), "raiselayer", 0, this, SLOT(layerRaise()), actionCollection(), "raiselayer");
	m_layerLower = new KAction(i18n("Lower Layer"), "lowerlayer", 0, this, SLOT(layerLower()), actionCollection(), "lowerlayer");
	m_layerTop = new KAction(i18n("Layer to Top"), 0, this, SLOT(layerFront()), actionCollection(), "toplayer");
	m_layerBottom = new KAction(i18n("Layer to Bottom"), 0, this, SLOT(layerBack()), actionCollection(), "bottomlayer");
	m_layerProperties = new KAction(i18n("Layer Properties..."), 0, this, SLOT(layerProperties()), actionCollection(), "layer_properties");
	(void)new KAction(i18n("I&nsert Image as Layer..."), 0, this, SLOT(slotInsertImageAsLayer()), actionCollection(), "insert_image_as_layer");
	m_layerSaveAs = new KAction(i18n("Save Layer as Image..."), 0, this, SLOT(save_layer_as_image()), actionCollection(), "save_layer_as_image");
	m_layerResize = new KAction(i18n("Resize Layer..."), 0, this, SLOT(layerResize()), actionCollection(), "resizelayer");
	m_layerResizeToImage = new KAction(i18n("Resize Layer to Image"), 0, this, SLOT(layerResizeToImage()), actionCollection(), "resizelayertoowner");
	m_layerToImage = new KAction(i18n("Layer to Image"), 0, this, SLOT(layerToImage()), actionCollection(), "layer_to_image");

	// layer transformations - should be generic, for selection too
	m_layerTransform = new KAction(i18n("Scale Layer..."), 0, this, SLOT(layerTransform()), actionCollection(), "transformlayer");
	(void)new KAction(i18n("Rotate &180"), 0, this, SLOT(layer_rotate180()), actionCollection(), "layer_rotate180");
	(void)new KAction(i18n("Rotate &270"), 0, this, SLOT(layer_rotateleft90()), actionCollection(), "layer_rotateleft90");
	(void)new KAction(i18n("Rotate &90"), 0, this, SLOT(layer_rotateright90()), actionCollection(), "layer_rotateright90");
	(void)new KAction(i18n("Rotate &Custom..."), 0, this, SLOT(layer_rotate_custom()), actionCollection(), "layer_rotate_custom");
	(void)new KAction(i18n("Mirror along &X axis"), 0, this, SLOT(layer_mirrorX()), actionCollection(), "layer_mirrorX");
	(void)new KAction(i18n("Mirror along &Y axis"), 0, this, SLOT(layer_mirrorY()), actionCollection(), "layer_mirrorY");

	// color actions
	(void)new KAction(i18n("Select Foreground Color..."), 0, this, SLOT(selectFGColor()), actionCollection(), "select_fgColor");
	(void)new KAction(i18n("Select Background Color..."), 0, this, SLOT(selectBGColor()), actionCollection(), "select_bgColor");
	(void)new KAction(i18n("Reverse Foreground/Background Colors"), 0, this, SLOT(reverseFGAndBGColors()), actionCollection(), "reverse_fg_bg");

	// image actions
        m_imgRename = new KAction(i18n("Rename Current Image"), 0, this, SLOT(slotRename()), actionCollection(), "rename_current_image_tab");
	(void)new KAction(i18n("Add New Image..."), 0, this, SLOT(add_new_image_tab()), actionCollection(), "add_new_image_tab");
	m_imgRm = new KAction(i18n("Remove Current Image"), 0, this, SLOT(remove_current_image_tab()), actionCollection(), "remove_current_image_tab");
	m_imgResize = new KAction(i18n("Resize Image..."), 0, this, SLOT(imageResize()), actionCollection(), "resize_image");
	m_imgDup = new KAction(i18n("Duplicate Image"), 0, this, SLOT(duplicateCurrentImg()), actionCollection(), "duplicate_image");
	m_imgMergeAll = new KAction(i18n("Merge &All Layers"), 0, this, SLOT(merge_all_layers()), actionCollection(), "merge_all_layers");
	m_imgMergeVisible = new KAction(i18n("Merge &Visible Layers"), 0, this, SLOT(merge_visible_layers()), actionCollection(), "merge_visible_layers");
	m_imgMergeLinked = new KAction(i18n("Merge &Linked Layers"), 0, this, SLOT(merge_linked_layers()), actionCollection(), "merge_linked_layers");

	// setting actions
	(void)new KAction(i18n("Paint Offset..."), "paint_offset", this, SLOT(setPaintOffset()), actionCollection(), "paint_offset");
	KStdAction::preferences(this, SLOT(preferences()), actionCollection(), "preferences");

	//docker actions
	(void)new KAction(i18n( "&Color Manager" ), 0, this, SLOT( viewColorDocker() ), actionCollection(), "view_color_docker" );
	(void)new KAction(i18n( "&Tool Properties" ), 0, this, SLOT( viewControlDocker() ), actionCollection(), "view_control_docker" );
	(void)new KAction(i18n( "&Layers/Channels" ), 0, this, SLOT( viewLayerChannelDocker() ), actionCollection(), "view_layer_docker" );
	(void)new KAction(i18n( "&Brushes/Pattern" ), 0, this, SLOT( viewResourceDocker() ), actionCollection(), "view_resource_docker" );
}

void KisView::reset()
{
	zoomUpdateGUI(0, 0, 1.0);
	canvasRefresh();
}

void KisView::resizeEvent(QResizeEvent *)
{
	KisImageSP img = currentImg();
	Q_INT32 rsideW = 0;
	Q_INT32 lsideW = 0;
	Q_INT32 ruler = 20;
	Q_INT32 scrollBarExtent = style().pixelMetric(QStyle::PM_ScrollBarExtent);
	Q_INT32 tbarOffset = 64;
	Q_INT32 tbarBtnH = scrollBarExtent;
	Q_INT32 tbarBtnW = scrollBarExtent;
	Q_INT32 drawH;
	Q_INT32 drawW;
	Q_INT32 docW;
	Q_INT32 docH;

	if (img) {
		KisGuideMgr *mgr = img -> guides();
		mgr -> resize(size());
	}

	m_hRuler -> setGeometry(ruler + lsideW, 0, width() - ruler - rsideW - lsideW, ruler);
	m_vRuler -> setGeometry(0 + lsideW, ruler, ruler, height() - (ruler + tbarBtnH));

	docW = static_cast<Q_INT32>(ceil(docWidth() * zoom()));
	docH = static_cast<Q_INT32>(ceil(docHeight() * zoom()));

	drawH = height() - ruler - tbarBtnH - canvasYOffset();
	drawW = width() - ruler - lsideW - rsideW - canvasXOffset();

	if (drawH < docH) {
		// Will need vert scrollbar
		drawW -= scrollBarExtent;
	}

	m_vScroll -> setEnabled(docH > drawH);
	m_hScroll -> setEnabled(docW > drawW);

	if (docH <= drawH && docW <= drawW) {
		// we need no scrollbars
		m_vScroll -> hide();
		m_hScroll -> hide();
		m_vScroll -> setValue(0);
		m_hScroll -> setValue(0);

 		if (m_tabBar)
 			m_tabBar -> setGeometry(tbarOffset + lsideW, height() - tbarBtnH, width() - rsideW - lsideW - tbarOffset, tbarBtnH);
	} else if (docH <= drawH) {
		// we need a horizontal scrollbar only
		m_vScroll -> hide();
		m_vScroll -> setValue(0);
		m_hScroll -> setRange(0, docW - drawW);
		m_hScroll -> setGeometry(tbarOffset + lsideW + (width() - rsideW -lsideW - tbarOffset) / 2,
					 height() - scrollBarExtent,
					 (width() - rsideW -lsideW - tbarOffset) / 2,
					 scrollBarExtent);
		m_hScroll -> show();

 		if (m_tabBar)
 			m_tabBar -> setGeometry(tbarOffset + lsideW, height() - tbarBtnH, (width() - rsideW - lsideW - tbarOffset) / 2, tbarBtnH);
	} else if(docW <= drawW) {
		// we need a vertical scrollbar only
		m_hScroll -> hide();
		m_hScroll -> setValue(0);
		m_vScroll -> setRange(0, docH - drawH);
		m_vScroll -> setGeometry(width() - scrollBarExtent - rsideW, ruler, scrollBarExtent, height() - (ruler + tbarBtnH));
		m_vScroll -> show();

 		if (m_tabBar)
 			m_tabBar -> setGeometry(tbarOffset + lsideW, height() - tbarBtnH, width() - rsideW -lsideW - tbarOffset, tbarBtnH);
	} else {
		// we need both scrollbars
		m_vScroll -> setRange(0, docH - drawH);
		m_vScroll -> setGeometry(width() - scrollBarExtent - rsideW, ruler, scrollBarExtent, height() - (ruler + tbarBtnH));
		m_hScroll -> setRange(0, docW - drawW);
		m_hScroll -> setGeometry(tbarOffset + lsideW + (width() - rsideW -lsideW - tbarOffset) / 2,
					 height() - scrollBarExtent,
					 (width() - rsideW -lsideW - tbarOffset) / 2,
					 scrollBarExtent);
		m_vScroll -> show();
		m_hScroll -> show();

 		if (m_tabBar)
 			m_tabBar -> setGeometry(tbarOffset + lsideW, height() - tbarBtnH, (width() - rsideW -lsideW - tbarOffset)/2, tbarBtnH);
	}

	m_canvas -> setGeometry(ruler + lsideW, ruler, drawW, drawH);
	m_canvas -> show();

	m_vScroll -> setPageStep(drawH);
	m_hScroll -> setPageStep(drawW);

	if (m_vScroll -> isVisible())
		m_vRuler -> updateVisibleArea(0, m_vScroll -> value());
	else
		m_vRuler -> updateVisibleArea(0, -canvasYOffset());

	if (m_hScroll -> isVisible())
		m_hRuler -> updateVisibleArea(m_hScroll -> value(), 0);
	else
		m_hRuler -> updateVisibleArea(-canvasXOffset(), 0);

	m_hRuler -> show();
	m_vRuler -> show();
}

void KisView::updateReadWrite(bool readwrite)
{
	layerUpdateGUI(readwrite);
	selectionUpdateGUI(readwrite);
}

void KisView::clearCanvas(const QRect& rc)
{
	QPainter gc(m_canvas);

	gc.eraseRect(rc);
}

void KisView::setCurrentTool(KisTool *tool)
{
	if (m_tool)
	{
		if(m_tool -> optionWidget())
			m_toolcontroldocker -> unplug(m_tool -> optionWidget());
		m_tool -> clear();
	}

	if (tool) {
		m_tool = tool;
		m_tool -> cursor(m_canvas);

		if(tool -> createoptionWidget(m_toolcontroldocker)){
			m_toolcontroldocker -> plug(tool -> optionWidget());
			m_toolcontroldocker -> showPage(tool -> optionWidget());
		}
		m_canvas -> enableMoveEventCompressionHint(dynamic_cast<KisToolNonPaint *>(tool) != NULL);
		notify();
	}
	else{
		m_tool = 0;
		m_canvas -> setCursor(KisCursor::arrowCursor());
	}
}

KisTool *KisView::currentTool() const
{
	return m_tool;
}

Q_INT32 KisView::horzValue() const
{
	return m_hScroll -> value();
}

Q_INT32 KisView::vertValue() const
{
	return m_vScroll -> value();
}

void KisView::paintView(const KisRect& r)
{
	KisImageSP img = currentImg();

	if (img) {

		KisRect vr = windowToView(r);
		vr &= KisRect(0, 0, m_canvas -> width(), m_canvas -> height());

		if (!vr.isNull()) {

			QPainter gc(m_canvas);
			QRect wr = viewToWindow(vr).qRect();

			if (wr.left() < 0 || wr.right() >= img -> width() || wr.top() < 0 || wr.bottom() >= img -> height()) {
				// Erase areas outside document
				QRegion rg(vr.qRect());
				rg -= QRegion(windowToView(KisRect(0, 0, img -> width(), img -> height())).qRect());

				QMemArray<QRect> rects = rg.rects();

				for (unsigned int i = 0; i < rects.count(); i++) {
					QRect er = rects[i];
					gc.eraseRect(er);
				}

				wr &= QRect(0, 0, img -> width(), img -> height());
			}

			if (!wr.isNull()) {

				if (zoom() < 1.0 || zoom() > 1.0)
					gc.setViewport(0, 0, static_cast<Q_INT32>(m_canvas -> width() * zoom()), static_cast<Q_INT32>(m_canvas -> height() * zoom()));

				gc.translate((canvasXOffset() - horzValue()) / zoom(), (canvasYOffset() - vertValue()) / zoom());

				m_doc -> setProjection(img);
				m_doc -> paintContent(gc, wr, false, 1.0, 1.0);

				if (m_tool)
					m_tool -> paint(gc, wr);
			}

			paintGuides();
		}
	} else {
		clearCanvas(r.qRect());
	}
}

QWidget *KisView::canvas() const
{
	return m_canvas;
}

void KisView::updateCanvas()
{
	KisRect vr(0, 0, m_canvas -> width(), m_canvas -> height());
	KisRect wr = viewToWindow(vr);

	updateCanvas(wr);
}

void KisView::updateCanvas(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
	KisRect rc(x, y, w, h);

	updateCanvas(rc);
}

void KisView::updateCanvas(const QRect& rc)
{
	updateCanvas(KisRect(rc));
}

void KisView::updateCanvas(const KisRect& rc)
{
	paintView(rc);
}

void KisView::tool_properties()
{
	if (m_tool) {
		KDialog * optionsDialog = m_tool -> options(this);
		if (optionsDialog)
			optionsDialog -> show();
	}
}

void KisView::layerUpdateGUI(bool enable)
{
	KisImageSP img = currentImg();
	KisLayerSP layer;
	Q_INT32 nlayers = 0;
	Q_INT32 layerPos = 0;

	if (img) {
		layer = img -> activeLayer();
		nlayers = img -> nlayers();
	}

	if (layer)
		layerPos = img -> index(layer);

	enable = enable && img && layer;
	m_layerDup -> setEnabled(enable);
	m_layerRm -> setEnabled(enable);
	m_layerLink -> setEnabled(enable);
	m_layerHide -> setEnabled(enable);
	m_layerProperties -> setEnabled(enable);
	m_layerSaveAs -> setEnabled(enable);
	m_layerResize -> setEnabled(enable);
	m_layerResizeToImage -> setEnabled(enable);
	m_layerToImage -> setEnabled(enable);
	m_layerTransform -> setEnabled(enable);
	m_layerRaise -> setEnabled(enable && nlayers > 1 && layerPos);
	m_layerLower -> setEnabled(enable && nlayers > 1 && layerPos != nlayers - 1);
	m_layerTop -> setEnabled(enable && nlayers > 1 && layerPos);
	m_layerBottom -> setEnabled(enable && nlayers > 1 && layerPos != nlayers - 1);
	imgUpdateGUI();
}

void KisView::selectionUpdateGUI(bool enable)
{
	KisImageSP img = currentImg();

	enable = enable && img && img -> selection() && img -> selection() -> parent();
	m_selectionCut -> setEnabled(enable);
	m_selectionCopy -> setEnabled(enable);
	m_selectionToNewLayer -> setEnabled(enable);
	m_selectionPaste -> setEnabled(img != 0 && m_clipboardHasImage);
	m_selectionPasteInto -> setEnabled(img != 0 && m_clipboardHasImage);
	m_selectionRm -> setEnabled(enable);
	m_selectionFillBg -> setEnabled(enable);
	m_selectionFillFg -> setEnabled(enable);
	m_selectionSelectAll -> setEnabled(img != 0);
	m_selectionSelectNone -> setEnabled(enable);
}

void KisView::cut()
{
	copy();
	removeSelection();
}

void KisView::copy()
{
	KisImageSP img = currentImg();

	if (img) {
		KisFloatingSelectionSP selection = img -> selection();

		if (selection) {
			selection -> clearParentOnMove(false);
			m_doc -> setClipboardSelection(selection);
			imgSelectionChanged(currentImg());
		}
	}
}

void KisView::paste()
{
//	Q_ASSERT(!QApplication::clipboard() -> image().isNull());
//	activateTool(m_paste);
}

void KisView::paste_into()
{
	KisImageSP img = currentImg();

	Q_ASSERT(!QApplication::clipboard() -> image().isNull());

	if (img) {
		KisFloatingSelectionSP selection = m_doc -> clipboardSelection();
		KisLayerSP layer = m_doc -> layerAdd(img, img -> nextLayerName(), selection);

		img -> unsetSelection(false);

		if (layer) {
			layer -> setX(0);
			layer -> setY(0);
			img -> invalidate(layer -> bounds());
			updateCanvas(layer -> bounds());
		}
	}
}

void KisView::removeSelection()
{
	KisImageSP img = currentImg();

	if (img) {
		KisFloatingSelectionSP selection = img -> selection();

		if (selection) {
			KisPaintDeviceSP parent = selection -> parent();

			if (parent) {
				QRect rc = selection -> bounds();
				QRect ur;
				KisPainter gc(parent);

				Q_ASSERT(!m_adapter -> inMacro());
				m_adapter -> beginMacro(i18n("Remove Selection"));
				img -> unsetSelection(true);
				ur = rc;

				if (parent -> x() || parent -> y())
					rc.moveBy(-parent -> x(), -parent -> y());

				gc.beginTransaction("remove selection on parent");
				gc.eraseRect(rc);
				m_adapter -> addCommand(gc.end());
				m_adapter -> endMacro();
				m_doc -> setModified(true);
				img -> invalidate(rc);
				updateCanvas(ur);
			}
		}
	}
}

void KisView::copySelectionToNewLayer()
{
	KisImageSP img = currentImg();

	if (img) {
		KisFloatingSelectionSP selection = img -> selection();

		img -> unsetSelection(false);

		if (selection && m_doc -> layerAdd(img, img -> nextLayerName(), selection))
			layersUpdated();
		else
			img -> setSelection(selection);
	}
}


void KisView::imgUpdateGUI()
{
	const KisImageSP img = currentImg();
	Q_INT32 n = 0;
	Q_INT32 nvisible = 0;
	Q_INT32 nlinked = 0;

	m_imgRm -> setEnabled(img != 0);
	m_imgResize -> setEnabled(img != 0);
	m_imgDup -> setEnabled(img != 0);
	m_imgExport -> setEnabled(img != 0);
	m_layerAdd -> setEnabled(img != 0);
	m_imgResizeToLayer -> setEnabled(img && img -> activeLayer());

	if (img) {
		const vKisLayerSP& layers = img -> layers();

		n = layers.size();

		for (vKisLayerSP_cit it = layers.begin(); it != layers.end(); it++) {
			const KisLayerSP& layer = *it;

			if (layer -> linked())
				nlinked++;

			if (layer -> visible())
				nvisible++;

			if (nlinked > 1 && nvisible > 1)
				break;
		}
	}

	m_imgMergeAll -> setEnabled(n > 1);
	m_imgMergeVisible -> setEnabled(nvisible > 1);
	m_imgMergeLinked -> setEnabled(nlinked > 1);
}

void KisView::updateTabBar()
{
	QStringList lst = m_doc->images();

	m_tabBar->clear();

	// populate list
	if (!lst.empty()) {
		for (QStringList::Iterator it = lst.begin(); it != lst.end(); ++it)
			m_tabBar->addTab(*it);
	}

	if (currentImg())
		m_tabBar->setActiveTab(currentImgName());
}

void KisView::fillSelectionBg()
{
	fillSelection(bgColor(), OPACITY_OPAQUE);
}

void KisView::fillSelectionFg()
{
	fillSelection(fgColor(), OPACITY_OPAQUE);
}

void KisView::fillSelection(const KoColor& c, QUANTUM opacity)
{
	KisImageSP img = currentImg();

	if (img) {
		KisFloatingSelectionSP selection = img -> selection();

		if (selection) {
			QRect rc = selection -> bounds();
			QRect ur = rc;
			KisPainter gc(selection.data());

			rc.moveBy(-rc.x(), -rc.y());
			gc.beginTransaction(i18n("Fill Selection."));
			gc.fillRect(rc, c, opacity);
			m_adapter -> addCommand(gc.endTransaction());
			gc.end();
			img -> invalidate(ur);
			m_doc -> setModified(true);
			updateCanvas(ur);
		}
	}
}

void KisView::selectAll()
{
	KisImageSP img = currentImg();

	if (img) {
		KisPaintDeviceSP dev;

		img -> unsetSelection();
		dev = img -> activeDevice();

		if (dev) {
			KisFloatingSelectionSP selection = new KisFloatingSelection(dev, img, "Selection box from KisView", OPACITY_OPAQUE);

			selection -> setBounds(dev -> bounds());
			img -> setSelection(selection);
			updateCanvas();
		}
	}
}

void KisView::unSelectAll()
{
	KisImageSP img = currentImg();

	if (img) {
		img -> unsetSelection();
		updateCanvas();
	}
}

void KisView::zoomUpdateGUI(Q_INT32 x, Q_INT32 y, double zf)
{
	// Disable updates while we change the scrollbar settings.
	m_canvas -> setUpdatesEnabled(false);
	m_hScroll -> setUpdatesEnabled(false);
	m_vScroll -> setUpdatesEnabled(false);

	if (x < 0 || y < 0) {
		// Zoom about the centre of the current display
		KisImageSP img = currentImg();

		if (img) {
			if (m_hScroll -> isVisible()) {
				QPoint c = viewToWindow(QPoint(m_canvas -> width() / 2, m_canvas -> height() / 2));
				x = c.x();
			}
			else {
				x = img -> width() / 2;
			}

			if (m_vScroll -> isVisible()) {
				QPoint c = viewToWindow(QPoint(m_canvas -> width() / 2, m_canvas -> height() / 2));
				y = c.y();
			}
			else {
				y = img -> height() / 2;
			}
		}
		else {
			x = 0;
			y = 0;
		}
	}

	setZoom(zf);

	Q_ASSERT(m_zoomIn);
	Q_ASSERT(m_zoomOut);

        updateStatusBarZoomLabel ();

	m_zoomIn -> setEnabled(zf <= KISVIEW_MAX_ZOOM);
	m_zoomOut -> setEnabled(zf >= KISVIEW_MIN_ZOOM);

	resizeEvent(0);

	m_hRuler -> setZoom(zf);
	m_vRuler -> setZoom(zf);

	if (m_hScroll -> isVisible()) {
		Q_INT32 vcx = m_canvas -> width() / 2;
		Q_INT32 scrollX = qRound(x * zoom() - vcx);
		m_hScroll -> setValue(scrollX);
	}

	if (m_vScroll -> isVisible()) {
		Q_INT32 vcy = m_canvas -> height() / 2;
		Q_INT32 scrollY = qRound(y * zoom() - vcy);
		m_vScroll -> setValue(scrollY);
	}

	// Now update everything.
	m_canvas -> setUpdatesEnabled(true);
	m_hScroll -> setUpdatesEnabled(true);
	m_vScroll -> setUpdatesEnabled(true);
	m_hScroll -> update();
	m_vScroll -> update();
	canvasRefresh();
}

void KisView::zoomTo(const KisRect& r)
{
	if (!r.isNull()) {

		double wZoom = fabs(m_canvas -> width() / r.width());
		double hZoom = fabs(m_canvas -> height() / r.height());

		double zf = kMin(wZoom, hZoom);

		if (zf < KISVIEW_MIN_ZOOM) {
			zf = KISVIEW_MIN_ZOOM;
		}
		else
			if (zf > KISVIEW_MAX_ZOOM) {
				zf = KISVIEW_MAX_ZOOM;
			}

		Q_INT32 cx = qRound(r.center().x());
		Q_INT32 cy = qRound(r.center().y());

		zoomUpdateGUI(cx, cy, zf);
	}
}

void KisView::zoomTo(const QRect& r)
{
	zoomTo(KisRect(r));
}

void KisView::zoomTo(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
	zoomTo(KisRect(x, y, w, h));
}

void KisView::zoomIn(Q_INT32 x, Q_INT32 y)
{
	if (zoom() <= KISVIEW_MAX_ZOOM)
		zoomUpdateGUI(x, y, zoom() * 2);
}

void KisView::zoomOut(Q_INT32 x, Q_INT32 y)
{
	if (zoom() >= KISVIEW_MIN_ZOOM)
		zoomUpdateGUI(x, y, zoom() / 2);
}

void KisView::zoomIn()
{
	slotZoomIn();
}

void KisView::zoomOut()
{
	slotZoomOut();
}

void KisView::slotZoomIn()
{
	if (zoom() <= KISVIEW_MAX_ZOOM)
		zoomUpdateGUI(-1, -1, zoom() * 2);
}

void KisView::slotZoomOut()
{
	if (zoom() >= KISVIEW_MIN_ZOOM)
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
	GradientDialog *pGradientDialog = new GradientDialog(m_gradient);
 	pGradientDialog -> exec();

	if (pGradientDialog->result() == QDialog::Accepted) {
		/* set m_pGradient here and update gradientwidget in sidebar
		   to show sample of gradient selected. Also update effect for
		   the framebuffer's gradient, which is used in painting */

 		int type = pGradientDialog->gradientTab()->gradientType();
 		m_gradient->setEffect(static_cast<KImageEffect::GradientType>(type));

 		KisFrameBuffer *fb = m_doc->frameBuffer();
 		fb->setGradientEffect(static_cast<KImageEffect::GradientType>(type));

 		kdDebug() << "gradient type is " << type << endl;
	}
        delete pGradientDialog;
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

void KisView::imgResizeToActiveLayer()
{
	KisImageSP img = currentImg();
	KisLayerSP layer;

	if (img && (layer = img -> activeLayer())) {
		if (layer -> width() != img -> width() || layer -> height() != img -> height()) {
			img -> resize(layer -> width(), layer -> height());
			canvasRefresh();
		}
	}
}

void KisView::export_image()
{
	KURL url = KFileDialog::getSaveURL(QString::null, KisImageMagickConverter::writeFilters(), this, i18n("Export Image"));
	KisImageSP img = currentImg();
	KisLayerSP dst;

	if (url.isEmpty())
		return;

	Q_ASSERT(img);

	if (img) {
		KisImageMagickConverter ib(m_doc, m_adapter);
		img = new KisImage(*img);

		if (img -> nlayers() == 1) {
			dst = img -> layer(0);
			Q_ASSERT(dst);
		} else {
			dst = new KisLayer(img, img -> width(), img -> height(), "layer for exporting", OPACITY_OPAQUE);

			KisPainter gc;
			KisMerge<flattenAll> visitor(img, true);
			vKisLayerSP layers = img -> layers();

			gc.begin(dst.data());
			visitor(gc, layers);
			gc.end();
		}

		m_imgBuilderMgr -> attach(&ib);
		m_buildProgress -> changeSubject(&ib);

		switch (ib.buildFile(url, dst)) {
		case KisImageBuilder_RESULT_UNSUPPORTED:
			KMessageBox::error(this, i18n("No coder for this type of file."), i18n("Error Saving File"));
			break;
		case KisImageBuilder_RESULT_INVALID_ARG:
			KMessageBox::error(this, i18n("Invalid argument."), i18n("Error Saving File"));
			break;
		case KisImageBuilder_RESULT_NO_URI:
		case KisImageBuilder_RESULT_NOT_LOCAL:
			KMessageBox::error(this, i18n("Unable to locate file."), i18n("Error Saving File"));
			break;
		case KisImageBuilder_RESULT_BAD_FETCH:
			KMessageBox::error(this, i18n("Unable to upload file."), i18n("Error Saving File"));
			break;
		case KisImageBuilder_RESULT_EMPTY:
			KMessageBox::error(this, i18n("Empty file."), i18n("Error Saving File"));
			break;
		case KisImageBuilder_RESULT_FAILURE:
			KMessageBox::error(this, i18n("Error saving file."), i18n("Error Saving File"));
			break;
		case KisImageBuilder_RESULT_OK:
		default:
			break;
		}
	}
}

void KisView::slotInsertImageAsLayer()
{
	if (importImage(true) > 0)
		m_doc -> setModified(true);
}

void KisView::save_layer_as_image()
{
	KURL url = KFileDialog::getSaveURL(QString::null, KisImageMagickConverter::writeFilters(), this, i18n("Export Layer"));
	KisImageSP img = currentImg();

	if (url.isEmpty())
		return;

	Q_ASSERT(img);

	if (img) {
		KisImageMagickConverter ib(m_doc, m_adapter);
		KisLayerSP dst = img -> activeLayer();

		Q_ASSERT(dst);

		switch (ib.buildFile(url, dst)) {
		case KisImageBuilder_RESULT_UNSUPPORTED:
			KMessageBox::error(this, i18n("No coder for this type of file."), i18n("Error Saving File"));
			break;
		case KisImageBuilder_RESULT_INVALID_ARG:
			KMessageBox::error(this, i18n("Invalid argument."), i18n("Error Saving File"));
			break;
		case KisImageBuilder_RESULT_NO_URI:
		case KisImageBuilder_RESULT_NOT_LOCAL:
			KMessageBox::error(this, i18n("Unable to locate file."), i18n("Error Saving File"));
			break;
		case KisImageBuilder_RESULT_BAD_FETCH:
			KMessageBox::error(this, i18n("Unable to upload file."), i18n("Error Saving File"));
			break;
		case KisImageBuilder_RESULT_EMPTY:
			KMessageBox::error(this, i18n("Empty file."), i18n("Error Saving File"));
			break;
		case KisImageBuilder_RESULT_FAILURE:
			KMessageBox::error(this, i18n("Error saving file."), i18n("Error Saving File"));
			break;
		case KisImageBuilder_RESULT_OK:
		default:
			break;
		}
	}
}

void KisView::slotEmbedImage(const QString& filename)
{
	importImage(false, true, filename);
}

Q_INT32 KisView::importImage(bool createLayer, bool modal, const QString& filename)
{
	KURL::List urls;
	Q_INT32 rc = 0;

	if (filename.isEmpty())
		urls = KFileDialog::getOpenURLs(QString::null, KisImageMagickConverter::readFilters(), 0, i18n("Import Image"));
	else
		urls.push_back(filename);

	if (urls.empty())
		return 0;

	KisImageMagickConverter ib(m_doc, m_adapter);
	KisImageSP img;

	m_imgBuilderMgr -> attach(&ib);

	for (KURL::List::iterator it = urls.begin(); it != urls.end(); it++) {
		KURL url = *it;
		KisDlgBuilderProgress dlg(&ib);

		if (modal)
			dlg.show();
		else
			m_buildProgress -> changeSubject(&ib);

		switch (ib.buildImage(url)) {
		case KisImageBuilder_RESULT_UNSUPPORTED:
			KMessageBox::error(this, i18n("No coder for the type of file %1.").arg(url.path()), i18n("Error Importing File"));
			continue;
		case KisImageBuilder_RESULT_NO_URI:
		case KisImageBuilder_RESULT_NOT_LOCAL:
			KNotifyClient::event(this -> winId(), "cannotopenfile");
			continue;
		case KisImageBuilder_RESULT_NOT_EXIST:
			KMessageBox::error(this, i18n("File %1 does not exist.").arg(url.path()), i18n("Error Importing File"));
			KNotifyClient::event(this -> winId(), "cannotopenfile");
			continue;
		case KisImageBuilder_RESULT_BAD_FETCH:
			KMessageBox::error(this, i18n("Unable to download file %1.").arg(url.path()), i18n("Error Importing File"));
			KNotifyClient::event(this -> winId(), "cannotopenfile");
			continue;
		case KisImageBuilder_RESULT_EMPTY:
			KMessageBox::error(this, i18n("Empty file: %1").arg(url.path()), i18n("Error Importing File"));
			KNotifyClient::event(this -> winId(), "cannotopenfile");
			continue;
		case KisImageBuilder_RESULT_FAILURE:
			m_buildProgress -> changeSubject(0);
			KMessageBox::error(this, i18n("Error loading file %1.").arg(url.path()), i18n("Error Importing File"));
			KNotifyClient::event(this -> winId(), "cannotopenfile");
			continue;
		case KisImageBuilder_RESULT_PROGRESS:
			break;
		case KisImageBuilder_RESULT_OK:
		default:
			break;
		}

		if (!(img = ib.image()))
			continue;

		if (createLayer && currentImg()) {
			vKisLayerSP v = img -> layers();
			KisImageSP current = currentImg();

			rc += v.size();
			current -> unsetSelection();

			for (vKisLayerSP_it it = v.begin(); it != v.end(); it++) {
				KisLayerSP layer = *it;

				layer -> setImage(current);
				layer -> setName(current -> nextLayerName());
				m_doc -> layerAdd(current, layer, 0);
				m_layerBox -> setCurrentItem(img -> index(layer));
			}

			current -> invalidate();
			resizeEvent(0);
			updateCanvas();
		} else {
			m_doc -> addImage(img);
			rc++;
		}
	}

	if (img && !createLayer)
		selectImage(img);

	return rc;
}

void KisView::layerTransform(bool )
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
		kdDebug() << "layer_transform() failed" << endl;
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
	if (!currentImg()) return;
	KisLayerSP layer = currentImg() -> activeLayer();
	if (!layer) return;


	QWMatrix m;
	//m.rotate(180);
	layer->transform(m.rotate(180));

	layersUpdated();
	resizeEvent(0);
	currentImg() -> invalidate();
	updateCanvas();
}

void KisView::layer_rotateleft90()
{
	if (!currentImg()) return;
	KisLayerSP layer = currentImg() -> activeLayer();
	if (!layer) return;

	QWMatrix m;
	m.rotate(270);
	layer->transform(m);

	layersUpdated();
	resizeEvent(0);
	currentImg() -> invalidate();
	updateCanvas();

}

void KisView::layer_rotateright90()
{
	if (!currentImg()) return;
	KisLayerSP layer = currentImg() -> activeLayer();
	if (!layer) return;


	QWMatrix m;
	m.rotate(90);
	layer->transform(m);

	layersUpdated();
	resizeEvent(0);
	currentImg() -> invalidate();
	updateCanvas();

}

void KisView::layer_rotate_custom()
{

}

void KisView::layer_mirrorX()
{
	if (!currentImg()) return;
	KisLayerSP layer = currentImg() -> activeLayer();
	if (!layer) return;

	layer->mirrorX();
	layersUpdated();
	currentImg() -> invalidate();
	updateCanvas();
}

void KisView::layer_mirrorY()
{
	if (!currentImg()) return;
	KisLayerSP layer = currentImg() -> activeLayer();
	if (!layer) return;

	layer->mirrorY();
	layersUpdated();
	currentImg() -> invalidate();
	updateCanvas();
}

void KisView::add_new_image_tab()
{
	m_doc -> slotNewImage();
	updateTabBar();
}

void KisView::remove_current_image_tab()
{
	if ( m_doc->nimages() == 1 )
		return;

	KisImageSP current = currentImg();

	if (current) {
		disconnectCurrentImg();
		m_current = 0;
		m_doc -> removeImage(current);
	}
}

void KisView::imageResize()
{
	KisImageSP img = currentImg();

	if (img) {
		KisConfig cfg;
		KisDlgDimension dlg(cfg.maxImgWidth(), img -> width(), cfg.maxImgHeight(), img -> height(), this);

		if (dlg.exec() == QDialog::Accepted) {
			QSize size = dlg.getSize();

			img -> resize(size.width(), size.height());
			img -> invalidate();
			resizeEvent(0);
			layersUpdated();
			canvasRefresh();
		}
	}
}

void KisView::merge_all_layers()
{
	KisImageSP img = currentImg();

	if (img) {
		KisLayerSP dst = new KisLayer(img, img -> width(), img -> height(), img -> nextLayerName(), OPACITY_OPAQUE);
		KisPainter gc(dst.data());
		gc.fillRect(0, 0, dst -> width(), dst -> height(), KoColor(0, 0, 0), OPACITY_TRANSPARENT);
		vKisLayerSP layers = img -> layers();
		KisMerge<flattenAll> visitor(img, false);

		visitor(gc, layers);
		img -> add(dst, -1);
		layersUpdated();
		updateCanvas();
	}
}

void KisView::merge_visible_layers()
{
	KisImageSP img = currentImg();

	if (img) {
		KisLayerSP dst = new KisLayer(img, img -> width(), img -> height(), img -> nextLayerName(), OPACITY_OPAQUE);
		KisPainter gc(dst.data());
		gc.fillRect(0, 0, dst -> width(), dst -> height(), KoColor(0, 0, 0), OPACITY_TRANSPARENT);
		KisMerge<flattenAllVisible> visitor(img, false);
		vKisLayerSP layers = img -> layers();

		visitor(gc, layers);
		img -> add(dst, -1);
		layersUpdated();
		updateCanvas();
	}
}

void KisView::merge_linked_layers()
{
	KisImageSP img = currentImg();

	if (img) {
		KisLayerSP dst = new KisLayer(img, img -> width(), img -> height(), img -> nextLayerName(), OPACITY_OPAQUE);
		KisPainter gc(dst.data());
		gc.fillRect(0, 0, dst -> width(), dst -> height(), KoColor(0, 0, 0), OPACITY_TRANSPARENT);
		KisMerge<flattenAllLinked> visitor(img, false);
		vKisLayerSP layers = img -> layers();

		visitor(gc, layers);
		img -> add(dst, -1);
		layersUpdated();
		updateCanvas();
	}
}

/*
  preferences - the main Krayon preferences dialog - modeled
  after the konqueror prefs dialog - quite nice compound dialog
*/
void KisView::preferences()
{
	
	PreferencesDialog::editPreferences();
}

void KisView::viewColorDocker()
{
	if( m_colordocker->isVisible() == false )
	{
		mainWindow()->addDockWindow( m_colordocker, DockRight );
		m_colordocker->show();
	}
}

void KisView::viewControlDocker()
{
	if( m_toolcontroldocker->isVisible() == false )
	{
		mainWindow()->addDockWindow( m_toolcontroldocker, DockRight );
		m_toolcontroldocker->show();
	}
}

void KisView::viewLayerChannelDocker()
{
	if( m_layerchanneldocker->isVisible() == false )
	{
		mainWindow()->addDockWindow( m_layerchanneldocker, DockRight );
		m_layerchanneldocker->show();
	}
}

void KisView::viewResourceDocker()
{
	if( m_resourcedocker->isVisible() == false )
	{
		mainWindow()->addDockWindow( m_resourcedocker, DockRight );
		m_resourcedocker->show();
	}
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
	// with showScrollBars()
}

void KisView::brushActivated(KisResource *brush)
{
	KisIconItem *item;

	Q_ASSERT(m_toolcontroldocker);
	m_brush = dynamic_cast<KisBrush*>(brush);

	if (m_brush && (item = m_brushMediator -> itemFor(m_brush)))
		m_controlWidget -> slotSetBrush(item);

        if (m_brush)
		notify();
}

void KisView::patternActivated(KisResource *pattern)
{
	KisIconItem *item;

	Q_ASSERT(m_toolcontroldocker);
	m_pattern = dynamic_cast<KisPattern*>(pattern);

	if (m_pattern && (item = m_patternMediator -> itemFor(m_pattern)))
		m_controlWidget -> slotSetPattern(item);

        if (m_pattern)
		notify();
}

void KisView::setBGColor(const KoColor& c)
{
	emit bgColorChanged(c);
	m_bg = c;
	notify();
}

void KisView::setFGColor(const KoColor& c)
{
	emit fgColorChanged(c);
	m_fg = c;
        notify();
}

void KisView::slotSetFGColor(const KoColor& c)
{
	setFGColor(c);
}

void KisView::slotSetBGColor(const KoColor& c)
{
	setBGColor(c);
}

void KisView::setupPrinter(KPrinter& printer)
{
	KisImageSP img = currentImg();

	if (img) {
		Q_INT32 count;

		printer.setPageSelection(KPrinter::ApplicationSide);
		count = m_doc -> imageIndex(img);
		printer.setCurrentPage(1 + count);
		printer.setMinMax(1, m_doc -> nimages());
		printer.setPageSize(KPrinter::A4);
		printer.setOrientation(KPrinter::Portrait);
	}
}

void KisView::print(KPrinter& printer)
{
	QPainter gc(&printer);
	Q_INT32 from = printer.fromPage();
	Q_INT32 to = printer.toPage();
	KisImageSP img;

	printer.setFullPage(true);
	gc.setClipping(false);

	if (!from && !to) {
		from = printer.minPage();
		to = printer.maxPage();
	}

	for (Q_INT32 i = from; i < to; i++) {
		if (i)
			printer.newPage();

		img = m_doc -> imageNum(i - 1);
		Q_ASSERT(img);
		m_doc -> setProjection(img);
		m_doc -> paintContent(gc, img -> bounds());
	}
}

void KisView::setupTools()
{
	KisToolFactory *factory = KisToolFactory::singleton();

	Q_ASSERT(factory);
	factory -> create(actionCollection(), this);
}

void KisView::canvasGotPaintEvent(QPaintEvent *event)
{
	KisRect r = viewToWindow(KisRect(event -> rect()));
	paintView(r);
}

void KisView::canvasGotMousePressEvent(QMouseEvent *e)
{
	KisImageSP img = currentImg();

	if (img) {
		QPoint pt = mapToScreen(e -> pos());
		KisGuideMgr *mgr = img -> guides();

		m_lastGuidePoint = mapToScreen(e -> pos());
		m_currentGuide = 0;

		if ((e -> state() & ~ShiftButton) == Qt::NoButton) {
			KisGuideSP gd = mgr -> find(static_cast<Q_INT32>(pt.x() / zoom()), static_cast<Q_INT32>(pt.y() / zoom()), QMAX(2.0, 2.0 / zoom()));

			if (gd) {
				m_currentGuide = gd;

				if ((e -> button() == Qt::RightButton) || ((e -> button() & Qt::ShiftButton) == Qt::ShiftButton)) {
					if (gd -> isSelected())
						mgr -> unselect(gd);
					else
						mgr -> select(gd);
				} else {
					if (!gd -> isSelected()) {
						mgr -> unselectAll();
						mgr -> select(gd);
					}
				}

				updateGuides();
				return;
			}
		}
	}

	if (m_tool) {
		QPoint p = viewToWindow(e -> pos());
		QMouseEvent ev(QEvent::MouseButtonPress, p, e -> globalPos(), e -> button(), e -> state());

		m_tool -> mousePress(&ev);
	}
}

void KisView::canvasGotMouseMoveEvent(QMouseEvent *e)
{
	KisImageSP img = currentImg();

	m_hRuler -> updatePointer(e -> pos().x() - canvasXOffset(), e -> pos().y() - canvasYOffset());
	m_vRuler -> updatePointer(e -> pos().x() - canvasXOffset(), e -> pos().y() - canvasYOffset());

	QPoint wp = viewToWindow(e -> pos());

	if (img && m_currentGuide) {
		QPoint p = mapToScreen(e -> pos());
		KisGuideMgr *mgr = img -> guides();

		if ((e -> state() & LeftButton == LeftButton) && mgr -> hasSelected()) {
			eraseGuides();
			p -= m_lastGuidePoint;

			if (p.x())
				mgr -> moveSelectedByX(p.x() / zoom());

			if (p.y())
				mgr -> moveSelectedByY(p.y() / zoom());

			m_doc -> setModified(true);
			paintGuides();
		}
	} else if (m_tool) {
		QMouseEvent ev(QEvent::MouseButtonPress, wp, e -> globalPos(), e -> button(), e -> state());

		m_tool -> mouseMove(&ev);
	}

	m_lastGuidePoint = mapToScreen(e -> pos());
	emit cursorPosition(wp.x(), wp.y());
}

void KisView::canvasGotMouseReleaseEvent(QMouseEvent *e)
{
	KisImageSP img = currentImg();

	if (img && m_currentGuide) {
		m_currentGuide = 0;
	} else if (m_tool) {
		QPoint p = viewToWindow(e -> pos());
		QMouseEvent ev(QEvent::MouseButtonPress, p, e -> globalPos(), e -> button(), e -> state());

		m_tool -> mouseRelease(&ev);
	}
}

void KisView::canvasGotTabletEvent(QTabletEvent *e)
{
	KisImageSP img = currentImg();
	if (img ) {
		if ( m_tool ) {
			// Compute offset between screen coordinates and (zoomed) canvas coordinates.
			QPoint p = viewToWindow(e -> pos());
			// Synthesize a new tablet event.
			// Pity I haven't got a tablet that's got tilt, too
			QTabletEvent ev(e -> type(),
					p,
					e -> globalPos(),
					e -> device(),
					e -> pressure(),
					e -> xTilt(),
					e -> yTilt(),
					e -> uniqueId());

			m_tool->tabletEvent( &ev );
		}
	}
}

void KisView::canvasGotEnterEvent(QEvent *e)
{
	if (m_tool)
		m_tool -> enter(e);
}

void KisView::canvasGotLeaveEvent (QEvent *e)
{
	if (m_tool)
		m_tool -> leave(e);
}

void KisView::canvasGotMouseWheelEvent(QWheelEvent *event)
{
	QApplication::sendEvent(m_vScroll, event);
}

void KisView::canvasGotKeyPressEvent(QKeyEvent *event)
{
	if (m_tool)
		m_tool -> keyPress(event);
}

void KisView::canvasGotKeyReleaseEvent(QKeyEvent *event)
{
	if (m_tool)
		m_tool -> keyRelease(event);
}

void KisView::canvasRefresh()
{
	m_canvas -> repaint();
}

void KisView::layerToggleVisible()
{
	KisImageSP img = currentImg();

	if (img) {
		KisLayerSP layer = img -> activeLayer();

		if (layer) {
			layer -> visible(!layer -> visible());
			img -> invalidate();
			m_doc -> setModified(true);
			resizeEvent(0);
			layersUpdated();
			canvasRefresh();
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
	disconnectCurrentImg();
	m_current = 0;
	zoomUpdateGUI(0, 0, 1.0);
	resizeEvent(0);
	updateCanvas();

	if (!currentImg())
		layersUpdated();

	imgUpdateGUI();
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
			KisPaintPropertyDlg dlg(layer -> name(),
						QPoint(layer -> x(),
						       layer -> y()),
						layer -> opacity(),
						layer -> compositeOp());

			if (dlg.exec() == QDialog::Accepted) {
				QPoint pt = dlg.getPosition();

				bool changed = layer -> name() != dlg.getName()
					|| layer -> opacity() != dlg.getOpacity()
					|| layer -> compositeOp() != dlg.getCompositeOp()
					|| pt.x() != layer -> x()
					|| pt.y() != layer -> y();


				if (changed)
					m_adapter -> beginMacro(i18n("Property changes"));

				if (layer -> name() != dlg.getName()
				    || layer -> opacity() != dlg.getOpacity()
				    || layer -> compositeOp() != dlg.getCompositeOp())
					m_doc -> setLayerProperties(img, layer, dlg.getOpacity(), dlg.getCompositeOp(), dlg.getName());

				if (pt.x() != layer -> x() || pt.y() != layer -> y())
					KisStrategyMove(this).simpleMove(QPoint(layer -> x(), layer -> y()), pt);

				if (changed)
					m_adapter -> endMacro();
			}
		}
	}
}

void KisView::layerAdd()
{
	KisImageSP img = currentImg();

	if (img) {
		KisConfig cfg;
		NewLayerDialog dlg(cfg.maxLayerWidth(),
                                   cfg.defLayerWidth(),
                                   cfg.maxLayerHeight(),
                                   cfg.defLayerHeight(),
				   img -> imgType(),
				   img -> nextLayerName(),
                                   this);

		dlg.exec();

		if (dlg.result() == QDialog::Accepted) {
			KisLayerSP layer = m_doc -> layerAdd(img,
                                                             dlg.layerWidth(),
							     dlg.layerHeight(),
							     dlg.layerName(),
							     dlg.compositeOp(),
							     dlg.opacity(),
							     dlg.position(),
							     dlg.imageType());

			if (layer) {
				m_layerBox -> setCurrentItem(img -> index(layer));
				resizeEvent(0);
				updateCanvas(layer -> x(), layer -> y(), layer -> width(), layer -> height());
				cfg.defLayerWidth(dlg.layerWidth());
				cfg.defLayerHeight(dlg.layerHeight());
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

void KisView::layerDuplicate()
{
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
		layersUpdated();
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
		layersUpdated();
		resizeEvent(0);
		updateCanvas();
	}
}

void KisView::layerFront()
{
	KisImageSP img = currentImg();
	KisLayerSP layer;

	if (img && (layer = img -> activeLayer())) {
		img -> top(layer);
		layersUpdated();
		resizeEvent(0);
		updateCanvas();
	}
}

void KisView::layerBack()
{
	KisImageSP img = currentImg();
	KisLayerSP layer;

	if (img && (layer = img -> activeLayer())) {
		img -> bottom(layer);
		layersUpdated();
		resizeEvent(0);
		updateCanvas();
	}
}

void KisView::layerLevel(int /*n*/)
{
}

void KisView::layersUpdated()
{
	KisImageSP img = currentImg();

	layerUpdateGUI(img && img -> activeLayer());
	m_layerBox -> setUpdatesEnabled(false);
	m_layerBox -> clear();

	if (img) {
		vKisLayerSP l = img -> layers();

		for (vKisLayerSP_it it = l.begin(); it != l.end(); it++)
			m_layerBox -> insertItem((*it) -> name(), (*it) -> visible(), (*it) -> linked());

		m_layerBox -> setCurrentItem(img -> index(img -> activeLayer()));
		m_doc -> setModified(true);
	}

	m_layerBox -> setUpdatesEnabled(true);
}

void KisView::layersUpdated(KisImageSP img)
{
	if (img == currentImg())
		layersUpdated();
}

void KisView::selectImage(const QString& name)
{
	disconnectCurrentImg();
	m_current = m_doc -> findImage(name);
	connectCurrentImg();
	layersUpdated();
	selectionUpdateGUI(m_current && m_current -> selection());
	resizeEvent(0);
	updateCanvas();
	notify();
}

void KisView::selectImage(KisImageSP img)
{
	disconnectCurrentImg();
	m_current = img;
	connectCurrentImg();
	layersUpdated();
	resizeEvent(0);
	updateCanvas();

 	if (m_tabBar)
 		updateTabBar();

	selectionUpdateGUI(m_current && m_current -> selection());
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
	m_hRuler -> updateVisibleArea(value, 0);
	canvasRefresh();
}

void KisView::scrollV(int value)
{
	m_vRuler -> updateVisibleArea(0, value);
	canvasRefresh();
}

int KisView::canvasXOffset() const
{
	return static_cast<Q_INT32>(ceil(m_xoff * zoom()));
}

int KisView::canvasYOffset() const
{
	return static_cast<Q_INT32>(ceil(m_yoff * zoom()));
}

void KisView::setupCanvas()
{
	m_canvas = new KisCanvas(this, "kis_canvas");

	QObject::connect(m_canvas, SIGNAL(mousePressed(QMouseEvent*)), this, SLOT(canvasGotMousePressEvent(QMouseEvent*)));
	QObject::connect(m_canvas, SIGNAL(mouseMoved(QMouseEvent*)), this, SLOT(canvasGotMouseMoveEvent(QMouseEvent*)));
	QObject::connect(m_canvas, SIGNAL(mouseReleased(QMouseEvent*)), this, SLOT(canvasGotMouseReleaseEvent(QMouseEvent*)));
        QObject::connect(m_canvas, SIGNAL(gotTabletEvent(QTabletEvent*)), this, SLOT(canvasGotTabletEvent(QTabletEvent*)));
	QObject::connect(m_canvas, SIGNAL(gotPaintEvent(QPaintEvent*)), this, SLOT(canvasGotPaintEvent(QPaintEvent*)));
	QObject::connect(m_canvas, SIGNAL(gotEnterEvent(QEvent*)), this, SLOT(canvasGotEnterEvent(QEvent*)));
	QObject::connect(m_canvas, SIGNAL(gotLeaveEvent(QEvent*)), this, SLOT(canvasGotLeaveEvent(QEvent*)));
	QObject::connect(m_canvas, SIGNAL(mouseWheelEvent(QWheelEvent*)), this, SLOT(canvasGotMouseWheelEvent(QWheelEvent*)));
	QObject::connect(m_canvas, SIGNAL(gotKeyPressEvent(QKeyEvent*)), this, SLOT(canvasGotKeyPressEvent(QKeyEvent*)));
	QObject::connect(m_canvas, SIGNAL(gotKeyReleaseEvent(QKeyEvent*)), this, SLOT(canvasGotKeyReleaseEvent(QKeyEvent*)));
}

void KisView::projectionUpdated(KisImageSP img)
{
	if (img == currentImg())
		canvasRefresh();
}

bool KisView::selectColor(KoColor& result)
{
	QColor color;
	bool rc;

	if ((rc = (KColorDialog::getColor(color) == KColorDialog::Accepted)))
		result.setRGB(color.red(), color.green(), color.blue());

	return rc;
}

void KisView::selectFGColor()
{
	KoColor c;

	if (selectColor(c))
		setFGColor(c);
}

void KisView::selectBGColor()
{
	KoColor c;

	if (selectColor(c))
		setBGColor(c);
}

void KisView::reverseFGAndBGColors()
{
	KoColor oldFg = m_fg;
	KoColor oldBg = m_bg;

	setFGColor(oldBg);
	setBGColor(oldFg);
}

void KisView::imgSelectionChanged(KisImageSP img)
{
	if (img == currentImg())
		selectionUpdateGUI(img -> selection() != 0);
}

void KisView::connectCurrentImg() const
{
	if (m_current) {
		connect(m_current, SIGNAL(selectionChanged(KisImageSP)), SLOT(imgSelectionChanged(KisImageSP)));
		connect(m_current, SIGNAL(update(KisImageSP, const QRect&)), SLOT(imgUpdated(KisImageSP, const QRect&)));
	}
}

void KisView::disconnectCurrentImg() const
{
	if (m_current)
		m_current -> disconnect(this);
}

void KisView::duplicateCurrentImg()
{
	KisImageSP img = currentImg();

	Q_ASSERT(img);

	if (img) {
		KisImageSP dubbed = new KisImage(*img);

		dubbed -> setName(m_doc -> nextImageName());
		m_doc -> addImage(dubbed);
	}
}

void KisView::setupClipboard()
{
	QClipboard *cb = QApplication::clipboard();

	connect(cb, SIGNAL(dataChanged()), SLOT(clipboardDataChanged()));
}

void KisView::clipboardDataChanged()
{
	m_clipboardHasImage = !QApplication::clipboard() -> image().isNull();
}

void KisView::imgUpdated(KisImageSP img, const QRect& rc)
{
	if (img == currentImg()) {
		updateCanvas(rc);
	}
}

void KisView::layerResize()
{
	KisImageSP img = currentImg();

	if (img) {
		KisLayerSP layer = img -> activeLayer();

		if (layer) {
			KisConfig cfg;
			KisDlgDimension dlg(cfg.maxLayerWidth(), layer -> width(), cfg.maxLayerHeight(), layer -> height(), this);

			if (dlg.exec() == QDialog::Accepted) {
				QSize size = dlg.getSize();

				layer -> resize(size.width(), size.height());
				img -> invalidate();
				layersUpdated();
				resizeEvent(0);
				canvasRefresh();
			}
		}
	}
}

void KisView::layerResizeToImage()
{
	KisImageSP img = currentImg();

	if (img) {
		KisLayerSP layer = img -> activeLayer();

		if (layer) {
			if (layer -> width() == img -> width() || layer -> height() == img -> height())
				return;

			layer -> resize(img -> width(), img -> height());
			img -> invalidate();
			layersUpdated();
			resizeEvent(0);
			canvasRefresh();
		}
	}
}

void KisView::layerToImage()
{
	KisImageSP img = currentImg();

	if (img) {
		KisFloatingSelectionSP selection = img -> selection();
		KisLayerSP layer;

		if (selection)
			layer = selection.data();
		else
			img -> activeLayer();

		if (layer) {
			KisImageSP dupedImg = new KisImage(m_adapter, layer -> width(), layer -> height(), img -> nativeImgType(), m_doc -> nextImageName());
			KisLayerSP duped = new KisLayer(*layer);

			duped -> setName(dupedImg -> nextLayerName());
			duped -> setX(0);
			duped -> setY(0);
			dupedImg -> add(duped, -1);
			m_doc -> addImage(dupedImg);
			selectImage(dupedImg);
		}
	}
}

void KisView::layerTransform()
{
	if (!currentImg()) return;

// 	KisDlgTransform dlg(this);
// 	if (dlg.exec() == QDialog::Accepted) {
// 		// transform current layer
// 		KisLayerSP layer = currentImg() -> activeLayer();
// 		if (!layer) return;

// 		layer -> transform(dlg.matrix());

// 		layersUpdated();
// 		updateCanvas();
// 	}
}

QPoint KisView::viewToWindow(const QPoint& pt)
{
	QPoint converted;

	converted.rx() = static_cast<int>((pt.x() + horzValue() - canvasXOffset()) / zoom());
	converted.ry() = static_cast<int>((pt.y() + vertValue() - canvasYOffset()) / zoom());

	return converted;
}

KisPoint KisView::viewToWindow(const KisPoint& pt)
{
	KisPoint converted;

	converted.setX((pt.x() + horzValue() - canvasXOffset()) / zoom());
	converted.setY((pt.y() + vertValue() - canvasYOffset()) / zoom());

	return converted;
}

QRect KisView::viewToWindow(const QRect& rc)
{
	QRect r;
	QPoint p = viewToWindow(QPoint(rc.x(), rc.y()));
        r.setX(p.x());
	r.setY(p.y());
	r.setWidth(static_cast<int>(ceil(rc.width() / zoom())));
	r.setHeight(static_cast<int>(ceil(rc.height() / zoom())));

	return r;
}

KisRect KisView::viewToWindow(const KisRect& rc)
{
	KisRect r;
	KisPoint p = viewToWindow(KisPoint(rc.x(), rc.y()));
        r.setX(p.x());
	r.setY(p.y());
	r.setWidth(rc.width() / zoom());
	r.setHeight(rc.height() / zoom());

	return r;
}

void KisView::viewToWindow(Q_INT32 *x, Q_INT32 *y)
{
	if (x && y) {
		QPoint p = viewToWindow(QPoint(*x, *y));
		*x = p.x();
		*y = p.y();
	}
}

QPoint KisView::windowToView(const QPoint& pt)
{
	QPoint p;
	p.setX(static_cast<int>(pt.x() * zoom() + canvasXOffset() - horzValue()));
	p.setY(static_cast<int>(pt.y() * zoom() + canvasYOffset() - vertValue()));

	return p;
}

KisPoint KisView::windowToView(const KisPoint& pt)
{
	KisPoint p;
	p.setX(pt.x() * zoom() + canvasXOffset() - horzValue());
	p.setY(pt.y() * zoom() + canvasYOffset() - vertValue());

	return p;
}

QRect KisView::windowToView(const QRect& rc)
{
	QRect r;
	QPoint p = windowToView(QPoint(rc.x(), rc.y()));
	r.setX(p.x());
	r.setY(p.y());
	r.setWidth(static_cast<int>(ceil(rc.width() * zoom())));
	r.setHeight(static_cast<int>(ceil(rc.height() * zoom())));

	return r;
}

KisRect KisView::windowToView(const KisRect& rc)
{
	KisRect r;
	KisPoint p = windowToView(KisPoint(rc.x(), rc.y()));
	r.setX(p.x());
	r.setY(p.y());
	r.setWidth(rc.width() * zoom());
	r.setHeight(rc.height() * zoom());

	return r;
}

void KisView::windowToView(Q_INT32 *x, Q_INT32 *y)
{
	if (x && y) {
		QPoint p = windowToView(QPoint(*x, *y));
		*x = p.x();
		*y = p.y();
	}
}

void KisView::guiActivateEvent(KParts::GUIActivateEvent *event)
{
	KStatusBar *sb = statusBar();

	if (sb)
		sb -> show();

	super::guiActivateEvent(event);
}

void KisView::nBuilders(Q_INT32 size)
{
	m_imgImport -> setEnabled(size == 0);
	m_imgExport -> setEnabled(size == 0);

	if (m_imgScan)
		m_imgScan -> setEnabled(size == 0);
}

bool KisView::eventFilter(QObject *o, QEvent *e)
{
	if ((o == m_hRuler || o == m_vRuler) && (e -> type() == QEvent::MouseMove || e -> type() == QEvent::MouseButtonRelease)) {
		QMouseEvent *me = dynamic_cast<QMouseEvent*>(e);
		QPoint pt = mapFromGlobal(me -> globalPos());
		KisImageSP img = currentImg();
		KisGuideMgr *mgr;
		QMouseEvent *m;

		if (!img)
			return super::eventFilter(o, e);

		mgr = img -> guides();

		if (e -> type() == QEvent::MouseMove) {
			bool flag = geometry().contains(pt);
			KisGuideSP gd;

			if (m_currentGuide == 0 && flag) {
				// No guide is being edited and moving mouse over the canvas.
				// Create a new guide.
				enterEvent(0);
				eraseGuides();
				mgr -> unselectAll();

				if (o == m_vRuler)
					gd = mgr -> add((pt.x() - m_vRuler -> width() + horzValue()) / zoom(), Qt::Vertical);
				else
					gd = mgr -> add((pt.y() - m_hRuler -> height() + vertValue()) / zoom(), Qt::Horizontal);

				m_currentGuide = gd;
				mgr -> select(gd);
				m_lastGuidePoint = mapToScreen(pt);
			} else if (m_currentGuide) {
				if (flag) {
					// moved an existing guide.
					m = new QMouseEvent(QEvent::MouseMove, pt, me -> globalPos(), me -> button(), me -> state());
					canvasGotMouseMoveEvent(m);
					delete m;
				} else {
					//  moved a guide out of the frame, destroy it
					leaveEvent(0);
					eraseGuides();
					mgr -> remove(m_currentGuide);
					paintGuides();
					m_currentGuide = 0;
				}
			}
		} else if (e -> type() == QEvent::MouseButtonRelease && m_currentGuide) {
			eraseGuides();
			mgr -> unselect(m_currentGuide);
			paintGuides();
			m_currentGuide = 0;
			enterEvent(0);
			m = new QMouseEvent(QEvent::MouseMove, pt, me -> globalPos(), Qt::NoButton, Qt::NoButton);
			canvasGotMouseMoveEvent(m);
			delete m;
		}
	}

	return super::eventFilter(o, e);
}

void KisView::eraseGuides()
{
	KisImageSP img = currentImg();

	if (img) {
		KisGuideMgr *mgr = img -> guides();

		if (mgr)
			mgr -> erase(m_canvas, this, horzValue() - canvasXOffset(), vertValue() - canvasYOffset(), zoom());
	}
}

void KisView::paintGuides()
{
	KisImageSP img = currentImg();

	if (img) {
		KisGuideMgr *mgr = img -> guides();

		if (mgr)
			mgr -> paint(m_canvas, this, horzValue() - canvasXOffset(), vertValue() - canvasYOffset(), zoom());
	}
}

void KisView::updateGuides()
{
	eraseGuides();
	paintGuides();
}

QPoint KisView::mapToScreen(const QPoint& pt)
{
	QPoint converted;

	converted.rx() = pt.x() + horzValue() - canvasXOffset();
	converted.ry() = pt.y() + vertValue() - canvasYOffset();
	return converted;
}

void KisView::attach(KisCanvasObserver *observer)
{
	if (observer)
		m_observers.push_back(observer);
}

void KisView::detach(KisCanvasObserver *observer)
{
	vKisCanvasObserver_it it = std::find(m_observers.begin(), m_observers.end(), observer);

	if (it != m_observers.end())
		m_observers.erase(it);
}

void KisView::notify()
{
	for (vKisCanvasObserver_it it = m_observers.begin(); it != m_observers.end(); it++)
		(*it) -> update(this);
}

KisImageSP KisView::currentImg() const
{
	if (m_current && m_doc -> contains(m_current))
		return m_current;

	m_current = m_doc -> imageNum(m_doc -> nimages() - 1);
	connectCurrentImg();
	return m_current;
}

QString KisView::currentImgName() const
{
	if (currentImg())
		return currentImg() -> name();

	return QString::null;
}

KoColor KisView::bgColor() const
{
	return m_bg;
}

KoColor KisView::fgColor() const
{
	return m_fg;
}

KisBrush *KisView::currentBrush() const
{
	return m_brush;
}

KisPattern *KisView::currentPattern() const
{
	return m_pattern;
}

KisGradient *KisView::currentGradient() const
{
	return m_gradient;
}

double KisView::zoomFactor() const
{
	return zoom();
}

KisUndoAdapter *KisView::undoAdapter() const
{
	return m_adapter;
}

KisCanvasControllerInterface *KisView::canvasController() const
{
	return const_cast<KisCanvasControllerInterface*>(static_cast<const KisCanvasControllerInterface*>(this));
}

KisToolControllerInterface *KisView::toolController() const
{
	return const_cast<KisToolControllerInterface*>(static_cast<const KisToolControllerInterface*>(this));
}

KoDocument *KisView::document() const
{
	return koDocument();
}

#include "kis_view.moc"


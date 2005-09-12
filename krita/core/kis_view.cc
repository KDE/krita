/*
 * This file is part of KimageShop^WKrayon^WKrita
 *
*  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *                1999 Michael Koch    <koch@kde.org>
 *                1999 Carsten Pfeiffer <pfeiffer@kde.org>
 *                2002 Patrick Julien <freak@codepimps.org>
 *                2003-2005 Boudewijn Rempt <boud@valdyas.org>
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
#include <qcursor.h>
#include <qevent.h>
#include <qpainter.h>
#include <qscrollbar.h>
#include <qspinbox.h>
#include <qdockarea.h>
#include <qstringlist.h>
#include <qstyle.h>
#include <qpopupmenu.h>
#include <qvaluelist.h>

// KDE
#include <dcopobject.h>
#include <kaction.h>
#include <kcolordialog.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knotifyclient.h>
#include <kprinter.h>
#include <kpushbutton.h>
#include <kstatusbar.h>
#include <kstdaction.h>
#include <kinputdialog.h>
#include <kurldrag.h>
#include <kpopupmenu.h>
#include <kdebug.h>
#include <ksharedptr.h>

// KOffice
#include <qcolor.h>
#include <koMainWindow.h>
#include <koView.h>
#include "kotabbar.h"

// Local
#include "kis_brush.h"
#include "kis_canvas.h"
#include "kis_channelview.h"
#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_factory.h"
#include "kis_filter_registry.h"
#include "kis_gradient.h"
#include "kis_image_magick_converter.h"
#include "kis_imagepipe_brush.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_paint_device_visitor.h"
#include "kis_painter.h"
#include "kis_ruler.h"
#include "kis_selection.h"
#include "kis_tool.h"
#include "kis_tool_registry.h"
#include "kis_tool_non_paint.h"
#include "kis_types.h"
#include "kis_undo_adapter.h"
#include "kis_view.h"
#include "kis_rect.h"
#include "KRayonViewIface.h"
#include "labels/kis_label_progress.h"
#include "labels/kis_label_cursor_pos.h"
#include "strategy/kis_strategy_move.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_double_click_event.h"
#include "kis_move_event.h"
#include "kis_colorspace_registry.h"
#include "kis_profile.h"
#include "kis_transaction.h"

// Dialog boxes
#include "kis_dlg_progress.h"
#include "kis_dlg_new_layer.h"
#include "kis_dlg_paint_properties.h"
#include "kis_dlg_transform.h"
#include "kis_dlg_preferences.h"
#include "kis_dlg_image_properties.h"

// Action managers
#include "kis_selection_manager.h"
#include "kis_docker_manager.h"

#define KISVIEW_MIN_ZOOM (1.0 / 16.0)
#define KISVIEW_MAX_ZOOM 16.0

// Time in ms that must pass after a tablet event before a mouse event is allowed to
// change the input device to the mouse. This is needed because mouse events are always
// sent to a receiver if it does not accept the tablet event.
#define MOUSE_CHANGE_EVENT_DELAY 100

KisView::KisView(KisDoc *doc, KisUndoAdapter *adapter, QWidget *parent, const char *name) : super(doc, parent, name)
{
        setFocusPolicy( QWidget::StrongFocus );
	// XXX Temporary re-instatement of old way to load filters and tools
	m_toolRegistry = new KisToolRegistry();
	Q_CHECK_PTR(m_toolRegistry);

	m_filterRegistry = new KisFilterRegistry();
	Q_CHECK_PTR(m_filterRegistry);

	if (!doc -> isReadWrite())
		setXMLFile("krita_readonly.rc");
	else
		setXMLFile("krita.rc");

	m_inputDevice = INPUT_DEVICE_MOUSE;

	m_selectionManager = new KisSelectionManager(this, doc);
	Q_CHECK_PTR(m_selectionManager);

	m_doc = doc;
	m_adapter = adapter;
	m_canvas = 0;
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
	m_layerRaise = 0;
	m_layerLower = 0;
	m_layerTop = 0;
	m_layerBottom = 0;

	m_imgScan = 0;
	m_imgResizeToLayer = 0;
	m_imgFlatten = 0;
	m_imgMergeVisible = 0;
	m_imgMergeLinked = 0;
	m_imgMergeLayer = 0;

	m_hScroll = 0;
	m_vScroll = 0;

	m_dcop = 0;

	m_fg = Qt::black;
	m_bg = Qt::white;


	m_brush = 0;
	m_pattern = 0;
	m_gradient = 0;

	m_progress = 0;
	m_statusBarZoomLabel = 0;
	m_statusBarSelectionLabel = 0;
	m_statusBarProfileLabel = 0;

	setInstance(KisFactory::global());

	setupTools();
	setupCanvas();
	setupRulers();
	setupScrollBars();
	setupStatusBar();

	setupActions();

	dcopObject();

	connect(m_doc, SIGNAL(imageListUpdated()), SLOT(docImageListUpdate()));
	connect(m_doc, SIGNAL(layersUpdated(KisImageSP)), SLOT(layersUpdated(KisImageSP)));
	connect(m_doc, SIGNAL(currentImageUpdated(KisImageSP)), SLOT(currentImageUpdated(KisImageSP)));
	connect(this, SIGNAL(embeddImage(const QString&)), SLOT(slotEmbedImage(const QString&)));

	m_dockerManager = new KisDockerManager(this, actionCollection());
	Q_CHECK_PTR(m_dockerManager);

	resetMonitorProfile();
	layersUpdated();
	setCurrentTool(findTool("tool_brush"));
}


KisView::~KisView()
{
	delete m_dcop;
	delete m_dockerManager;
	delete m_selectionManager;
}

DCOPObject* KisView::dcopObject()
{
	if (!m_dcop) {
		m_dcop = new KRayonViewIface(this);
		Q_CHECK_PTR(m_dcop);
	}
	return m_dcop;
}




void KisView::setupScrollBars()
{
	m_scrollX = 0;
	m_scrollY = 0;
	m_vScroll = new QScrollBar(QScrollBar::Vertical, this);
	Q_CHECK_PTR(m_vScroll);

	m_hScroll = new QScrollBar(QScrollBar::Horizontal, this);
	Q_CHECK_PTR(m_hScroll);

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
	Q_CHECK_PTR(m_hRuler);

	m_vRuler = new KisRuler(Qt::Vertical, this);
	Q_CHECK_PTR(m_vRuler);

	m_hRuler -> setGeometry(20, 0, width() - 20, 20);
	m_vRuler -> setGeometry(0, 20, 20, height() - 20);

	if (statusBar()) {
		m_hRuler -> installEventFilter(this);
		m_vRuler -> installEventFilter(this);
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

void KisView::updateStatusBarSelectionLabel()
{
	if (m_statusBarSelectionLabel == 0) {
		return;
	}

	KisImageSP img = currentImg();
	if (img) {
		KisLayerSP layer = img -> activeLayer();
		if (layer) {
			if (layer -> hasSelection()) {
				m_statusBarSelectionLabel -> setText(i18n("Selection Active"));
				return;
			}
		}
	}

	m_statusBarSelectionLabel -> setText(i18n("No selection"));
}

void KisView::updateStatusBarProfileLabel()
{
	if (m_statusBarProfileLabel == 0) {
		return;
	}

	KisImageSP img = currentImg();
	if (!img) return;

	if (img -> profile() == 0) {
		m_statusBarProfileLabel -> setText(i18n("No profile"));
	}
	else {
		m_statusBarProfileLabel -> setText(img -> profile() -> productName());
	}
}


KisProfileSP KisView::monitorProfile()
{
	if (m_monitorProfile == 0) {
		resetMonitorProfile();
	}
	return m_monitorProfile;
}


void KisView::resetMonitorProfile()
{
	KisConfig cfg;
	QString monitorProfileName = cfg.monitorProfile();

	m_monitorProfile = KisColorSpaceRegistry::instance() -> getProfileByName(monitorProfileName);
//	if (m_monitorProfile) {
//		kdDebug() << "Monitor profile: " << m_monitorProfile -> productName() << "\n";
//	} else {
//		kdDebug() << "Empty monitor profile " << monitorProfileName << "\n";
//	}

}

void KisView::setupStatusBar()
{
	KStatusBar *sb = statusBar();

	if (sb) {
		QLabel *lbl;

		lbl = new KisLabelCursorPos(sb);
		connect(this, SIGNAL(cursorPosition(Q_INT32, Q_INT32)), lbl, SLOT(updatePos(Q_INT32, Q_INT32)));
		connect(this, SIGNAL(cursorEnter()), lbl, SLOT(enter()));
		connect(this, SIGNAL(cursorLeave()), lbl, SLOT(leave()));
		addStatusBarItem(lbl, 0);

		m_statusBarZoomLabel = new QLabel(sb);
		addStatusBarItem(m_statusBarZoomLabel, 1);
		updateStatusBarZoomLabel();


		m_statusBarSelectionLabel = new QLabel(sb);
		addStatusBarItem(m_statusBarSelectionLabel, 2);
		updateStatusBarSelectionLabel();


		m_statusBarProfileLabel = new QLabel(sb);
		addStatusBarItem(m_statusBarProfileLabel, 3);
		updateStatusBarProfileLabel();

		m_progress = new KisLabelProgress(this);
		m_progress -> setMaximumWidth(225);
		m_progress -> setMaximumHeight(sb -> height());

		addStatusBarItem(m_progress, 4, true);

		m_progress -> hide();
	}
}

void KisView::setupActions()
{
	m_selectionManager -> setup(actionCollection());

	m_fullScreen = KStdAction::fullScreen( NULL, NULL, actionCollection(), this );
	connect( m_fullScreen, SIGNAL( toggled( bool )), this, SLOT( slotUpdateFullScreen( bool )));

	// import/export actions
	m_imgProperties = new KAction(i18n("Image Properties"), 0, this, SLOT(slotImageProperties()), actionCollection(), "img_properties");
	m_imgScan = 0; // How the hell do I get a KAction to the scan plug-in?!?
	m_imgResizeToLayer = new KAction(i18n("Extend Image to Size of Current Layer"), 0, this, SLOT(imgResizeToActiveLayer()), actionCollection(), "resizeimgtolayer");
	// view actions
	m_zoomIn = KStdAction::zoomIn(this, SLOT(slotZoomIn()), actionCollection(), "zoom_in");
	m_zoomOut = KStdAction::zoomOut(this, SLOT(slotZoomOut()), actionCollection(), "zoom_out");

	// layer actions
	m_layerAdd = new KAction(i18n("&Add Layer..."), "newlayer", "Ctrl+Shift+N", this, SLOT(layerAdd()), actionCollection(), "insert_layer");
	m_layerRm = new KAction(i18n("&Remove Layer"), "deletelayer", 0, this, SLOT(layerRemove()), actionCollection(), "remove_layer");
	m_layerDup = new KAction(i18n("Duplicate Layer"), 0, this, SLOT(layerDuplicate()), actionCollection(), "duplicate_layer");
	m_layerLink = new KAction(i18n("&Link/Unlink Layer"), 0, this, SLOT(layerToggleLinked()), actionCollection(), "link_layer");
	m_layerHide = new KAction(i18n("&Hide/Show Layer"), 0, this, SLOT(layerToggleVisible()), actionCollection(), "hide_layer");
	m_layerRaise = new KAction(i18n("Raise Layer"), "raiselayer", "Ctrl+]", this, SLOT(layerRaise()), actionCollection(), "raiselayer");
	m_layerLower = new KAction(i18n("Lower Layer"), "lowerlayer", "Ctrl+[", this, SLOT(layerLower()), actionCollection(), "lowerlayer");
	m_layerTop = new KAction(i18n("Layer to Top"), "Ctrl+Shift+]", this, SLOT(layerFront()), actionCollection(), "toplayer");
	m_layerBottom = new KAction(i18n("Layer to Bottom"), "Ctrl+Shift+[", this, SLOT(layerBack()), actionCollection(), "bottomlayer");
	m_layerProperties = new KAction(i18n("Layer Properties"), 0, this, SLOT(layerProperties()), actionCollection(), "layer_properties");
	(void)new KAction(i18n("I&nsert Image as Layer..."), 0, this, SLOT(slotInsertImageAsLayer()), actionCollection(), "insert_image_as_layer");
	m_layerSaveAs = new KAction(i18n("Save Layer as Image..."), 0, this, SLOT(saveLayerAsImage()), actionCollection(), "save_layer_as_image");
	(void)new KAction(i18n("Mirror Along &X Axis"), "view_left_right", 0, this, SLOT(mirrorLayerX()), actionCollection(), "mirrorLayerX");
	(void)new KAction(i18n("Mirror Along &Y Axis"), "view_top_bottom", 0, this, SLOT(mirrorLayerY()), actionCollection(), "mirrorLayerY");

	// color actions
	(void)new KAction(i18n("Select Foreground Color..."), 0, this, SLOT(selectFGColor()), actionCollection(), "select_fgColor");
	(void)new KAction(i18n("Select Background Color..."), 0, this, SLOT(selectBGColor()), actionCollection(), "select_bgColor");
	(void)new KAction(i18n("Reverse Foreground/Background Colors"), 0, this, SLOT(reverseFGAndBGColors()), actionCollection(), "reverse_fg_bg");

	// image actions
	m_imgFlatten = new KAction(i18n("Flatten Image"), 0, this, SLOT(flattenImage()), actionCollection(), "flatten_image");
	m_imgMergeVisible = new KAction(i18n("Merge &Visible Layers"), "Ctrl+Shift+E", this, SLOT(mergeVisibleLayers()), actionCollection(), "merge_visible_layers");
	m_imgMergeLinked = new KAction(i18n("Merge &Linked Layers"), 0, this, SLOT(mergeLinkedLayers()), actionCollection(), "merge_linked_layers");
	m_imgMergeLayer = new KAction(i18n("&Merge Layer"), "Ctrl+E", this, SLOT(mergeLayer()), actionCollection(), "merge_layer");

	// setting actions
	KStdAction::preferences(this, SLOT(preferences()), actionCollection(), "preferences");

	m_RulerAction = new KToggleAction( i18n( "Show Rulers" ), 0, this, SLOT( showRuler() ), actionCollection(), "view_ruler" );
	m_RulerAction->setChecked( true );
}

void KisView::resizeEvent(QResizeEvent *)
{
	KisImageSP img = currentImg();
	Q_INT32 rulerThickness = m_RulerAction -> isChecked() ? 20 : 0;
	Q_INT32 scrollBarExtent = style().pixelMetric(QStyle::PM_ScrollBarExtent);
	Q_INT32 drawH;
	Q_INT32 drawW;
	Q_INT32 docW;
	Q_INT32 docH;

	docW = static_cast<Q_INT32>(ceil(docWidth() * zoom()));
	docH = static_cast<Q_INT32>(ceil(docHeight() * zoom()));

	drawH = height() - rulerThickness;
	drawW = width() - rulerThickness;

	if (drawH < docH) {
		// Will need vert scrollbar
		drawW -= scrollBarExtent;
		if (drawW < docW)
			// Will need horiz scrollbar
			drawH -= scrollBarExtent;
	} else if (drawW < docW) {
		// Will need horiz scrollbar
		drawH -= scrollBarExtent;
		if (drawH < docH)
			// Will need vert scrollbar
			drawW -= scrollBarExtent;
	}

	m_hRuler -> setGeometry(rulerThickness, 0, drawW, rulerThickness);
	m_vRuler -> setGeometry(0, rulerThickness, rulerThickness, drawH);

	m_vScroll -> setEnabled(docH > drawH);
	m_hScroll -> setEnabled(docW > drawW);

	if (docH <= drawH && docW <= drawW) {
		// we need no scrollbars
		m_vScroll -> hide();
		m_hScroll -> hide();
		m_vScroll -> setValue(0);
		m_hScroll -> setValue(0);
	} else if (docH <= drawH) {
		// we need a horizontal scrollbar only
		m_vScroll -> hide();
		m_vScroll -> setValue(0);
		m_hScroll -> setRange(0, docW - drawW);
		m_hScroll -> setGeometry(rulerThickness,
					 height() - scrollBarExtent,
					 width() - rulerThickness,
					 scrollBarExtent);
		m_hScroll -> show();
	} else if(docW <= drawW) {
		// we need a vertical scrollbar only
		m_hScroll -> hide();
		m_hScroll -> setValue(0);
		m_vScroll -> setRange(0, docH - drawH);
		m_vScroll -> setGeometry(width() - scrollBarExtent, rulerThickness, scrollBarExtent, height()  - rulerThickness);
		m_vScroll -> show();
	} else {
		// we need both scrollbars
		m_vScroll -> setRange(0, docH - drawH);
		m_vScroll -> setGeometry(width() - scrollBarExtent,
					rulerThickness,
					scrollBarExtent,
					height() -2* rulerThickness);
		m_hScroll -> setRange(0, docW - drawW);
		m_hScroll -> setGeometry(rulerThickness,
					 height() - scrollBarExtent,
					 width() - 2*rulerThickness,
					 scrollBarExtent);
		m_vScroll -> show();
		m_hScroll -> show();
	}

	//Check if rulers are visible
	if( m_RulerAction->isChecked() )
		m_canvas -> setGeometry(rulerThickness, rulerThickness, drawW, drawH);
	else
		m_canvas -> setGeometry(0, 0, drawW, drawH);
	m_canvas -> show();

	Q_INT32 oldWidth = m_canvasPixmap.width();
	Q_INT32 oldHeight = m_canvasPixmap.height();

	m_canvasPixmap.resize(drawW, drawH);

	if (!m_canvasPixmap.isNull()) {
		if (drawW > oldWidth) {
			KisRect drawRect(oldWidth, 0, drawW - oldWidth, drawH);
			paintView(viewToWindow(drawRect));
		}

		if (drawH > oldHeight) {
			KisRect drawRect(0, oldHeight, drawW, drawH - oldHeight);
			paintView(viewToWindow(drawRect));
		}
	}

	m_vScroll -> setPageStep(drawH);
	m_vScroll -> setLineStep(drawH/4);
	m_hScroll -> setPageStep(drawW);
	m_hScroll -> setLineStep(drawW/4);

	if (m_vScroll -> isVisible())
		m_vRuler -> updateVisibleArea(0, m_vScroll -> value());
	else
		m_vRuler -> updateVisibleArea(0, 0);

	if (m_hScroll -> isVisible())
		m_hRuler -> updateVisibleArea(m_hScroll -> value(), 0);
	else
		m_hRuler -> updateVisibleArea(0, 0);

	if( m_RulerAction->isChecked() )
	{
		m_hRuler -> show();
		m_vRuler -> show();
	}
}

void KisView::updateReadWrite(bool readwrite)
{
	layerUpdateGUI(readwrite);
}

void KisView::clearCanvas(const QRect& rc)
{
	QPainter gc;

	if (gc.begin(&m_canvasPixmap)) {
		gc.fillRect(rc, backgroundColor());
	}
}

void KisView::setCurrentTool(KisTool *tool)
{
	KisTool *oldTool = currentTool();

	m_dockerManager ->setToolOptionWidget(oldTool, tool);

	if (oldTool)
	{
		oldTool -> clear();
		oldTool -> action() -> setChecked( false );
	}

	if (tool) {
		m_inputDeviceToolMap[currentInputDevice()] = tool;
		setCanvasCursor(tool -> cursor());

		m_canvas -> enableMoveEventCompressionHint(dynamic_cast<KisToolNonPaint *>(tool) != NULL);

		notify();
		tool -> action() -> setChecked( true );

	} else {
		m_inputDeviceToolMap[currentInputDevice()] = 0;
		m_canvas -> setCursor(KisCursor::arrowCursor());
	}

}

KisTool *KisView::currentTool() const
{
	InputDeviceToolMap::const_iterator it = m_inputDeviceToolMap.find(currentInputDevice());

	if (it != m_inputDeviceToolMap.end()) {
		return (*it).second;
	} else {
		return 0;
	}
}

KisTool *KisView::findTool(QString toolName, enumInputDevice inputDevice) const
{
	if (inputDevice == INPUT_DEVICE_UNKNOWN) {
		inputDevice = currentInputDevice();
	}

	KisTool *tool = 0;

	InputDeviceToolSetMap::const_iterator vit = m_inputDeviceToolSetMap.find(inputDevice);

	Q_ASSERT(vit != m_inputDeviceToolSetMap.end());

	const vKisTool& tools = (*vit).second;

	for (vKisTool::const_iterator it = tools.begin(); it != tools.end(); it++) {
		KisTool *t = *it;
		if (t -> name() == toolName) {
			tool = t;
			break;
		}
	}

	return tool;
}

void KisView::setInputDevice(enumInputDevice inputDevice)
{
	if (inputDevice != m_inputDevice) {
		KisConfig cfg;
		InputDeviceToolSetMap::iterator vit = m_inputDeviceToolSetMap.find(m_inputDevice);

		if (vit != m_inputDeviceToolSetMap.end()) {
			vKisTool& oldTools = (*vit).second;
			for (vKisTool::iterator it = oldTools.begin(); it != oldTools.end(); it++) {
				KisTool *tool = *it;
				KAction *toolAction = tool -> action();
				toolAction -> disconnect(SIGNAL(activated()), tool, SLOT(activate()));
			}
		}
		KisTool *oldTool = currentTool();
		if (oldTool)
		{
			m_dockerManager -> unsetToolOptionWidget(oldTool);
			oldTool -> clear();
		}

		m_inputDevice = inputDevice;

		vit = m_inputDeviceToolSetMap.find(m_inputDevice);

		Q_ASSERT(vit != m_inputDeviceToolSetMap.end());

		vKisTool& tools = (*vit).second;

		for (vKisTool::iterator it = tools.begin(); it != tools.end(); it++) {
			KisTool *tool = *it;
			KAction *toolAction = tool -> action();

			connect(toolAction, SIGNAL(activated()), tool, SLOT(activate()));
		}

		if (currentTool() == 0) {
			if (m_inputDevice == INPUT_DEVICE_ERASER) {
				setCurrentTool(findTool("tool_eraser"));
			} else {
				setCurrentTool(findTool("tool_brush"));
			}
		} else {
			setCurrentTool(currentTool());
		}

		currentTool() -> action() -> activate();
	}

}

enumInputDevice KisView::currentInputDevice() const
{
	return m_inputDevice;
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

			QPainter gc;

			if (gc.begin(&m_canvasPixmap)) {
				QRect wr = viewToWindow(vr).qRect();

				if (wr.left() < 0 || wr.right() >= img -> width() || wr.top() < 0 || wr.bottom() >= img -> height()) {
					// Erase areas outside document
					QRegion rg(vr.qRect());
					rg -= QRegion(windowToView(KisRect(0, 0, img -> width(), img -> height())).qRect());

					QMemArray<QRect> rects = rg.rects();

					for (unsigned int i = 0; i < rects.count(); i++) {
						QRect er = rects[i];
						if (er.isValid())
							gc.fillRect(er, backgroundColor());
					}

					wr &= QRect(0, 0, img -> width(), img -> height());
				}

				if (!wr.isNull()) {

					if (zoom() < 1.0 || zoom() > 1.0) {
						gc.setViewport(0, 0, static_cast<Q_INT32>(m_canvasPixmap.width() * zoom()), static_cast<Q_INT32>(m_canvasPixmap.height() * zoom()));
					}
					gc.translate((-horzValue()) / zoom(), (-vertValue()) / zoom());

					m_doc -> setCurrentImage(img);
					m_doc -> paintContent(gc, wr, monitorProfile());
					m_doc -> setCurrentImage(0);
				}
			}

			m_canvas -> update(vr.qRect());
		}
	} else {
		clearCanvas(r.qRect());
		m_canvas -> update(r.qRect());
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

void KisView::layerUpdateGUI(bool enable)
{
	KisImageSP img = currentImg();
	KisLayerSP layer;
	Q_INT32 nlayers = 0;
	Q_INT32 nvisible = 0;
	Q_INT32 nlinked = 0;
	Q_INT32 layerPos = 0;

	if (img) {
		layer = img -> activeLayer();
		nlayers = img -> nlayers();
		nvisible = nlayers - img -> nHiddenLayers();
		nlinked = img -> nLinkedLayers();
	}

	if (layer)
		layerPos = img->index(layer);
	enable = enable && img && layer;
	m_layerDup -> setEnabled(enable);
	m_layerRm -> setEnabled(enable);
	m_layerLink -> setEnabled(enable);
	m_layerHide -> setEnabled(enable);
	m_layerProperties -> setEnabled(enable);
	m_layerSaveAs -> setEnabled(enable);
//        m_layerTransform -> setEnabled(enable);
	m_layerRaise -> setEnabled(enable && nlayers > 1 && layerPos);
	m_layerLower -> setEnabled(enable && nlayers > 1 && layerPos != nlayers - 1);
	m_layerTop -> setEnabled(enable && nlayers > 1 && layerPos);
	m_layerBottom -> setEnabled(enable && nlayers > 1 && layerPos != nlayers - 1);

	// XXX thes should be named layer instead of img
	m_imgFlatten -> setEnabled(nlayers > 1);

	m_imgMergeVisible -> setEnabled(nvisible > 1);
	m_imgMergeLinked -> setEnabled(nlinked > 1);
	m_imgMergeLayer -> setEnabled(nlayers > 1 && layerPos < nlayers - 1);

	m_selectionManager -> updateGUI();
	imgUpdateGUI();
}


void KisView::imgUpdateGUI()
{
	KisImageSP img = currentImg();

	m_imgResizeToLayer -> setEnabled(img && img -> activeLayer());

	updateStatusBarProfileLabel();
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


void KisView::imgResizeToActiveLayer()
{
        KisImageSP img = currentImg();
        KisLayerSP layer;


        if (img && (layer = img -> activeLayer())) {
                QRect r = layer -> exactBounds();
                img -> resize(r.width(), r.height());
        }
}

void KisView::slotImageProperties()
{
	KisImageSP img = currentImg();

	if (!img) return;

	KisDlgImageProperties * dlg = new KisDlgImageProperties(img, this);
	Q_CHECK_PTR(dlg);

	dlg -> exec();
	delete dlg;
}

void KisView::slotInsertImageAsLayer()
{
	if (importImage(true) > 0)
		m_doc -> setModified(true);
}

void KisView::saveLayerAsImage()
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


Q_INT32 KisView::importImage(bool createLayer, bool modal, const KURL& urlArg)
{
	KURL::List urls;
	Q_INT32 rc = 0;

	if (urlArg.isEmpty())
		urls = KFileDialog::getOpenURLs(QString::null, KisImageMagickConverter::readFilters(), 0, i18n("Import Image"));
	else
		urls.push_back(urlArg);

	if (urls.empty())
		return 0;

	KisImageMagickConverter ib(m_doc, m_adapter);
	KisImageSP img;

	//m_imgBuilderMgr -> attach(&ib);

	for (KURL::List::iterator it = urls.begin(); it != urls.end(); it++) {
		KURL url = *it;
		KisDlgProgress dlg(&ib);

		if (modal)
			dlg.show();
		else
			m_progress -> setSubject(&ib, false, true);

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
			m_progress -> setSubject(0, true, true);
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

		if (/*NOTABcreateLayer && */currentImg()) {
			vKisLayerSP v = img -> layers();
			KisImageSP current = currentImg();

			rc += v.size();
			current -> activeLayer() -> deselect();

			for (vKisLayerSP_it it = v.begin(); it != v.end(); it++) {
				KisLayerSP layer = *it;

				layer -> setImage(current);
				layer -> setName(current -> nextLayerName());
				m_doc -> layerAdd(current, layer, 0);
				emit currentLayerChanged(img -> index(layer));
			}
			resizeEvent(0);
			updateCanvas();
		}
	}

	return rc;
}


void KisView::rotateLayer180()
{
	rotateLayer( 180 );
}

void KisView::rotateLayerLeft90()
{
	rotateLayer( 270 );
}

void KisView::rotateLayerRight90()
{
	rotateLayer( 90 );
}

void KisView::mirrorLayerX()
{
	if (!currentImg()) return;
	KisLayerSP layer = currentImg() -> activeLayer();
	if (!layer) return;

	KisUndoAdapter * undo = 0;
	KisTransaction * t = 0;
	if ((undo = currentImg() -> undoAdapter())) {
		t = new KisTransaction(i18n("Mirror Layer X"), layer.data());
		Q_CHECK_PTR(t);
	}

	layer->mirrorX();

	if (undo) undo -> addCommand(t);

	m_doc -> setModified(true);
	layersUpdated();
	updateCanvas();
}

void KisView::mirrorLayerY()
{
	if (!currentImg()) return;
	KisLayerSP layer = currentImg() -> activeLayer();
	if (!layer) return;

	KisUndoAdapter * undo = 0;
	KisTransaction * t = 0;
	if ((undo = currentImg() -> undoAdapter())) {
		t = new KisTransaction(i18n("Mirror Layer Y"), layer.data());
		Q_CHECK_PTR(t);
	}

	layer->mirrorY();

	if (undo) undo -> addCommand(t);

	m_doc -> setModified(true);
	layersUpdated();
	updateCanvas();
}

void KisView::scaleLayer(double sx, double sy, enumFilterType ftype)
{
	if (!currentImg()) return;

	KisLayerSP layer = currentImg() -> activeLayer();
	if (!layer) return;

	KisUndoAdapter * undo = 0;
	KisTransaction * t = 0;
	if ((undo = currentImg() -> undoAdapter())) {
		t = new KisTransaction(i18n("Scale Layer"), layer.data());
		Q_CHECK_PTR(t);
	}

	layer -> scale(sx, sy, m_progress, ftype);

	if (undo) undo -> addCommand(t);

	m_doc -> setModified(true);
	layersUpdated();
	resizeEvent(0);
	updateCanvas();
	canvasRefresh();
}

void KisView::rotateLayer(double angle)
{
	if (!currentImg()) return;

	KisLayerSP layer = currentImg() -> activeLayer();
	if (!layer) return;

	KisUndoAdapter * undo = 0;
	KisTransaction * t = 0;
	if ((undo = currentImg() -> undoAdapter())) {
		t = new KisTransaction(i18n("Rotate Layer"), layer.data());
		Q_CHECK_PTR(t);
	}

	// Rotate
	layer -> rotate(angle, false, m_progress);

	if (undo) undo -> addCommand(t);

	m_doc -> setModified(true);
	layersUpdated();
	resizeEvent(0);
	updateCanvas();
	canvasRefresh();
}

void KisView::shearLayer(double angleX, double angleY)
{
	if (!currentImg()) return;

	KisLayerSP layer = currentImg() -> activeLayer();
	if (!layer) return;

	KisUndoAdapter * undo = 0;
	KisTransaction * t = 0;
	if ((undo = currentImg() -> undoAdapter())) {
		t = new KisTransaction(i18n("Shear layer"), layer.data());
		Q_CHECK_PTR(t);
	}

	layer -> shear(angleX, angleY, m_progress);

	if (undo) undo -> addCommand(t);

	m_doc -> setModified(true);
	layersUpdated();
	resizeEvent(0);
	updateCanvas();
	canvasRefresh();
}

void KisView::cropLayer(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
	if (!currentImg()) return;

	KisLayerSP layer = currentImg() -> activeLayer();
	if (!layer) return;

	KisUndoAdapter * undo = 0;
	KisTransaction * t = 0;
	if ((undo = currentImg() -> undoAdapter())) {
		t = new KisTransaction(i18n("Mirror Layer Y"), layer.data());
		Q_CHECK_PTR(t);
	}

	if (undo) undo -> addCommand(t);

	layer -> crop(x, y, w, h);

	m_doc -> setModified(true);
	layersUpdated();
	resizeEvent(0);
	updateCanvas();
	canvasRefresh();

}

void KisView::flattenImage()
{
	KisImageSP img = currentImg();

	if (img) {
		bool doIt = true;

		if (img -> nHiddenLayers() > 0) {
			int answer = KMessageBox::warningYesNo(this,
							       i18n("The image contains hidden layers that will be lost."),
							       i18n("Flatten Image"),
							       i18n("Flatten Image"),
							       i18n("Cancel"));

			if (answer != KMessageBox::Yes) {
				doIt = false;
			}
		}

		if (doIt) {
			img -> flatten();
		}
	}
}

void KisView::mergeVisibleLayers()
{
	KisImageSP img = currentImg();

	if (img) {
		img -> mergeVisibleLayers();
	}
}

void KisView::mergeLinkedLayers()
{
	KisImageSP img = currentImg();

	if (img) {
		img -> mergeLinkedLayers();
	}
}

void KisView::mergeLayer()
{
	KisImageSP img = currentImg();
	if (!img) return;

	KisLayerSP layer = img -> activeLayer();
	if (!layer) return;

	img -> mergeLayer(layer);
}

void KisView::preferences()
{
	PreferencesDialog::editPreferences();
	resetMonitorProfile();
	canvasRefresh();
	if (currentTool()) {
		setCanvasCursor(currentTool() -> cursor());
	}
}

void KisView::layerCompositeOp(const KisCompositeOp& compositeOp)
{
	KisImageSP img = currentImg();
	if (!img) return;

	KisLayerSP layer = img -> activeLayer();
	if (!layer) return;

	layer -> setCompositeOp(compositeOp);
	layersUpdated();
	canvasRefresh();
}

// range: 0 - 100
void KisView::layerOpacity(int opacity)
{

	kdDebug() << "Opacity set to " << opacity << endl;
	KisImageSP img = currentImg();
	if (!img) return;

	KisLayerSP layer = img -> activeLayer();
	if (!layer) return;

	if (opacity != 0) {
		opacity = opacity * 255 / 100;
		opacity = upscale(opacity - 1);
	}

	layer -> setOpacity(opacity);
	layersUpdated();
	canvasRefresh();
}


void KisView::showRuler()
{
	if( m_RulerAction->isChecked() )
	{
		m_hRuler->show();
		m_vRuler->show();
	}
	else
	{
		m_hRuler->hide();
		m_vRuler->hide();
	}

	resizeEvent(0);
	canvasRefresh();
}

void KisView::slotUpdateFullScreen(bool toggle)
{
	if (KoView::shell()) {

		uint newState = KoView::shell() -> windowState();

		if (toggle) {
			newState |= Qt::WindowFullScreen;
		} else {
			newState &= ~Qt::WindowFullScreen;
		}

		KoView::shell() -> setWindowState(newState);
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
	if (m_hScroll -> isVisible()) {
		m_hScroll -> setValue(x);
	}
	if (m_vScroll -> isVisible()) {
		m_vScroll -> setValue(y);
	}
}

void KisView::brushActivated(KisResource *brush)
{

	m_brush = dynamic_cast<KisBrush*>(brush);

	if (m_brush )
	{
		emit brushChanged(m_brush);
		notify();
	}
}

void KisView::patternActivated(KisResource *pattern)
{

	m_pattern = dynamic_cast<KisPattern*>(pattern);

	if (m_pattern) {
		emit patternChanged(m_pattern);
		notify();
	}
}

void KisView::gradientActivated(KisResource *gradient)
{

	m_gradient = dynamic_cast<KisGradient*>(gradient);

	if (m_gradient) {
		emit gradientChanged(m_gradient);
		notify();
	}
}

void KisView::setBGColor(const QColor& c)
{
	emit bgColorChanged(c);
	m_bg = c;
	notify();
}

void KisView::setFGColor(const QColor& c)
{
	emit fgColorChanged(c);
	m_fg = c;
	notify();
}

void KisView::slotSetFGColor(const QColor& c)
{
	setFGColor(c);
}

void KisView::slotSetBGColor(const QColor& c)
{
	setBGColor(c);
}

void KisView::setupPrinter(KPrinter& printer)
{
	KisImageSP img = currentImg();

	if (img) {
		printer.setPageSelection(KPrinter::ApplicationSide);
		printer.setPageSize(KPrinter::A4);
		printer.setOrientation(KPrinter::Portrait);
	}
}

void KisView::print(KPrinter& printer)
{
	QPainter gc(&printer);

	KisImageSP img = currentImg();
	if (!img) return;

	printer.setFullPage(true);
	gc.setClipping(false);

	KisConfig cfg;
	QString printerProfileName = cfg.monitorProfile();
	KisProfileSP printerProfile = KisColorSpaceRegistry::instance() -> getProfileByName(printerProfileName);

	if (printerProfile != 0)
		kdDebug() << "Printer profile: " << printerProfile -> productName() << "\n";

	QRect r = img -> bounds();
	img -> renderToPainter(r.x(), r.y(), r.width(), r.height(), gc, printerProfile);
}

void KisView::setupTools()
{
	m_inputDeviceToolSetMap[INPUT_DEVICE_MOUSE] = m_toolRegistry -> createTools(this);
	m_inputDeviceToolSetMap[INPUT_DEVICE_STYLUS] = m_toolRegistry -> createTools(this);
	m_inputDeviceToolSetMap[INPUT_DEVICE_ERASER] = m_toolRegistry -> createTools(this);
	m_inputDeviceToolSetMap[INPUT_DEVICE_PUCK] = m_toolRegistry -> createTools(this);

	qApp -> installEventFilter(this);
	m_tabletEventTimer.start();
}



void KisView::canvasGotPaintEvent(QPaintEvent *event)
{
	QMemArray<QRect> rects = event -> region().rects();

	for (unsigned int i = 0; i < rects.count(); i++) {
		QRect er = rects[i];

		bitBlt(m_canvas, er.x(), er.y(), &m_canvasPixmap, er.x(), er.y(), er.width(), er.height());
	}

	if (currentTool()) {
		QPainter gc(m_canvas);

		gc.setClipRegion(event -> region());
		gc.setClipping(true);

		currentTool() -> paint(gc, event -> rect());
	}
}

void KisView::canvasGotButtonPressEvent(KisButtonPressEvent *e)
{
#if defined(EXTENDED_X11_TABLET_SUPPORT)
	// The event filter doesn't see tablet events going to the canvas.
	if (e -> device() != INPUT_DEVICE_MOUSE) {
		m_tabletEventTimer.start();
	}
#endif // EXTENDED_X11_TABLET_SUPPORT

	if (e -> device() != currentInputDevice()) {
		if (e -> device() == INPUT_DEVICE_MOUSE) {
			if (m_tabletEventTimer.elapsed() > MOUSE_CHANGE_EVENT_DELAY) {
				setInputDevice(INPUT_DEVICE_MOUSE);
			}
		} else {
			setInputDevice(e -> device());
		}
	}

	if (e -> device() == currentInputDevice() && currentTool()) {
		KisPoint p = viewToWindow(e -> pos());
		KisButtonPressEvent ev(e -> device(), p, e -> globalPos(), e -> pressure(), e -> xTilt(), e -> yTilt(), e -> button(), e -> state());

		currentTool() -> buttonPress(&ev);
	}
}

void KisView::canvasGotMoveEvent(KisMoveEvent *e)
{
#if defined(EXTENDED_X11_TABLET_SUPPORT)
	// The event filter doesn't see tablet events going to the canvas.
	if (e -> device() != INPUT_DEVICE_MOUSE) {
		m_tabletEventTimer.start();
	}
#endif // EXTENDED_X11_TABLET_SUPPORT

	if (e -> device() != currentInputDevice()) {
		if (e -> device() == INPUT_DEVICE_MOUSE) {
			if (m_tabletEventTimer.elapsed() > MOUSE_CHANGE_EVENT_DELAY) {
				setInputDevice(INPUT_DEVICE_MOUSE);
			}
		} else {
			setInputDevice(e -> device());
		}
	}

	KisImageSP img = currentImg();

	m_hRuler -> updatePointer(e -> pos().floorX(), e -> pos().floorY());
	m_vRuler -> updatePointer(e -> pos().floorX(), e -> pos().floorY());

	KisPoint wp = viewToWindow(e -> pos());

	if (e -> device() == currentInputDevice() && currentTool()) {
		KisMoveEvent ev(e -> device(), wp, e -> globalPos(), e -> pressure(), e -> xTilt(), e -> yTilt(), e -> state());

		currentTool() -> move(&ev);
	}

	emit cursorPosition(wp.floorX(), wp.floorY());
}

void KisView::canvasGotButtonReleaseEvent(KisButtonReleaseEvent *e)
{
#if defined(EXTENDED_X11_TABLET_SUPPORT)
	// The event filter doesn't see tablet events going to the canvas.
	if (e -> device() != INPUT_DEVICE_MOUSE) {
		m_tabletEventTimer.start();
	}
#endif // EXTENDED_X11_TABLET_SUPPORT

	if (e -> device() != currentInputDevice()) {
		if (e -> device() == INPUT_DEVICE_MOUSE) {
			if (m_tabletEventTimer.elapsed() > MOUSE_CHANGE_EVENT_DELAY) {
				setInputDevice(INPUT_DEVICE_MOUSE);
			}
		} else {
			setInputDevice(e -> device());
		}
	}

	if (e -> device() == currentInputDevice() && currentTool()) {
		KisPoint p = viewToWindow(e -> pos());
		KisButtonReleaseEvent ev(e -> device(), p, e -> globalPos(), e -> pressure(), e -> xTilt(), e -> yTilt(), e -> button(), e -> state());

		if (currentTool()) {
			currentTool() -> buttonRelease(&ev);
		}
	}
}

void KisView::canvasGotDoubleClickEvent(KisDoubleClickEvent *e)
{
#if defined(EXTENDED_X11_TABLET_SUPPORT)
	// The event filter doesn't see tablet events going to the canvas.
	if (e -> device() != INPUT_DEVICE_MOUSE) {
		m_tabletEventTimer.start();
	}
#endif // EXTENDED_X11_TABLET_SUPPORT

	if (e -> device() != currentInputDevice()) {
		if (e -> device() == INPUT_DEVICE_MOUSE) {
			if (m_tabletEventTimer.elapsed() > MOUSE_CHANGE_EVENT_DELAY) {
				setInputDevice(INPUT_DEVICE_MOUSE);
			}
		} else {
			setInputDevice(e -> device());
		}
	}

	if (e -> device() == currentInputDevice() && currentTool()) {
		KisPoint p = viewToWindow(e -> pos());
		KisDoubleClickEvent ev(e -> device(), p, e -> globalPos(), e -> pressure(), e -> xTilt(), e -> yTilt(), e -> button(), e -> state());

		if (currentTool()) {
			currentTool() -> doubleClick(&ev);
		}
	}
}

void KisView::canvasGotEnterEvent(QEvent *e)
{
	if (currentTool())
		currentTool() -> enter(e);
}

void KisView::canvasGotLeaveEvent (QEvent *e)
{
	if (currentTool())
		currentTool() -> leave(e);
}

void KisView::canvasGotMouseWheelEvent(QWheelEvent *event)
{
	if(event->state() == ControlButton )
	{
		if(event->delta() / 120 != 0)
		{
			if(event->delta() > 0)
			{
				zoomIn();
			} else {
				zoomOut();
			}
		}
	} else {
		QApplication::sendEvent(m_vScroll, event);
	}
}

void KisView::canvasGotKeyPressEvent(QKeyEvent *event)
{
	if (currentTool())
		currentTool() -> keyPress(event);
}

void KisView::canvasGotKeyReleaseEvent(QKeyEvent *event)
{
	if (currentTool())
		currentTool() -> keyRelease(event);
}

void KisView::canvasGotDragEnterEvent(QDragEnterEvent *event)
{
	event -> accept(KURLDrag::canDecode(event));
}

void KisView::canvasGotDropEvent(QDropEvent *event)
{
	KURL::List urls;

	if (KURLDrag::decode(event, urls))
	{
		if (urls.count() > 0) {
			enum enumActionId {
				addLayerId = 1,
				addImageId,
				addDocumentId,
				cancelId
			};

			KPopupMenu popup(this, "drop_popup");

			if (urls.count() == 1) {
				if (currentImg() != 0) {
					popup.insertItem(i18n("Insert as New Layer"), addLayerId);
				}

				popup.insertSeparator();
				popup.insertItem(i18n("Open in New Document"), addDocumentId);
			}
			else {
				if (currentImg() != 0) {
					popup.insertItem(i18n("Insert as New Layers"), addLayerId);
				}

				popup.insertSeparator();
				popup.insertItem(i18n("Open in New Documents"), addDocumentId);
			}

			popup.insertSeparator();
			popup.insertItem(i18n("Cancel"), cancelId);

			int actionId = popup.exec(QCursor::pos());

			if (actionId >= 0 && actionId != cancelId) {
				for (KURL::List::ConstIterator it = urls.begin (); it != urls.end (); it++) {
					KURL url = *it;

					switch (actionId) {
					case addLayerId:
						importImage(true, false, url);
						break;
					case addImageId:
						importImage(false, false, url);
						break;
					case addDocumentId:
						if (shell() != 0) {
							shell() -> openDocument(url);
						}
						break;
					}
				}
			}
		}
	}
}

void KisView::canvasRefresh()
{
	KisRect rc(0, 0, m_canvasPixmap.width(), m_canvasPixmap.height());
	paintView(viewToWindow(rc));
	m_canvas -> repaint();
}

void KisView::layerToggleVisible()
{
	KisImageSP img = currentImg();

	if (img) {
		KisLayerSP layer = img -> activeLayer();

		if (layer) {
			layer -> setVisible(!layer -> visible());
			m_doc -> setModified(true);
			resizeEvent(0);
			layersUpdated();
			canvasRefresh();
		}
	}
}

void KisView::layerToggleLocked()
{
	KisImageSP img = currentImg();
	if (!img) return;

	KisLayerSP layer = img -> activeLayer();
	if (!layer) return;

	layer -> setLocked(!layer -> locked());
	m_doc -> setModified(true);
	layersUpdated();
}

void KisView::layerSelected(int n)
{
	KisImageSP img = currentImg();

	layerUpdateGUI(img -> activateLayer(n));
	notify();
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
			layer -> setLinked(!layer -> linked());
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
						QPoint(layer->getX(),
						       layer->getY()),
						layer->opacity(),
						layer->compositeOp(),
						layer->colorStrategy());

			if (dlg.exec() == QDialog::Accepted) {
				QPoint pt = dlg.getPosition();

				bool changed = layer -> name() != dlg.getName()
					       || layer -> opacity() != dlg.getOpacity()
					       || layer -> compositeOp() != dlg.getCompositeOp()
					       || pt.x() != layer -> getX()
					       || pt.y() != layer -> getY();


				if (changed)
					m_adapter -> beginMacro(i18n("Property changes"));

				if (layer -> name() != dlg.getName()
				    || layer -> opacity() != dlg.getOpacity()
				    || layer -> compositeOp() != dlg.getCompositeOp())
					m_doc -> setLayerProperties(img, layer, dlg.getOpacity(), dlg.getCompositeOp(), dlg.getName());

				if (pt.x() != layer->getX() || pt.y() != layer->getY())
					KisStrategyMove(this).simpleMove(QPoint(layer->getX(), layer->getY()), pt);

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
		NewLayerDialog dlg(img->colorStrategy()->id(), img->nextLayerName(), this);

		dlg.exec();

		if (dlg.result() == QDialog::Accepted) {
			KisLayerSP layer = m_doc -> layerAdd(img,
							     dlg.layerName(),
							     dlg.compositeOp(),
							     dlg.opacity(),
							     KisColorSpaceRegistry::instance() -> get(dlg.colorStrategyID()));
			if (layer) {
				emit currentLayerChanged(img -> index(layer));
				resizeEvent(0);
				updateCanvas(0, 0, img -> width(), img -> height());
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
			emit currentLayerChanged(n - 1);
			resizeEvent(0);
			updateCanvas();
			layerUpdateGUI(img -> activeLayer() != 0);
		}
	}
}

void KisView::layerDuplicate()
{
	KisImageSP img = currentImg();

	if (!img)
		return;

	KisLayerSP active = img -> activeLayer();

	if (!active)
		return;

	Q_INT32 index = img -> index(active);
	KisLayerSP dup = new KisLayer(*active);
	dup -> setName(QString(i18n("Duplicate of '%1'")).arg(active -> name()));
	KisLayerSP layer = m_doc -> layerAdd(img, dup, index);

	if (layer) {
		emit currentLayerChanged(img -> index(layer));
		resizeEvent(0);
		updateCanvas(0, 0, img -> width(), img -> height());
	} else {
		KMessageBox::error(this, i18n("Could not add layer to image."), i18n("Layer Error"));
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
		m_doc -> setModified(true);
		layersUpdated();
		resizeEvent(0);
		updateCanvas();
	}
}

void KisView::layerBack()
{
	KisImageSP img = currentImg();
	if (!img) return;

	KisLayerSP layer;

	if (img && (layer = img -> activeLayer())) {
		img -> bottom(layer);
		m_doc -> setModified(true);
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
	if (!img) return;

	KisLayerSP layer = img -> activeLayer();

	layerUpdateGUI(img && layer);

	m_dockerManager -> resetLayerBox(img, layer);
	notify();
}

void KisView::layersUpdated(KisImageSP img)
{
	if (img == currentImg())
		layersUpdated();
}

void KisView::scrollH(int value)
{
	m_hRuler -> updateVisibleArea(value, 0);

	int xShift = m_scrollX - value;
	m_scrollX = value;

	if (xShift > 0) {
		bitBlt(&m_canvasPixmap, xShift, 0, &m_canvasPixmap, 0, 0, m_canvasPixmap.width() - xShift, m_canvasPixmap.height());

		KisRect drawRect(0, 0, xShift, m_canvasPixmap.height());
		paintView(viewToWindow(drawRect));
		m_canvas -> repaint();
	}
	else
		if (xShift < 0) {
			bitBlt(&m_canvasPixmap, 0, 0, &m_canvasPixmap, -xShift, 0, m_canvasPixmap.width() + xShift, m_canvasPixmap.height());

			KisRect drawRect(m_canvasPixmap.width() + xShift, 0, -xShift, m_canvasPixmap.height());
			paintView(viewToWindow(drawRect));
			m_canvas -> repaint();
		}
}

void KisView::scrollV(int value)
{
	m_vRuler -> updateVisibleArea(0, value);

	int yShift = m_scrollY - value;
	m_scrollY = value;

	if (yShift > 0) {
		bitBlt(&m_canvasPixmap, 0, yShift, &m_canvasPixmap, 0, 0, m_canvasPixmap.width(), m_canvasPixmap.height() - yShift);

		KisRect drawRect(0, 0, m_canvasPixmap.width(), yShift);
		paintView(viewToWindow(drawRect));
		m_canvas -> repaint();
	}
	else
		if (yShift < 0) {
			bitBlt(&m_canvasPixmap, 0, 0, &m_canvasPixmap, 0, -yShift, m_canvasPixmap.width(), m_canvasPixmap.height() + yShift);

			KisRect drawRect(0, m_canvasPixmap.height() + yShift, m_canvasPixmap.width(), -yShift);
			paintView(viewToWindow(drawRect));
			m_canvas -> repaint();
		}
}


void KisView::setupCanvas()
{
	m_canvas = new KisCanvas(this, "kis_canvas");
        m_canvas->setFocusPolicy( QWidget::StrongFocus );
	QObject::connect(m_canvas, SIGNAL(gotButtonPressEvent(KisButtonPressEvent*)), this, SLOT(canvasGotButtonPressEvent(KisButtonPressEvent*)));
	QObject::connect(m_canvas, SIGNAL(gotButtonReleaseEvent(KisButtonReleaseEvent*)), this, SLOT(canvasGotButtonReleaseEvent(KisButtonReleaseEvent*)));
	QObject::connect(m_canvas, SIGNAL(gotDoubleClickEvent(KisDoubleClickEvent*)), this, SLOT(canvasGotDoubleClickEvent(KisDoubleClickEvent*)));
	QObject::connect(m_canvas, SIGNAL(gotMoveEvent(KisMoveEvent*)), this, SLOT(canvasGotMoveEvent(KisMoveEvent*)));
	QObject::connect(m_canvas, SIGNAL(gotPaintEvent(QPaintEvent*)), this, SLOT(canvasGotPaintEvent(QPaintEvent*)));
	QObject::connect(m_canvas, SIGNAL(gotEnterEvent(QEvent*)), this, SLOT(canvasGotEnterEvent(QEvent*)));
	QObject::connect(m_canvas, SIGNAL(gotLeaveEvent(QEvent*)), this, SLOT(canvasGotLeaveEvent(QEvent*)));
	QObject::connect(m_canvas, SIGNAL(mouseWheelEvent(QWheelEvent*)), this, SLOT(canvasGotMouseWheelEvent(QWheelEvent*)));
	QObject::connect(m_canvas, SIGNAL(gotKeyPressEvent(QKeyEvent*)), this, SLOT(canvasGotKeyPressEvent(QKeyEvent*)));
	QObject::connect(m_canvas, SIGNAL(gotKeyReleaseEvent(QKeyEvent*)), this, SLOT(canvasGotKeyReleaseEvent(QKeyEvent*)));
	QObject::connect(m_canvas, SIGNAL(gotDragEnterEvent(QDragEnterEvent*)), this, SLOT(canvasGotDragEnterEvent(QDragEnterEvent*)));
	QObject::connect(m_canvas, SIGNAL(gotDropEvent(QDropEvent*)), this, SLOT(canvasGotDropEvent(QDropEvent*)));
}

void KisView::currentImageUpdated(KisImageSP img)
{
	if (img == currentImg())
		canvasRefresh();
}

bool KisView::selectColor(QColor& result)
{
	QColor color;
	bool rc;

	if ((rc = (KColorDialog::getColor(color) == KColorDialog::Accepted)))
		result.setRgb(color.red(), color.green(), color.blue());

	return rc;
}

void KisView::selectFGColor()
{
	QColor c;

	if (selectColor(c))
		setFGColor(c);
}

void KisView::selectBGColor()
{
	QColor c;

	if (selectColor(c))
		setBGColor(c);
}

void KisView::reverseFGAndBGColors()
{
	QColor oldFg = m_fg;
	QColor oldBg = m_bg;

	setFGColor(oldBg);
	setBGColor(oldFg);
}

void KisView::connectCurrentImg() const
{
	if (m_current) {
		connect(m_current, SIGNAL(activeSelectionChanged(KisImageSP)), m_selectionManager, SLOT(imgSelectionChanged(KisImageSP)));
		connect(m_current, SIGNAL(selectionCreated(KisImageSP)), m_selectionManager, SLOT(imgSelectionChanged(KisImageSP)));

// 		connect(m_current, SIGNAL(selectionCreated(KisImageSP)), SLOT(imgUpdated(KisImageSP)));
		connect(m_current, SIGNAL(profileChanged(KisProfileSP)), SLOT(profileChanged(KisProfileSP)));
		connect(m_current, SIGNAL(update(KisImageSP, const QRect&)), SLOT(imgUpdated(KisImageSP, const QRect&)));
		connect(m_current, SIGNAL(layersChanged(KisImageSP)), SLOT(layersUpdated(KisImageSP)));
		connect(m_current, SIGNAL(sizeChanged(KisImageSP, Q_INT32, Q_INT32)), SLOT(slotImageSizeChanged(KisImageSP, Q_INT32, Q_INT32)));
	}
}

void KisView::disconnectCurrentImg() const
{
	if (m_current)
		m_current -> disconnect(this);
}

void KisView::imgUpdated(KisImageSP img, const QRect& rc)
{
	if (img == currentImg()) {
		updateCanvas(rc);
	}
}

void KisView::imgUpdated(KisImageSP img)
{
	imgUpdated(img, QRect(img -> bounds()));
}

void KisView::profileChanged(KisProfileSP /*profile*/)
{
	updateStatusBarProfileLabel();
}

void KisView::slotImageSizeChanged(KisImageSP img, Q_INT32 /*w*/, Q_INT32 /*h*/)
{

	if (img == currentImg()) {
		resizeEvent(0);
	}
	canvasRefresh();
}

void KisView::resizeCurrentImage(Q_INT32 w, Q_INT32 h, bool cropLayers)
{
	if (!currentImg()) return;

	currentImg() -> resize(w, h, cropLayers);
	m_doc -> setModified(true);
	resizeEvent(0);
	layersUpdated();
	canvasRefresh();
}

void KisView::scaleCurrentImage(double sx, double sy, enumFilterType ftype)
{
	if (!currentImg()) return;
	currentImg() -> scale(sx, sy, m_progress, ftype);
	m_doc -> setModified(true);
	resizeEvent(0);
	layersUpdated();
	updateCanvas();
	canvasRefresh();
}

void KisView::rotateCurrentImage(double angle)
{
	if (!currentImg()) return;
	currentImg() -> rotate(angle, m_progress);
	m_doc -> setModified(true);
	resizeEvent(0);
	layersUpdated();
	updateCanvas();
	canvasRefresh();
}

void KisView::shearCurrentImage(double angleX, double angleY)
{
	if (!currentImg()) return;
	currentImg() -> shear(angleX, angleY ,m_progress);
	m_doc -> setModified(true);
	resizeEvent(0);
	layersUpdated();
	updateCanvas();
	canvasRefresh();
}


QPoint KisView::viewToWindow(const QPoint& pt)
{
	QPoint converted;

	converted.rx() = static_cast<int>((pt.x() + horzValue()) / zoom());
	converted.ry() = static_cast<int>((pt.y() + vertValue()) / zoom());

	return converted;
}

KisPoint KisView::viewToWindow(const KisPoint& pt)
{
	KisPoint converted;

	converted.setX((pt.x() + horzValue()) / zoom());
	converted.setY((pt.y() + vertValue()) / zoom());

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
	p.setX(static_cast<int>(pt.x() * zoom() - horzValue()));
	p.setY(static_cast<int>(pt.y() * zoom() - vertValue()));

	return p;
}

KisPoint KisView::windowToView(const KisPoint& pt)
{
	KisPoint p;
	p.setX(pt.x() * zoom() - horzValue());
	p.setY(pt.y() * zoom() - vertValue());

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


bool KisView::eventFilter(QObject *o, QEvent *e)
{
	switch (e -> type()) {
	case QEvent::TabletMove:
	case QEvent::TabletPress:
	case QEvent::TabletRelease:
	{
		QTabletEvent *te = static_cast<QTabletEvent *>(e);
		enumInputDevice device;

		switch (te -> device()) {
		default:
		case QTabletEvent::NoDevice:
		case QTabletEvent::Stylus:
			device = INPUT_DEVICE_STYLUS;
			break;
		case QTabletEvent::Puck:
			device = INPUT_DEVICE_PUCK;
			break;
		case QTabletEvent::Eraser:
			device = INPUT_DEVICE_ERASER;
			break;
		}

		setInputDevice(device);

		// We ignore device change due to mouse events for a short duration
		// after a tablet event, since these are almost certainly mouse events
		// sent to receivers that don't accept the tablet event.
		m_tabletEventTimer.start();
		break;
	}
	case QEvent::MouseButtonPress:
	case QEvent::MouseMove:
	case QEvent::MouseButtonRelease:
		if (currentInputDevice() != INPUT_DEVICE_MOUSE && m_tabletEventTimer.elapsed() > MOUSE_CHANGE_EVENT_DELAY) {
			setInputDevice(INPUT_DEVICE_MOUSE);
		}
		break;
	default:
		// Ignore
		break;
	}

	return super::eventFilter(o, e);
}


QPoint KisView::mapToScreen(const QPoint& pt)
{
	QPoint converted;

	converted.rx() = pt.x() + horzValue();
	converted.ry() = pt.y() + vertValue();
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

	if (m_current && m_doc -> contains(m_current)) {
		return m_current;
	}

	if (m_doc -> nimages() < 1) {
		return 0;
	}

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

QColor KisView::bgColor() const
{
	return m_bg;
}

QColor KisView::fgColor() const
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

KisProgressDisplayInterface *KisView::progressDisplay() const
{
	return m_progress;
}


// XXX: Temporary re-instatement of old way of getting filter and tools plugins
KisFilterSP KisView::filterGet(const KisID& id)
{
	return filterRegistry()->get( id);
}

KisIDList KisView::filterList()
{
	return filterRegistry()->listKeys();
}

KisToolRegistry * KisView::toolRegistry() const
{
	return m_toolRegistry;
};

KisFilterRegistry * KisView::filterRegistry() const
{
	return m_filterRegistry;
};

QCursor KisView::setCanvasCursor(const QCursor & cursor)
{
	QCursor oldCursor = m_canvas -> cursor();
	QCursor newCursor;

	KisConfig cfg;

	switch (cfg.defCursorStyle()) {
	case CURSOR_STYLE_TOOLICON:
		newCursor = cursor;
		break;
	case CURSOR_STYLE_CROSSHAIR:
		newCursor = KisCursor::crossCursor();
		break;
	case CURSOR_STYLE_POINTER:
		newCursor = KisCursor::arrowCursor();
		break;
	default:
		newCursor = KisCursor::crossCursor();
	}

	m_canvas -> setCursor(newCursor);
	return oldCursor;
}

#include "kis_view.moc"

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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include <qstringlist.h>

// KDE
#include <kglobalsettings.h>
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
#include <ktoolbar.h>

// KOffice
#include <koPartSelectAction.h>
#include <koFilterManager.h>
#include <koMainWindow.h>
#include <koView.h>
#include "kotabbar.h"

// Local
#include "kis_brush.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_canvas.h"
#include "kis_color.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_config.h"
#include "kis_controlframe.h"
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_double_click_event.h"
#include "kis_factory.h"
#include "kis_gradient.h"
#include "kis_guide.h"
#include "kis_layerbox.h"
#include "kis_layer.h"
#include "kis_move_event.h"
#include "kis_paint_device_impl.h"
#include "kis_paint_device_visitor.h"
#include "kis_painter.h"
#include "kis_paintop_registry.h"
#include "kis_part_layer.h"
#include "kis_pattern.h"
#include "kis_profile.h"
#include "kis_rect.h"
#include "kis_resource.h"
#include "kis_ruler.h"
#include "kis_selection.h"
#include "kotoolbox.h"
#include "kis_tool.h"
#include "kis_tool_manager.h"
#include "kis_transaction.h"
#include "kis_types.h"
#include "kis_undo_adapter.h"
#include "kis_view.h"
#include "kis_view_iface.h"
#include "labels/kis_label_cursor_pos.h"
#include "labels/kis_label_progress.h"
#include "strategy/kis_strategy_move.h"


#include <kis_resourceserver.h>
#include <kis_resource_mediator.h>

#include "kis_gray_widget.h"
#include "kis_hsv_widget.h"
#include "kis_icon_item.h"
#include "kis_palette_widget.h"
#include "kis_rgb_widget.h"
#include "kis_birdeye_box.h"
#include "kis_color.h"
#include "kis_factory.h"

// Dialog boxes
#include "kis_dlg_progress.h"
#include "kis_dlg_new_layer.h"
#include "kis_dlg_layer_properties.h"
#include "kis_dlg_transform.h"
#include "kis_dlg_preferences.h"
#include "kis_dlg_image_properties.h"

// Action managers
#include "kis_selection_manager.h"
#include "kopalettemanager.h"
#include "kis_filter_manager.h"

#define KISVIEW_MIN_ZOOM (1.0 / 16.0)
#define KISVIEW_MAX_ZOOM 16.0

// Time in ms that must pass after a tablet event before a mouse event is allowed to
// change the input device to the mouse. This is needed because mouse events are always
// sent to a receiver if it does not accept the tablet event.
#define MOUSE_CHANGE_EVENT_DELAY 100

KisView::KisView(KisDoc *doc, KisUndoAdapter *adapter, QWidget *parent, const char *name)
    : super(doc, parent, name)
    , KXMLGUIBuilder( shell() )
    , m_doc( doc )
    , m_canvas( 0 )
    , m_selectionManager( 0 )
    , m_filterManager( 0 )
    , m_paletteManager( 0 )
    , m_toolManager( 0 )
    , m_hRuler( 0 )
    , m_vRuler( 0 )
    , m_imgFlatten( 0 )
    , m_imgMergeLinked( 0 )
    , m_imgMergeVisible( 0 )
    , m_imgMergeLayer( 0 )
    , m_imgRename( 0 )
    , m_imgResizeToLayer( 0 )
    , m_imgScan( 0 )
    , m_actionPartLayer( 0 )
    , m_layerAdd( 0 )
    , m_layerBottom( 0 )
    , m_layerDup( 0 )
    , m_layerHide( 0 )
    , m_layerLink( 0 )
    , m_layerLower( 0 )
    , m_layerProperties( 0 )
    , m_layerRaise( 0 )
    , m_layerRm( 0 )
    , m_layerSaveAs( 0 )
    , m_layerTop( 0 )
    , m_zoomIn( 0 )
    , m_zoomOut( 0 )
    , m_actualPixels( 0 )
    , m_actualSize( 0 )
    , m_fullScreen( 0 )
    , m_imgProperties( 0 )
    , m_RulerAction( 0 )
    , m_dcop( 0 )
    , m_hScroll( 0 )
    , m_vScroll( 0 )
    , m_scrollX( 0 )
    , m_scrollY( 0 )
    , m_currentGuide( 0 )
    , m_adapter( adapter )
    , m_statusBarZoomLabel( 0 )
    , m_statusBarSelectionLabel( 0 )
    , m_statusBarProfileLabel( 0 )
    , m_progress( 0 )
    , m_layerBox( 0 )
    , m_toolBox( 0 )
    , m_brush( 0 )
    , m_pattern( 0 )
    , m_gradient( 0 )
    , m_monitorProfile( 0 )
    , m_HDRExposure( 0 )
    , m_inputDevice ( INPUT_DEVICE_MOUSE )
{

    kdDebug() << "Creating the view\n";

    setFocusPolicy( QWidget::StrongFocus );

    m_paletteManager = new KoPaletteManager(this, actionCollection(), "Krita palette manager");
    m_paletteManager->createPalette( krita::CONTROL_PALETTE, i18n("Control box"));
    m_paletteManager->createPalette( krita::COLORBOX, i18n("Colors"));
    m_paletteManager->createPalette( krita::LAYERBOX, i18n("Layers"));

    m_selectionManager = new KisSelectionManager(this, doc);
    m_filterManager = new KisFilterManager(this, doc);
    m_toolManager = new KisToolManager(getCanvasSubject(), getCanvasController());

    createDockers();

    setInstance(KisFactory::instance(), true );
    setClientBuilder( this );

    if (!doc -> isReadWrite())
        setXMLFile("krita_readonly.rc");
    else
        setXMLFile("krita.rc");

    m_fg = KisColor(Qt::black);
    m_bg = KisColor(Qt::white);

    createLayerBox();

    setupCanvas();
    setupRulers();
    setupScrollBars();
    setupStatusBar();

    setupActions();
    dcopObject();

    connect(m_doc, SIGNAL(imageListUpdated()), SLOT(docImageListUpdate()));

    resetMonitorProfile();

    layersUpdated();

    qApp -> installEventFilter(this);
    m_tabletEventTimer.start();

    m_brushesAndStuffToolBar = new KisControlFrame(mainWindow(), this);

}

KisView::~KisView()
{
    KisConfig cfg;
    cfg.setShowRulers( m_RulerAction->isChecked() );

    delete m_dcop;
    delete m_paletteManager;
    delete m_selectionManager;
    delete m_filterManager;
    delete m_toolManager;

}

QWidget * KisView::createContainer( QWidget *parent, int index, const QDomElement &element, int &id )
{
    kdDebug() << "Create container: " << element.attribute("name") << "\n";
    
    if( element.attribute( "name" ) == "ToolBox" )
    {
        m_toolBox = new KoToolBox(mainWindow(), "toolbox", KisFactory::instance(), NUMBER_OF_TOOLTYPES);
        m_toolBox -> setLabel(i18n("Krita"));
        m_toolManager->setUp(m_toolBox, m_paletteManager, actionCollection());
        return m_toolBox;
    }

    return KXMLGUIBuilder::createContainer( parent, index, element, id );

}

void KisView::removeContainer( QWidget *container, QWidget *parent, QDomElement &element, int id )
{
    kdDebug() << "remvoe container: " << element.attribute("name") << "\n";
    if( shell() && container == m_toolBox )
    {
        delete m_toolBox;
        m_toolManager->youAintGotNoToolBox();
    }
    else {
        KXMLGUIBuilder::removeContainer( container, parent, element, id );
    }
}

KoPaletteManager * KisView::paletteManager()
{
    if (!m_paletteManager) {
        m_paletteManager = new KoPaletteManager(this, actionCollection(), "Krita palette manager");
        Q_CHECK_PTR(m_paletteManager);
    }
    return m_paletteManager;
}


void KisView::createLayerBox()
{
    m_layerBox = new KisLayerBox(i18n("Layer"), KisLayerBox::SHOWALL, this);
    m_layerBox -> setCaption(i18n("Layers"));

    connect(m_layerBox, SIGNAL(itemToggleVisible()), this, SLOT(layerToggleVisible()));
    connect(m_layerBox, SIGNAL(itemSelected(int)), this, SLOT(layerSelected(int)));
    connect(m_layerBox, SIGNAL(itemToggleLinked()), this, SLOT(layerToggleLinked()));
    connect(m_layerBox, SIGNAL(itemToggleLocked()), this, SLOT(layerToggleLocked()));
    connect(m_layerBox, SIGNAL(itemProperties()), this, SLOT(layerProperties()));
    connect(m_layerBox, SIGNAL(itemAdd()), this, SLOT(layerAdd()));
    connect(m_layerBox, SIGNAL(itemRemove()), this, SLOT(layerRemove()));
    connect(m_layerBox, SIGNAL(itemRaise()), this, SLOT(layerRaise()));
    connect(m_layerBox, SIGNAL(itemLower()), this, SLOT(layerLower()));
    connect(m_layerBox, SIGNAL(itemFront()), this, SLOT(layerFront()));
    connect(m_layerBox, SIGNAL(itemBack()), this, SLOT(layerBack()));
    connect(m_layerBox, SIGNAL(opacityChanged(int)), this, SLOT(layerOpacity(int)));
    connect(m_layerBox, SIGNAL(itemComposite(const KisCompositeOp&)), this, SLOT(layerCompositeOp(const KisCompositeOp&)));
    
    paletteManager()->addWidget(m_layerBox, "layerbox", krita::LAYERBOX, 0);

}


DCOPObject* KisView::dcopObject()
{
    if (!m_dcop) {
        m_dcop = new KisViewIface(this);
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
        KisLayerSP layer = img->activeLayer();
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


KisProfile *  KisView::monitorProfile()
{
    if (m_monitorProfile == 0) {
        resetMonitorProfile();
    }
    return m_monitorProfile;
}


void KisView::resetMonitorProfile()
{
    m_monitorProfile = KisProfile::getScreenProfile();

    if (m_monitorProfile == 0) {
        KisConfig cfg;
        QString monitorProfileName = cfg.monitorProfile();
        m_monitorProfile = KisColorSpaceFactoryRegistry::instance() -> getProfileByName(monitorProfileName);
    }

}

void KisView::setupStatusBar()
{
    KStatusBar *sb = statusBar();

    if (sb) {
        QLabel *lbl;

        lbl = new KisLabelCursorPos(sb);
        connect(this, SIGNAL(cursorPosition(Q_INT32, Q_INT32)), lbl, SLOT(updatePos(Q_INT32, Q_INT32)));
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
    KisConfig cfg;

    m_selectionManager->setup(actionCollection());
    m_filterManager->setup(actionCollection());

    m_fullScreen = KStdAction::fullScreen( NULL, NULL, actionCollection(), this );
    connect( m_fullScreen, SIGNAL( toggled( bool )), this, SLOT( slotUpdateFullScreen( bool )));

    m_imgProperties = new KAction(i18n("Image Properties"), 0, this, SLOT(slotImageProperties()), actionCollection(), "img_properties");
    m_imgScan = 0; // How the hell do I get a KAction to the scan plug-in?!?
    m_imgResizeToLayer = new KAction(i18n("Resize Image to Size of Current Layer"), 0, this, SLOT(imgResizeToActiveLayer()), actionCollection(), "resizeimgtolayer");

    // view actions
    m_zoomIn = KStdAction::zoomIn(this, SLOT(slotZoomIn()), actionCollection(), "zoom_in");
    m_zoomOut = KStdAction::zoomOut(this, SLOT(slotZoomOut()), actionCollection(), "zoom_out");
    m_actualPixels = new KAction(i18n("Actual Pixels"), "Ctrl+0", this, SLOT(slotActualPixels()), actionCollection(), "actual_pixels");
    m_actualSize = KStdAction::actualSize(this, SLOT(slotActualSize()), actionCollection(), "actual_size");
    m_actualSize->setEnabled(false);

    // layer actions
    m_layerAdd = new KAction(i18n("&Add Layer..."), "Ctrl+Shift+N", this, SLOT(layerAdd()), actionCollection(), "insert_layer");

    m_actionPartLayer = new KoPartSelectAction( i18n( "&Object layer" ), "frame_query",
                                                    this, SLOT( addPartLayer() ),
                                                    actionCollection(), "insert_part_layer" );

    m_layerRm = new KAction(i18n("&Remove Layer"), 0, this, SLOT(layerRemove()), actionCollection(), "remove_layer");
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
    (void)new KAction(i18n("Flip on &X Axis"), "view_left_right", 0, this, SLOT(mirrorLayerX()), actionCollection(), "mirrorLayerX");
    (void)new KAction(i18n("Flip on &Y Axis"), "view_top_bottom", 0, this, SLOT(mirrorLayerY()), actionCollection(), "mirrorLayerY");

    // image actions
    m_imgMergeVisible = new KAction(i18n("Merge &Visible Layers"), "Ctrl+Shift+E", this, SLOT(mergeVisibleLayers()), actionCollection(), "merge_visible_layers");
    m_imgMergeLinked = new KAction(i18n("Merge &Linked Layers"), 0, this, SLOT(mergeLinkedLayers()), actionCollection(), "merge_linked_layers");
    m_imgMergeLayer = new KAction(i18n("&Merge Layer"), "Ctrl+E", this, SLOT(mergeLayer()), actionCollection(), "merge_layer");
    m_imgFlatten = new KAction(i18n("Merge &All Layers"), 0, this, SLOT(flattenImage()), actionCollection(), "flatten_image");

    // setting actions
    KStdAction::preferences(this, SLOT(preferences()), actionCollection(), "preferences");

    m_RulerAction = new KToggleAction( i18n( "Show Rulers" ), "Ctrl+R", this, SLOT( showRuler() ), actionCollection(), "view_ruler" );
    m_RulerAction->setChecked(cfg.showRulers());
    m_RulerAction->setCheckedState(i18n("Hide Rulers"));
    m_RulerAction->setToolTip( i18n( "Shows or hides rulers." ) );
    m_RulerAction->setWhatsThis( i18n("The rulers show the position and width of pages and of frames and can "
                                      "be used to position tabulators among others.<p>Uncheck this to disable "
                                      "the rulers from being displayed." ) );
    showRuler();

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

    if (img) {
        KisGuideMgr *mgr = img -> guides();
        mgr -> resize(size());
    }

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
    int fontheight = QFontMetrics(KGlobalSettings::generalFont()).height() * 3;
    m_vScroll -> setPageStep(drawH);
    m_vScroll -> setLineStep(fontheight);
    m_hScroll -> setPageStep(drawW);
    m_hScroll -> setLineStep(fontheight);

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
    else {
        m_hRuler -> hide();
        m_vRuler -> hide();

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
                        gc.fillRect(er, backgroundColor());
                    }

                    wr &= QRect(0, 0, img -> width(), img -> height());
                }

                if (!wr.isNull()) {

                    if (zoom() < 1.0 || zoom() > 1.0) {
                        gc.setViewport(0, 0, static_cast<Q_INT32>(m_canvasPixmap.width() * zoom()), static_cast<Q_INT32>(m_canvasPixmap.height() * zoom()));
                    }
                    gc.translate((-horzValue()) / zoom(), (-vertValue()) / zoom());

                    m_doc -> paintContent(gc, wr, monitorProfile(), HDRExposure());
                }

                paintGuides();
            }

            m_canvas -> update(vr.qRect());
        }
    } else {
        clearCanvas(r.qRect());
        m_canvas -> update(r.qRect());
    }
}

void KisView::setInputDevice(enumInputDevice inputDevice)
{
    if (inputDevice != m_inputDevice) {
        m_inputDevice = inputDevice;

        m_toolManager->setToolForInputDevice(m_inputDevice, inputDevice);

        // XXX: This is incorrect
        // On initialisation for an input device, set to eraser if the current input device
        // is a wacom eraser, else to brush.
        if (m_toolManager->currentTool() == 0) {
            if (m_inputDevice == INPUT_DEVICE_ERASER) {
                m_paintop = KisID("eraser", "");
                // XXX: Set the right entry in the paintop box
            } else {
                m_paintop = KisID("paintbrush", "");
                // XXX: Set the right entry in the paintop box
            }
            m_toolManager->setCurrentTool(m_toolManager->findTool("tool_brush", m_inputDevice));
        }
        else {
            m_toolManager->setCurrentTool(m_toolManager->currentTool());
        }
        m_toolManager->activateCurrentTool();
    }

}

enumInputDevice KisView::currentInputDevice() const
{
    return m_inputDevice;
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

void KisView::canvasRefresh()
{
    KisRect rc(0, 0, m_canvasPixmap.width(), m_canvasPixmap.height());

    paintView(viewToWindow(rc));
    m_canvas -> repaint();
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

    enable = enable && img && layer && layer->visible() && !layer->locked();
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

    // XXX these should be named layer instead of img
    m_imgFlatten -> setEnabled(nlayers > 1);

    m_imgMergeVisible -> setEnabled(nvisible > 1);
    m_imgMergeLinked -> setEnabled(nlinked > 1);
    m_imgMergeLayer -> setEnabled(nlayers > 1 && layerPos < nlayers - 1);

    m_selectionManager->updateGUI();
    m_filterManager->updateGUI();
    m_toolManager->updateGUI();

    imgUpdateGUI();
}


void KisView::imgUpdateGUI()
{
    KisImageSP img = currentImg();

    m_imgResizeToLayer -> setEnabled(img && img -> activeLayer());

    updateStatusBarProfileLabel();
}

void KisView::zoomAroundPoint(Q_INT32 x, Q_INT32 y, double zf)
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

        zoomAroundPoint(cx, cy, zf);
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
        zoomAroundPoint(x, y, zoom() * 2);
}

void KisView::zoomOut(Q_INT32 x, Q_INT32 y)
{
    if (zoom() >= KISVIEW_MIN_ZOOM)
        zoomAroundPoint(x, y, zoom() / 2);
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
        zoomAroundPoint(-1, -1, zoom() * 2);
}

void KisView::slotZoomOut()
{
    if (zoom() >= KISVIEW_MIN_ZOOM)
        zoomAroundPoint(-1, -1, zoom() / 2);
}

void KisView::slotActualPixels()
{
    zoomAroundPoint(-1, -1, 1.0);
}

void KisView::slotActualSize()
{
    //XXX later this should be update to take screen res and image res into consideration
    zoomAroundPoint(-1, -1, 1.0);
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
    if (importImage() > 0)
        m_doc -> setModified(true);
}

void KisView::saveLayerAsImage()
{
    QStringList listMimeFilter = KoFilterManager::mimeFilter("application/x-krita", KoFilterManager::Export);
    QString mimelist = listMimeFilter.join(" ");

    KFileDialog fd (QString::null, mimelist, this, "Export Layer", true);
    fd.setCaption(i18n("Export Layer"));
    fd.setMimeFilter(listMimeFilter);
    fd.setOperationMode(KFileDialog::Saving);

    if (!fd.exec()) return;

    KURL url = fd.selectedURL();
    QString mimefilter = fd.currentMimeFilter();

    if (url.isEmpty())
        return;


    KisImageSP img = currentImg();
    if (!img) return;

    KisLayerSP l = img -> activeLayer();
    if (!l) return;

    QRect r = l -> exactBounds();

    KisDoc d;
    d.prepareForImport();

    KisImageSP dst = new KisImage(&d, r.width(), r.height(), l->colorSpace(), l->name());
    d.setCurrentImage( dst );
    KisLayerSP layer = dst->layerAdd(l->name(), COMPOSITE_COPY, l->opacity(), l->colorSpace());
    if (!layer) return;

    kdDebug() << "Exporting layer to colorspace " << layer->colorSpace()->id().name() << ", image: " << dst->colorSpace()->id().name() << "\n";
    KisPainter p(layer);
    p.bitBlt(0, 0, COMPOSITE_COPY, l.data(), r.x(), r.y(), r.width(), r.height());
    p.end();
    d.setOutputMimeType(mimefilter.latin1());
    d.exp0rt(url);
}



Q_INT32 KisView::importImage(const KURL& urlArg)
{
    KURL::List urls;
    Q_INT32 rc = 0;

    if (urlArg.isEmpty()) {
        QString mimelist = KoFilterManager::mimeFilter("application/x-krita", KoFilterManager::Import).join(" ");
        urls = KFileDialog::getOpenURLs(QString::null, mimelist, 0, i18n("Import Image"));
    } else {
        urls.push_back(urlArg);
    }

    if (urls.empty())
        return 0;

    KisImageSP img;

    for (KURL::List::iterator it = urls.begin(); it != urls.end(); ++it) {
        KURL url = *it;
        KisDoc d;
        d.import(url);
        img = d.currentImage();

        if (currentImg()) {
            vKisLayerSP v = img -> layers();
            KisImageSP current = currentImg();

            rc += v.size();
            current -> activeLayer() -> deselect();

            for (vKisLayerSP_it it = v.begin(); it != v.end(); ++it) {
                KisLayerSP layer = *it;

                layer -> setImage(current);
                layer -> setName(current -> nextLayerName());
                current->layerAdd(layer, 0);
                m_layerBox->slotSetCurrentItem(img -> index(layer));
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

void KisView::scaleLayer(double sx, double sy, KisFilterStrategy *filterStrategy)
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

    layer -> scale(sx, sy, m_progress, filterStrategy);

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
    if ( PreferencesDialog::editPreferences() )
    {
    resetMonitorProfile();
    canvasRefresh();
    if (m_toolManager->currentTool()) {
            setCanvasCursor(m_toolManager->currentTool() -> cursor());
    }
    }
}

void KisView::layerCompositeOp(const KisCompositeOp& compositeOp)
{
    KisImageSP img = currentImg();
    if (!img) return;

    KisLayerSP layer = img -> activeLayer();
    if (!layer) return;

    KNamedCommand *cmd = layer -> setCompositeOpCommand(compositeOp);
    cmd -> execute();
    undoAdapter() -> addCommand(cmd);
}

// range: 0 - 100
void KisView::layerOpacity(int opacity)
{
    KisImageSP img = currentImg();
    if (!img) return;

    KisLayerSP layer = img -> activeLayer();
    if (!layer) return;

    opacity = opacity * 255 / 100;
    if (opacity > 255)
        opacity = 255;

    KNamedCommand *cmd = layer -> setOpacityCommand(opacity);
    cmd -> execute();
    undoAdapter() -> addCommand(cmd);
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
        notifyObservers();
    }
}

void KisView::patternActivated(KisResource *pattern)
{
    m_pattern = dynamic_cast<KisPattern*>(pattern);

    if (m_pattern) {
        emit patternChanged(m_pattern);
        notifyObservers();
    }
}

void KisView::gradientActivated(KisResource *gradient)
{

    m_gradient = dynamic_cast<KisGradient*>(gradient);

    if (m_gradient) {
        emit gradientChanged(m_gradient);
        notifyObservers();
    }
}

void KisView::paintopActivated(const KisID & paintop)
{

    if (paintop.id().isNull() || paintop.id().isEmpty()) {
        return;
    }

    m_paintop = paintop;
    emit paintopChanged(m_paintop);
    notifyObservers();
}

void KisView::setBGColor(const KisColor& c)
{
    m_bg = c;
    notifyObservers();
}

void KisView::setFGColor(const KisColor& c)
{
    m_fg = c;
    notifyObservers();
}

void KisView::slotSetFGColor(const KisColor& c)
{
    setFGColor(c);
}

void KisView::slotSetBGColor(const KisColor& c)
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
    QString printerProfileName = cfg.printerProfile();
    KisProfile *  printerProfile = KisColorSpaceFactoryRegistry::instance() -> getProfileByName(printerProfileName);

    if (printerProfile != 0)
        kdDebug(DBG_AREA_CMS) << "Printer profile: " << printerProfile -> productName() << "\n";

    QRect r = img -> bounds();
    img -> renderToPainter(r.x(), r.y(), r.width(), r.height(), gc, printerProfile, HDRExposure());
}




void KisView::canvasGotPaintEvent(QPaintEvent *event)
{
    QMemArray<QRect> rects = event -> region().rects();

    for (unsigned int i = 0; i < rects.count(); i++) {
        QRect er = rects[i];

        bitBlt(m_canvas, er.x(), er.y(), &m_canvasPixmap, er.x(), er.y(), er.width(), er.height());
    }

    if (m_toolManager->currentTool()) {
        QPainter gc(m_canvas);

        gc.setClipRegion(event -> region());
        gc.setClipping(true);

        m_toolManager->currentTool()->paint(gc, event -> rect());
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

    KisImageSP img = currentImg();

    if (img) {
        QPoint pt = mapToScreen(e -> pos().floorQPoint());
        KisGuideMgr *mgr = img -> guides();

        m_lastGuidePoint = mapToScreen(e -> pos().floorQPoint());
        m_currentGuide = 0;

        if ((e -> state() & ~Qt::ShiftButton) == Qt::NoButton) {
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

    if (e -> device() == currentInputDevice() && m_toolManager->currentTool()) {
        KisPoint p = viewToWindow(e -> pos());
        KisButtonPressEvent ev(e -> device(), p, e -> globalPos(), e -> pressure(), e -> xTilt(), e -> yTilt(), e -> button(), e -> state());

        m_toolManager->currentTool() -> buttonPress(&ev);
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

    if (img && m_currentGuide) {
        QPoint p = mapToScreen(e -> pos().floorQPoint());
        KisGuideMgr *mgr = img -> guides();

        if (((e -> state() & Qt::LeftButton) == Qt::LeftButton) && mgr -> hasSelected()) {
            eraseGuides();
            p -= m_lastGuidePoint;

            if (p.x())
                mgr -> moveSelectedByX(p.x() / zoom());

            if (p.y())
                mgr -> moveSelectedByY(p.y() / zoom());

            m_doc -> setModified(true);
            paintGuides();
        }
    } else if (e -> device() == currentInputDevice() && m_toolManager->currentTool()) {
        KisMoveEvent ev(e -> device(), wp, e -> globalPos(), e -> pressure(), e -> xTilt(), e -> yTilt(), e -> state());

        m_toolManager->currentTool() -> move(&ev);
    }

    m_lastGuidePoint = mapToScreen(e -> pos().floorQPoint());
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

    KisImageSP img = currentImg();

    if (img && m_currentGuide) {
        m_currentGuide = 0;
    } else if (e -> device() == currentInputDevice() && m_toolManager->currentTool()) {
        KisPoint p = viewToWindow(e -> pos());
        KisButtonReleaseEvent ev(e -> device(), p, e -> globalPos(), e -> pressure(), e -> xTilt(), e -> yTilt(), e -> button(), e -> state());

        if (m_toolManager->currentTool()) {
            m_toolManager->currentTool() -> buttonRelease(&ev);
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

    if (e -> device() == currentInputDevice() && m_toolManager->currentTool()) {
        KisPoint p = viewToWindow(e -> pos());
        KisDoubleClickEvent ev(e -> device(), p, e -> globalPos(), e -> pressure(), e -> xTilt(), e -> yTilt(), e -> button(), e -> state());

        if (m_toolManager->currentTool()) {
            m_toolManager->currentTool() -> doubleClick(&ev);
        }
    }
}

void KisView::canvasGotEnterEvent(QEvent *e)
{
    if (m_toolManager->currentTool())
        m_toolManager->currentTool() -> enter(e);
}

void KisView::canvasGotLeaveEvent (QEvent *e)
{
    if (m_toolManager->currentTool())
        m_toolManager->currentTool() -> leave(e);
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
    if (m_toolManager->currentTool())
        m_toolManager->currentTool() -> keyPress(event);
}

void KisView::canvasGotKeyReleaseEvent(QKeyEvent *event)
{
    if (m_toolManager->currentTool())
        m_toolManager->currentTool() -> keyRelease(event);
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
                addDocumentId = 2,
                cancelId
            };

            KPopupMenu popup(this, "drop_popup");

            if (urls.count() == 1) {
                if (currentImg() != 0) {
                    popup.insertItem(i18n("Insert as New Layer"), addLayerId);
                }
                popup.insertItem(i18n("Open in New Document"), addDocumentId);
            }
            else {
                if (currentImg() != 0) {
                    popup.insertItem(i18n("Insert as New Layers"), addLayerId);
                }
                popup.insertItem(i18n("Open in New Documents"), addDocumentId);
            }

            popup.insertSeparator();
            popup.insertItem(i18n("Cancel"), cancelId);

            int actionId = popup.exec(QCursor::pos());

            if (actionId >= 0 && actionId != cancelId) {
                for (KURL::List::ConstIterator it = urls.begin (); it != urls.end (); ++it) {
                    KURL url = *it;

                    switch (actionId) {
                    case addLayerId:
                        importImage(url);
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

void KisView::docImageListUpdate()
{
    disconnectCurrentImg();
    m_current = 0;
    zoomAroundPoint(0, 0, 1.0);
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
            KNamedCommand *cmd = layer -> setLinkedCommand(!layer -> linked());
            cmd -> execute();
            undoAdapter() -> addCommand(cmd);
        }
    }
}

void KisView::layerProperties()
{
    KisImageSP img = currentImg();

    if (img) {
        KisLayerSP layer = img -> activeLayer();

        if (layer) {
            KisDlgLayerProperties dlg(layer -> name(),
                                     layer->opacity(),
                                     layer->compositeOp(),
                                     layer->colorSpace());

            if (dlg.exec() == QDialog::Accepted) {
                bool changed = layer -> name() != dlg.getName()
                           || layer -> opacity() != dlg.getOpacity()
                           || layer -> compositeOp() != dlg.getCompositeOp();

                if (changed)
                    m_adapter -> beginMacro(i18n("Property changes"));

                if (layer -> name() != dlg.getName()
                    || layer -> opacity() != dlg.getOpacity()
                    || layer -> compositeOp() != dlg.getCompositeOp())
                {
                    img->setLayerProperties(layer, dlg.getOpacity(), dlg.getCompositeOp(), dlg.getName());
                }

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
        NewLayerDialog dlg(img->colorSpace()->id(), img->nextLayerName(), this);

        if (dlg.exec() == QDialog::Accepted) {
            KisLayerSP layer = img->layerAdd(dlg.layerName(), dlg.compositeOp(), dlg.opacity(), KisColorSpaceFactoryRegistry::instance() -> getColorSpace(dlg.colorSpaceID(),""));
            if (layer) {
                m_layerBox->slotSetCurrentItem(img -> index(layer));
                resizeEvent(0);
                updateCanvas(0, 0, img -> width(), img -> height());
            } else {
                KMessageBox::error(this, i18n("Could not add layer to image."), i18n("Layer Error"));
            }
        }
    }
}

void KisView::addPartLayer()
{
    KisImageSP img = currentImg();
    if (!img) return;

    KoDocumentEntry  e = m_actionPartLayer->documentEntry();

    KoDocument* doc = e.createDoc(m_doc);
    if ( !doc )
        return;

    if ( !doc->initDoc(KoDocument::InitDocEmbedded) )
        return;

    KisChildDoc * childDoc = m_doc->createChildDoc(img->bounds(), doc);

    KisPartLayer * partLayer = new KisPartLayer(img, childDoc);
    img->layerAdd(partLayer, 0);

    m_doc->setModified(true);


}



void KisView::layerRemove()
{
    KisImageSP img = currentImg();

    if (img) {
        KisLayerSP layer = img -> activeLayer();

        if (layer) {
            Q_INT32 n = img -> index(layer);

            img->layerRemove(layer);
            m_layerBox->slotSetCurrentItem(n - 1);
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
    KisLayerSP layer = img->layerAdd(dup, index);

    if (layer) {
        m_layerBox->slotSetCurrentItem(img -> index(layer));
        resizeEvent(0);
        updateCanvas(0, 0, img -> width(), img -> height());
    } else {
        KMessageBox::error(this, i18n("Could not add layer to image."), i18n("Layer Error"));
    }
}

void KisView::layerRaise()
{
    KisImageSP img = currentImg();
    KisLayerSP layer;

    if (!img)
        return;

    layer = img -> activeLayer();

    if (layer) {
        KCommand *cmd = img -> raiseLayerCommand(layer);
        cmd -> execute();
        undoAdapter() -> addCommand(cmd);
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
        KCommand *cmd = img -> lowerLayerCommand(layer);
        cmd -> execute();
        undoAdapter() -> addCommand(cmd);
    }
}

void KisView::layerFront()
{
    KisImageSP img = currentImg();
    KisLayerSP layer;

    if (img && (layer = img -> activeLayer())) {
        KCommand *cmd = img -> topLayerCommand(layer);
        cmd -> execute();
        undoAdapter() -> addCommand(cmd);
    }
}

void KisView::layerBack()
{
    KisImageSP img = currentImg();
    if (!img) return;

    KisLayerSP layer;

    if (img && (layer = img -> activeLayer())) {
        KCommand *cmd = img -> bottomLayerCommand(layer);
        cmd -> execute();
        undoAdapter() -> addCommand(cmd);
    }
}

void KisView::layersUpdated()
{
    KisImageSP img = currentImg();
    if (!img) return;

    KisLayerSP layer = img -> activeLayer();

    layerUpdateGUI(img && layer);

    m_layerBox -> setUpdatesAndSignalsEnabled(false);
    m_layerBox -> clear();

    if (img) {
         vKisLayerSP l = img -> layers();
         for (vKisLayerSP_it it = l.begin(); it != l.end(); ++it)
              m_layerBox -> insertItem((*it) -> name(), (*it) -> visible(), (*it) -> linked(), (*it) -> locked());
        
        layerSelected( img -> index(layer) );
        m_layerBox -> slotSetCurrentItem(img -> index(layer));
    }

    m_layerBox -> setUpdatesAndSignalsEnabled(true);
    m_layerBox -> updateAll();
    img->notify();
    notifyObservers();
}

void KisView::layersUpdated(KisImageSP img)
{
    if (img == currentImg())
        layersUpdated();
}

void KisView::layerToggleVisible()
{
    KisImageSP img = currentImg();
    if (!img) return;

    KisLayerSP layer = img -> activeLayer();
    if (!layer) return;

    KNamedCommand *cmd = layer -> setVisibleCommand(!layer -> visible());
    cmd -> execute();
    img->notify(); // We have changed the visual appearance of the image here, so notify.
    undoAdapter() -> addCommand(cmd);
}

void KisView::layerToggleLocked()
{
    KisImageSP img = currentImg();
    if (!img) return;

    KisLayerSP layer = img -> activeLayer();
    if (!layer) return;

    KNamedCommand *cmd = layer -> setLockedCommand(!layer -> locked());
    cmd -> execute();
    undoAdapter() -> addCommand(cmd);
}

void KisView::layerSelected(int n)
{
    KisImageSP img = currentImg();
    if (!img) return;
    
    KisLayerSP l = img -> layer(n);
    if (!l) return;

    layerUpdateGUI(img -> activateLayer(n));
    notifyObservers();

    Q_INT32 opacity = l -> opacity();
    opacity = opacity * 100 / 255;
    if (opacity)
        opacity++;

    m_layerBox -> setOpacity(opacity);
    m_layerBox -> setColorSpace(l -> colorSpace());
    m_layerBox -> setCompositeOp(l -> compositeOp());
    m_layerBox -> slotSetCurrentItem(n);

    updateCanvas();
}

void KisView::scrollH(int value)
{
    m_hRuler -> updateVisibleArea(value, 0);

    int xShift = m_scrollX - value;
    m_scrollX = value;

    if (m_canvas -> isUpdatesEnabled()) {
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
}

void KisView::scrollV(int value)
{
    m_vRuler -> updateVisibleArea(0, value);

    int yShift = m_scrollY - value;
    m_scrollY = value;

    if (m_canvas -> isUpdatesEnabled()) {
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

void KisView::connectCurrentImg() const
{
    if (m_current) {
        connect(m_current, SIGNAL(sigActiveSelectionChanged(KisImageSP)), m_selectionManager, SLOT(imgSelectionChanged(KisImageSP)));

        connect(m_current, SIGNAL(sigLayersUpdated(KisImageSP)), SLOT(layersUpdated(KisImageSP)));
        connect(m_current, SIGNAL(sigProfileChanged(KisProfile * )), SLOT(profileChanged(KisProfile * )));
        
        connect(m_current, SIGNAL(sigImageUpdated(KisImageSP, const QRect&)), SLOT(imgUpdated(KisImageSP, const QRect&)));
    
        connect(m_current, SIGNAL(sigLayersChanged(KisImageSP)), SLOT(layersUpdated(KisImageSP)));
        connect(m_current, SIGNAL(sigSizeChanged(KisImageSP, Q_INT32, Q_INT32)), SLOT(slotImageSizeChanged(KisImageSP, Q_INT32, Q_INT32)));
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

void KisView::profileChanged(KisProfile *  /*profile*/)
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

void KisView::scaleCurrentImage(double sx, double sy, KisFilterStrategy *filterStrategy)
{
    if (!currentImg()) return;
    currentImg() -> scale(sx, sy, m_progress, filterStrategy);
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

    if ((o == m_hRuler || o == m_vRuler) && (e -> type() == QEvent::MouseMove || e -> type() == QEvent::MouseButtonRelease)) {
        QMouseEvent *me = dynamic_cast<QMouseEvent*>(e);
        QPoint pt = mapFromGlobal(me -> globalPos());
        KisImageSP img = currentImg();
        KisGuideMgr *mgr;

        if (!img)
            return super::eventFilter(o, e);

        mgr = img -> guides();

        if (e -> type() == QEvent::MouseMove && (me -> state() & Qt::LeftButton)) {
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
                    KisMoveEvent kme(currentInputDevice(), pt, me -> globalPos(), PRESSURE_DEFAULT, 0, 0, me -> state());
                    canvasGotMoveEvent(&kme);
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
            KisMoveEvent kme(currentInputDevice(), pt, me -> globalPos(), PRESSURE_DEFAULT, 0, 0, Qt::NoButton);
            canvasGotMoveEvent(&kme);
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
            mgr -> erase(&m_canvasPixmap, this, horzValue(), vertValue(), zoom());
    }
}

void KisView::paintGuides()
{
    KisImageSP img = currentImg();

    if (img) {
        KisGuideMgr *mgr = img -> guides();

        if (mgr)
            mgr -> paint(&m_canvasPixmap, this, horzValue(), vertValue(), zoom());
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

void KisView::notifyObservers()
{
    for (vKisCanvasObserver_it it = m_observers.begin(); it != m_observers.end(); ++it) {
        (*it) -> update(this);
    }
}

KisImageSP KisView::currentImg() const
{
    if (m_current != m_doc -> currentImage())
    {
        m_current = m_doc -> currentImage();
        m_current->notify();
        connectCurrentImg();
    }

    return m_current;
}

KisColor KisView::bgColor() const
{
    return m_bg;
}

KisColor KisView::fgColor() const
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

KisID KisView::currentPaintop() const
{
    return m_paintop;
}

double KisView::zoomFactor() const
{
    return zoom();
}

KisUndoAdapter *KisView::undoAdapter() const
{
    return m_adapter;
}

KisCanvasController *KisView::canvasController() const
{
    return const_cast<KisCanvasController*>(static_cast<const KisCanvasController*>(this));
}

KisToolControllerInterface *KisView::toolController() const
{
    return const_cast<KisToolControllerInterface*>(static_cast<const KisToolControllerInterface*>(m_toolManager));
}

KisDoc *KisView::document() const
{
    return m_doc;
}

KisProgressDisplayInterface *KisView::progressDisplay() const
{
    return m_progress;
}

QCursor KisView::setCanvasCursor(const QCursor & cursor)
{
    QCursor oldCursor = m_canvas -> cursor();
    QCursor newCursor;

    KisConfig cfg;

    switch (cfg.cursorStyle()) {
    case CURSOR_STYLE_TOOLICON:
        newCursor = cursor;
        break;
    case CURSOR_STYLE_CROSSHAIR:
        newCursor = KisCursor::crossCursor();
        break;
    case CURSOR_STYLE_POINTER:
        newCursor = KisCursor::arrowCursor();
        break;
    case CURSOR_STYLE_OUTLINE:
        kdDebug() << "Outline\n";
        newCursor = cursor;
        break;
    default:
        newCursor = cursor;
    }

    m_canvas -> setCursor(newCursor);
    return oldCursor;
}

float KisView::HDRExposure() const
{
    return m_HDRExposure;
}

void KisView::setHDRExposure(float exposure)
{
    if (exposure != m_HDRExposure) {
        m_HDRExposure = exposure;
        updateCanvas();
    }
}

void KisView::createDockers()
{

    m_birdEyeBox = new KisBirdEyeBox(this);
    m_birdEyeBox -> setCaption(i18n("Overview"));
    m_paletteManager->addWidget( m_birdEyeBox, "birdeyebox", krita::CONTROL_PALETTE);

    m_hsvwidget = new KisHSVWidget(this, "hsv");
    m_hsvwidget -> setCaption(i18n("HSV"));
    m_paletteManager->addWidget( m_hsvwidget, "hsvwidget", krita::COLORBOX);
    attach(m_hsvwidget);

    m_rgbwidget = new KisRGBWidget(this, "rgb");
    m_rgbwidget -> setCaption(i18n("RGB"));
    m_paletteManager->addWidget( m_rgbwidget, "rgbwidget", krita::COLORBOX);
    attach(m_rgbwidget);

    m_graywidget = new KisGrayWidget(this, "gray");
    m_graywidget -> setCaption(i18n("Gray"));
    m_paletteManager->addWidget( m_graywidget, "graywidget", krita::COLORBOX);
    attach(m_graywidget);

    m_palettewidget = new KisPaletteWidget(this);
    m_palettewidget -> setCaption(i18n("Palettes"));

    KisResourceServerBase* rServer;
    rServer = KisFactory::rServerRegistry() -> get("PaletteServer");
    QValueList<KisResource*> resources = rServer->resources();
    QValueList<KisResource*>::iterator it;
    for ( it = resources.begin(); it != resources.end(); ++it ) {
        m_palettewidget -> slotAddPalette( *it );
    }
    connect(m_palettewidget, SIGNAL(colorSelected(const KisColor &)), this, SLOT(slotSetFGColor(const KisColor &)));
    m_paletteManager->addWidget( m_palettewidget, "palettewidget", krita::COLORBOX);

    m_paletteManager->showWidget("hsvwidget");
    m_paletteManager->showWidget("layerbox");
    m_paletteManager->showWidget(krita::TOOL_OPTION_WIDGET);


}

#include "kis_view.moc"


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
#include <kis_meta_registry.h>
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
#include <kparts/plugin.h>
#include <kservice.h>
#include <ktrader.h>
#include <kparts/componentfactory.h>

// KOffice
#include <koPartSelectAction.h>
#include <koFilterManager.h>
#include <koMainWindow.h>
#include <koView.h>
#include <kotabbar.h>
#include <ko_gray_widget.h>
#include <ko_hsv_widget.h>
#include <ko_rgb_widget.h>

// Local
#include "kis_brush.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_canvas.h"
#include "kis_canvas_painter.h"
#include "kis_color.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_config.h"
#include "kis_controlframe.h"
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_double_click_event.h"
#include "kis_factory.h"
#include "kis_gradient.h"
#include "kis_group_layer.h"
//#include "kis_guide.h"

#include "kis_layerbox.h"

#include "kis_layer.h"
#include "kis_paint_layer.h"
#include "kis_move_event.h"
#include "kis_paint_device_impl.h"
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
#include "kis_label_cursor_pos.h"
#include "kis_label_progress.h"
#include "kis_opengl_image_context.h"
#include "kis_background.h"
#include "kis_paint_device_action.h"

#include <kis_resourceserver.h>
#include <kis_resource_mediator.h>

#include "kis_icon_item.h"
#include "kis_palette_widget.h"
#include "kis_birdeye_box.h"
#include "kis_color.h"
#include "kis_factory.h"

// Dialog boxes
#include "kis_dlg_new_layer.h"
#include "kis_dlg_layer_properties.h"
#include "kis_dlg_transform.h"
#include "kis_dlg_preferences.h"
#include "kis_dlg_image_properties.h"

// Action managers
#include "kis_selection_manager.h"
#include "kopalettemanager.h"
#include "kis_filter_manager.h"

#include "kis_custom_palette.h"

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
    , m_actLayerVis( false )
    , m_hRuler( 0 )
    , m_vRuler( 0 )
    , m_imgFlatten( 0 )
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
//    , m_currentGuide( 0 )
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

    setFocusPolicy( QWidget::StrongFocus );

    m_paletteManager = new KoPaletteManager(this, actionCollection(), "Krita palette manager");
    m_paletteManager->createPalette( krita::CONTROL_PALETTE, i18n("Control box"));
    m_paletteManager->createPalette( krita::COLORBOX, i18n("Colors"));
    m_paletteManager->createPalette( krita::LAYERBOX, i18n("Layers"));

    m_selectionManager = new KisSelectionManager(this, doc);
    m_filterManager = new KisFilterManager(this, doc);
    m_toolManager = new KisToolManager(getCanvasSubject(), getCanvasController());

    // This needs to be set before the dockers are created.
    m_current = m_doc -> currentImage();

    createDockers();

    setInstance(KisFactory::instance(), false);
    setClientBuilder( this );

    if (!doc -> isReadWrite())
        setXMLFile("krita_readonly.rc");
    else
        setXMLFile("krita.rc");

    KStdAction::keyBindings( mainWindow()->guiFactory(), SLOT( configureShortcuts() ), actionCollection() );

    KisColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getRGB8();
    m_fg = KisColor(Qt::black, cs);
    m_bg = KisColor(Qt::white, cs);

    createLayerBox();

    setupCanvas();
    setupRulers();
    setupScrollBars();
    setupStatusBar();

    setupActions();
    dcopObject();


    connect(this, SIGNAL(autoScroll(const QPoint &)), SLOT(slotAutoScroll(const QPoint &)));

    setMouseTracking(true);

    resetMonitorProfile();

    layersUpdated();

    qApp -> installEventFilter(this);
    m_tabletEventTimer.start();

    m_brushesAndStuffToolBar = new KisControlFrame(mainWindow(), this);

    // Load all plugins
    KTrader::OfferList offers = KTrader::self() -> query(QString::fromLatin1("Krita/ViewPlugin"));//,
//                                                         QString::fromLatin1("(Type == 'Service') and "
//                                                                             "([X-KDE-Version] == 2)"));
    KTrader::OfferList::ConstIterator iter;
    kdDebug() << "Found " << offers.count() << " view plugins\n";
    for(iter = offers.begin(); iter != offers.end(); ++iter)
    {
        KService::Ptr service = *iter;
        int errCode = 0;
        KParts::Plugin* plugin =
             KParts::ComponentFactory::createInstanceFromService<KParts::Plugin> ( service, this, 0, QStringList(), &errCode);
        if ( plugin ) {
            kdDebug() << "found plugin " << service -> property("Name").toString() << "\n";
            insertChildClient(plugin);
        }
        else {
            kdDebug() << "found plugin " << service -> property("Name").toString() << ", " << errCode << "\n";
        }
    }

    // Set the current image for real now everything is ready to go.
    setCurrentImage(m_current);
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


static Qt::Dock stringToDock( const QString& attrPosition )
{
    KToolBar::Dock dock = KToolBar::DockTop;
    if ( !attrPosition.isEmpty() ) {
        if ( attrPosition == "top" )
            dock = Qt::DockTop;
        else if ( attrPosition == "left" )
            dock = Qt::DockLeft;
        else if ( attrPosition == "right" )
            dock = Qt::DockRight;
        else if ( attrPosition == "bottom" )
            dock = Qt::DockBottom;
        else if ( attrPosition == "floating" )
            dock = Qt::DockTornOff;
        else if ( attrPosition == "flat" )
            dock = Qt::DockMinimized;
    }
    return dock;
}

QWidget * KisView::createContainer( QWidget *parent, int index, const QDomElement &element, int &id )
{
    if( element.attribute( "name" ) == "ToolBox" )
    {
        m_toolBox = new KoToolBox(mainWindow(), "ToolBox", KisFactory::instance(), NUMBER_OF_TOOLTYPES);
        m_toolBox -> setLabel(i18n("Krita"));
        m_toolManager->setUp(m_toolBox, m_paletteManager, actionCollection());

        kdDebug() << "Toolbox position: " << element.attribute( "position" ) << "\n";
        Dock dock = stringToDock( element.attribute( "position" ).lower() );

        mainWindow()->addDockWindow( m_toolBox, dock, false);
        mainWindow()->moveDockWindow( m_toolBox, dock, false, 0, 0 );
    }

    return KXMLGUIBuilder::createContainer( parent, index, element, id );

}

void KisView::removeContainer( QWidget *container, QWidget *parent, QDomElement &element, int id )
{
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
    m_layerBox = new KisLayerBox(this);
    m_layerBox -> setCaption(i18n("Layers"));

    m_layerBox -> setImage(currentImg());
    connect(m_layerBox, SIGNAL(sigRequestLayer(KisGroupLayerSP, KisLayerSP)),
            this, SLOT(addLayer(KisGroupLayerSP, KisLayerSP)));
    connect(m_layerBox, SIGNAL(sigRequestGroupLayer(KisGroupLayerSP, KisLayerSP)),
            this, SLOT(addGroupLayer(KisGroupLayerSP, KisLayerSP)));
    connect(m_layerBox, SIGNAL(sigRequestLayerProperties(KisLayerSP)),
            this, SLOT(showLayerProperties(KisLayerSP)));
    connect(m_layerBox, SIGNAL(sigOpacityChanged(int)), this, SLOT(layerOpacity(int)));
    connect(m_layerBox, SIGNAL(sigItemComposite(const KisCompositeOp&)), this, SLOT(layerCompositeOp(const KisCompositeOp&)));

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
        KisPaintDeviceImplSP dev = img->activeDevice();
        if (dev) {
            if (dev -> hasSelection()) {
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

    if (img -> getProfile() == 0) {
        m_statusBarProfileLabel -> setText(i18n("No profile"));
    }
    else {
        m_statusBarProfileLabel -> setText(img -> getProfile() -> productName());
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
        m_monitorProfile = KisMetaRegistry::instance()->csRegistry()->getProfileByName(monitorProfileName);
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

    m_actionPartLayer = new KoPartSelectAction( i18n( "&Object Layer" ), "frame_query",
                                                    this, SLOT( addPartLayer() ),
                                                    actionCollection(), "insert_part_layer" );

    m_layerRm = new KAction(i18n("&Remove Layer"), 0, this, SLOT(layerRemove()), actionCollection(), "remove_layer");
    m_layerDup = new KAction(i18n("Duplicate Layer"), 0, this, SLOT(layerDuplicate()), actionCollection(), "duplicate_layer");
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

    // Add new palette
    // FIXME this should be in some kind of 'Resources' dialog as a seperate tab, or so,
    // merged with 'editing' of existing, writeable palettes
    new KAction(i18n("Add New Palette..."), 0, this, SLOT(slotAddPalette()),
                actionCollection(), "add_palette");

    showRuler();

}

void KisView::resizeEvent(QResizeEvent *)
{
    KisImageSP img = currentImg();
    Q_INT32 scrollBarExtent = style().pixelMetric(QStyle::PM_ScrollBarExtent);
    Q_INT32 drawH;
    Q_INT32 drawW;
    Q_INT32 docW;
    Q_INT32 docH;

//    if (img) {
//        KisGuideMgr *mgr = img -> guides();
//        mgr -> resize(size());
//    }

    docW = static_cast<Q_INT32>(ceil(docWidth() * zoom()));
    docH = static_cast<Q_INT32>(ceil(docHeight() * zoom()));

    m_rulerThickness = m_RulerAction -> isChecked() ? 20 : 0;
    drawH = height() - m_rulerThickness;
    drawW = width() - m_rulerThickness;

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

    m_hRuler -> setGeometry(m_rulerThickness, 0, drawW, m_rulerThickness);
    m_vRuler -> setGeometry(0, m_rulerThickness, m_rulerThickness, drawH);

    m_vScroll -> setEnabled(docH > drawH);
    m_hScroll -> setEnabled(docW > drawW);

    if (docH <= drawH && docW <= drawW) {
        // we need no scrollbars
        m_vScroll -> hide();
        m_hScroll -> hide();
        m_vScroll -> setValue(0);
        m_hScroll -> setValue(0);
        m_vScrollBarExtent = 0;
        m_hScrollBarExtent = 0;
    } else if (docH <= drawH) {
        // we need a horizontal scrollbar only
        m_vScroll -> hide();
        m_vScroll -> setValue(0);
        m_hScroll -> setRange(0, docW - drawW);
        m_hScroll -> setGeometry(m_rulerThickness,
                     height() - scrollBarExtent,
                     width() - m_rulerThickness,
                     scrollBarExtent);
        m_hScroll -> show();
        m_hScrollBarExtent = scrollBarExtent;
        m_hScrollBarExtent = scrollBarExtent;
    } else if(docW <= drawW) {
        // we need a vertical scrollbar only
        m_hScroll -> hide();
        m_hScroll -> setValue(0);
        m_vScroll -> setRange(0, docH - drawH);
        m_vScroll -> setGeometry(width() - scrollBarExtent, m_rulerThickness, scrollBarExtent, height()  - m_rulerThickness);
        m_vScroll -> show();
        m_vScrollBarExtent = scrollBarExtent;
    } else {
        // we need both scrollbars
        m_vScroll -> setRange(0, docH - drawH);
        m_vScroll -> setGeometry(width() - scrollBarExtent,
                    m_rulerThickness,
                    scrollBarExtent,
                    height() -2* m_rulerThickness);
        m_hScroll -> setRange(0, docW - drawW);
        m_hScroll -> setGeometry(m_rulerThickness,
                     height() - scrollBarExtent,
                     width() - 2*m_rulerThickness,
                     scrollBarExtent);
        m_vScroll -> show();
        m_hScroll -> show();
        m_vScrollBarExtent = scrollBarExtent;
        m_hScrollBarExtent = scrollBarExtent;
    }

    //Check if rulers are visible
    if( m_RulerAction->isChecked() )
        m_canvas -> setGeometry(m_rulerThickness, m_rulerThickness, drawW, drawH);
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
    if (m_canvas -> isOpenGLCanvas()) {
        paintOpenGLView(r);
    } else {
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

                        if(wr.isEmpty())
                            return;
                    }

                    if (!wr.isNull()) {
                        if (zoom() < 1.0 || zoom() > 1.0) {
                            gc.setViewport(0, 0, static_cast<Q_INT32>(m_canvasPixmap.width() * zoom()), static_cast<Q_INT32>(m_canvasPixmap.height() * zoom()));
                        }
                        gc.translate((-horzValue()) / zoom(), (-vertValue()) / zoom());

                        if(m_actLayerVis)
                            m_current -> renderToPainter(wr.left(), wr.top(),
                                wr.right(), wr.bottom(), gc, monitorProfile(),
                                (KisImage::PaintFlags)(
                                         KisImage::PAINT_BACKGROUND
                                        |KisImage::PAINT_SELECTION
                                        |KisImage::PAINT_MASKINACTIVELAYERS
                                        |KisImage::PAINT_EMBEDDED_RECT
                                                      ), HDRExposure());
                        else
                            m_current -> renderToPainter(wr.left(), wr.top(),
                                wr.right(), wr.bottom(), gc, monitorProfile(),
                                (KisImage::PaintFlags)(
                                         KisImage::PAINT_BACKGROUND
                                        |KisImage::PAINT_SELECTION
                                        |KisImage::PAINT_EMBEDDED_RECT
                                                      ), HDRExposure());
                    }

//                    paintGuides();
                }

                m_canvas -> update(vr.qRect());
            }
        } else {
            clearCanvas(r.qRect());
            m_canvas -> update(r.qRect());
        }
    }
}

void KisView::paintOpenGLView(const KisRect& r)
{
#ifdef HAVE_GL
    if (!m_canvas -> isUpdatesEnabled()) {
        return;
    }

    m_canvas -> OpenGLWidget() -> makeCurrent();

    glDrawBuffer(GL_BACK);

    QColor widgetBackgroundColor = eraseColor();

    glClearColor(widgetBackgroundColor.red() / 255.0, widgetBackgroundColor.green() / 255.0, widgetBackgroundColor.blue() / 255.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    KisImageSP img = currentImg();

    if (img) {

        KisRect vr = windowToView(r);
        vr &= KisRect(0, 0, m_canvas -> width(), m_canvas -> height());

        if (!vr.isNull()) {

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glViewport(0, 0, m_canvas -> width(), m_canvas -> height());
            glOrtho(0, m_canvas -> width(), m_canvas -> height(), 0, -1, 1);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glBindTexture(GL_TEXTURE_2D, m_OpenGLImageContext -> backgroundTexture());

            glEnable(GL_TEXTURE_2D);
            glBegin(GL_QUADS);

            glTexCoord2f(0.0, 0.0);
            glVertex2f(0.0, 0.0);

            glTexCoord2f((img -> width() * zoom()) / KisOpenGLImageContext::BACKGROUND_TEXTURE_WIDTH, 0.0);
            glVertex2f(img -> width() * zoom(), 0.0);

            glTexCoord2f((img -> width() * zoom()) / KisOpenGLImageContext::BACKGROUND_TEXTURE_WIDTH,
                         (img -> height() * zoom()) / KisOpenGLImageContext::BACKGROUND_TEXTURE_HEIGHT);
            glVertex2f(img -> width() * zoom(), img -> height() * zoom());

            glTexCoord2f(0.0, (img -> height() * zoom()) / KisOpenGLImageContext::BACKGROUND_TEXTURE_HEIGHT);
            glVertex2f(0.0, img -> height() * zoom());

            glEnd();

            glTranslatef(-m_scrollX, -m_scrollY, 0.0);
            glScalef(zoomFactor(), zoomFactor(), 1.0);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            QRect wr = viewToWindow(QRect(0, 0, m_canvas -> width(), m_canvas -> height()));
            wr &= QRect(0, 0, img -> width(), img -> height());

            m_OpenGLImageContext -> setHDRExposure(HDRExposure());

            m_canvas -> OpenGLWidget() -> makeCurrent();

            for (int x = (wr.left() / m_OpenGLImageContext -> imageTextureTileWidth()) * m_OpenGLImageContext -> imageTextureTileWidth();
                  x <= wr.right();
                  x += m_OpenGLImageContext -> imageTextureTileWidth()) {
                for (int y = (wr.top() / m_OpenGLImageContext -> imageTextureTileHeight()) * m_OpenGLImageContext -> imageTextureTileHeight();
                      y <= wr.bottom();
                      y += m_OpenGLImageContext -> imageTextureTileHeight()) {

                    glBindTexture(GL_TEXTURE_2D, m_OpenGLImageContext -> imageTextureTile(x, y));

                    glBegin(GL_QUADS);

                    glTexCoord2f(0.0, 0.0);
                    glVertex2f(x, y);

                    glTexCoord2f(1.0, 0.0);
                    glVertex2f(x + m_OpenGLImageContext -> imageTextureTileWidth(), y);

                    glTexCoord2f(1.0, 1.0);
                    glVertex2f(x + m_OpenGLImageContext -> imageTextureTileWidth(), y + m_OpenGLImageContext -> imageTextureTileHeight());

                    glTexCoord2f(0.0, 1.0);
                    glVertex2f(x, y + m_OpenGLImageContext -> imageTextureTileHeight());

                    glEnd();
                }
            }

            glDisable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);

            // Unbind the texture otherwise the ATI driver crashes when the canvas context is
            // made current after the textures are deleted following an image resize.
            glBindTexture(GL_TEXTURE_2D, 0);

            //paintGuides();

            m_canvas -> OpenGLWidget() -> swapBuffers();
        }
    }
#else
    Q_UNUSED(r);
#endif
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


KisCanvas *KisView::kiscanvas() const
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

    if (img) {
        layer = img -> activeLayer();
        nlayers = img -> nlayers();
        nvisible = nlayers - img -> nHiddenLayers();
    }
    enable = enable && img && layer && layer->visible() && !layer->locked();
    m_layerDup -> setEnabled(enable);
    m_layerRm -> setEnabled(enable);
    m_layerHide -> setEnabled(enable);
    m_layerProperties -> setEnabled(enable);
    m_layerSaveAs -> setEnabled(enable);
//    m_layerTransform -> setEnabled(enable);
    m_layerRaise -> setEnabled(enable && layer -> prevSibling());
    m_layerLower -> setEnabled(enable && layer -> nextSibling());
    m_layerTop -> setEnabled(enable && nlayers > 1 && layer != img -> rootLayer() -> firstChild());
    m_layerBottom -> setEnabled(enable && nlayers > 1 && layer != img -> rootLayer() -> lastChild());

    // XXX these should be named layer instead of img
    m_imgFlatten -> setEnabled(nlayers > 1);
    m_imgMergeVisible -> setEnabled(nvisible > 1);
    m_imgMergeLayer -> setEnabled(nlayers > 1 && layer && layer -> nextSibling());

    m_selectionManager->updateGUI();
    m_filterManager->updateGUI();
    m_toolManager->updateGUI();

    if (img -> activeDevice())
        emit currentColorSpaceChanged(img -> activeDevice() -> colorSpace());

    imgUpdateGUI();
}


void KisView::imgUpdateGUI()
{
    KisImageSP img = currentImg();

    m_imgResizeToLayer -> setEnabled(img && img -> activeLayer());

    updateStatusBarProfileLabel();
}

void KisView::zoomAroundPoint(double x, double y, double zf)
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
                KisPoint c = viewToWindow(KisPoint(m_canvas -> width() / 2.0, m_canvas -> height() / 2.0));
                x = c.x();
            }
            else {
                x = img -> width() / 2.0;
            }

            if (m_vScroll -> isVisible()) {
                KisPoint c = viewToWindow(KisPoint(m_canvas -> width() / 2.0, m_canvas -> height() / 2.0));
                y = c.y();
            }
            else {
                y = img -> height() / 2.0;
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
        double vcx = m_canvas -> width() / 2.0;
        Q_INT32 scrollX = qRound(x * zoom() - vcx);
        m_hScroll -> setValue(scrollX);
    }

    if (m_vScroll -> isVisible()) {
        double vcy = m_canvas -> height() / 2.0;
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

        zoomAroundPoint(r.center().x(), r.center().y(), zf);
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

    KisDlgImageProperties dlg(img, this);

    if (dlg.exec() == QDialog::Accepted) {
        if (dlg.imageWidth() != img -> width() ||
            dlg.imageHeight() != img -> height()) {

            resizeCurrentImage(dlg.imageWidth(),
                               dlg.imageHeight());
        }
        Q_INT32 opacity = dlg.opacity();
        opacity = opacity * 255 / 100;
        img -> setName(dlg.imageName());
        img -> setResolution(dlg.resolution(), dlg.resolution());
        img -> setDescription(dlg.description());
        img -> setProfile(dlg.profile());
    }
}

void KisView::slotInsertImageAsLayer()
{
    if (importImage() > 0)
        m_doc -> setModified(true);
}

void KisView::slotAddPalette()
{
    (new KisCustomPalette(0, "add palette", i18n("Add Palette"), this)) -> show();
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

    KisImageSP dst = new KisImage(d.undoAdapter(), r.width(), r.height(), img->colorSpace(), l->name());
    d.setCurrentImage( dst );
    dst->addLayer(l->clone(),dst->rootLayer(),0);

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
/*LAYERREMOVE
            vKisLayerSP v = img -> layers();
            KisImageSP current = currentImg();

            rc += v.size();
            current -> activeLayer() -> deselect();

            for (vKisLayerSP_it it = v.begin(); it != v.end(); ++it) {
                KisLayerSP layer = *it;

                layer -> setImage(current);
                layer -> setName(current -> nextLayerName());
                current -> addLayer(layer, current -> rootLayer().data(), current -> rootLayer().firstChild());
                m_layerBox->slotSetCurrentItem(img -> index(layer));
            }
*/
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
    KisPaintDeviceImplSP dev = currentImg() -> activeDevice();
    if (!dev) return;

    KisUndoAdapter * undo = 0;
    KisTransaction * t = 0;
    if ((undo = currentImg() -> undoAdapter())) {
        t = new KisTransaction(i18n("Mirror Layer X"), dev);
        Q_CHECK_PTR(t);
    }

    dev->mirrorX();

    if (undo) undo -> addCommand(t);

    m_doc -> setModified(true);
    layersUpdated();
    updateCanvas();
}

void KisView::mirrorLayerY()
{
    if (!currentImg()) return;
    KisPaintDeviceImplSP dev = currentImg() -> activeDevice();
    if (!dev) return;

    KisUndoAdapter * undo = 0;
    KisTransaction * t = 0;
    if ((undo = currentImg() -> undoAdapter())) {
        t = new KisTransaction(i18n("Mirror Layer Y"), dev);
        Q_CHECK_PTR(t);
    }

    dev->mirrorY();

    if (undo) undo -> addCommand(t);

    m_doc -> setModified(true);
    layersUpdated();
    updateCanvas();
}

void KisView::scaleLayer(double sx, double sy, KisFilterStrategy *filterStrategy)
{
    if (!currentImg()) return;

    KisPaintDeviceImplSP dev = currentImg() -> activeDevice();
    if (!dev) return;

    KisUndoAdapter * undo = 0;
    KisTransaction * t = 0;
    if ((undo = currentImg() -> undoAdapter())) {
        t = new KisTransaction(i18n("Scale Layer"), dev);
        Q_CHECK_PTR(t);
    }

    KisScaleWorker worker (dev, sx, sy, filterStrategy);
    worker.run();

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

    KisPaintDeviceImplSP dev = currentImg() -> activeDevice();
    if (!dev) return;

    KisUndoAdapter * undo = 0;
    KisTransaction * t = 0;
    if ((undo = currentImg() -> undoAdapter())) {
        t = new KisTransaction(i18n("Rotate Layer"), dev);
        Q_CHECK_PTR(t);
    }

    // Rotate XXX: LAYERREMOVE
    // dev -> rotate(angle, false, m_progress);

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

    KisPaintDeviceImplSP dev = currentImg() -> activeDevice();
    if (!dev) return;

    KisUndoAdapter * undo = 0;
    KisTransaction * t = 0;
    if ((undo = currentImg() -> undoAdapter())) {
        t = new KisTransaction(i18n("Shear layer"), dev);
        Q_CHECK_PTR(t);
    }

    // XXX LAYERREMOVE
    //dev -> shear(angleX, angleY, m_progress);

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
                                   i18n("&Flatten Image"),
                                   KStdGuiItem::cancel());

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
#ifdef HAVE_GL
    bool canvasWasOpenGL = m_canvas -> isOpenGLCanvas();
#endif

    if (PreferencesDialog::editPreferences())
    {
        KisConfig cfg;

        resetMonitorProfile();

#ifdef HAVE_GL
        if (cfg.useOpenGL() != canvasWasOpenGL) {

            disconnectCurrentImg();

            //XXX: Need to notify other views that this global setting has changed.
            if (cfg.useOpenGL()) {
                m_OpenGLImageContext = KisOpenGLImageContext::getImageContext(m_current, monitorProfile());
                m_canvas -> createOpenGLCanvas(m_OpenGLImageContext -> sharedContextWidget());
            } else
            {
                m_OpenGLImageContext = 0;
                m_canvas -> createQPaintDeviceCanvas();
            }

            connectCurrentImg();

            resizeEvent(0);
        }

        if (cfg.useOpenGL()) {
            m_OpenGLImageContext -> setMonitorProfile(monitorProfile());
        }
#endif

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

    opacity = int(float(opacity * 255) / 100 + 0.5);
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

void KisView::slotSetFGQColor(const QColor& c)
{
    KisColorSpace * monitorSpace = KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("RGBA"), m_monitorProfile);
    setFGColor(KisColor(c, monitorSpace));
    emit sigFGQColorChanged(c);
}

void KisView::slotSetBGQColor(const QColor& c)
{
    KisColorSpace * monitorSpace = KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("RGBA"), m_monitorProfile);
    setBGColor(KisColor(c, monitorSpace));
    emit sigBGQColorChanged(c);
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
    KisProfile *  printerProfile = KisMetaRegistry::instance()->csRegistry() ->getProfileByName(printerProfileName);

    if (printerProfile != 0)
        kdDebug(DBG_AREA_CMS) << "Printer profile: " << printerProfile -> productName() << "\n";

    QRect r = img -> bounds();
    img -> renderToPainter(r.x(), r.y(), r.width(), r.height(), gc, printerProfile, KisImage::PAINT_IMAGE_ONLY, HDRExposure());
}

void KisView::canvasGotPaintEvent(QPaintEvent *event)
{
    if (!m_canvas -> isOpenGLCanvas()) {
        Q_ASSERT(m_canvas -> QPaintDeviceWidget() != 0);

        QMemArray<QRect> rects = event -> region().rects();

        for (unsigned int i = 0; i < rects.count(); i++) {
            QRect er = rects[i];

            bitBlt(m_canvas -> QPaintDeviceWidget(), er.x(), er.y(), &m_canvasPixmap, er.x(), er.y(), er.width(), er.height());
        }
    } else {
        updateCanvas(event -> rect());
    }

    if (m_toolManager->currentTool()) {
        KisCanvasPainter gc(m_canvas);

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

//    if (img) {
//        QPoint pt = mapToScreen(e -> pos().floorQPoint());
//        KisGuideMgr *mgr = img -> guides();
//
//        m_lastGuidePoint = mapToScreen(e -> pos().floorQPoint());
//        m_currentGuide = 0;
//
//        if ((e -> state() & ~Qt::ShiftButton) == Qt::NoButton) {
//            KisGuideSP gd = mgr -> find(static_cast<Q_INT32>(pt.x() / zoom()), static_cast<Q_INT32>(pt.y() / zoom()), QMAX(2.0, 2.0 / zoom()));
//
//            if (gd) {
//                m_currentGuide = gd;
//
//                if ((e -> button() == Qt::RightButton) || ((e -> button() & Qt::ShiftButton) == Qt::ShiftButton)) {
//                    if (gd -> isSelected())
//                        mgr -> unselect(gd);
//                    else
//                        mgr -> select(gd);
//              } else {
//                    if (!gd -> isSelected()) {
//                        mgr -> unselectAll();
//                        mgr -> select(gd);
//                    }
//                }
//
//                updateGuides();
//                return;
//            }
//        }
//    }

    if (e -> device() == currentInputDevice() && m_toolManager->currentTool()) {
        KisPoint p = viewToWindow(e -> pos());
        // somewhat of a hack: we should actually test if we intersect with the scrollers,
        // but the globalPos seems to be off by a few pixels
        if (m_vScroll -> draggingSlider() || m_hScroll -> draggingSlider())
            return;

        KisButtonPressEvent ev(e -> device(), p, e -> globalPos(), e -> pressure(), e -> xTilt(), e -> yTilt(), e -> button(), e -> state());

        enableAutoScroll();
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

#if 0
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
    } else
#endif
    if (e -> device() == currentInputDevice() && m_toolManager->currentTool()) {
        KisMoveEvent ev(e -> device(), wp, e -> globalPos(), e -> pressure(), e -> xTilt(), e -> yTilt(), e -> state());

        m_toolManager->currentTool() -> move(&ev);
    }

//    m_lastGuidePoint = mapToScreen(e -> pos().floorQPoint());
    emit cursorPosition(wp.floorX(), wp.floorY());
}

int KisView::leftBorder() const
{
  return m_rulerThickness;
}

int KisView::rightBorder() const
{
  return m_hScrollBarExtent;
}

int KisView::topBorder() const
{
  return m_rulerThickness;
}

int KisView::bottomBorder() const
{
  return m_vScrollBarExtent;
}

void KisView::mouseMoveEvent(QMouseEvent *e)
{
    KisMoveEvent ke(currentInputDevice(), e -> pos(), e -> globalPos(), PRESSURE_DEFAULT, 0, 0, e -> state());
    canvasGotMoveEvent(&ke);
}

void KisView::slotAutoScroll(const QPoint &p)
{
    scrollTo(horzValue()+p.x(), vertValue()+p.y());
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

//    if (img && m_currentGuide) {
//        m_currentGuide = 0;
//    } else
    if (e -> device() == currentInputDevice() && m_toolManager->currentTool()) {
        KisPoint p = viewToWindow(e -> pos());
        KisButtonReleaseEvent ev(e -> device(), p, e -> globalPos(), e -> pressure(), e -> xTilt(), e -> yTilt(), e -> button(), e -> state());

        disableAutoScroll();
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

void KisView::layerProperties()
{
    if (currentImg() && currentImg() -> activeLayer())
        showLayerProperties(currentImg() -> activeLayer());
}

void KisView::showLayerProperties(KisLayerSP layer)
{
    KisDlgLayerProperties dlg(layer -> name(),
                              layer -> opacity(),
                              layer -> compositeOp(),
                              layer -> image() ->colorSpace()); // LAYERREMOVE used to be cs of layer

    if (dlg.exec() == QDialog::Accepted)
    {
        if (layer -> name() != dlg.getName() ||
            layer -> opacity() != dlg.getOpacity() ||
            layer -> compositeOp() != dlg.getCompositeOp())
        {
            m_adapter -> beginMacro(i18n("Property changes"));
            layer -> image() -> setLayerProperties(layer, dlg.getOpacity(), dlg.getCompositeOp(), dlg.getName());
            m_adapter -> endMacro();
        }
    }
}

void KisView::layerAdd()
{
    KisImageSP img = currentImg();
    if (img && img -> activeLayer())
        addLayer(img -> activeLayer() -> parent(), img -> activeLayer());
    else if (img)
        addLayer(static_cast<KisGroupLayer*>(img -> rootLayer().data()), 0);
}

void KisView::addLayer(KisGroupLayerSP parent, KisLayerSP above)
{
    KisImageSP img = currentImg();
    if (img) {
        KisConfig cfg;
        NewLayerDialog dlg(img->colorSpace()->id(), img->nextLayerName(), this);

        if (dlg.exec() == QDialog::Accepted) {
            KisColorSpace* cs = KisMetaRegistry::instance() ->  csRegistry() ->
                    getColorSpace(dlg.colorSpaceID(),"");
            KisLayerSP layer = new KisPaintLayer(img, dlg.layerName(), dlg.opacity(), cs);
            if (layer) {
                layer->setCompositeOp(dlg.compositeOp());
                img->addLayer(layer, parent.data(), above);
                resizeEvent(0);
                updateCanvas(0, 0, img -> width(), img -> height());
            } else {
                KMessageBox::error(this, i18n("Could not add layer to image."), i18n("Layer Error"));
            }
        }
    }
}

void KisView::addGroupLayer(KisGroupLayerSP parent, KisLayerSP above)
{
    KisImageSP img = currentImg();
    if (img) {
        KisConfig cfg;
        NewLayerDialog dlg(img->colorSpace()->id(), img->nextLayerName(), this);

        if (dlg.exec() == QDialog::Accepted) {
            KisLayerSP layer = new KisGroupLayer(img, dlg.layerName(), dlg.opacity());
            if (layer) {
                layer->setCompositeOp(dlg.compositeOp());
                img->addLayer(layer, parent.data(), above);
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

    if ( !doc->showEmbedInitDialog(this) )
        return;

    kdDebug() << "AddPartLayer: KoDocument is " << doc << endl;
    //img->bounds()
    KisChildDoc * childDoc = m_doc->createChildDoc(QRect(0,0,255,255), doc);
    kdDebug() << "AddPartLayer: KisChildDoc is " << childDoc << endl;

    KisPartLayer* partLayer = new KisPartLayer(img, childDoc);
    img -> addLayer(partLayer, img -> rootLayer().data(), img -> rootLayer() -> firstChild());

    m_doc->setModified(true);
}

void KisView::slotChildActivated(bool a) {
    // It should be so that the only part (child) we can activate, is the current layer:
    if (currentImg() && currentImg() -> activeLayer())
    {
        if (a) {
            currentImg() -> activeLayer() -> activate();
        } else {
            currentImg() -> activeLayer() -> deactivate();
        }
    }

    super::slotChildActivated(a);
}

void KisView::layerRemove()
{
    KisImageSP img = currentImg();

    if (img) {
        KisLayerSP layer = img -> activeLayer();

        if (layer) {
            //Q_INT32 n = img -> index(layer);

            img->removeLayer(layer);
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

    KisLayerSP dup = active->clone();
    dup -> setName(QString(i18n("Duplicate of '%1'")).arg(active -> name()));
    img->addLayer(dup, active->parent().data(), active);

    if (dup) {
        //        m_layerBox->slotSetCurrentItem(img -> index(layer)); // LAYERREMOVE
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

    img -> raiseLayer(layer);
}

void KisView::layerLower()
{
    KisImageSP img = currentImg();
    KisLayerSP layer;

    if (!img)
        return;

    layer = img -> activeLayer();

    img -> lowerLayer(layer);
}

void KisView::layerFront()
{
    KisImageSP img = currentImg();
    KisLayerSP layer;

    if (!img)
        return;

    layer = img -> activeLayer();
    img -> toTop(layer);
}

void KisView::layerBack()
{
    KisImageSP img = currentImg();
    if (!img) return;

    KisLayerSP layer;

    layer = img -> activeLayer();
    img -> toBottom(layer);
}

void KisView::layersUpdated()
{
    KisImageSP img = currentImg();
    if (!img) return;

    KisLayerSP layer = img -> activeLayer();

    layerUpdateGUI(img && layer);

    m_layerBox -> updateAll();
    img->notify();
    notifyObservers();
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

void KisView::actLayerVisChanged(int show)
{
    m_actLayerVis = (show != 0);
}

void KisView::scrollH(int value)
{
    m_hRuler -> updateVisibleArea(value, 0);

    int xShift = m_scrollX - value;
    m_scrollX = value;

    if (m_canvas -> isUpdatesEnabled()) {
        if (xShift > 0) {
            if (!m_canvas -> isOpenGLCanvas()) {
                bitBlt(&m_canvasPixmap, xShift, 0, &m_canvasPixmap, 0, 0, m_canvasPixmap.width() - xShift, m_canvasPixmap.height());

                KisRect drawRect(0, 0, xShift, m_canvasPixmap.height());
                paintView(viewToWindow(drawRect));
            }
            m_canvas -> repaint();
        }
        else
            if (xShift < 0) {
                if (!m_canvas -> isOpenGLCanvas()) {
                    bitBlt(&m_canvasPixmap, 0, 0, &m_canvasPixmap, -xShift, 0, m_canvasPixmap.width() + xShift, m_canvasPixmap.height());

                    KisRect drawRect(m_canvasPixmap.width() + xShift, 0, -xShift, m_canvasPixmap.height());
                    paintView(viewToWindow(drawRect));
                }
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
            if (!m_canvas -> isOpenGLCanvas()) {
                bitBlt(&m_canvasPixmap, 0, yShift, &m_canvasPixmap, 0, 0, m_canvasPixmap.width(), m_canvasPixmap.height() - yShift);

                KisRect drawRect(0, 0, m_canvasPixmap.width(), yShift);
                paintView(viewToWindow(drawRect));
            }
            m_canvas -> repaint();
        } else {
            if (yShift < 0) {
                if (!m_canvas -> isOpenGLCanvas()) {
                    bitBlt(&m_canvasPixmap, 0, 0, &m_canvasPixmap, 0, -yShift, m_canvasPixmap.width(), m_canvasPixmap.height() + yShift);

                    KisRect drawRect(0, m_canvasPixmap.height() + yShift, m_canvasPixmap.width(), -yShift);
                    paintView(viewToWindow(drawRect));
                }
                m_canvas -> repaint();
            }
        }
    }
}


void KisView::setupCanvas()
{
    m_canvas = new KisCanvas(this, "kis_canvas");
    m_canvas -> setFocusPolicy( QWidget::StrongFocus );
    QObject::connect(m_canvas, SIGNAL(sigGotButtonPressEvent(KisButtonPressEvent*)), this, SLOT(canvasGotButtonPressEvent(KisButtonPressEvent*)));
    QObject::connect(m_canvas, SIGNAL(sigGotButtonReleaseEvent(KisButtonReleaseEvent*)), this, SLOT(canvasGotButtonReleaseEvent(KisButtonReleaseEvent*)));
    QObject::connect(m_canvas, SIGNAL(sigGotDoubleClickEvent(KisDoubleClickEvent*)), this, SLOT(canvasGotDoubleClickEvent(KisDoubleClickEvent*)));
    QObject::connect(m_canvas, SIGNAL(sigGotMoveEvent(KisMoveEvent*)), this, SLOT(canvasGotMoveEvent(KisMoveEvent*)));
    QObject::connect(m_canvas, SIGNAL(sigGotPaintEvent(QPaintEvent*)), this, SLOT(canvasGotPaintEvent(QPaintEvent*)));
    QObject::connect(m_canvas, SIGNAL(sigGotEnterEvent(QEvent*)), this, SLOT(canvasGotEnterEvent(QEvent*)));
    QObject::connect(m_canvas, SIGNAL(sigGotLeaveEvent(QEvent*)), this, SLOT(canvasGotLeaveEvent(QEvent*)));
    QObject::connect(m_canvas, SIGNAL(sigGotMouseWheelEvent(QWheelEvent*)), this, SLOT(canvasGotMouseWheelEvent(QWheelEvent*)));
    QObject::connect(m_canvas, SIGNAL(sigGotKeyPressEvent(QKeyEvent*)), this, SLOT(canvasGotKeyPressEvent(QKeyEvent*)));
    QObject::connect(m_canvas, SIGNAL(sigGotKeyReleaseEvent(QKeyEvent*)), this, SLOT(canvasGotKeyReleaseEvent(QKeyEvent*)));
    QObject::connect(m_canvas, SIGNAL(sigGotDragEnterEvent(QDragEnterEvent*)), this, SLOT(canvasGotDragEnterEvent(QDragEnterEvent*)));
    QObject::connect(m_canvas, SIGNAL(sigGotDropEvent(QDropEvent*)), this, SLOT(canvasGotDropEvent(QDropEvent*)));
}

void KisView::connectCurrentImg()
{
    if (m_current) {
        connect(m_current, SIGNAL(sigActiveSelectionChanged(KisImageSP)), m_selectionManager, SLOT(imgSelectionChanged(KisImageSP)));

        connect(m_current, SIGNAL(sigProfileChanged(KisProfile * )), SLOT(profileChanged(KisProfile * )));

        connect(m_current, SIGNAL(sigLayersChanged()), SLOT(layersUpdated()));
        connect(m_current, SIGNAL(sigLayerAdded(KisLayerSP)), SLOT(layersUpdated()));
        connect(m_current, SIGNAL(sigLayerRemoved(KisLayerSP, KisGroupLayerSP, KisLayerSP)), SLOT(layersUpdated()));
        connect(m_current, SIGNAL(sigLayerMoved(KisLayerSP, KisGroupLayerSP, KisLayerSP)), SLOT(layersUpdated()));
        connect(m_current, SIGNAL(sigLayerActivated(KisLayerSP)), SLOT(layersUpdated()));
        connect(m_current, SIGNAL(sigLayerPropertiesChanged(KisLayerSP)), SLOT(layersUpdated()));

#ifdef HAVE_GL
        if (m_OpenGLImageContext != 0) {
            connect(m_OpenGLImageContext, SIGNAL(sigImageUpdated(const QRect&)),
                    SLOT(imgUpdated(const QRect&)));
            connect(m_OpenGLImageContext, SIGNAL(sigSizeChanged(Q_INT32, Q_INT32)),
                    SLOT(slotImageSizeChanged(Q_INT32, Q_INT32)));
        } else 
#endif
        {
            connect(m_current, SIGNAL(sigImageUpdated(const QRect&)),
                    SLOT(imgUpdated(const QRect&)));
            connect(m_current, SIGNAL(sigSizeChanged(Q_INT32, Q_INT32)),
                    SLOT(slotImageSizeChanged(Q_INT32, Q_INT32)));
        }
    }

    m_layerBox -> setImage(m_current);
}

void KisView::disconnectCurrentImg()
{
    if (m_current) {
        m_current -> disconnect(this);
        m_layerBox -> setImage(0);
    }

#ifdef HAVE_GL
    if (m_OpenGLImageContext != 0) {
        m_OpenGLImageContext -> disconnect(this);
    }
#endif
}

void KisView::imgUpdated(const QRect& rc)
{
    updateCanvas(rc);
}

void KisView::profileChanged(KisProfile *  /*profile*/)
{
    updateStatusBarProfileLabel();
}

void KisView::slotImageSizeChanged(Q_INT32 /*w*/, Q_INT32 /*h*/)
{
    resizeEvent(0);
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

#if 0
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
#endif

    return super::eventFilter(o, e);
}

#if 0
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
#endif

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
        //setCurrentImage(m_doc -> currentImage());
        //m_current = m_doc -> currentImage();
        //m_current -> notify();
        //connectCurrentImg();
    }

    return m_current;
}

void KisView::setCurrentImage(KisImageSP image)
{
    disconnectCurrentImg();
    m_current = image;

    KisConfig cfg;

#ifdef HAVE_GL
    if (cfg.useOpenGL()) {
        m_OpenGLImageContext = KisOpenGLImageContext::getImageContext(image, monitorProfile());
        m_canvas -> createOpenGLCanvas(m_OpenGLImageContext -> sharedContextWidget());
    }
#endif
    connectCurrentImg();
    m_current -> notify();

    zoomAroundPoint(0, 0, 1.0);
    resizeEvent(0);
    updateCanvas();

    if (!currentImg())
        layersUpdated();

    imgUpdateGUI();
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
        notifyObservers();
        updateCanvas();
    }
}

void KisView::createDockers()
{

    m_birdEyeBox = new KisBirdEyeBox(this);
    m_birdEyeBox -> setCaption(i18n("Overview"));
    m_paletteManager->addWidget( m_birdEyeBox, "birdeyebox", krita::CONTROL_PALETTE);

    m_hsvwidget = new KoHSVWidget(this, "hsv");
    m_hsvwidget -> setCaption(i18n("HSV"));

    connect(m_hsvwidget, SIGNAL(sigFgColorChanged(const QColor &)), this, SLOT(slotSetFGQColor(const QColor &)));
    connect(m_hsvwidget, SIGNAL(sigBgColorChanged(const QColor &)), this, SLOT(slotSetBGQColor(const QColor &)));
    connect(this, SIGNAL(sigFGQColorChanged(const QColor &)), m_hsvwidget, SLOT(setFgColor(const QColor &)));
    connect(this, SIGNAL(sigBGQColorChanged(const QColor &)), m_hsvwidget, SLOT(setBgColor(const QColor &)));
    m_paletteManager->addWidget( m_hsvwidget, "hsvwidget", krita::COLORBOX);
    
    m_rgbwidget = new KoRGBWidget(this, "rgb");
    m_rgbwidget -> setCaption(i18n("RGB"));
    connect(m_rgbwidget, SIGNAL(sigFgColorChanged(const QColor &)), this, SLOT(slotSetFGQColor(const QColor &)));
    connect(m_rgbwidget, SIGNAL(sigBgColorChanged(const QColor &)), this, SLOT(slotSetBGQColor(const QColor &)));
    connect(this, SIGNAL(sigFGQColorChanged(const QColor &)), m_rgbwidget, SLOT(setFgColor(const QColor &)));
    connect(this, SIGNAL(sigBGQColorChanged(const QColor &)), m_rgbwidget, SLOT(setBgColor(const QColor &)));
    m_paletteManager->addWidget( m_rgbwidget, "rgbwidget", krita::COLORBOX);
    
    m_graywidget = new KoGrayWidget(this, "gray");
    m_graywidget -> setCaption(i18n("Gray"));
    connect(m_graywidget, SIGNAL(sigFgColorChanged(const QColor &)), this, SLOT(slotSetFGQColor(const QColor &)));
    connect(m_graywidget, SIGNAL(sigBgColorChanged(const QColor &)), this, SLOT(slotSetBGQColor(const QColor &)));
    connect(this, SIGNAL(sigFGQColorChanged(const QColor &)), m_graywidget, SLOT(setFgColor(const QColor &)));
    connect(this, SIGNAL(sigBGQColorChanged(const QColor &)), m_graywidget, SLOT(setBgColor(const QColor &)));
    m_paletteManager->addWidget( m_graywidget, "graywidget", krita::COLORBOX);

    m_palettewidget = new KisPaletteWidget(this);
    m_palettewidget -> setCaption(i18n("Palettes"));

    KisResourceServerBase* rServer;
    rServer = KisResourceServerRegistry::instance() -> get("PaletteServer");
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


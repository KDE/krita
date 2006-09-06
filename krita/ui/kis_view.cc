/* This file is part of KimageShop^WKrayon^WKrita
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
#include <qobjectlist.h>

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
#include <kparts/event.h>

// KOffice
#include <KoPartSelectAction.h>
#include <KoFilterManager.h>
#include <KoMainWindow.h>
#include <KoView.h>
#include <KoTabBar.h>
#include <ko_gray_widget.h>
#include <ko_hsv_widget.h>
#include <ko_rgb_widget.h>
#include <kopalettemanager.h>
#include <kopalette.h>

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
#include "kis_filter_strategy.h"
#include "kis_gradient.h"
#include "kis_group_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_paint_device.h"
#include "kis_tool_freehand.h"
//#include "kis_guide.h"

#include "kis_layerbox.h"

#include "kis_layer.h"
#include "kis_paint_layer.h"
#include "kis_move_event.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_paintop_registry.h"
#include "kis_part_layer.h"
#include "kis_part_layer_handler.h"
#include "kis_pattern.h"
#include "kis_profile.h"
#include "kis_rect.h"
#include "kis_resource.h"
#include "kis_palette.h"
#include "kis_ruler.h"
#include "kis_selection.h"
#include "KoToolBox.h"
#include "kis_tool.h"
#include "kis_tool_manager.h"
#include "kis_transaction.h"
#include "kis_types.h"
#include "kis_undo_adapter.h"
#include "kis_view.h"
#include "kis_view_iface.h"
#include "kis_label_progress.h"
#include "kis_opengl_image_context.h"
#include "kis_background.h"
#include "kis_paint_device_action.h"
#include "kis_filter_configuration.h"
#include "kis_transform_worker.h"
#include "kis_shear_visitor.h"

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
#include "kis_dlg_preferences.h"
#include "kis_dlg_image_properties.h"
#include "kis_dlg_adjustment_layer.h"
#include "kis_dlg_adj_layer_props.h"

// Action managers
#include "kis_selection_manager.h"
#include "kis_filter_manager.h"
#include "kis_grid_manager.h"
#include "kis_perspective_grid_manager.h"

#include "kis_custom_palette.h"
#include "wdgpalettechooser.h"

#include <fixx11h.h>

// Time in ms that must pass after a tablet event before a mouse event is allowed to
// change the input device to the mouse. This is needed because mouse events are always
// sent to a receiver if it does not accept the tablet event.
#define MOUSE_CHANGE_EVENT_DELAY 100

KisView::KisView(KisDoc *doc, KisUndoAdapter *adapter, QWidget *parent, const char *name)
    : super(doc, parent, name)
    , KXMLGUIBuilder( shell() )
    , m_panning( false )
    , m_oldTool( 0 )
    , m_doc( doc )
    , m_canvas( 0 )
    , m_popup( 0 )
    , m_partHandler( 0 )
    , m_gridManager( 0 )
    , m_perspectiveGridManager( 0 )
    , m_selectionManager( 0 )
    , m_filterManager( 0 )
    , m_paletteManager( 0 )
    , m_toolManager( 0 )
    , m_actLayerVis( false )
    , m_hRuler( 0 )
    , m_vRuler( 0 )
    , m_imgFlatten( 0 )
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
    , m_fitToCanvas( 0 )
    , m_fullScreen( 0 )
    , m_imgProperties( 0 )
    , m_RulerAction( 0 )
    , m_guideAction( 0 )
    , m_dcop( 0 )
    , m_hScroll( 0 )
    , m_vScroll( 0 )
    , m_scrollX( 0 )
    , m_scrollY( 0 )
    , m_canvasXOffset( 0)
    , m_canvasYOffset( 0)
    , m_paintViewEnabled( false )
    , m_guiActivateEventReceived( false )
    , m_showEventReceived( false )
    , m_imageLoaded( false )
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
    , m_toolIsPainting( false )
    , m_monitorProfile( 0 )
    , m_HDRExposure( 0 )
{

    Q_ASSERT(doc);
    Q_ASSERT(adapter);
    Q_ASSERT(parent);

    KisConfig cfg;

    m_currentColorChooserDisplay = KisID("BLA");
    setFocusPolicy( QWidget::StrongFocus );

    // Must come before input devices are referenced as this detects them.
#ifdef Q_WS_X11
    KisCanvasWidget::initX11Support();
#endif
    // Install event filter before we create any child widgets so they can see
    // the tablet events.
    qApp->installEventFilter(this);

    m_tabletEventTimer.start();
    m_inputDevice = KisInputDevice::mouse();

    connect(&m_initialZoomTimer, SIGNAL(timeout()), SLOT(slotInitialZoomTimeout()));

    m_paletteManager = new KoPaletteManager(this, actionCollection(), "Krita palette manager");
    if (cfg.fixDockerWidth()) m_paletteManager->setFixedWidth( 360 );

    m_paletteManager->createPalette( krita::CONTROL_PALETTE, i18n("Control box"));
    m_paletteManager->createPalette( krita::COLORBOX, i18n("Colors"));
    m_paletteManager->createPalette( krita::LAYERBOX, i18n("Layers"));

    m_selectionManager = new KisSelectionManager(this, doc);
    m_filterManager = new KisFilterManager(this, doc);
    m_toolManager = new KisToolManager(canvasSubject(), getCanvasController());
    m_gridManager = new KisGridManager(this);
    m_perspectiveGridManager = new KisPerspectiveGridManager(this);

    // This needs to be set before the dockers are created.
    m_image = m_doc->currentImage();
    KisColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getRGB8();
    m_fg = KisColor(Qt::black, cs);
    m_bg = KisColor(Qt::white, cs);

    createDockers();

    setInstance(KisFactory::instance(), false);
    setClientBuilder( this );

    if (!doc->isReadWrite())
        setXMLFile("krita_readonly.rc");
    else
        setXMLFile("krita.rc");

    KStdAction::keyBindings( mainWindow()->guiFactory(), SLOT( configureShortcuts() ), actionCollection() );

    createLayerBox();

    setupCanvas();
    m_canvas->hide();
    setupRulers();
    setupScrollBars();
    setupStatusBar();

    setupActions();
    dcopObject();


    connect(this, SIGNAL(autoScroll(const QPoint &)), SLOT(slotAutoScroll(const QPoint &)));

    setMouseTracking(true);

    resetMonitorProfile();

    layersUpdated();

    m_brushesAndStuffToolBar = new KisControlFrame(mainWindow(), this);

    // Load all plugins
    KTrader::OfferList offers = KTrader::self()->query(QString::fromLatin1("Krita/ViewPlugin"),
                                                         QString::fromLatin1("(Type == 'Service') and "
                                                                             "([X-Krita-Version] == 2)"));
    KTrader::OfferList::ConstIterator iter;
    for(iter = offers.begin(); iter != offers.end(); ++iter)
    {
        KService::Ptr service = *iter;
        int errCode = 0;
        KParts::Plugin* plugin =
             KParts::ComponentFactory::createInstanceFromService<KParts::Plugin> ( service, this, 0, QStringList(), &errCode);
        if ( plugin ) {
            kdDebug(41006) << "found plugin " << service->property("Name").toString() << "\n";
            insertChildClient(plugin);
        }
        else {
            kdDebug(41006) << "found plugin " << service->property("Name").toString() << ", " << errCode << "\n";
	    if( errCode == KParts::ComponentFactory::ErrNoLibrary)
	    {
		kdWarning(41006) << " Error loading plugin was : ErrNoLibrary " << KLibLoader::self()->lastErrorMessage() << endl;
	    }
        }
    }

    if(!doc->isLoading())
    {
        slotLoadingFinished();
    } else {
        connect(doc, SIGNAL(loadingFinished()), this, SLOT(slotLoadingFinished()));
    }

    setFocus();
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
        m_toolBox->setLabel(i18n("Krita"));
        m_toolManager->setUp(m_toolBox, m_paletteManager, actionCollection());

        Dock dock = stringToDock( element.attribute( "position" ).lower() );

        mainWindow()->addDockWindow( m_toolBox, dock, false);
        mainWindow()->moveDockWindow( m_toolBox, dock, false, 0, 0 );
    }

    return KXMLGUIBuilder::createContainer( parent, index, element, id );

}

void KisView::removeContainer( QWidget *container, QWidget *parent, QDomElement &element, int id )
{
    Q_ASSERT(container);

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
    m_layerBox->setCaption(i18n("Layers"));

    connect(m_layerBox, SIGNAL(sigRequestLayer(KisGroupLayerSP, KisLayerSP)),
            this, SLOT(addLayer(KisGroupLayerSP, KisLayerSP)));
    connect(m_layerBox, SIGNAL(sigRequestGroupLayer(KisGroupLayerSP, KisLayerSP)),
            this, SLOT(addGroupLayer(KisGroupLayerSP, KisLayerSP)));
    connect(m_layerBox, SIGNAL(sigRequestAdjustmentLayer(KisGroupLayerSP, KisLayerSP)),
            this, SLOT(addAdjustmentLayer(KisGroupLayerSP, KisLayerSP)));
    connect(m_layerBox, SIGNAL(sigRequestPartLayer(KisGroupLayerSP, KisLayerSP, const KoDocumentEntry&)),
            this, SLOT(addPartLayer(KisGroupLayerSP, KisLayerSP, const KoDocumentEntry&)));
    connect(m_layerBox, SIGNAL(sigRequestLayerProperties(KisLayerSP)),
            this, SLOT(showLayerProperties(KisLayerSP)));
    connect(m_layerBox, SIGNAL(sigOpacityChanged(int, bool)), this, SLOT(layerOpacity(int, bool)));
    connect(m_layerBox, SIGNAL(sigOpacityFinishedChanging(int, int)),
            this, SLOT(layerOpacityFinishedChanging(int, int)));
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

    m_vScroll->setGeometry(width() - 16, 20, 16, height() - 36);
    m_hScroll->setGeometry(20, height() - 16, width() - 36, 16);
    m_hScroll->setValue(0);
    m_vScroll->setValue(0);
    QObject::connect(m_vScroll, SIGNAL(valueChanged(int)), this, SLOT(scrollV(int)));
    QObject::connect(m_hScroll, SIGNAL(valueChanged(int)), this, SLOT(scrollH(int)));
}

void KisView::setupRulers()
{
    m_hRuler = new KisRuler(Qt::Horizontal, this);
    Q_CHECK_PTR(m_hRuler);

    m_vRuler = new KisRuler(Qt::Vertical, this);
    Q_CHECK_PTR(m_vRuler);

    m_hRuler->setGeometry(20, 0, width() - 20, 20);
    m_vRuler->setGeometry(0, 20, 20, height() - 20);

    if (statusBar()) {
        m_hRuler->installEventFilter(this);
        m_vRuler->installEventFilter(this);
    }
}

#define EPSILON 1e-6

void KisView::updateStatusBarZoomLabel ()
{
    if (zoom() < 1 - EPSILON) {
        m_statusBarZoomLabel->setText(i18n("Zoom %1%").arg(zoom() * 100, 0, 'g', 4));
    } else {
        m_statusBarZoomLabel->setText(i18n("Zoom %1%").arg(zoom() * 100, 0, 'f', 0));
    }
    m_statusBarZoomLabel->setMaximumWidth(m_statusBarZoomLabel->fontMetrics().width(i18n("Zoom %1%").arg("0.8888  ")));
}

void KisView::updateStatusBarSelectionLabel()
{
    if (m_statusBarSelectionLabel == 0) {
        return;
    }

    KisImageSP img = currentImg();
    if (img) {
        KisPaintDeviceSP dev = img->activeDevice();
        if (dev) {
            if (dev->hasSelection()) {
                QRect r = dev->selection()->selectedExactRect();
                m_statusBarSelectionLabel->setText( i18n("Selection Active: x = %1 y = %2 width = %3 height = %4").arg(r.x()).arg(r.y()).arg( r.width()).arg( r.height()));
                return;
            }
        }
    }

    m_statusBarSelectionLabel->setText(i18n("No Selection"));
}

void KisView::updateStatusBarProfileLabel()
{
    if (m_statusBarProfileLabel == 0) {
        return;
    }

    KisImageSP img = currentImg();
    if (!img) return;

    if (img->getProfile() == 0) {
        m_statusBarProfileLabel->setText(i18n("No profile"));
    }
    else {
        m_statusBarProfileLabel->setText(img->colorSpace()->id().name() + "  " + img->getProfile()->productName());
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
        m_statusBarZoomLabel = new QLabel(sb);
        addStatusBarItem(m_statusBarZoomLabel,1);
        updateStatusBarZoomLabel();

        m_statusBarSelectionLabel = new KSqueezedTextLabel(sb);
        addStatusBarItem(m_statusBarSelectionLabel,2);
        updateStatusBarSelectionLabel();

        m_statusBarProfileLabel = new KSqueezedTextLabel(sb);
        addStatusBarItem(m_statusBarProfileLabel,3);
        updateStatusBarProfileLabel();

        //int height = m_statusBarProfileLabel->height();

        m_progress = new KisLabelProgress(this);
        m_progress->setMaximumWidth(225);
        m_progress->setMinimumWidth(225);
        m_progress->setMaximumHeight(fontMetrics().height() );
        addStatusBarItem(m_progress, 2, true);

        m_progress->hide();
    }
}

void KisView::setupActions()
{
    KisConfig cfg;

    m_selectionManager->setup(actionCollection());
    m_filterManager->setup(actionCollection());
    m_gridManager->setup(actionCollection());
    m_perspectiveGridManager->setup(actionCollection());


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
    m_fitToCanvas = KStdAction::fitToPage(this, SLOT(slotFitToCanvas()), actionCollection(), "fit_to_canvas");

    // layer actions
    m_layerAdd = new KAction(i18n("&Add..."), "Ctrl+Shift+N", this, SLOT(layerAdd()), actionCollection(), "insert_layer");

    m_actionPartLayer = new KoPartSelectAction( i18n( "&Object Layer" ), "frame_query",
                                                    this, SLOT( addPartLayer() ),
                                                    actionCollection(), "insert_part_layer" );


    m_actionAdjustmentLayer = new KAction( i18n( "&Adjustment Layer" ), 0,
            this, SLOT( addAdjustmentLayer() ),
            actionCollection(), "insert_adjustment_layer" );


    m_layerRm = new KAction(i18n("&Remove"), 0, this, SLOT(layerRemove()), actionCollection(), "remove_layer");
    m_layerDup = new KAction(i18n("Duplicate"), 0, this, SLOT(layerDuplicate()), actionCollection(), "duplicate_layer");
    m_layerHide = new KAction(i18n("&Hide/Show"), 0, this, SLOT(layerToggleVisible()), actionCollection(), "hide_layer");
    m_layerRaise = new KAction(i18n("Raise"), "raise", "Ctrl+]", this, SLOT(layerRaise()), actionCollection(), "raiselayer");
    m_layerLower = new KAction(i18n("Lower"), "lower", "Ctrl+[", this, SLOT(layerLower()), actionCollection(), "lowerlayer");
    m_layerTop = new KAction(i18n("To Top"), "bring_forward", "Ctrl+Shift+]", this, SLOT(layerFront()), actionCollection(), "toplayer");
    m_layerBottom = new KAction(i18n("To Bottom"), "send_backward", "Ctrl+Shift+[", this, SLOT(layerBack()), actionCollection(), "bottomlayer");
    m_layerProperties = new KAction(i18n("Properties"), 0, this, SLOT(layerProperties()), actionCollection(), "layer_properties");
    (void)new KAction(i18n("I&nsert Image as Layer..."), 0, this, SLOT(slotInsertImageAsLayer()), actionCollection(), "insert_image_as_layer");
    m_layerSaveAs = new KAction(i18n("Save Layer as Image..."), "filesave", this, SLOT(saveLayerAsImage()), actionCollection(), "save_layer_as_image");
    (void)new KAction(i18n("Flip on &X Axis"), "view_left_right", 0, this, SLOT(mirrorLayerX()), actionCollection(), "mirrorLayerX");
    (void)new KAction(i18n("Flip on &Y Axis"), "view_top_bottom", 0, this, SLOT(mirrorLayerY()), actionCollection(), "mirrorLayerY");

    m_createMask = new KAction(i18n("Create Mask"), 0, this,
                               SLOT(slotCreateMask()), actionCollection(), "create_mask");
    m_maskFromSelection = new KAction(i18n("Mask From Selection"), 0, this,
                                      SLOT(slotMaskFromSelection()), actionCollection(),
                                      "mask_fromsel");
    m_maskToSelection = new KAction(i18n("Mask to Selection"), 0, this,
                               SLOT(slotMaskToSelection()), actionCollection(), "mask_tosel");
    m_applyMask = new KAction(i18n("Apply Mask"), 0, this, SLOT(slotApplyMask()),
                              actionCollection(), "apply_mask");
    m_removeMask = new KAction(i18n("Remove Mask"), 0, this,
                               SLOT(slotRemoveMask()), actionCollection(), "remove_mask");
    m_showMask = new KToggleAction(i18n( "Show Mask" ), 0, this,
                                   SLOT(slotShowMask()), actionCollection(), "show_mask");
    m_editMask = new KToggleAction(i18n( "Edit Mask" ), 0, this,
                                   SLOT(slotEditMask()), actionCollection(), "edit_mask");

    // image actions
    m_imgFlatten = new KAction(i18n("&Flatten Image"), "Ctrl+Shift+E", this, SLOT(flattenImage()), actionCollection(), "flatten_image");
    m_imgMergeLayer = new KAction(i18n("&Merge with Layer Below"), "Ctrl+E", this, SLOT(mergeLayer()), actionCollection(), "merge_layer");

    // setting actions
    KStdAction::preferences(this, SLOT(preferences()), actionCollection(), "preferences");

    m_RulerAction = new KToggleAction( i18n( "Show Rulers" ), "Ctrl+R", this, SLOT( showRuler() ), actionCollection(), "view_ruler" );
    m_RulerAction->setChecked(cfg.showRulers());
    m_RulerAction->setCheckedState(i18n("Hide Rulers"));
    m_RulerAction->setWhatsThis( i18n("The rulers show the horizontal and vertical positions of the mouse on the image "
                                      "and can be used to position your mouse at the right place on the canvas. <p>Uncheck this to disable "
                                      "the rulers from being displayed." ) );

    //m_guideAction = new KToggleAction( i18n( "Guide Lines" ), 0, this, SLOT( viewGuideLines() ), actionCollection(), "view_guidelines" );

    // Add new palette
    new KAction(i18n("Add New Palette..."), 0, this, SLOT(slotAddPalette()),
                actionCollection(), "add_palette");
    new KAction(i18n("Edit Palette..."), 0, this, SLOT(slotEditPalette()),
                actionCollection(), "edit_palette");

    // XXX: This triggers a repaint of the image, but way too early
    //showRuler();

}

void KisView::resizeEvent(QResizeEvent *)
{
    if (!m_paintViewEnabled) {
        startInitialZoomTimerIfReady();
    }

    KisImageSP img = currentImg();
    Q_INT32 scrollBarExtent = style().pixelMetric(QStyle::PM_ScrollBarExtent);
    Q_INT32 drawH;
    Q_INT32 drawW;
    Q_INT32 docW;
    Q_INT32 docH;

//    if (img) {
//        KisGuideMgr *mgr = img->guides();
//        mgr->resize(size());
//    }

    docW = static_cast<Q_INT32>(ceil(docWidth() * zoom()));
    docH = static_cast<Q_INT32>(ceil(docHeight() * zoom()));

    m_rulerThickness = m_RulerAction->isChecked() ? RULER_THICKNESS : 0;
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

    m_vScroll->setEnabled(docH > drawH);
    m_hScroll->setEnabled(docW > drawW);

    if (docH <= drawH && docW <= drawW) {
        // we need no scrollbars
        m_vScroll->hide();
        m_hScroll->hide();
        m_vScroll->setValue(0);
        m_hScroll->setValue(0);
        m_vScrollBarExtent = 0;
        m_hScrollBarExtent = 0;
    } else if (docH <= drawH) {
        // we need a horizontal scrollbar only
        m_vScroll->hide();
        m_vScroll->setValue(0);
        m_hScroll->setRange(0, docW - drawW);
        m_hScroll->setGeometry(m_rulerThickness,
                     height() - scrollBarExtent,
                     width() - m_rulerThickness,
                     scrollBarExtent);
        m_hScroll->show();
        m_hScrollBarExtent = scrollBarExtent;
        m_hScrollBarExtent = scrollBarExtent;
    } else if(docW <= drawW) {
        // we need a vertical scrollbar only
        m_hScroll->hide();
        m_hScroll->setValue(0);
        m_vScroll->setRange(0, docH - drawH);
        m_vScroll->setGeometry(width() - scrollBarExtent, m_rulerThickness, scrollBarExtent, height()  - m_rulerThickness);
        m_vScroll->show();
        m_vScrollBarExtent = scrollBarExtent;
    } else {
        // we need both scrollbars
        m_vScroll->setRange(0, docH - drawH);
        m_vScroll->setGeometry(width() - scrollBarExtent,
                    m_rulerThickness,
                    scrollBarExtent,
                    height() -2* m_rulerThickness);
        m_hScroll->setRange(0, docW - drawW);
        m_hScroll->setGeometry(m_rulerThickness,
                     height() - scrollBarExtent,
                     width() - 2*m_rulerThickness,
                     scrollBarExtent);
        m_vScroll->show();
        m_hScroll->show();
        m_vScrollBarExtent = scrollBarExtent;
        m_hScrollBarExtent = scrollBarExtent;
    }

    Q_INT32 oldCanvasXOffset = m_canvasXOffset;
    Q_INT32 oldCanvasYOffset = m_canvasYOffset;

    if (docW < drawW) {
        m_canvasXOffset = (drawW - docW) / 2;
    } else {
        m_canvasXOffset = 0;
    }

    if (docH < drawH) {
        m_canvasYOffset = (drawH - docH) / 2;
    } else {
        m_canvasYOffset = 0;
    }

    //Check if rulers are visible
    if( m_RulerAction->isChecked() )
        m_canvas->setGeometry(m_rulerThickness, m_rulerThickness, drawW, drawH);
    else
        m_canvas->setGeometry(0, 0, drawW, drawH);
    m_canvas->show();

    if (!m_canvas->isOpenGLCanvas()) {

        if (m_canvasPixmap.size() != QSize(drawW, drawH)) {

            Q_INT32 oldCanvasWidth = m_canvasPixmap.width();
            Q_INT32 oldCanvasHeight = m_canvasPixmap.height();

            Q_INT32 newCanvasWidth = drawW;
            Q_INT32 newCanvasHeight = drawH;

            QRegion exposedRegion = QRect(0, 0, newCanvasWidth, newCanvasHeight);

            // Increase size first so that we can copy the old image area to the new one.
            m_canvasPixmap.resize(QMAX(oldCanvasWidth, newCanvasWidth), QMAX(oldCanvasHeight, newCanvasHeight));

            if (!m_canvasPixmap.isNull()) {

                if (oldCanvasXOffset != m_canvasXOffset || oldCanvasYOffset != m_canvasYOffset) {

                    Q_INT32 srcX;
                    Q_INT32 srcY;
                    Q_INT32 srcWidth;
                    Q_INT32 srcHeight;
                    Q_INT32 dstX;
                    Q_INT32 dstY;

                    if (oldCanvasXOffset <= m_canvasXOffset) {
                        // Move to the right
                        srcX = 0;
                        dstX = m_canvasXOffset - oldCanvasXOffset;
                        srcWidth = oldCanvasWidth;
                    } else {
                        // Move to the left
                        srcX = oldCanvasXOffset - m_canvasXOffset;
                        dstX = 0;
                        srcWidth = newCanvasWidth;
                    }

                    if (oldCanvasYOffset <= m_canvasYOffset) {
                        // Move down
                        srcY = 0;
                        dstY = m_canvasYOffset - oldCanvasYOffset;
                        srcHeight = oldCanvasHeight;
                    } else {
                        // Move up
                        srcY = oldCanvasYOffset - m_canvasYOffset;
                        dstY = 0;
                        srcHeight = newCanvasHeight;
                    }

                    bitBlt(&m_canvasPixmap, dstX, dstY, &m_canvasPixmap, srcX, srcY, srcWidth, srcHeight);
                    exposedRegion -= QRegion(QRect(dstX, dstY, srcWidth, srcHeight));
                } else {
                    exposedRegion -= QRegion(QRect(0, 0, oldCanvasWidth, oldCanvasHeight));
                }
            }

            m_canvasPixmap.resize(newCanvasWidth, newCanvasHeight);

            if (!m_canvasPixmap.isNull() && !exposedRegion.isEmpty()) {

                QMemArray<QRect> rects = exposedRegion.rects();

                for (unsigned int i = 0; i < rects.count(); i++) {
                    QRect r = rects[i];
                    updateQPaintDeviceCanvas(viewToWindow(r));
                }
            }
        }
    }

    int fontheight = QFontMetrics(KGlobalSettings::generalFont()).height() * 3;
    m_vScroll->setPageStep(drawH);
    m_vScroll->setLineStep(fontheight);
    m_hScroll->setPageStep(drawW);
    m_hScroll->setLineStep(fontheight);

    m_hRuler->setGeometry(m_rulerThickness + m_canvasXOffset, 0, QMIN(docW, drawW), m_rulerThickness);
    m_vRuler->setGeometry(0, m_rulerThickness + m_canvasYOffset, m_rulerThickness, QMIN(docH, drawH));

    if (m_vScroll->isVisible())
        m_vRuler->updateVisibleArea(0, m_vScroll->value());
    else
        m_vRuler->updateVisibleArea(0, 0);

    if (m_hScroll->isVisible())
        m_hRuler->updateVisibleArea(m_hScroll->value(), 0);
    else
        m_hRuler->updateVisibleArea(0, 0);

    if( m_RulerAction->isChecked() )
    {
        m_hRuler->show();
        m_vRuler->show();
    }
    else {
        m_hRuler->hide();
        m_vRuler->hide();
    }

    emit viewTransformationsChanged();
}

void KisView::styleChange(QStyle& oldStyle)
{
    Q_UNUSED(oldStyle);
    m_canvas->updateGeometry();
    refreshKisCanvas();
}

void KisView::paletteChange(const QPalette& oldPalette)
{
    Q_UNUSED(oldPalette);
    refreshKisCanvas();
}

void KisView::showEvent(QShowEvent *)
{
    if (!m_showEventReceived) {
        m_showEventReceived = true;
        startInitialZoomTimerIfReady();
    }
}

void KisView::updateReadWrite(bool readwrite)
{
    layerUpdateGUI(readwrite);
}

Q_INT32 KisView::horzValue() const
{
    return m_hScroll->value() - m_canvasXOffset;
}

Q_INT32 KisView::vertValue() const
{
    return m_vScroll->value() - m_canvasYOffset;
}

void KisView::updateQPaintDeviceCanvas(const QRect& imageRect)
{
    QRect vr = windowToView(imageRect);
    vr &= QRect(0, 0, m_canvas->width(), m_canvas->height());

    if (!vr.isEmpty()) {

        QPainter gc;

        if (gc.begin(&m_canvasPixmap)) {

            KisImageSP img = currentImg();

            if (img && m_paintViewEnabled) {

                QRect wr = viewToWindow(vr);

                if (wr.left() < 0 || wr.right() >= img->width() || wr.top() < 0 || wr.bottom() >= img->height()) {
                    // Erase areas outside document
                    QRegion rg(vr);
                    rg -= QRegion(windowToView(QRect(0, 0, img->width(), img->height())));

                    QMemArray<QRect> rects = rg.rects();

                    for (unsigned int i = 0; i < rects.count(); i++) {
                        QRect er = rects[i];
                        gc.fillRect(er, colorGroup().mid());
                    }
                    wr &= QRect(0, 0, img->width(), img->height());
                }

                if (!wr.isEmpty()) {

                    KisImage::PaintFlags paintFlags = (KisImage::PaintFlags)KisImage::PAINT_BACKGROUND;

                    if (m_actLayerVis) {
                        paintFlags = (KisImage::PaintFlags)(paintFlags|KisImage::PAINT_MASKINACTIVELAYERS);
                    }

                    if (m_selectionManager->displaySelection())
                    {
                        paintFlags = (KisImage::PaintFlags)(paintFlags|KisImage::PAINT_SELECTION);
                    }

                    if (zoom() > 1.0 - EPSILON) {

                        gc.setWorldXForm(true);
                        gc.translate(-horzValue(), -vertValue());
                        gc.scale(zoomFactor(), zoomFactor());

                        m_image->renderToPainter(wr.left(), wr.top(),
                            wr.right(), wr.bottom(), gc, monitorProfile(),
                            paintFlags, HDRExposure());
                    } else {

                        QRect canvasRect = windowToView(wr);
                        QRect scaledImageRect = canvasRect;
                        scaledImageRect.moveBy(horzValue(), vertValue());

                        QSize scaledImageSize(static_cast<Q_INT32>(ceil(docWidth() * zoom())),
                                            static_cast<Q_INT32>(ceil(docHeight() * zoom())));

                        QImage image = m_image->convertToQImage(scaledImageRect, scaledImageSize,
                                                                monitorProfile(), paintFlags, HDRExposure());

                        gc.drawImage(canvasRect.topLeft(), image, image.rect());

                        // Set up for the grid drawer.
                        gc.setWorldXForm(true);
                        gc.translate(-horzValue(), -vertValue());
                        gc.scale(zoomFactor(), zoomFactor());
                    }

                    m_gridManager->drawGrid( wr, &gc );
                    m_perspectiveGridManager->drawGrid( wr, &gc );
                }
//                    paintGuides();
            } else {
                gc.fillRect(vr, colorGroup().mid());
            }
        }
    }
}

void KisView::paintQPaintDeviceView(const QRegion& canvasRegion)
{
    Q_ASSERT(m_canvas->QPaintDeviceWidget() != 0);

    if (m_canvas->QPaintDeviceWidget() != 0 && !m_canvasPixmap.isNull()) {
        QMemArray<QRect> rects = canvasRegion.rects();

        for (unsigned int i = 0; i < rects.count(); i++) {
            QRect r = rects[i];

            bitBlt(m_canvas->QPaintDeviceWidget(), r.x(), r.y(), &m_canvasPixmap,
                   r.x(), r.y(), r.width(), r.height());
        }

        paintToolOverlay(canvasRegion);
    }
}

void KisView::updateOpenGLCanvas(const QRect& imageRect)
{
#ifdef HAVE_GL
    KisImageSP img = currentImg();

    if (img && m_paintViewEnabled) {
        Q_ASSERT(m_OpenGLImageContext != 0);

        if (m_OpenGLImageContext != 0) {
            m_OpenGLImageContext->update(imageRect);
        }
    }
#else
    Q_UNUSED(imageRect);
#endif
}

void KisView::paintOpenGLView(const QRect& canvasRect)
{
#ifdef HAVE_GL
    if (!m_canvas->isUpdatesEnabled()) {
        return;
    }

    m_canvas->OpenGLWidget()->makeCurrent();

    glDrawBuffer(GL_BACK);

    QColor widgetBackgroundColor = colorGroup().mid();

    glClearColor(widgetBackgroundColor.red() / 255.0, widgetBackgroundColor.green() / 255.0, widgetBackgroundColor.blue() / 255.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    KisImageSP img = currentImg();

    if (img && m_paintViewEnabled) {

        QRect vr = canvasRect;
        vr &= QRect(0, 0, m_canvas->width(), m_canvas->height());

        if (!vr.isNull()) {

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glViewport(0, 0, m_canvas->width(), m_canvas->height());
            glOrtho(0, m_canvas->width(), m_canvas->height(), 0, -1, 1);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glBindTexture(GL_TEXTURE_2D, m_OpenGLImageContext->backgroundTexture());

            glTranslatef(m_canvasXOffset, m_canvasYOffset, 0.0);

            glEnable(GL_TEXTURE_2D);
            glBegin(GL_QUADS);

            glTexCoord2f(0.0, 0.0);
            glVertex2f(0.0, 0.0);

            glTexCoord2f((img->width() * zoom()) / KisOpenGLImageContext::BACKGROUND_TEXTURE_WIDTH, 0.0);
            glVertex2f(img->width() * zoom(), 0.0);

            glTexCoord2f((img->width() * zoom()) / KisOpenGLImageContext::BACKGROUND_TEXTURE_WIDTH,
                         (img->height() * zoom()) / KisOpenGLImageContext::BACKGROUND_TEXTURE_HEIGHT);
            glVertex2f(img->width() * zoom(), img->height() * zoom());

            glTexCoord2f(0.0, (img->height() * zoom()) / KisOpenGLImageContext::BACKGROUND_TEXTURE_HEIGHT);
            glVertex2f(0.0, img->height() * zoom());

            glEnd();

            glTranslatef(-m_canvasXOffset, -m_canvasYOffset, 0.0);

            glTranslatef(-horzValue(), -vertValue(), 0.0);
            glScalef(zoomFactor(), zoomFactor(), 1.0);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            QRect wr = viewToWindow(QRect(0, 0, m_canvas->width(), m_canvas->height()));
            wr &= QRect(0, 0, img->width(), img->height());

            m_OpenGLImageContext->setHDRExposure(HDRExposure());

            m_canvas->OpenGLWidget()->makeCurrent();

            for (int x = (wr.left() / m_OpenGLImageContext->imageTextureTileWidth()) * m_OpenGLImageContext->imageTextureTileWidth();
                  x <= wr.right();
                  x += m_OpenGLImageContext->imageTextureTileWidth()) {
                for (int y = (wr.top() / m_OpenGLImageContext->imageTextureTileHeight()) * m_OpenGLImageContext->imageTextureTileHeight();
                      y <= wr.bottom();
                      y += m_OpenGLImageContext->imageTextureTileHeight()) {

                    glBindTexture(GL_TEXTURE_2D, m_OpenGLImageContext->imageTextureTile(x, y));

                    glBegin(GL_QUADS);

                    glTexCoord2f(0.0, 0.0);
                    glVertex2f(x, y);

                    glTexCoord2f(1.0, 0.0);
                    glVertex2f(x + m_OpenGLImageContext->imageTextureTileWidth(), y);

                    glTexCoord2f(1.0, 1.0);
                    glVertex2f(x + m_OpenGLImageContext->imageTextureTileWidth(), y + m_OpenGLImageContext->imageTextureTileHeight());

                    glTexCoord2f(0.0, 1.0);
                    glVertex2f(x, y + m_OpenGLImageContext->imageTextureTileHeight());

                    glEnd();
                }
            }

            glDisable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);

            m_gridManager->drawGrid(wr, 0, true);
            m_perspectiveGridManager->drawGrid( wr, 0, true );

            // Unbind the texture otherwise the ATI driver crashes when the canvas context is
            // made current after the textures are deleted following an image resize.
            glBindTexture(GL_TEXTURE_2D, 0);

            //paintGuides();
        }
    }

    m_canvas->OpenGLWidget()->swapBuffers();

    paintToolOverlay(QRegion(canvasRect));

#else
    Q_UNUSED(canvasRect);
#endif
}

void KisView::setInputDevice(KisInputDevice inputDevice)
{
    if (inputDevice != m_inputDevice) {
        m_inputDevice = inputDevice;

        m_toolManager->setToolForInputDevice(m_inputDevice, inputDevice);

        if (m_toolManager->currentTool() == 0) {
            m_toolManager->setCurrentTool(m_toolManager->findTool("tool_brush", m_inputDevice));
        }
        else {
            m_toolManager->setCurrentTool(m_toolManager->currentTool());
        }
        m_toolManager->activateCurrentTool();

        emit sigInputDeviceChanged(inputDevice);
    }

}

KisInputDevice KisView::currentInputDevice() const
{
    return m_inputDevice;
}


KisCanvas *KisView::kiscanvas() const
{
    return m_canvas;
}

void KisView::updateCanvas()
{
    if (m_image) {
        updateCanvas(m_image->bounds());
    }
}

void KisView::updateCanvas(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
    updateCanvas(QRect(x, y, w, h));
}

void KisView::updateCanvas(const QRect& imageRect)
{
    if (m_canvas->isOpenGLCanvas()) {
        updateOpenGLCanvas(imageRect);
        paintOpenGLView(windowToView(imageRect));
    } else {
        updateQPaintDeviceCanvas(imageRect);
        //m_canvas->update(windowToView(imageRect));
        m_canvas->repaint(windowToView(imageRect));
    }
}

void KisView::refreshKisCanvas()
{
    QRect imageRect = viewToWindow(QRect(0, 0, m_canvas->width(), m_canvas->height()));

    if (m_image) {
        imageRect |= m_image->bounds();
    }

    updateCanvas(imageRect);

    // Enable this if updateCanvas does an m_canvas->update()
    //m_canvas->repaint();
}

void KisView::selectionDisplayToggled(bool displaySelection)
{
#ifdef HAVE_GL
    if (m_canvas->isOpenGLCanvas()) {
        if (m_OpenGLImageContext) {
            m_OpenGLImageContext->setSelectionDisplayEnabled(displaySelection);
        }
    }
#else
    Q_UNUSED(displaySelection);
#endif
    updateCanvas();
}

void KisView::layerUpdateGUI(bool enable)
{
    KisImageSP img = currentImg();

    KisLayerSP layer;
    Q_INT32 nlayers = 0;
    Q_INT32 nvisible = 0;



    if (img) {
        layer = img->activeLayer();
        nlayers = img->nlayers();
        nvisible = nlayers - img->nHiddenLayers();
    }

    KisPaintLayer * pl = dynamic_cast<KisPaintLayer*>(layer.data());

    if (pl && ( m_currentColorChooserDisplay != KisID("BLA") ||
                pl->paintDevice()->colorSpace()->id() != m_currentColorChooserDisplay)) {
        if (pl->paintDevice()->colorSpace()->id() == KisID("WET")) {
            m_paletteManager->hideWidget( "hsvwidget" );
            m_paletteManager->hideWidget( "rgbwidget" );
            m_paletteManager->hideWidget( "graywidget" );
            m_paletteManager->hideWidget( "palettewidget" );
            m_paletteManager->showWidget( "watercolor docker" );
        }
        else {
            m_paletteManager->hideWidget( "watercolor docker" );
            m_paletteManager->showWidget( "palettewidget" );
            m_paletteManager->showWidget( "graywidget" );
            m_paletteManager->showWidget( "rgbwidget" );
            m_paletteManager->showWidget( "hsvwidget" );
        }
        m_currentColorChooserDisplay = pl->paintDevice()->colorSpace()->id();
    }

    enable = enable && img && layer && layer->visible() && !layer->locked();
    m_layerDup->setEnabled(enable);
    m_layerRm->setEnabled(enable);
    m_layerHide->setEnabled(img && layer);
    m_layerProperties->setEnabled(enable);
    m_layerSaveAs->setEnabled(enable);
    m_layerRaise->setEnabled(enable && layer->prevSibling());
    m_layerLower->setEnabled(enable && layer->nextSibling());
    m_layerTop->setEnabled(enable && nlayers > 1 && layer != img->rootLayer()->firstChild());
    m_layerBottom->setEnabled(enable && nlayers > 1 && layer != img->rootLayer()->lastChild());

    // XXX these should be named layer instead of img
    m_imgFlatten->setEnabled(nlayers > 1);
    m_imgMergeLayer->setEnabled(nlayers > 1 && layer && layer->nextSibling());


    m_selectionManager->updateGUI();
    m_filterManager->updateGUI();
    m_toolManager->updateGUI();
    m_gridManager->updateGUI();
    m_perspectiveGridManager->updateGUI();


    KisPartLayer * partLayer = dynamic_cast<KisPartLayer*>(layer.data());
    if (partLayer) {
        setCanvasCursor( KisCursor::arrowCursor() );
    }

    if (img && img->activeDevice())
        emit currentColorSpaceChanged(img->activeDevice()->colorSpace());

    imgUpdateGUI();
}


void KisView::imgUpdateGUI()
{
    KisImageSP img = currentImg();

    m_imgResizeToLayer->setEnabled(img && img->activeLayer());

    updateStatusBarProfileLabel();
}

static const double zoomLevels[] = {
    1.0 / 500,
    1.0 / 333.333333,
    1.0 / 250,
    1.0 / 200,
    1.0 / 150,
    1.0 / 100,
    1.0 / 66.666667,
    1.0 / 50,
    1.0 / 33.333333,
    1.0 / 25,
    1.0 / 20,
    1.0 / 16,
    1.0 / 12,
    1.0 / 8,
    1.0 / 6,
    1.0 / 4,
    1.0 / 3,
    1.0 / 2,
    1.0 / 1.5,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    12,
    16
};

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define NUM_ZOOM_LEVELS ARRAY_SIZE(zoomLevels)

#define FIRST_ZOOM_LEVEL_INDEX 0
#define LAST_ZOOM_LEVEL_INDEX (NUM_ZOOM_LEVELS - 1)

#define KISVIEW_MIN_ZOOM (zoomLevels[FIRST_ZOOM_LEVEL_INDEX])
#define KISVIEW_MAX_ZOOM (zoomLevels[LAST_ZOOM_LEVEL_INDEX])

double KisView::nextZoomInLevel() const
{
    uint zoomLevelIndex = FIRST_ZOOM_LEVEL_INDEX;

    while (zoom() >= zoomLevels[zoomLevelIndex] && zoomLevelIndex < LAST_ZOOM_LEVEL_INDEX) {
        zoomLevelIndex++;
    }

    return zoomLevels[zoomLevelIndex];
}

double KisView::nextZoomOutLevel(double zoomLevel) const
{
    int zoomLevelIndex = LAST_ZOOM_LEVEL_INDEX;

    while (zoomLevel <= zoomLevels[zoomLevelIndex] && zoomLevelIndex > FIRST_ZOOM_LEVEL_INDEX) {
        zoomLevelIndex--;
    }

    return zoomLevels[zoomLevelIndex];
}

double KisView::nextZoomOutLevel() const
{
    return nextZoomOutLevel(zoom());
}

void KisView::zoomAroundPoint(double x, double y, double zf)
{
    // Disable updates while we change the scrollbar settings.
    m_canvas->setUpdatesEnabled(false);
    m_hScroll->setUpdatesEnabled(false);
    m_vScroll->setUpdatesEnabled(false);

    if (x < 0 || y < 0) {
        // Zoom about the centre of the current display
        KisImageSP img = currentImg();

        if (img) {
            if (m_hScroll->isVisible()) {
                KisPoint c = viewToWindow(KisPoint(m_canvas->width() / 2.0, m_canvas->height() / 2.0));
                x = c.x();
            }
            else {
                x = img->width() / 2.0;
            }

            if (m_vScroll->isVisible()) {
                KisPoint c = viewToWindow(KisPoint(m_canvas->width() / 2.0, m_canvas->height() / 2.0));
                y = c.y();
            }
            else {
                y = img->height() / 2.0;
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

    m_zoomIn->setEnabled(zf < KISVIEW_MAX_ZOOM);
    m_zoomOut->setEnabled(zf > KISVIEW_MIN_ZOOM);
    resizeEvent(0);

    m_hRuler->setZoom(zf);
    m_vRuler->setZoom(zf);

    if (m_hScroll->isVisible()) {
        double vcx = m_canvas->width() / 2.0;
        Q_INT32 scrollX = qRound(x * zoom() - vcx);
        m_hScroll->setValue(scrollX);
    }

    if (m_vScroll->isVisible()) {
        double vcy = m_canvas->height() / 2.0;
        Q_INT32 scrollY = qRound(y * zoom() - vcy);
        m_vScroll->setValue(scrollY);
    }

    // Now update everything.
    m_canvas->setUpdatesEnabled(true);
    m_hScroll->setUpdatesEnabled(true);
    m_vScroll->setUpdatesEnabled(true);
    m_hScroll->update();
    m_vScroll->update();

    if (m_canvas->isOpenGLCanvas()) {
        paintOpenGLView(QRect(0, 0, m_canvas->width(), m_canvas->height()));
    } else {
        refreshKisCanvas();
    }

    emit viewTransformationsChanged();
}

void KisView::zoomTo(const KisRect& r)
{
    if (!r.isNull()) {

        double wZoom = fabs(m_canvas->width() / r.width());
        double hZoom = fabs(m_canvas->height() / r.height());

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
    zoomAroundPoint(x, y, nextZoomInLevel());
}

void KisView::zoomOut(Q_INT32 x, Q_INT32 y)
{
    zoomAroundPoint(x, y, nextZoomOutLevel());
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
    zoomIn(-1, -1);
}

void KisView::slotZoomOut()
{
    zoomOut(-1, -1);
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

double KisView::fitToCanvasZoomLevel() const
{
    int fullCanvasWidth = width();

    if (m_vRuler->isVisible()) {
        fullCanvasWidth -= m_vRuler->width();
    }

    int fullCanvasHeight = height();

    if (m_hRuler->isVisible()) {
        fullCanvasHeight -= m_hRuler->height();
    }

    KisImageSP img = currentImg();
    if (img) {
        double xZoomLevel = static_cast<double>(fullCanvasWidth) / img->width();
        double yZoomLevel = static_cast<double>(fullCanvasHeight) / img->height();

        return QMIN(xZoomLevel, yZoomLevel);
    }
    else {
        return 1;
    }
}

void KisView::slotFitToCanvas()
{
    zoomAroundPoint(-1, -1, fitToCanvasZoomLevel());
}

void KisView::setInitialZoomLevel()
{
    double zoomLevel = fitToCanvasZoomLevel();

    if (zoomLevel > 1) {
        zoomLevel = 1;
    } else {
        zoomLevel = nextZoomOutLevel(zoomLevel);
    }

    zoomAroundPoint(-1, -1, zoomLevel);
}

void KisView::imgResizeToActiveLayer()
{
    KisImageSP img = currentImg();
    KisLayerSP layer;

    if (img && (layer = img->activeLayer())) {

        if (m_adapter && m_adapter->undo()) {
            m_adapter->beginMacro(i18n("Resize Image to Size of Current Layer"));
        }

        img->lock();

        QRect r = layer->exactBounds();
        img->resize(r.width(), r.height(), r.x(), r.y(), true);

        img->unlock();

        if (m_adapter && m_adapter->undo()) {
            m_adapter->endMacro();
        }
    }
}

void KisView::slotImageProperties()
{
    KisImageSP img = currentImg();

    if (!img) return;

    KisDlgImageProperties dlg(img, this);

    if (dlg.exec() == QDialog::Accepted) {
        if (dlg.imageWidth() != img->width() ||
            dlg.imageHeight() != img->height()) {

            resizeCurrentImage(dlg.imageWidth(),
                               dlg.imageHeight());
        }
        Q_INT32 opacity = dlg.opacity();
        opacity = opacity * 255 / 100;
        img->setName(dlg.imageName());
        img->setColorSpace(dlg.colorSpace());
        img->setResolution(dlg.resolution(), dlg.resolution());
        img->setDescription(dlg.description());
        img->setProfile(dlg.profile());
    }
}

void KisView::slotInsertImageAsLayer()
{
    if (importImage() > 0)
        m_doc->setModified(true);
}

void KisView::slotAddPalette()
{
    KDialogBase* base = new KDialogBase(this, 0, true, i18n("Add Palette"), KDialogBase::Ok | KDialogBase::Cancel);
    KisCustomPalette* p = new KisCustomPalette(base, "add palette", i18n("Add Palette"), this);
    base->setMainWidget(p);
    base->show();
}

void KisView::slotEditPalette()
{
    KisPaletteChooser chooser(this);
    KisResourceServerBase* srv = KisResourceServerRegistry::instance()->get("PaletteServer");
    if (!srv) {
        return;
    }
    QValueList<KisResource*> resources = srv->resources();
    QValueList<KisPalette*> palettes;

    for(uint i = 0; i < resources.count(); i++) {
        KisPalette* palette = dynamic_cast<KisPalette*>(*resources.at(i));

        chooser.paletteList->insertItem(palette->name());
        palettes.append(palette);
    }

    if (chooser.exec() != QDialog::Accepted ) {
        return;
    }

    int index = chooser.paletteList->currentItem();
    if (index < 0) {
        KMessageBox::error(this, i18n("No palette selected."), i18n("Palette"));
        return;
    }

    KDialogBase* base = new KDialogBase(this, 0, true, i18n("Edit Palette") , KDialogBase::Ok);
    KisCustomPalette* cp = new KisCustomPalette(base, "edit palette",
            i18n("Edit Palette"), this);
    cp->setEditMode(true);
    cp->setPalette(*palettes.at(index));
    base->setMainWidget(cp);
    base->show();
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

    KisLayerSP l = img->activeLayer();
    if (!l) return;

    QRect r = l->exactBounds();

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
    KisImageSP currentImage = currentImg();

    if (!currentImage) {
        return 0;
    }

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

    for (KURL::List::iterator it = urls.begin(); it != urls.end(); ++it) {
        KURL url = *it;
        KisDoc d;
        d.import(url);
        KisImageSP importedImage = d.currentImage();

        if (importedImage) {
            KisLayerSP importedImageLayer = importedImage->rootLayer().data();

            if (importedImageLayer != 0) {

                if (importedImageLayer->numLayers() == 2) {
                    // Don't import the root if this is not a layered image (1 group layer
                    // plus 1 other).
                    importedImageLayer = importedImageLayer->firstChild();
                    importedImageLayer->parent()->removeLayer(importedImageLayer);
                }

                importedImageLayer->setName(url.prettyURL());

                KisGroupLayerSP parent = 0;
                KisLayerSP currentActiveLayer = currentImage->activeLayer();

                if (currentActiveLayer) {
                    parent = currentActiveLayer->parent();
                }

                if (parent == 0) {
                    parent = currentImage->rootLayer();
                }

                currentImage->addLayer(importedImageLayer.data(), parent, currentActiveLayer);
                rc += importedImageLayer->numLayers();
            }
        }
    }

    updateCanvas();

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
    KisPaintDeviceSP dev = currentImg()->activeDevice();
    if (!dev) return;

    KisTransaction * t = 0;
    if (undoAdapter() && undoAdapter()->undo()) {
        t = new KisTransaction(i18n("Mirror Layer X"), dev);
        Q_CHECK_PTR(t);
    }

    dev->mirrorX();

    if (t) undoAdapter()->addCommand(t);

    m_doc->setModified(true);
    layersUpdated();
    updateCanvas();
}

void KisView::mirrorLayerY()
{
    if (!currentImg()) return;
    KisPaintDeviceSP dev = currentImg()->activeDevice();
    if (!dev) return;

    KisTransaction * t = 0;
    if (undoAdapter() && undoAdapter()->undo()) {
        t = new KisTransaction(i18n("Mirror Layer Y"), dev);
        Q_CHECK_PTR(t);
    }

    dev->mirrorY();

    if (t) undoAdapter()->addCommand(t);

    m_doc->setModified(true);
    layersUpdated();
    updateCanvas();
}

void KisView::scaleLayer(double sx, double sy, KisFilterStrategy *filterStrategy)
{
    if (!currentImg()) return;

    KisPaintDeviceSP dev = currentImg()->activeDevice();
    if (!dev) return;

    KisTransaction * t = 0;
    if (undoAdapter() && undoAdapter()->undo()) {
        t = new KisTransaction(i18n("Scale Layer"), dev);
        Q_CHECK_PTR(t);
    }

    KisTransformWorker worker(dev, sx, sy, 0, 0, 0.0, 0, 0, m_progress, filterStrategy);
    worker.run();

    if (t) undoAdapter()->addCommand(t);

    m_doc->setModified(true);
    layersUpdated();
    updateCanvas();
}

void KisView::rotateLayer(double angle)
{
    if (!currentImg()) return;

    KisPaintDeviceSP dev = currentImg()->activeDevice();
    if (!dev) return;

    KisTransaction * t = 0;
    if (undoAdapter() && undoAdapter()->undo()) {
        t = new KisTransaction(i18n("Rotate Layer"), dev);
        Q_CHECK_PTR(t);
    }

    KisFilterStrategy *filter = KisFilterStrategyRegistry::instance()->get(KisID("Triangle"));
    angle *= M_PI/180;
    Q_INT32 w = currentImg()->width();
    Q_INT32 h = currentImg()->height();
    Q_INT32 tx = Q_INT32((w*cos(angle) - h*sin(angle) - w) / 2 + 0.5);
    Q_INT32 ty = Q_INT32((h*cos(angle) + w*sin(angle) - h) / 2 + 0.5);

    KisTransformWorker tw(dev, 1.0, 1.0, 0, 0, angle, -tx, -ty, m_progress, filter);
    tw.run();

    if (t) undoAdapter()->addCommand(t);

    m_doc->setModified(true);
    layersUpdated();
    updateCanvas();
}

void KisView::shearLayer(double angleX, double angleY)
{
    if (!currentImg()) return;

    KisLayerSP layer = currentImg()->activeLayer();
    if (!layer) return;

    KisUndoAdapter * undo = 0;
    if ((undo = currentImg()->undoAdapter())) {
        undo->beginMacro(i18n("Shear layer"));
    }

    KisShearVisitor v(angleX, angleY, m_progress);
    v.setUndoAdapter(undo);
    layer->accept(v);

    if (undo) undo->endMacro();

    m_doc->setModified(true);
    layersUpdated();
    updateCanvas();
}

void KisView::flattenImage()
{
    KisImageSP img = currentImg();

    if (img) {
        bool doIt = true;

        if (img->nHiddenLayers() > 0) {
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
            img->flatten();
        }
    }
}

void KisView::mergeLayer()
{
    KisImageSP img = currentImg();
    if (!img) return;

    KisLayerSP layer = img->activeLayer();
    if (!layer) return;

    img->mergeLayer(layer);
}

void KisView::preferences()
{
#ifdef HAVE_GL
    bool canvasWasOpenGL = m_canvas->isOpenGLCanvas();
#endif

    if (PreferencesDialog::editPreferences())
    {
        KisConfig cfg;
        m_paletteManager->slotResetFont();
        resetMonitorProfile();

#ifdef HAVE_GL
        if (cfg.useOpenGL() != canvasWasOpenGL) {

            disconnectCurrentImg();

            //XXX: Need to notify other views that this global setting has changed.
            if (cfg.useOpenGL()) {
                m_OpenGLImageContext = KisOpenGLImageContext::getImageContext(m_image, monitorProfile());
                m_canvas->createOpenGLCanvas(m_OpenGLImageContext->sharedContextWidget());
            } else
            {
                m_OpenGLImageContext = 0;
                m_canvas->createQPaintDeviceCanvas();
            }

            connectCurrentImg();

            resizeEvent(0);
        }

        if (cfg.useOpenGL()) {
            m_OpenGLImageContext->setMonitorProfile(monitorProfile());
        }
#endif

        refreshKisCanvas();

        if (m_toolManager->currentTool()) {
            setCanvasCursor(m_toolManager->currentTool()->cursor());
        }

#if defined(EXTENDED_X11_TABLET_SUPPORT)
        m_canvas->selectTabletDeviceEvents();
#endif

    }
}

void KisView::layerCompositeOp(const KisCompositeOp& compositeOp)
{
    KisImageSP img = currentImg();
    if (!img) return;

    KisLayerSP layer = img->activeLayer();
    if (!layer) return;

    if (img->undo()) {
        KNamedCommand *cmd = layer->setCompositeOpCommand(compositeOp);
        cmd->execute();
        undoAdapter()->addCommand(cmd);
    }
}

// range: 0 - 100
void KisView::layerOpacity(int opacity, bool dontundo)
{
    KisImageSP img = currentImg();
    if (!img) return;

    KisLayerSP layer = img->activeLayer();
    if (!layer) return;

    opacity = int(float(opacity * 255) / 100 + 0.5);
    if (opacity > 255)
        opacity = 255;

    if (opacity == layer->opacity()) return;

    if (dontundo)
        layer->setOpacity( opacity );
    else
    {
        if (img->undo()) {
            KNamedCommand *cmd = layer->setOpacityCommand(opacity);
            cmd->execute();
            undoAdapter()->addCommand(cmd);
        }
    }
}

void KisView::layerOpacityFinishedChanging( int previous, int opacity )
{
    KisImageSP img = currentImg();
    if (!img) return;

    KisLayerSP layer = img->activeLayer();
    if (!layer) return;

    opacity = int(float(opacity * 255) / 100 + 0.5);
    if (opacity > 255)
        opacity = 255;

    previous = int(float(previous * 255) / 100 + 0.5);
    if (previous > 255)
        previous = 255;

    if (previous == opacity) return;

    if (img->undo()) {
        KNamedCommand *cmd = layer->setOpacityCommand(previous, opacity);
        m_adapter->addCommand(cmd);
    }
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
    refreshKisCanvas();
}

void KisView::slotUpdateFullScreen(bool toggle)
{
    if (KoView::shell()) {

        uint newState = KoView::shell()->windowState();

        if (toggle) {
            newState |= Qt::WindowFullScreen;
        } else {
            newState &= ~Qt::WindowFullScreen;
        }

        KoView::shell()->setWindowState(newState);
    }
}

Q_INT32 KisView::docWidth() const
{
    return currentImg() ? currentImg()->width() : 0;
}

Q_INT32 KisView::docHeight() const
{
    return currentImg() ? currentImg()->height() : 0;
}

void KisView::scrollTo(Q_INT32 x, Q_INT32 y)
{
    if (m_hScroll->isVisible()) {
        m_hScroll->setValue(x);
    }
    if (m_vScroll->isVisible()) {
        m_vScroll->setValue(y);
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

void KisView::paintopActivated(const KisID & paintop, const KisPaintOpSettings *paintopSettings)
{
    if (paintop.id().isNull() || paintop.id().isEmpty()) {
        return;
    }

    m_paintop = paintop;
    m_paintopSettings = paintopSettings;
    emit paintopChanged(m_paintop, paintopSettings);
    notifyObservers();
}

void KisView::setBGColor(const KisColor& c)
{
    m_bg = c;
    notifyObservers();
    emit sigBGQColorChanged( c.toQColor() );
}

void KisView::setFGColor(const KisColor& c)
{
    m_fg = c;
    notifyObservers();
    emit sigFGQColorChanged( c.toQColor() );
}

void KisView::slotSetFGColor(const KisColor& c)
{

    m_fg = c;
    notifyObservers();
}

void KisView::slotSetBGColor(const KisColor& c)
{

    m_bg = c;
    notifyObservers();
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

    QRect r = img->bounds();
    img->renderToPainter(r.x(), r.y(), r.width(), r.height(), gc, printerProfile, KisImage::PAINT_IMAGE_ONLY, HDRExposure());
}

void KisView::paintToolOverlay(const QRegion& region)
{
    if (!region.isEmpty() && m_toolManager->currentTool() && !m_toolIsPainting) {
        KisCanvasPainter gc(m_canvas);

        gc.setClipRegion(region);
        gc.setClipping(true);

        // Prevent endless loop if the tool needs to have the canvas repainted
        m_toolIsPainting = true;
        m_toolManager->currentTool()->paint(gc, region.boundingRect());
        m_toolIsPainting = false;
    }
}

void KisView::canvasGotPaintEvent(QPaintEvent *event)
{
    if (m_canvas->isOpenGLCanvas()) {
        paintOpenGLView(event->rect());
    } else {
        paintQPaintDeviceView(event->region());
    }
}

void KisView::canvasGotButtonPressEvent(KisButtonPressEvent *e)
{
#if defined(EXTENDED_X11_TABLET_SUPPORT)
    // The event filter doesn't see tablet events going to the canvas.
    if (e->device() != KisInputDevice::mouse()) {
        m_tabletEventTimer.start();
    }
#endif // EXTENDED_X11_TABLET_SUPPORT

    if (e->device() != currentInputDevice()) {
        if (e->device() == KisInputDevice::mouse()) {
            if (m_tabletEventTimer.elapsed() > MOUSE_CHANGE_EVENT_DELAY) {
                setInputDevice(KisInputDevice::mouse());
            }
        } else {
            setInputDevice(e->device());
        }
    }

    KisImageSP img = currentImg();

//    if (img) {
//        QPoint pt = mapToScreen(e->pos().floorQPoint());
//        KisGuideMgr *mgr = img->guides();
//
//        m_lastGuidePoint = mapToScreen(e->pos().floorQPoint());
//        m_currentGuide = 0;
//
//        if ((e->state() & ~Qt::ShiftButton) == Qt::NoButton) {
//            KisGuideSP gd = mgr->find(static_cast<Q_INT32>(pt.x() / zoom()), static_cast<Q_INT32>(pt.y() / zoom()), QMAX(2.0, 2.0 / zoom()));
//
//            if (gd) {
//                m_currentGuide = gd;
//
//                if ((e->button() == Qt::RightButton) || ((e->button() & Qt::ShiftButton) == Qt::ShiftButton)) {
//                    if (gd->isSelected())
//                        mgr->unselect(gd);
//                    else
//                        mgr->select(gd);
//              } else {
//                    if (!gd->isSelected()) {
//                        mgr->unselectAll();
//                        mgr->select(gd);
//                    }
//                }
//
//                updateGuides();
//                return;
//            }
//        }
//    }
    if (e->button() == Qt::RightButton) {

        if (m_popup == 0 && factory()) {
            Q_ASSERT(factory());
            m_popup = (QPopupMenu *)factory()->container("image_popup", this);
        }
        if (m_popup) m_popup->popup(e->globalPos().roundQPoint());
    }
    else if (e->device() == currentInputDevice() && m_toolManager->currentTool()) {
        KisPoint p = viewToWindow(e->pos());
        // somewhat of a hack: we should actually test if we intersect with the scrollers,
        // but the globalPos seems to be off by a few pixels
        if (m_vScroll->draggingSlider() || m_hScroll->draggingSlider())
            return;

        if (m_toolManager->currentTool()->wantsAutoScroll()) {
            enableAutoScroll();
        }

        KisButtonPressEvent ev(e->device(), p, e->globalPos(), e->pressure(), e->xTilt(), e->yTilt(), e->button(), e->state());
        m_toolManager->currentTool()->buttonPress(&ev);
    }
}

void KisView::canvasGotMoveEvent(KisMoveEvent *e)
{
#if defined(EXTENDED_X11_TABLET_SUPPORT)
    // The event filter doesn't see tablet events going to the canvas.
    if (e->device() != KisInputDevice::mouse()) {
        m_tabletEventTimer.start();
    }
#endif // EXTENDED_X11_TABLET_SUPPORT

    if (e->device() != currentInputDevice()) {
        if (e->device() == KisInputDevice::mouse()) {
            if (m_tabletEventTimer.elapsed() > MOUSE_CHANGE_EVENT_DELAY) {
                setInputDevice(KisInputDevice::mouse());
            }
        } else {
            setInputDevice(e->device());
        }
    }

    KisImageSP img = currentImg();

    m_hRuler->updatePointer(e->pos().floorX() - m_canvasXOffset, e->pos().floorY() - m_canvasYOffset);
    m_vRuler->updatePointer(e->pos().floorX() - m_canvasXOffset, e->pos().floorY() - m_canvasYOffset);

    KisPoint wp = viewToWindow(e->pos());

#if 0
    if (img && m_currentGuide) {
        QPoint p = mapToScreen(e->pos().floorQPoint());
        KisGuideMgr *mgr = img->guides();

        if (((e->state() & Qt::LeftButton) == Qt::LeftButton) && mgr->hasSelected()) {
            eraseGuides();
            p -= m_lastGuidePoint;

            if (p.x())
                mgr->moveSelectedByX(p.x() / zoom());

            if (p.y())
                mgr->moveSelectedByY(p.y() / zoom());

            m_doc->setModified(true);
            paintGuides();
        }
    } else
#endif
    if (e->device() == currentInputDevice() && m_toolManager->currentTool()) {
        KisMoveEvent ev(e->device(), wp, e->globalPos(), e->pressure(), e->xTilt(), e->yTilt(), e->state());

        m_toolManager->currentTool()->move(&ev);
    }

//    m_lastGuidePoint = mapToScreen(e->pos().floorQPoint());
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
    KisMoveEvent ke(currentInputDevice(), e->pos(), e->globalPos(), PRESSURE_DEFAULT, 0, 0, e->state());
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
    if (e->device() != KisInputDevice::mouse()) {
        m_tabletEventTimer.start();
    }
#endif // EXTENDED_X11_TABLET_SUPPORT

    if (e->device() != currentInputDevice()) {
        if (e->device() == KisInputDevice::mouse()) {
            if (m_tabletEventTimer.elapsed() > MOUSE_CHANGE_EVENT_DELAY) {
                setInputDevice(KisInputDevice::mouse());
            }
        } else {
            setInputDevice(e->device());
        }
    }

    KisImageSP img = currentImg();

//    if (img && m_currentGuide) {
//        m_currentGuide = 0;
//    } else
    if (e->device() == currentInputDevice() && m_toolManager->currentTool()) {
        KisPoint p = viewToWindow(e->pos());
        KisButtonReleaseEvent ev(e->device(), p, e->globalPos(), e->pressure(), e->xTilt(), e->yTilt(), e->button(), e->state());

        disableAutoScroll();
        if (m_toolManager->currentTool()) {
            m_toolManager->currentTool()->buttonRelease(&ev);
        }
    }
}

void KisView::canvasGotDoubleClickEvent(KisDoubleClickEvent *e)
{
#if defined(EXTENDED_X11_TABLET_SUPPORT)
    // The event filter doesn't see tablet events going to the canvas.
    if (e->device() != KisInputDevice::mouse()) {
        m_tabletEventTimer.start();
    }
#endif // EXTENDED_X11_TABLET_SUPPORT

    if (e->device() != currentInputDevice()) {
        if (e->device() == KisInputDevice::mouse()) {
            if (m_tabletEventTimer.elapsed() > MOUSE_CHANGE_EVENT_DELAY) {
                setInputDevice(KisInputDevice::mouse());
            }
        } else {
            setInputDevice(e->device());
        }
    }

    if (e->device() == currentInputDevice() && m_toolManager->currentTool()) {
        KisPoint p = viewToWindow(e->pos());
        KisDoubleClickEvent ev(e->device(), p, e->globalPos(), e->pressure(), e->xTilt(), e->yTilt(), e->button(), e->state());

        if (m_toolManager->currentTool()) {
            m_toolManager->currentTool()->doubleClick(&ev);
        }
    }
}

void KisView::canvasGotEnterEvent(QEvent *e)
{
}

void KisView::canvasGotLeaveEvent (QEvent *e)
{
}

void KisView::canvasGotMouseWheelEvent(QWheelEvent *event)
{
    //if(event->state() == ControlButton )
    //{
        if(event->delta() / 120 != 0)
        {
            if(event->delta() > 0)
            {
                zoomIn();
            } else {
                zoomOut();
            }
	    if (m_oldTool) {
                KisCanvasPainter gc(m_canvas);
                m_oldTool->paint(gc);
            }
        }
    //} else {
    //    QApplication::sendEvent(m_vScroll, event);
    //}
}

void KisView::canvasGotKeyPressEvent(QKeyEvent *event)
{
    if (!m_toolManager->currentTool()) {
        event->ignore();
        return;
    }

    if (event->key() == Qt::Key_Space) {
        if (!m_panning) {
            // Set tool temporarily to pan
            m_panning = true;
            m_oldTool = m_toolManager->currentTool();
            m_toolManager->setCurrentTool( "tool_pan" );
        }
        else {
            // Unset panning
            m_panning = false;
            m_toolManager->setCurrentTool( m_oldTool );
            m_oldTool = 0;
        }
    }
    if (m_toolManager->currentTool())
        m_toolManager->currentTool()->keyPress(event);
}

void KisView::canvasGotKeyReleaseEvent(QKeyEvent *event)
{
    if (m_toolManager->currentTool())
        m_toolManager->currentTool()->keyRelease(event);
}

void KisView::canvasGotDragEnterEvent(QDragEnterEvent *event)
{
    bool accept = false;

    // Only accept drag if we're not busy, particularly as we may
    // be showing a progress bar and calling qApp->processEvents().
    if (KURLDrag::canDecode(event) && QApplication::overrideCursor() == 0) {
        accept = true;
    }

    event->accept(accept);
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
                            shell()->openDocument(url);
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
    if (currentImg() && currentImg()->activeLayer())
        showLayerProperties(currentImg()->activeLayer());
}

namespace {
    class KisChangeFilterCmd : public KNamedCommand {
        typedef KNamedCommand super;

        public:
            // The QStrings are the _serialized_ configs
            KisChangeFilterCmd(KisAdjustmentLayerSP layer,
                               KisFilterConfiguration* config,
                               const QString& before,
                               const QString& after) : super(i18n("Change Filter"))
            {
                m_layer = layer;
                m_config = config;
                m_before = before;
                m_after = after;
            }
        public:
            virtual void execute()
            {
                QApplication::setOverrideCursor(KisCursor::waitCursor());
                m_config->fromXML(m_after);
                //Q_ASSERT(m_after == m_config->toString());
                m_layer->setFilter(m_config);
                m_layer->setDirty();
                QApplication::restoreOverrideCursor();
            }

            virtual void unexecute()
            {
                QApplication::setOverrideCursor(KisCursor::waitCursor());
                m_config->fromXML(m_before);
                //Q_ASSERT(m_before == m_config->toString());
                m_layer->setFilter(m_config);
                m_layer->setDirty();
                QApplication::restoreOverrideCursor();
            }
        private:
            KisAdjustmentLayerSP m_layer;
            KisFilterConfiguration* m_config;
            QString m_before;
            QString m_after;
    };
}

void KisView::showLayerProperties(KisLayerSP layer)
{
    Q_ASSERT( layer );
    if ( !layer ) return;

    KisColorSpace * cs = 0;
    KisPaintLayer * pl = dynamic_cast<KisPaintLayer*>( layer.data() );
    if ( pl ) {
        cs = pl->paintDevice()->colorSpace();
    }
    else {
        cs = layer->image()->colorSpace();
    }


    if (KisAdjustmentLayerSP alayer = dynamic_cast<KisAdjustmentLayer*>(layer.data()))
    {
        KisDlgAdjLayerProps dlg(alayer, alayer->name(), i18n("Adjustment Layer Properties"), this, "dlgadjlayerprops");
        QString before = dlg.filterConfiguration()->toString();
        if (dlg.exec() == QDialog::Accepted)
        {
            KisChangeFilterCmd * cmd = new KisChangeFilterCmd(alayer,
                    dlg.filterConfiguration(),
                    before,
                    dlg.filterConfiguration()->toString());
            cmd->execute();
            m_adapter->addCommand(cmd);
            m_doc->setModified( true );
        }
    }
    else
    {
        KisDlgLayerProperties dlg(layer->name(),
                                  layer->opacity(),
                                  layer->compositeOp(),
                                  cs);
        if (dlg.exec() == QDialog::Accepted)
        {
            if (layer->name() != dlg.getName() ||
                layer->opacity() != dlg.getOpacity() ||
                layer->compositeOp() != dlg.getCompositeOp())
            {
                QApplication::setOverrideCursor(KisCursor::waitCursor());
                m_adapter->beginMacro(i18n("Property Changes"));
                layer->image()->setLayerProperties(layer, dlg.getOpacity(), dlg.getCompositeOp(), dlg.getName());
                layer->setDirty();
                m_adapter->endMacro();
                QApplication::restoreOverrideCursor();
                m_doc->setModified( true );
            }
        }
    }
}

void KisView::layerAdd()
{
    KisImageSP img = currentImg();
    if (img && img->activeLayer()) {
        addLayer(img->activeLayer()->parent(), img->activeLayer());
    }
    else if (img)
        addLayer(static_cast<KisGroupLayer*>(img->rootLayer().data()), 0);
}

void KisView::addLayer(KisGroupLayerSP parent, KisLayerSP above)
{
    KisImageSP img = currentImg();
    if (img) {
        KisConfig cfg;
        QString profilename;
        if(img->colorSpace()->getProfile())
            profilename = img->colorSpace()->getProfile()->productName();
        NewLayerDialog dlg(img->colorSpace()->id(), profilename, img->nextLayerName(), this);

        if (dlg.exec() == QDialog::Accepted) {
            KisColorSpace* cs = KisMetaRegistry::instance()-> csRegistry() ->
                    getColorSpace(dlg.colorSpaceID(),dlg.profileName());
            KisLayerSP layer = new KisPaintLayer(img, dlg.layerName(), dlg.opacity(), cs);
            if (layer) {
                layer->setCompositeOp(dlg.compositeOp());
                img->addLayer(layer, parent.data(), above);
                updateCanvas();
            } else {
                KMessageBox::error(this, i18n("Could not add layer to image."), i18n("Layer Error"));
            }
        }
        else {
            img->rollBackLayerName();
        }
    }
}

void KisView::addGroupLayer(KisGroupLayerSP parent, KisLayerSP above)
{
    KisImageSP img = currentImg();
    if (img) {
        QString profilename;
        if(img->colorSpace()->getProfile())
            profilename = img->colorSpace()->getProfile()->productName();
        KisConfig cfg;
        NewLayerDialog dlg(img->colorSpace()->id(), profilename, img->nextLayerName(), this);
        dlg.setColorSpaceEnabled(false);

        if (dlg.exec() == QDialog::Accepted) {
            KisLayerSP layer = new KisGroupLayer(img, dlg.layerName(), dlg.opacity());
            if (layer) {
                layer->setCompositeOp(dlg.compositeOp());
                img->addLayer(layer, parent.data(), above);
                updateCanvas();
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

    addPartLayer(img->rootLayer(), img->rootLayer()->firstChild(), m_actionPartLayer->documentEntry());
}

void KisView::addPartLayer(KisGroupLayerSP parent, KisLayerSP above, const KoDocumentEntry& entry)
{
    delete m_partHandler; // Only one at a time
    m_partHandler = new KisPartLayerHandler(this, entry, parent, above);

    disconnect(m_canvas, SIGNAL(sigGotButtonPressEvent(KisButtonPressEvent*)), this, 0);
    disconnect(m_canvas, SIGNAL(sigGotButtonReleaseEvent(KisButtonReleaseEvent*)), this, 0);
    disconnect(m_canvas, SIGNAL(sigGotMoveEvent(KisMoveEvent*)), this, 0);
    disconnect(m_canvas, SIGNAL(sigGotKeyPressEvent(QKeyEvent*)), this, 0);

    connect(m_canvas, SIGNAL(sigGotButtonPressEvent(KisButtonPressEvent*)),
            m_partHandler, SLOT(gotButtonPressEvent(KisButtonPressEvent*)));
    connect(m_canvas, SIGNAL(sigGotButtonReleaseEvent(KisButtonReleaseEvent*)),
            m_partHandler, SLOT(gotButtonReleaseEvent(KisButtonReleaseEvent*)));
    connect(m_canvas, SIGNAL(sigGotMoveEvent(KisMoveEvent*)),
            m_partHandler, SLOT(gotMoveEvent(KisMoveEvent*)));
    connect(m_canvas, SIGNAL(sigGotKeyPressEvent(QKeyEvent*)),
            m_partHandler, SLOT(gotKeyPressEvent(QKeyEvent*)));

    connect(m_partHandler, SIGNAL(sigGotMoveEvent(KisMoveEvent*)),
            this, SLOT(canvasGotMoveEvent(KisMoveEvent*)));
    connect(m_partHandler, SIGNAL(sigGotKeyPressEvent(QKeyEvent*)),
            this, SLOT(canvasGotKeyPressEvent(QKeyEvent*)));
    connect(m_partHandler, SIGNAL(handlerDone()),
            this, SLOT(reconnectAfterPartInsert()));
}

void KisView::insertPart(const QRect& viewRect, const KoDocumentEntry& entry,
                         KisGroupLayerSP parent, KisLayerSP above) {
    KisImageSP img = currentImg();
    if (!img) return;

    KoDocument* doc = entry.createDoc(m_doc);
    if ( !doc )
        return;

    if ( !doc->showEmbedInitDialog(this) )
        return;

    QRect rect = viewToWindow(viewRect);

    KisChildDoc * childDoc = m_doc->createChildDoc(rect, doc);

    KisPartLayerImpl* partLayer = new KisPartLayerImpl(img, childDoc);
    partLayer->setDocType(entry.service()->genericName());
    img->addLayer(partLayer, parent, above);
    m_doc->setModified(true);

    reconnectAfterPartInsert();
}

void KisView::reconnectAfterPartInsert() {
    connect(m_canvas, SIGNAL(sigGotButtonPressEvent(KisButtonPressEvent*)),
            this, SLOT(canvasGotButtonPressEvent(KisButtonPressEvent*)));
    connect(m_canvas, SIGNAL(sigGotButtonReleaseEvent(KisButtonReleaseEvent*)),
            this, SLOT(canvasGotButtonReleaseEvent(KisButtonReleaseEvent*)));
    connect(m_canvas, SIGNAL(sigGotMoveEvent(KisMoveEvent*)),
            this, SLOT(canvasGotMoveEvent(KisMoveEvent*)));
    connect(m_canvas, SIGNAL(sigGotKeyPressEvent(QKeyEvent*)),
            this, SLOT(canvasGotKeyPressEvent(QKeyEvent*)));

    delete m_partHandler;
    m_partHandler = 0;
}

void KisView::addAdjustmentLayer()
{
    KisImageSP img = currentImg();
    if (!img) return;

    addAdjustmentLayer( img->activeLayer()->parent(), img->activeLayer() );
}

void KisView::addAdjustmentLayer(KisGroupLayerSP parent, KisLayerSP above)
{
    Q_ASSERT(parent);
    Q_ASSERT(above);

    KisImageSP img = currentImg();
    if (!img) return;

    KisLayerSP l = img->activeLayer();

    KisPaintDeviceSP dev;

    //  Argh! I hate having to cast, cast and cast again to see what kind of a layer I've got!
    KisPaintLayer * pl = dynamic_cast<KisPaintLayer*>(l.data());
    if (pl) {
        dev = pl->paintDevice();
    }
    else {
        KisGroupLayer * gl = dynamic_cast<KisGroupLayer*>(l.data());
        if (gl) {
            dev = gl->projection(img->bounds());
        }
        else {
            KisAdjustmentLayer * al = dynamic_cast<KisAdjustmentLayer*>(l.data());
            if (al) {
                dev = al->cachedPaintDevice();
            }
            else {
                return;
            }
        }
    }

    KisDlgAdjustmentLayer dlg(img, img->nextLayerName(), i18n("New Adjustment Layer"), this, "dlgadjustmentlayer");
    if (dlg.exec() == QDialog::Accepted) {
        KisSelectionSP selection = 0;
        if (dev->hasSelection()) {
            selection = dev->selection();
        }
        KisFilterConfiguration * filter = dlg.filterConfiguration();
        QString name = dlg.layerName();

        addAdjustmentLayer( parent, above, name, filter, selection);

    }
}

void KisView::addAdjustmentLayer(KisGroupLayerSP parent, KisLayerSP above, const QString & name,
                                 KisFilterConfiguration * filter, KisSelectionSP selection)
{
    Q_ASSERT(parent);
    Q_ASSERT(above);
    Q_ASSERT(filter);

    KisImageSP img = currentImg();
    if (!img) return;

    KisAdjustmentLayer * l = new KisAdjustmentLayer(img, name, filter, selection);
    img->addLayer(l, parent, above);
}

void KisView::slotChildActivated(bool a) {
    // It should be so that the only part (child) we can activate, is the current layer:
    if (currentImg() && currentImg()->activeLayer())
    {
        if (a) {
            currentImg()->activeLayer()->activate();
        } else {
            currentImg()->activeLayer()->deactivate();
        }
    }

    super::slotChildActivated(a);
}

void KisView::layerRemove()
{
    KisImageSP img = currentImg();

    if (img) {
        KisLayerSP layer = img->activeLayer();

        if (layer) {


            img->removeLayer(layer);

            if (layer->parent())
                layer->parent()->setDirty(layer->extent());

            updateCanvas();
            layerUpdateGUI(img->activeLayer() != 0);
        }
    }
}

void KisView::layerDuplicate()
{
    KisImageSP img = currentImg();

    if (!img)
        return;

    KisLayerSP active = img->activeLayer();

    if (!active)
        return;

    KisLayerSP dup = active->clone();
    dup->setName(i18n("Duplicate of '%1'").arg(active->name()));
    img->addLayer(dup, active->parent().data(), active);
    if (dup) {
        img->activate( dup );
        updateCanvas();
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

    layer = img->activeLayer();

    img->raiseLayer(layer);
}

void KisView::layerLower()
{
    KisImageSP img = currentImg();
    KisLayerSP layer;

    if (!img)
        return;

    layer = img->activeLayer();

    img->lowerLayer(layer);
}

void KisView::layerFront()
{
    KisImageSP img = currentImg();
    KisLayerSP layer;

    if (!img)
        return;

    layer = img->activeLayer();
    img->toTop(layer);
}

void KisView::layerBack()
{
    KisImageSP img = currentImg();
    if (!img) return;

    KisLayerSP layer;

    layer = img->activeLayer();
    img->toBottom(layer);
}

void KisView::layersUpdated()
{
    KisImageSP img = currentImg();
    if (!img) return;

    KisLayerSP layer = img->activeLayer();

    layerUpdateGUI(img && layer);

    notifyObservers();
}

void KisView::layerToggleVisible()
{
    KisImageSP img = currentImg();
    if (!img) return;

    KisLayerSP layer = img->activeLayer();
    if (!layer) return;

    layer->setVisible(!layer->visible());
}

void KisView::layerToggleLocked()
{
    KisImageSP img = currentImg();
    if (!img) return;

    KisLayerSP layer = img->activeLayer();
    if (!layer) return;

    layer->setLocked(!layer->locked());
}

void KisView::actLayerVisChanged(int show)
{
    m_actLayerVis = (show != 0);
}

bool KisView::activeLayerHasSelection()
{
    return m_image && m_image->activeDevice() && m_image->activeDevice()->hasSelection();
}

void KisView::scrollH(int value)
{
    m_hRuler->updateVisibleArea(value, 0);

    int xShift = m_scrollX - value;
    m_scrollX = value;

    if (m_canvas->isUpdatesEnabled()) {
        if (xShift > 0) {

            if (m_canvas->isOpenGLCanvas()) {
                paintOpenGLView(QRect(0, 0, m_canvas->width(), m_canvas->height()));
            } else {
                QRect drawRect(0, 0, xShift, m_canvasPixmap.height());

                bitBlt(&m_canvasPixmap, xShift, 0, &m_canvasPixmap, 0, 0, m_canvasPixmap.width() - xShift, m_canvasPixmap.height());

                updateQPaintDeviceCanvas(viewToWindow(drawRect));
                m_canvas->repaint();
            }
        } else if (xShift < 0) {

            QRect drawRect(m_canvasPixmap.width() + xShift, 0, -xShift, m_canvasPixmap.height());

            if (m_canvas->isOpenGLCanvas()) {
                paintOpenGLView(QRect(0, 0, m_canvas->width(), m_canvas->height()));
            } else {
                bitBlt(&m_canvasPixmap, 0, 0, &m_canvasPixmap, -xShift, 0, m_canvasPixmap.width() + xShift, m_canvasPixmap.height());

                updateQPaintDeviceCanvas(viewToWindow(drawRect));
                m_canvas->repaint();
            }
        }
	if (m_oldTool) {
            KisCanvasPainter gc(m_canvas);
            m_oldTool->paint(gc);
        }
    }

    if (xShift != 0) {
        // XXX do sth with the childframe or so
    }
    emit viewTransformationsChanged();
}

void KisView::scrollV(int value)
{
    m_vRuler->updateVisibleArea(0, value);

    int yShift = m_scrollY - value;
    m_scrollY = value;

    if (m_canvas->isUpdatesEnabled()) {
        if (yShift > 0) {

            if (m_canvas->isOpenGLCanvas()) {
                paintOpenGLView(QRect(0, 0, m_canvas->width(), m_canvas->height()));
            } else {
                QRect drawRect(0, 0, m_canvasPixmap.width(), yShift);

                bitBlt(&m_canvasPixmap, 0, yShift, &m_canvasPixmap, 0, 0, m_canvasPixmap.width(), m_canvasPixmap.height() - yShift);

                updateQPaintDeviceCanvas(viewToWindow(drawRect));
                m_canvas->repaint();
            }
        } else if (yShift < 0) {

            if (m_canvas->isOpenGLCanvas()) {
                paintOpenGLView(QRect(0, 0, m_canvas->width(), m_canvas->height()));
            } else {
                QRect drawRect(0, m_canvasPixmap.height() + yShift, m_canvasPixmap.width(), -yShift);

                bitBlt(&m_canvasPixmap, 0, 0, &m_canvasPixmap, 0, -yShift, m_canvasPixmap.width(), m_canvasPixmap.height() + yShift);

                updateQPaintDeviceCanvas(viewToWindow(drawRect));
                m_canvas->repaint();
            }
        }
	if (m_oldTool) {
            KisCanvasPainter gc(m_canvas);
            m_oldTool->paint(gc);
        }
    }

    if (yShift != 0) {
        // XXX do sth with the childframe or so
    }
    emit viewTransformationsChanged();
}

void KisView::setupCanvas()
{
    m_canvas = new KisCanvas(this, "kis_canvas");
    m_canvas->setFocusPolicy( QWidget::StrongFocus );
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
    if (m_image) {
        connect(m_image, SIGNAL(sigActiveSelectionChanged(KisImageSP)), m_selectionManager, SLOT(imgSelectionChanged(KisImageSP)));
        connect(m_image, SIGNAL(sigActiveSelectionChanged(KisImageSP)), this, SLOT(updateCanvas()));
        connect(m_image, SIGNAL(sigColorSpaceChanged(KisColorSpace *)), this, SLOT(updateStatusBarProfileLabel()));
        connect(m_image, SIGNAL(sigProfileChanged(KisProfile * )), SLOT(profileChanged(KisProfile * )));

        connect(m_image, SIGNAL(sigLayersChanged(KisGroupLayerSP)), SLOT(layersUpdated()));
        connect(m_image, SIGNAL(sigMaskInfoChanged()), SLOT(maskUpdated()));
        connect(m_image, SIGNAL(sigLayerAdded(KisLayerSP)), SLOT(layersUpdated()));
        connect(m_image, SIGNAL(sigLayerRemoved(KisLayerSP, KisGroupLayerSP, KisLayerSP)), SLOT(layersUpdated()));
        connect(m_image, SIGNAL(sigLayerMoved(KisLayerSP, KisGroupLayerSP, KisLayerSP)), SLOT(layersUpdated()));
        connect(m_image, SIGNAL(sigLayerActivated(KisLayerSP)), SLOT(layersUpdated()));
        connect(m_image, SIGNAL(sigLayerActivated(KisLayerSP)), SLOT(updateCanvas()));
        connect(m_image, SIGNAL(sigLayerPropertiesChanged(KisLayerSP)), SLOT(layersUpdated()));

        KisConnectPartLayerVisitor v(m_image, this, true);
        m_image->rootLayer()->accept(v);
        connect(m_image, SIGNAL(sigLayerAdded(KisLayerSP)),
                SLOT(handlePartLayerAdded(KisLayerSP)));

        maskUpdated();
#ifdef HAVE_GL
        if (m_OpenGLImageContext != 0) {
            connect(m_OpenGLImageContext, SIGNAL(sigImageUpdated(QRect)), SLOT(slotOpenGLImageUpdated(QRect)));
            connect(m_OpenGLImageContext, SIGNAL(sigSizeChanged(Q_INT32, Q_INT32)), SLOT(slotImageSizeChanged(Q_INT32, Q_INT32)));
        } else
#endif
        {
            connect(m_image, SIGNAL(sigImageUpdated(QRect)), SLOT(imgUpdated(QRect)));
            connect(m_image, SIGNAL(sigSizeChanged(Q_INT32, Q_INT32)), SLOT(slotImageSizeChanged(Q_INT32, Q_INT32)));
        }
    }

    m_layerBox->setImage(m_image);
    m_birdEyeBox->setImage(m_image);
}

void KisView::disconnectCurrentImg()
{
    if (m_image) {
        m_image->disconnect(this);
        m_layerBox->setImage(0);
        m_birdEyeBox->setImage(0);

        KisConnectPartLayerVisitor v(m_image, this, false);
        m_image->rootLayer()->accept(v);
    }

#ifdef HAVE_GL
    if (m_OpenGLImageContext != 0) {
        m_OpenGLImageContext->disconnect(this);
    }
#endif
}

void KisView::handlePartLayerAdded(KisLayerSP layer)
{
    KisPartLayer* l = dynamic_cast<KisPartLayer*>(layer.data());
    if (!l)
        return;

    connect(this, SIGNAL(childActivated(KoDocumentChild*)),
            layer, SLOT(childActivated(KoDocumentChild*)));
}

void KisView::imgUpdated(QRect rc)
{
    updateCanvas(rc);
}

void KisView::slotOpenGLImageUpdated(QRect rc)
{
    paintOpenGLView(windowToView(rc));
}

void KisView::profileChanged(KisProfile *  /*profile*/)
{
    updateStatusBarProfileLabel();
}

void KisView::slotImageSizeChanged(Q_INT32 /*w*/, Q_INT32 /*h*/)
{
    resizeEvent(0);
    refreshKisCanvas();
}

void KisView::resizeCurrentImage(Q_INT32 w, Q_INT32 h, bool cropLayers)
{
    if (!currentImg()) return;

    currentImg()->resize(w, h, cropLayers);
    m_doc->setModified(true);
    layersUpdated();
}

void KisView::scaleCurrentImage(double sx, double sy, KisFilterStrategy *filterStrategy)
{
    if (!currentImg()) return;
    currentImg()->scale(sx, sy, m_progress, filterStrategy);
    m_doc->setModified(true);
    layersUpdated();
}

void KisView::rotateCurrentImage(double angle)
{
    if (!currentImg()) return;
    currentImg()->rotate(angle, m_progress);
    m_doc->setModified(true);
    layersUpdated();
}

void KisView::shearCurrentImage(double angleX, double angleY)
{
    if (!currentImg()) return;
    currentImg()->shear(angleX, angleY, m_progress);
    m_doc->setModified(true);
    layersUpdated();
}


QPoint KisView::viewToWindow(const QPoint& pt)
{
    QPoint converted;

    converted.rx() = static_cast<int>((pt.x() + horzValue()) / zoom());
    converted.ry() = static_cast<int>((pt.y() + vertValue()) / zoom());

    return converted;
}

QPoint KisView::viewToWindow(const QPoint& pt) const
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

    r.setTopLeft(viewToWindow(rc.topLeft()));
    r.setRight((int)(ceil((rc.right() + 1.0 + horzValue()) / zoom()) - 1));
    r.setBottom((int)(ceil((rc.bottom() + 1.0 + vertValue()) / zoom()) - 1));

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

QPoint KisView::windowToView(const QPoint& pt) const
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

    r.setTopLeft(windowToView(rc.topLeft()));
    r.setRight((int)(ceil((rc.right() + 1.0) * zoom()) - horzValue() - 1));
    r.setBottom((int)(ceil((rc.bottom() + 1.0) * zoom()) - vertValue() - 1));

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
    Q_ASSERT(event);

    if (event->activated()) {

        KStatusBar *sb = statusBar();
        if (sb) {
            sb->show();
        }

        if (!m_guiActivateEventReceived) {
            m_guiActivateEventReceived = true;
            startInitialZoomTimerIfReady();
        }
    }

    super::guiActivateEvent(event);
}

bool KisView::eventFilter(QObject *o, QEvent *e)
{
    Q_ASSERT(o);
    Q_ASSERT(e);

    switch (e->type()) {
    case QEvent::TabletMove:
    case QEvent::TabletPress:
    case QEvent::TabletRelease:
    {
        QTabletEvent *te = static_cast<QTabletEvent *>(e);
        KisInputDevice device;

        switch (te->device()) {
        default:
        case QTabletEvent::Stylus:
        case QTabletEvent::NoDevice:
            device = KisInputDevice::stylus();
            break;
        case QTabletEvent::Puck:
            device = KisInputDevice::puck();
            break;
        case QTabletEvent::Eraser:
            device = KisInputDevice::eraser();
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
    {
#ifdef EXTENDED_X11_TABLET_SUPPORT
        KisInputDevice device = KisCanvasWidget::findActiveInputDevice();

        if (device != KisInputDevice::mouse()) {
            setInputDevice(device);
            m_tabletEventTimer.start();
        } else
#endif
        {
            if (currentInputDevice() != KisInputDevice::mouse() && m_tabletEventTimer.elapsed() > MOUSE_CHANGE_EVENT_DELAY) {
                setInputDevice(KisInputDevice::mouse());
            }
        }
        break;
    }
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    {
        if (m_canvas->cursorIsOverCanvas()) {
            m_canvas->handleKeyEvent(e);
            return true;
        }
        break;
    }
#if 0
    // This code is unnecessary now that there's an application event filter
    // This eventFilter is called for all widgets already, no need to install event filters multiple times
    // Even worse: with multiple views, they would all install event filters on each other's widgets,
    // due to the qapp event filter triggering in all views!
    case QEvent::ChildInserted:
    {
        QChildEvent *childEvent = static_cast<QChildEvent *>(e);
        QObject *child = childEvent->child();
        if ( child->isWidgetType() ) {

            child->installEventFilter(this);

            QObjectList *objectList = child->queryList("QWidget");
            QObjectListIt it(*objectList);
            QObject *obj;
            while ((obj = it.current()) != 0) {
                obj->installEventFilter(this);
                ++it;
            }
            delete objectList;
        }
    }
#endif
    default:
        // Ignore
        break;
    }

#if 0
    if ((o == m_hRuler || o == m_vRuler) && (e->type() == QEvent::MouseMove || e->type() == QEvent::MouseButtonRelease)) {
        QMouseEvent *me = dynamic_cast<QMouseEvent*>(e);
        QPoint pt = mapFromGlobal(me->globalPos());
        KisImageSP img = currentImg();
        KisGuideMgr *mgr;

        if (!img)
            return super::eventFilter(o, e);

        mgr = img->guides();

        if (e->type() == QEvent::MouseMove && (me->state() & Qt::LeftButton)) {
            bool flag = geometry().contains(pt);
            KisGuideSP gd;

            if (m_currentGuide == 0 && flag) {
                // No guide is being edited and moving mouse over the canvas.
                // Create a new guide.
                enterEvent(0);
                eraseGuides();
                mgr->unselectAll();

                if (o == m_vRuler)
                    gd = mgr->add((pt.x() - m_vRuler->width() + horzValue()) / zoom(), Qt::Vertical);
                else
                    gd = mgr->add((pt.y() - m_hRuler->height() + vertValue()) / zoom(), Qt::Horizontal);

                m_currentGuide = gd;
                mgr->select(gd);
                m_lastGuidePoint = mapToScreen(pt);
            } else if (m_currentGuide) {
                if (flag) {
                    // moved an existing guide.
                    KisMoveEvent kme(currentInputDevice(), pt, me->globalPos(), PRESSURE_DEFAULT, 0, 0, me->state());
                    canvasGotMoveEvent(&kme);
                } else {
                    //  moved a guide out of the frame, destroy it
                    leaveEvent(0);
                    eraseGuides();
                    mgr->remove(m_currentGuide);
                    paintGuides();
                    m_currentGuide = 0;
                }
            }
        } else if (e->type() == QEvent::MouseButtonRelease && m_currentGuide) {
            eraseGuides();
            mgr->unselect(m_currentGuide);
            paintGuides();
            m_currentGuide = 0;
            enterEvent(0);
            KisMoveEvent kme(currentInputDevice(), pt, me->globalPos(), PRESSURE_DEFAULT, 0, 0, Qt::NoButton);
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
        KisGuideMgr *mgr = img->guides();

        if (mgr)
            mgr->erase(&m_canvasPixmap, this, horzValue(), vertValue(), zoom());
    }
}

void KisView::paintGuides()
{
    KisImageSP img = currentImg();

    if (img) {
        KisGuideMgr *mgr = img->guides();

        if (mgr)
            mgr->paint(&m_canvasPixmap, this, horzValue(), vertValue(), zoom());
    }
}

void KisView::updateGuides()
{
    eraseGuides();
    paintGuides();
}
#endif

//void KisView::viewGuideLines()
//{
//}

QPoint KisView::mapToScreen(const QPoint& pt)
{
    QPoint converted;

    converted.rx() = pt.x() + horzValue();
    converted.ry() = pt.y() + vertValue();
    return converted;
}

void KisView::attach(KisCanvasObserver *observer)
{
    Q_ASSERT(observer);
    if (observer)
        m_observers.push_back(observer);
}

void KisView::detach(KisCanvasObserver *observer)
{
    Q_ASSERT(observer);
    if (observer) {
        vKisCanvasObserver_it it = std::find(m_observers.begin(), m_observers.end(), observer);

        if (it != m_observers.end())
            m_observers.erase(it);
    }
}

void KisView::notifyObservers()
{
    for (vKisCanvasObserver_it it = m_observers.begin(); it != m_observers.end(); ++it) {
        (*it)->update(this);
    }
}

KisImageSP KisView::currentImg() const
{
    return m_image;
}

void KisView::setCurrentImage(KisImageSP image)
{
    if(!image) return;

    disconnectCurrentImg();
    m_image = image;

    KisConfig cfg;

#ifdef HAVE_GL
    if (cfg.useOpenGL()) {
        m_OpenGLImageContext = KisOpenGLImageContext::getImageContext(image, monitorProfile());
        m_canvas->createOpenGLCanvas(m_OpenGLImageContext->sharedContextWidget());
    }
#endif
    connectCurrentImg();
    m_layerBox->setImage(currentImg());

    zoomAroundPoint(0, 0, 1.0);

    if (!currentImg())
        layersUpdated();

    imgUpdateGUI();

    image->blockSignals(false);
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

const KisPaintOpSettings *KisView::currentPaintopSettings() const
{
    return m_paintopSettings;
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
    QCursor oldCursor = m_canvas->cursor();
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

    m_canvas->setCursor(newCursor);
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
    m_birdEyeBox->setCaption(i18n("Overview"));
    m_paletteManager->addWidget( m_birdEyeBox, "birdeyebox", krita::CONTROL_PALETTE);

    m_hsvwidget = new KoHSVWidget(this, "hsv");
    m_hsvwidget->setCaption(i18n("HSV"));

    connect(m_hsvwidget, SIGNAL(sigFgColorChanged(const QColor &)), this, SLOT(slotSetFGQColor(const QColor &)));
    connect(m_hsvwidget, SIGNAL(sigBgColorChanged(const QColor &)), this, SLOT(slotSetBGQColor(const QColor &)));
    connect(this, SIGNAL(sigFGQColorChanged(const QColor &)), m_hsvwidget, SLOT(setFgColor(const QColor &)));
    connect(this, SIGNAL(sigBGQColorChanged(const QColor &)), m_hsvwidget, SLOT(setBgColor(const QColor &)));
    m_paletteManager->addWidget( m_hsvwidget, "hsvwidget", krita::COLORBOX, 0, PALETTE_DOCKER, true);

    m_rgbwidget = new KoRGBWidget(this, "rgb");
    m_rgbwidget->setCaption(i18n("RGB"));
    connect(m_rgbwidget, SIGNAL(sigFgColorChanged(const QColor &)), this, SLOT(slotSetFGQColor(const QColor &)));
    connect(m_rgbwidget, SIGNAL(sigBgColorChanged(const QColor &)), this, SLOT(slotSetBGQColor(const QColor &)));
    connect(this, SIGNAL(sigFGQColorChanged(const QColor &)), m_rgbwidget, SLOT(setFgColor(const QColor &)));
    connect(this, SIGNAL(sigBGQColorChanged(const QColor &)), m_rgbwidget, SLOT(setBgColor(const QColor &)));
    m_paletteManager->addWidget( m_rgbwidget, "rgbwidget", krita::COLORBOX);

    m_graywidget = new KoGrayWidget(this, "gray");
    m_graywidget->setCaption(i18n("Gray"));
    connect(m_graywidget, SIGNAL(sigFgColorChanged(const QColor &)), this, SLOT(slotSetFGQColor(const QColor &)));
    connect(m_graywidget, SIGNAL(sigBgColorChanged(const QColor &)), this, SLOT(slotSetBGQColor(const QColor &)));
    connect(this, SIGNAL(sigFGQColorChanged(const QColor &)), m_graywidget, SLOT(setFgColor(const QColor &)));
    connect(this, SIGNAL(sigBGQColorChanged(const QColor &)), m_graywidget, SLOT(setBgColor(const QColor &)));
    m_paletteManager->addWidget( m_graywidget, "graywidget", krita::COLORBOX);

    //make sure the color chooser get right default values
    emit sigFGQColorChanged(m_fg.toQColor());
    emit sigBGQColorChanged(m_bg.toQColor());

    m_palettewidget = new KisPaletteWidget(this);
    m_palettewidget->setCaption(i18n("Palettes"));
    connect(m_palettewidget, SIGNAL(colorSelected(const QColor &)),
            this, SLOT(slotSetFGQColor(const QColor &)));
    // No BGColor or reverse slotFGChanged->palette connections, since that's not useful here

    KisResourceServerBase* rServer;
    rServer = KisResourceServerRegistry::instance()->get("PaletteServer");
    QValueList<KisResource*> resources = rServer->resources();
    QValueList<KisResource*>::iterator it;
    for ( it = resources.begin(); it != resources.end(); ++it ) {
        m_palettewidget->slotAddPalette( *it );
    }
    connect(m_palettewidget, SIGNAL(colorSelected(const KisColor &)), this, SLOT(slotSetFGColor(const KisColor &)));
    m_paletteManager->addWidget( m_palettewidget, "palettewidget", krita::COLORBOX, 10, PALETTE_DOCKER, true);
}

QPoint KisView::applyViewTransformations(const QPoint& p) const {
    QPoint point(windowToView(p));

    if (m_hRuler->isShown())
        point.ry() += m_hRuler->height();
    if (m_vRuler -> isShown())
        point.rx() += m_vRuler->width();

    return point;
}

QPoint KisView::reverseViewTransformations(const QPoint& p) const {
    // Since we now zoom ourselves, the only thing super::~ does is nothing anymore.
    // Hence, zoom ourselves, like super would
    // viewToWindow doesn't take the rulers into account, do that ourselves
    QPoint point(p);
    if (m_hRuler -> isShown())
        point.ry() -= m_hRuler -> height();
    if (m_vRuler -> isShown())
        point.rx() -= m_vRuler -> width();

    return viewToWindow(point);
}

void KisView::canvasAddChild(KoViewChild *child) {
    super::canvasAddChild(child);
    connect(this, SIGNAL(viewTransformationsChanged()), child, SLOT(reposition()));
    m_vScroll->raise();
    m_hScroll->raise();
    m_vScroll->raise();
    m_hRuler->raise();
    m_vRuler->raise();
}

void KisView::slotLoadingFinished()
{
    // Set the current image for real now everything is ready to go.
    setCurrentImage(document()->currentImage());
    m_paletteManager->showWidget( "layerbox" );
    m_canvas->show();
    disconnect(document(), SIGNAL(loadingFinished()), this, SLOT(slotLoadingFinished()));

    m_imageLoaded = true;
    startInitialZoomTimerIfReady();
}

void KisView::startInitialZoomTimerIfReady()
{
    if (m_imageLoaded && m_showEventReceived && m_guiActivateEventReceived) {
        m_initialZoomTimer.start(250, true);
    }
}

void KisView::slotInitialZoomTimeout()
{
    Q_ASSERT(!m_paintViewEnabled);

    m_paintViewEnabled = true;
    setInitialZoomLevel();
}


void KisView::slotCreateMask() {
    KisPaintLayer* layer = dynamic_cast<KisPaintLayer*>(currentImg()->activeLayer().data());
    if (!layer)
        return;

    KNamedCommand *cmd = layer->createMaskCommand();
    cmd->execute();
    if (undoAdapter() && undoAdapter()->undo()) {
        undoAdapter()->addCommand(cmd);
    }
}

void KisView::slotMaskFromSelection() {
    KisPaintLayer* layer = dynamic_cast<KisPaintLayer*>(currentImg()->activeLayer().data());
    if (!layer)
        return;

    KNamedCommand *cmd = layer->maskFromSelectionCommand();
    cmd->execute();
    if (undoAdapter() && undoAdapter()->undo()) {
        undoAdapter()->addCommand(cmd);
    }
}

void KisView::slotMaskToSelection() {
    KisPaintLayer* layer = dynamic_cast<KisPaintLayer*>(currentImg()->activeLayer().data());
    if (!layer)
        return;

    KNamedCommand *cmd = layer->maskToSelectionCommand();
    cmd->execute();
    if (undoAdapter() && undoAdapter()->undo()) {
        undoAdapter()->addCommand(cmd);
    }
}

void KisView::slotApplyMask() {
    KisPaintLayer* layer = dynamic_cast<KisPaintLayer*>(currentImg()->activeLayer().data());
    if (!layer)
        return;

    KNamedCommand *cmd = layer->applyMaskCommand();
    cmd->execute();
    if (undoAdapter() && undoAdapter()->undo()) {
        undoAdapter()->addCommand(cmd);
    }
}

void KisView::slotRemoveMask() {
    KisPaintLayer* layer = dynamic_cast<KisPaintLayer*>(currentImg()->activeLayer().data());
    if (!layer)
        return;

    KNamedCommand *cmd = layer->removeMaskCommand();
    cmd->execute();
    if (undoAdapter() && undoAdapter()->undo()) {
        undoAdapter()->addCommand(cmd);
    }
}

void KisView::slotEditMask() {
    KisPaintLayer* layer = dynamic_cast<KisPaintLayer*>(currentImg()->activeLayer().data());
    if (!layer)
        return;

    layer->setEditMask(m_editMask->isChecked());
}

void KisView::slotShowMask() {
    KisPaintLayer* layer = dynamic_cast<KisPaintLayer*>(currentImg()->activeLayer().data());
    if (!layer)
        return;

    layer->setRenderMask(m_showMask->isChecked());
}

void KisView::maskUpdated() {
    KisPaintLayer* layer = dynamic_cast<KisPaintLayer*>(currentImg()->activeLayer().data());
    if (!layer) {
        m_createMask->setEnabled(false);
        m_applyMask->setEnabled(false);
        m_removeMask->setEnabled(false);
        m_editMask->setEnabled(false);
        m_showMask->setEnabled(false);
        return;
    }
    m_createMask->setEnabled(!layer->hasMask());
    m_maskFromSelection->setEnabled(true); // Perhaps also update this to false when no selection?
    m_maskToSelection->setEnabled(layer->hasMask());
    m_applyMask->setEnabled(layer->hasMask());
    m_removeMask->setEnabled(layer->hasMask());

    m_editMask->setEnabled(layer->hasMask());
    m_editMask->setChecked(layer->editMask());
    m_showMask->setEnabled(layer->hasMask());
    m_showMask->setChecked(layer->renderMask());
}

#include "kis_view.moc"


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
// #include <qclipboard.h>
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
#include "kis_filter_registry.h"
#include "kis_guide.h"
#include "kis_gradient.h"
#include "kis_icon_item.h"
#include "kis_image_magick_converter.h"
#include "kis_imagepipe_brush.h"
#include "kis_layer.h"
#include "kis_layerbox.h"
#include "kis_paint_device.h"
#include "kis_paint_device_visitor.h"
#include "kis_painter.h"
#include "kis_resource_mediator.h"
#include "kis_resourceserver.h"
#include "kis_ruler.h"
#include "kis_floatingselection.h"
#include "kis_selection.h"
#include "kis_controlframe.h"
#include "kis_dockframedocker.h"
#include "kis_tool.h"
#include "kis_tool_factory.h"
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
#include "kis_move_event.h"
#include "kis_colorspace_registry.h"

// Widgets
#include "kis_autobrush.h"
#include "kis_text_brush.h"
#include "kis_hsv_widget.h"
#include "kis_rgb_widget.h"
#include "kis_gray_widget.h"

// Dialog boxes
#include "kis_dlg_progress.h"
#include "kis_dlg_gradient.h"
#include "kis_dlg_new_layer.h"
#include "kis_dlg_paint_properties.h"
#include "kis_dlg_transform.h"
#include "kis_dlg_preferences.h"

// Action managers
#include "kis_selection_manager.h"

#define KISVIEW_MIN_ZOOM (1.0 / 16.0)
#define KISVIEW_MAX_ZOOM 16.0

// Time in ms that must pass after a tablet event before a mouse event is allowed to
// change the input device to the mouse. This is needed because mouse events are always
// sent to a receiver if it does not accept the tablet event.
#define MOUSE_CHANGE_EVENT_DELAY 100

KisView::KisView(KisDoc *doc, KisUndoAdapter *adapter, QWidget *parent, const char *name) : super(doc, parent, name)
{
        if (!doc -> isReadWrite())
                setXMLFile("krita_readonly.rc");
        else
                setXMLFile("krita.rc");

	m_inputDevice = INPUT_DEVICE_UNKNOWN;

	m_selectionManager = new KisSelectionManager(this, doc);

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
        m_layerResizeToImage = 0;
        m_layerToImage = 0;
        m_layerTransform = 0;
        m_layerRaise = 0;
        m_layerLower = 0;
        m_layerTop = 0;
        m_layerBottom = 0;

        m_imgRm = 0;
        m_imgDup = 0;
        m_imgImport = 0;
        m_imgExport = 0;
        m_imgScan = 0;
        m_imgResizeToLayer = 0;
        m_imgFlatten = 0;
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

	m_hsvwidget = 0;
	m_rgbwidget = 0;
	m_graywidget = 0;

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
        m_progress = 0;
        m_statusBarZoomLabel = 0;
	m_filterRegistry = new KisFilterRegistry();

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
        connect(m_doc, SIGNAL(currentImageUpdated(KisImageSP)), SLOT(currentImageUpdated(KisImageSP)));
        connect(this, SIGNAL(embeddImage(const QString&)), SLOT(slotEmbedImage(const QString&)));
        connect(m_imgBuilderMgr, SIGNAL(size(Q_INT32)), SLOT(nBuilders(Q_INT32)));

        setupTools();

	setInputDevice(INPUT_DEVICE_MOUSE);

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

	m_colordocker = new DockFrameDocker(this);
	m_colordocker -> setCaption(i18n("Color Manager"));

	m_hsvwidget = new KisHSVWidget(this, "hsv");
	m_hsvwidget -> setCaption(i18n("HSV"));
	m_colordocker -> plug(m_hsvwidget);
	attach(m_hsvwidget);

	m_rgbwidget = new KisRGBWidget(this, "rgb");
	m_rgbwidget -> setCaption(i18n("RGB"));
	m_colordocker -> plug(m_rgbwidget);
	attach(m_rgbwidget);

	m_graywidget = new KisGrayWidget(this, "gray");
	m_graywidget -> setCaption(i18n("Gray"));
	m_colordocker -> plug(m_graywidget);
	attach(m_graywidget);

        m_historydocker = new DockFrameDocker(this);
        m_historydocker -> setCaption(i18n("History/Actions"));


	// Setup all brushes
        m_brushMediator = new KisResourceMediator(MEDIATE_BRUSHES, rserver, i18n("Brushes"),
                                                  m_resourcedocker, "brush_chooser", this);
        m_brush = dynamic_cast<KisBrush*>(m_brushMediator -> currentResource());
        m_resourcedocker -> plug(m_brushMediator -> chooserWidget());
        connect(m_brushMediator, SIGNAL(activatedResource(KisResource*)), this, SLOT(brushActivated(KisResource*)));


	// Setup patterns
        m_patternMediator = new KisResourceMediator(MEDIATE_PATTERNS, rserver, i18n("Patterns"),
                                                    m_resourcedocker, "pattern chooser", this);
        m_pattern = dynamic_cast<KisPattern*>(m_patternMediator -> currentResource());
        m_resourcedocker -> plug(m_patternMediator -> chooserWidget());
        connect(m_patternMediator, SIGNAL(activatedResource(KisResource*)), this, SLOT(patternActivated(KisResource*)));


	// Setup gradients
	m_gradientMediator = new KisResourceMediator(MEDIATE_GRADIENTS, rserver, i18n("Gradients"),
						    m_resourcedocker, "gradient chooser", this);
	m_gradient = dynamic_cast<KisGradient*>(m_gradientMediator -> currentResource());
	m_resourcedocker -> plug(m_gradientMediator -> chooserWidget());
	connect(m_gradientMediator, SIGNAL(activatedResource(KisResource*)), this, SLOT(gradientActivated(KisResource*)));

	// AutoBrush
	m_autobrush = new KisAutobrush(m_resourcedocker, "autobrush", i18n("Autobrush"));
	m_resourcedocker -> plug(m_autobrush);
	connect(m_autobrush, SIGNAL(activatedResource(KisResource*)), this, SLOT(brushActivated(KisResource*)));
	
	// TextBrush
	m_textBrush = new KisTextBrush(m_resourcedocker, "textbrush", i18n("Text Brush"));
	m_resourcedocker -> plug(m_textBrush);
	connect(m_textBrush, SIGNAL(activatedResource(KisResource*)), this, SLOT(brushActivated(KisResource*)));

	// Layers
        m_layerBox = new KisLayerBox(i18n("layer"), KisLayerBox::SHOWALL, m_layerchanneldocker);
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


	// Channels
        m_channelView = new KisChannelView(m_doc, this);
        m_channelView -> setCaption(i18n("Channels"));
        m_layerchanneldocker -> plug(m_channelView);

	// Control box
        m_controlWidget = new ControlFrame(m_toolcontroldocker);
        m_controlWidget -> setCaption(i18n("General"));
        m_toolcontroldocker -> plug(m_controlWidget);

        m_controlWidget -> slotSetBGColor(m_bg);
        m_controlWidget -> slotSetFGColor(m_fg);
        connect(m_controlWidget, SIGNAL(fgColorChanged(const KoColor&)), SLOT(slotSetFGColor(const KoColor&)));
        connect(m_controlWidget, SIGNAL(bgColorChanged(const KoColor&)), SLOT(slotSetBGColor(const KoColor&)));
        connect(this, SIGNAL(fgColorChanged(const KoColor&)), m_controlWidget, SLOT(slotSetFGColor(const KoColor&)));
        connect(this, SIGNAL(bgColorChanged(const KoColor&)), m_controlWidget, SLOT(slotSetBGColor(const KoColor&)));

        // TODO Here should be a better check
        if ( mainWindow() -> isDockEnabled( DockBottom))
        {
                viewControlDocker();
                viewLayerChannelDocker();
                viewResourceDocker();
                viewColorDocker();
                mainWindow() -> setDockEnabled( DockBottom, false);
                mainWindow() -> setDockEnabled( DockTop, false);
        }

}

void KisView::setupScrollBars()
{
	m_scrollX = 0;
	m_scrollY = 0;
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
        // XXX: todo
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
        QString newName = KInputDialog::getText( i18n("Image Name"), i18n("Enter name:"), activeName, &ok, this );

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
                QLabel *lbl;

                lbl = new KisLabelCursorPos(sb);
                connect(this, SIGNAL(cursorPosition(Q_INT32, Q_INT32)), lbl, SLOT(updatePos(Q_INT32, Q_INT32)));
                connect(this, SIGNAL(cursorEnter()), lbl, SLOT(enter()));
                connect(this, SIGNAL(cursorLeave()), lbl, SLOT(leave()));
                addStatusBarItem(lbl, 0);
                m_statusBarZoomLabel = new QLabel(sb);
                addStatusBarItem(m_statusBarZoomLabel, 1);
                updateStatusBarZoomLabel ();
                m_progress = new KisLabelProgress(this);
                m_progress -> setMaximumWidth(225);
                m_progress -> setMaximumHeight(sb -> height());
                addStatusBarItem(m_progress, 2, true);
                m_progress -> hide();
        }
}

void KisView::setupActions()
{
	m_selectionManager -> setup(actionCollection());


        // navigation actions
        KStdAction::redisplay(this, SLOT(canvasRefresh()), actionCollection(), "refresh_canvas");
        (void)new KAction(i18n("Reset Button"), "stop", 0, this, SLOT(reset()), actionCollection(), "panic_button");


	m_fullScreen = KStdAction::fullScreen( NULL, NULL, actionCollection(), this );
	connect( m_fullScreen, SIGNAL( toggled( bool )), this, SLOT( slotUpdateFullScreen( bool )));

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
        m_layerSaveAs = new KAction(i18n("Save Layer as Image..."), 0, this, SLOT(saveLayerAsImage()), actionCollection(), "save_layer_as_image");
        m_layerResizeToImage = new KAction(i18n("Resize Layer to Image"), 0, this, SLOT(layerResizeToImage()), actionCollection(), "resizelayertoowner");
        m_layerToImage = new KAction(i18n("Layer to Image"), 0, this, SLOT(layerToImage()), actionCollection(), "layer_to_image");

        // layer transformations
        m_layerTransform = new KAction(i18n("Scale Layer..."), 0, this, SLOT(layerTransform()), actionCollection(), "transformlayer");
        (void)new KAction(i18n("Rotate &180"), 0, this, SLOT(rotateLayer180()), actionCollection(), "rotateLayer180");
        (void)new KAction(i18n("Rotate &270"), 0, this, SLOT(rotateLayerLeft90()), actionCollection(), "rotateLayerLeft90");
        (void)new KAction(i18n("Rotate &90"), 0, this, SLOT(rotateLayerRight90()), actionCollection(), "rotateLayerRight90");
        (void)new KAction(i18n("Rotate &Custom..."), 0, this, SLOT(rotateLayerCustom()), actionCollection(), "rotateLayerCustom");
        (void)new KAction(i18n("Mirror Along &X Axis"), 0, this, SLOT(mirrorLayerX()), actionCollection(), "mirrorLayerX");
        (void)new KAction(i18n("Mirror Along &Y Axis"), 0, this, SLOT(mirrorLayerY()), actionCollection(), "mirrorLayerY");

        // color actions
        (void)new KAction(i18n("Select Foreground Color..."), 0, this, SLOT(selectFGColor()), actionCollection(), "select_fgColor");
        (void)new KAction(i18n("Select Background Color..."), 0, this, SLOT(selectBGColor()), actionCollection(), "select_bgColor");
        (void)new KAction(i18n("Reverse Foreground/Background Colors"), 0, this, SLOT(reverseFGAndBGColors()), actionCollection(), "reverse_fg_bg");

        // image actions
        m_imgRename = new KAction(i18n("Rename Current Image"), 0, this, SLOT(slotRename()), actionCollection(), "rename_current_image_tab");
        (void)new KAction(i18n("Add New Image..."), 0, this, SLOT(add_new_image_tab()), actionCollection(), "add_new_image_tab");
        m_imgRm = new KAction(i18n("Remove Current Image"), 0, this, SLOT(remove_current_image_tab()), actionCollection(), "remove_current_image_tab");
        m_imgDup = new KAction(i18n("Duplicate Image"), 0, this, SLOT(duplicateCurrentImg()), actionCollection(), "duplicate_image");
        m_imgFlatten = new KAction(i18n("Flatten Image"), 0, this, SLOT(flattenImage()), actionCollection(), "flatten_image");
        m_imgMergeVisible = new KAction(i18n("Merge &Visible Layers"), 0, this, SLOT(mergeVisibleLayers()), actionCollection(), "merge_visible_layers");
        m_imgMergeLinked = new KAction(i18n("Merge &Linked Layers"), 0, this, SLOT(mergeLinkedLayers()), actionCollection(), "merge_linked_layers");

        // setting actions
        KStdAction::preferences(this, SLOT(preferences()), actionCollection(), "preferences");

        //docker actions
        (void)new KAction(i18n( "&Color Manager" ), 0, this, SLOT( viewColorDocker() ), actionCollection(), "view_color_docker" );
        (void)new KAction(i18n( "&Tool Properties" ), 0, this, SLOT( viewControlDocker() ), actionCollection(), "view_control_docker" );
        (void)new KAction(i18n( "&Layers/Channels" ), 0, this, SLOT( viewLayerChannelDocker() ), actionCollection(), "view_layer_docker" );
        (void)new KAction(i18n( "&Brushes/Patterns/Gradients" ), 0, this, SLOT( viewResourceDocker() ), actionCollection(), "view_resource_docker" );

	m_RulerAction = new KToggleAction( i18n( "Show Rulers" ), 0, this, SLOT( showRuler() ), actionCollection(), "view_ruler" );
	m_RulerAction->setChecked( true );
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

	//Check if rulers are visible
	if( m_RulerAction->isChecked() )
		m_canvas -> setGeometry(ruler + lsideW, ruler, drawW, drawH);
	else
		m_canvas -> setGeometry(1 + lsideW, 1, drawW, drawH);
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

	if( m_RulerAction->isChecked() )
	{
		m_hRuler -> show();
		m_vRuler -> show();
	}
}

void KisView::updateReadWrite(bool readwrite)
{
        layerUpdateGUI(readwrite);
        m_selectionManager -> updateGUI(readwrite);
}

void KisView::clearCanvas(const QRect& rc)
{
        QPainter gc(m_canvas);

        gc.eraseRect(rc);
}

void KisView::setCurrentTool(KisTool *tool)
{
	KisTool *oldTool = currentTool();

        if (oldTool)
        {
                if (oldTool -> optionWidget())
                        m_toolcontroldocker -> unplug(oldTool -> optionWidget());
                oldTool -> clear();
        }

        if (tool) {
                m_inputDeviceToolMap[currentInputDevice()] = tool;
                tool -> cursor(m_canvas);

                if(tool -> createOptionWidget(m_toolcontroldocker)){
                        m_toolcontroldocker -> plug(tool -> optionWidget());
                        m_toolcontroldocker -> showPage(tool -> optionWidget());
                }
                m_canvas -> enableMoveEventCompressionHint(dynamic_cast<KisToolNonPaint *>(tool) != NULL);
                notify();
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
			if (oldTool -> optionWidget()) {
				m_toolcontroldocker -> unplug(oldTool -> optionWidget());
			}
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

                                m_doc -> setCurrentImage(img);
                                m_doc -> paintContent(gc, wr, false, 1.0, 1.0);
				m_doc -> setCurrentImage(0);

                                if (currentTool())
                                        currentTool() -> paint(gc, wr);
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
        m_layerResizeToImage -> setEnabled(enable);
        m_layerToImage -> setEnabled(enable);
        m_layerTransform -> setEnabled(enable);
        m_layerRaise -> setEnabled(enable && nlayers > 1 && layerPos);
        m_layerLower -> setEnabled(enable && nlayers > 1 && layerPos != nlayers - 1);
        m_layerTop -> setEnabled(enable && nlayers > 1 && layerPos);
        m_layerBottom -> setEnabled(enable && nlayers > 1 && layerPos != nlayers - 1);
        imgUpdateGUI();
}


void KisView::imgUpdateGUI()
{
        const KisImageSP img = currentImg();
        Q_INT32 n = 0;
        Q_INT32 nvisible = 0;
        Q_INT32 nlinked = 0;

        m_imgRm -> setEnabled(img != 0);
        m_imgDup -> setEnabled(img != 0);
        m_imgExport -> setEnabled(img != 0);
        m_layerAdd -> setEnabled(img != 0);
        m_imgResizeToLayer -> setEnabled(img && img -> activeLayer());

        if (img) {
		n = img -> nlayers();
		nvisible = n - img -> nHiddenLayers();
		nlinked = img -> nLinkedLayers();
        }

        m_imgFlatten -> setEnabled(n > 1);
        m_imgMergeVisible -> setEnabled(nvisible > 1 && nvisible != n);
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
		img -> flatten();

                dst = img -> layer(0);
                Q_ASSERT(dst);

                //m_imgBuilderMgr -> attach(&ib);
                m_progress -> setSubject(&ib, true, true);

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

                if (createLayer && currentImg()) {
                        vKisLayerSP v = img -> layers();
                        KisImageSP current = currentImg();

                        rc += v.size();
			current -> activeLayer() -> removeSelection();

                        for (vKisLayerSP_it it = v.begin(); it != v.end(); it++) {
                                KisLayerSP layer = *it;

                                layer -> setImage(current);
                                layer -> setName(current -> nextLayerName());
                                m_doc -> layerAdd(current, layer, 0);
                                m_layerBox -> setCurrentItem(img -> index(layer));
                        }

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


void KisView::rotateLayer180()
{
        if (!currentImg()) return;
        KisLayerSP layer = currentImg() -> activeLayer();
        if (!layer) return;


        QWMatrix m;
        //m.rotate(180);
        layer->transform(m.rotate(180));
	m_doc -> setModified(true);

        layersUpdated();
        resizeEvent(0);
        updateCanvas();
}

void KisView::rotateLayerLeft90()
{
        if (!currentImg()) return;
        KisLayerSP layer = currentImg() -> activeLayer();
        if (!layer) return;

        QWMatrix m;
        m.rotate(270);
        layer->transform(m);
	m_doc -> setModified(true);

        layersUpdated();
        resizeEvent(0);
        updateCanvas();

}

void KisView::rotateLayerRight90()
{
        if (!currentImg()) return;
        KisLayerSP layer = currentImg() -> activeLayer();
        if (!layer) return;


        QWMatrix m;
        m.rotate(90);
        layer -> transform(m);
	m_doc -> setModified(true);

        layersUpdated();
        resizeEvent(0);
        updateCanvas();

}

void KisView::rotateLayerCustom()
{
	// XXX
}

void KisView::mirrorLayerX()
{
        if (!currentImg()) return;
        KisLayerSP layer = currentImg() -> activeLayer();
        if (!layer) return;

        layer->mirrorX();
	m_doc -> setModified(true);
        layersUpdated();
        updateCanvas();
}

void KisView::mirrorLayerY()
{
        if (!currentImg()) return;
        KisLayerSP layer = currentImg() -> activeLayer();
        if (!layer) return;

        layer->mirrorY();
	m_doc -> setModified(true);
        layersUpdated();
        updateCanvas();
}

void KisView::scaleLayer(double sx, double sy)
{
	if (!currentImg()) return;

	KisLayerSP layer = currentImg() -> activeLayer();
	if (!layer) return;

	layer -> scale(sx, sy);

	m_doc -> setModified(true);
	layersUpdated();
	resizeEvent(0);
	updateCanvas();
}

void KisView::add_new_image_tab()
{
        m_doc -> slotNewImage();
        updateTabBar();
}

void KisView::remove_current_image_tab()
{
        KisImageSP current = currentImg();

        if (current) {
                disconnectCurrentImg();
                m_current = 0;
                m_doc -> removeImage(current);
        }
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

void KisView::showRuler()
{
	if( m_RulerAction->isChecked() )
	{
		m_hRuler->show();
		m_vRuler->show();

		m_canvas -> setGeometry(20, 20, width() - canvasXOffset(), height() - m_tabBar->height() - canvasYOffset());
		m_canvas -> show();
	}
	else
	{
		m_hRuler->hide();
		m_vRuler->hide();

		m_canvas -> setGeometry(1, 1, width() - canvasXOffset(), height() - m_tabBar->height() - canvasYOffset());
		m_canvas -> show();
	}
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
	KisIconItem *item;

	Q_ASSERT(m_toolcontroldocker);
	m_brush = dynamic_cast<KisBrush*>(brush);

	if (m_brush )
	{
		if((item = m_brushMediator -> itemFor(m_brush)))
		{
			m_controlWidget -> slotSetBrush(item);
		} else {
			m_controlWidget -> slotSetBrush( new KisIconItem(m_brush) );
		}
		notify();
	}
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

void KisView::gradientActivated(KisResource *gradient)
{
        KisIconItem *item;

        Q_ASSERT(m_toolcontroldocker);
        m_gradient = dynamic_cast<KisGradient*>(gradient);

        if (m_gradient && (item = m_gradientMediator -> itemFor(m_gradient)))
                m_controlWidget -> slotSetGradient(item);

        if (m_gradient)
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
	QValueList< int > pagesList = printer.pageList();
	KisImageSP img;
	printer.setFullPage(true);
	gc.setClipping(false);
	
	QValueList< int >::iterator it = pagesList.begin();
	QValueList< int >::iterator itEnd = pagesList.end();
	
	while(it != itEnd)
	{
		if (*it)
			printer.newPage();
		img = m_doc -> imageNum(*it - 1);
		Q_ASSERT(img);
		m_doc -> setCurrentImage(img);
		m_doc -> paintContent(gc, img -> bounds());
		m_doc -> setCurrentImage(0);
	}
}

void KisView::setupTools()
{
	KisToolFactory *factory = KisToolFactory::singleton();
	Q_ASSERT(factory);

	m_inputDeviceToolSetMap[INPUT_DEVICE_MOUSE] = factory -> create(actionCollection(), this);
	m_inputDeviceToolSetMap[INPUT_DEVICE_STYLUS] = factory -> create(actionCollection(), this);
	m_inputDeviceToolSetMap[INPUT_DEVICE_ERASER] = factory -> create(actionCollection(), this);
	m_inputDeviceToolSetMap[INPUT_DEVICE_PUCK] = factory -> create(actionCollection(), this);

	qApp -> installEventFilter(this);
	m_tabletEventTimer.start();
}

void KisView::canvasGotPaintEvent(QPaintEvent *event)
{
        KisRect r = viewToWindow(KisRect(event -> rect()));
        paintView(r);
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

	m_hRuler -> updatePointer(e -> pos().floorX() - canvasXOffset(), e -> pos().floorY() - canvasYOffset());
	m_vRuler -> updatePointer(e -> pos().floorX() - canvasXOffset(), e -> pos().floorY() - canvasYOffset());

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
	} else if (e -> device() == currentInputDevice() && currentTool()) {
		KisMoveEvent ev(e -> device(), wp, e -> globalPos(), e -> pressure(), e -> xTilt(), e -> yTilt(), e -> state());

		currentTool() -> move(&ev);
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
	} else if (e -> device() == currentInputDevice() && currentTool()) {
		KisPoint p = viewToWindow(e -> pos());
		KisButtonReleaseEvent ev(e -> device(), p, e -> globalPos(), e -> pressure(), e -> xTilt(), e -> yTilt(), e -> button(), e -> state());

		if (currentTool()) {
			currentTool() -> buttonRelease(&ev);
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

				popup.insertItem(i18n("Insert as New Image"), addImageId);
				popup.insertSeparator();
				popup.insertItem(i18n("Open in New Document"), addDocumentId);
			}
			else {
				if (currentImg() != 0) {
					popup.insertItem(i18n("Insert as New Layers"), addLayerId);
				}

				popup.insertItem(i18n("Insert as New Images"), addImageId);
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
                                   img -> colorStrategy() -> name(),
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
                                                             KisColorSpaceRegistry::singleton()->colorSpace(dlg.colorStrategyName()));

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
		m_doc -> setModified(true);
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

        layerUpdateGUI(img && img -> activeLayer());
        m_layerBox -> setUpdatesEnabled(false);
        m_layerBox -> clear();

        if (img) {
                vKisLayerSP l = img -> layers();

                for (vKisLayerSP_it it = l.begin(); it != l.end(); it++)
                        m_layerBox -> insertItem((*it) -> name(), (*it) -> visible(), (*it) -> linked());

                m_layerBox -> setCurrentItem(img -> index(img -> activeLayer()));
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
        m_selectionManager -> updateGUI(m_current && m_current -> activeLayer() -> selection());
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

        m_selectionManager -> updateGUI(m_current && m_current -> activeLayer() -> selection());
}

void KisView::scrollH(int value)
{
        m_hRuler -> updateVisibleArea(value, 0);
	m_canvas -> scroll(m_scrollX - value, 0);
	m_scrollX = value;
}

void KisView::scrollV(int value)
{
        m_vRuler -> updateVisibleArea(0, value);
	m_canvas -> scroll(0, m_scrollY - value);
	m_scrollY = value;
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

        QObject::connect(m_canvas, SIGNAL(gotButtonPressEvent(KisButtonPressEvent*)), this, SLOT(canvasGotButtonPressEvent(KisButtonPressEvent*)));
        QObject::connect(m_canvas, SIGNAL(gotButtonReleaseEvent(KisButtonReleaseEvent*)), this, SLOT(canvasGotButtonReleaseEvent(KisButtonReleaseEvent*)));
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

void KisView::connectCurrentImg() const
{
        if (m_current) {
                connect(m_current, SIGNAL(activeSelectionChanged(KisImageSP)), m_selectionManager, SLOT(imgSelectionChanged(KisImageSP)));
		connect(m_current, SIGNAL(selectionCreated(KisImageSP)), m_selectionManager, SLOT(imgSelectionChanged(KisImageSP)));

// 		connect(m_current, SIGNAL(selectionCreated(KisImageSP)), SLOT(imgUpdated(KisImageSP)));
                connect(m_current, SIGNAL(update(KisImageSP, const QRect&)), SLOT(imgUpdated(KisImageSP, const QRect&)));
		connect(m_current, SIGNAL(layersChanged(KisImageSP)), SLOT(layersUpdated(KisImageSP)));
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


void KisView::resizeLayer(Q_INT32 w, Q_INT32 h)
{
	KisImageSP img = currentImg();
	if (img) {
		KisLayerSP layer = img -> activeLayer();
		if (layer) {
			layer -> resize(w, h);
			m_doc -> setModified(true);
			layersUpdated();
			resizeEvent(0);
			canvasRefresh();
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
			m_doc -> setModified(true);
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
		KisLayerSP layer = img -> activeLayer();
		
                KisSelectionSP selection = layer-> selection();
                

//                 if (selection)
//                         layer = selection.data();
//                 else
                        img -> activeLayer();

                if (layer) {
                        KisImageSP dupedImg = new KisImage(m_adapter, layer -> width(), layer -> height(), img -> colorStrategy(), m_doc -> nextImageName());
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

//      KisDlgTransform dlg(this);
//      if (dlg.exec() == QDialog::Accepted) {
//              // transform current layer
//              KisLayerSP layer = currentImg() -> activeLayer();
//              if (!layer) return;

//              layer -> transform(dlg.matrix());
//		m_doc -> setModified(true);

//              layersUpdated();
//              updateCanvas();
//      }
}


void KisView::resizeCurrentImage(Q_INT32 w, Q_INT32 h) 
{
        if (!currentImg()) return;

	currentImg() -> resize(w, h);
	m_doc -> setModified(true);

	resizeEvent(0);
	layersUpdated();
	canvasRefresh();
}

void KisView::scaleCurrentImage(double sx, double sy)
{
	if (!currentImg()) return;
	kdDebug() << "Going to scale image to (sx, sy): " << sx << ", " << sy << "\n";
	currentImg() -> scale(sx, sy);
	m_doc -> setModified(true);

	resizeEvent(0);
	layersUpdated();
	canvasRefresh();
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

KisProgressDisplayInterface *KisView::progressDisplay() const
{
	return m_progress;
}

KisFilterSP KisView::filterGet(const QString& name)
{
	return filterRegistry()->get( name );
}

QStringList KisView::filterList()
{
	return filterRegistry()->listKeys();
}

KisFilterRegistrySP KisView::filterRegistry() const
{
	return m_filterRegistry;
}


#include "kis_view.moc"

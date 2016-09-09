/*
 *  kis_paintop_box.cc - part of KImageShop/Krayon/Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
 *  Copyright (c) 2009-2011 Sven Langkamp (sven.langkamp@gmail.com)
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
 *  Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *  Copyright (c) 2014 Mohit Goyal <mohit.bits2011@gmail.com>
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

#include "kis_paintop_box.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QPixmap>
#include <QWidgetAction>
#include <QApplication>
#include <QMenu>

#include <kis_debug.h>

#include <kactioncollection.h>
#include <kacceleratormanager.h>
#include <QKeySequence>


#include <kis_icon.h>
#include <KoColorSpace.h>
#include <KoCompositeOpRegistry.h>
#include <KoResourceSelector.h>
#include <KoResourceServerAdapter.h>
#include <KoToolManager.h>
#include <QTemporaryFile>
#include <KoColorSpaceRegistry.h>

#include <kis_paint_device.h>
#include <brushengine/kis_paintop_registry.h>
#include <brushengine/kis_paintop_preset.h>
#include <brushengine/kis_paintop_settings.h>
#include <brushengine/kis_paintop_settings_update_proxy.h>
#include <kis_config_widget.h>
#include <kis_image.h>
#include <kis_node.h>
#include <brushengine/kis_paintop_config_widget.h>
#include <kis_action.h>

#include "kis_canvas2.h"
#include "kis_node_manager.h"
#include "KisViewManager.h"
#include "kis_canvas_resource_provider.h"
#include "kis_resource_server_provider.h"
#include "kis_favorite_resource_manager.h"
#include "kis_config.h"

#include "widgets/kis_popup_button.h"
#include "widgets/kis_tool_options_popup.h"
#include "widgets/kis_paintop_presets_popup.h"
#include "widgets/kis_tool_options_popup.h"
#include "widgets/kis_paintop_presets_chooser_popup.h"
#include "widgets/kis_workspace_chooser.h"
#include "widgets/kis_paintop_list_widget.h"
#include "widgets/kis_slider_spin_box.h"
#include "widgets/kis_cmb_composite.h"
#include "widgets/kis_widget_chooser.h"
#include "tool/kis_tool.h"
#include "kis_signals_blocker.h"
#include "kis_action_manager.h"
#include "kis_highlighted_button.h"

typedef KoResourceServerSimpleConstruction<KisPaintOpPreset, SharedPointerStoragePolicy<KisPaintOpPresetSP> > KisPaintOpPresetResourceServer;
typedef KoResourceServerAdapter<KisPaintOpPreset, SharedPointerStoragePolicy<KisPaintOpPresetSP> > KisPaintOpPresetResourceServerAdapter;

KisPaintopBox::KisPaintopBox(KisViewManager *view, QWidget *parent, const char *name)
    : QWidget(parent)
    , m_resourceProvider(view->resourceProvider())
    , m_optionWidget(0)
    , m_toolOptionsPopupButton(0)
    , m_brushEditorPopupButton(0)
    , m_presetSelectorPopupButton(0)
    , m_toolOptionsPopup(0)
    , m_viewManager(view)
    , m_previousNode(0)
    , m_currTabletToolID(KoInputDevice::invalid())
    , m_presetsEnabled(true)
    , m_blockUpdate(false)
    , m_dirtyPresetsEnabled(false)
    , m_eraserBrushSizeEnabled(false)
    , m_presetUpdateCompressor(200, KisSignalCompressor::FIRST_ACTIVE)
{
    Q_ASSERT(view != 0);


    setObjectName(name);
    KisConfig cfg;
    m_dirtyPresetsEnabled = cfg.useDirtyPresets();
    m_eraserBrushSizeEnabled = cfg.useEraserBrushSize();

    KAcceleratorManager::setNoAccel(this);

    setWindowTitle(i18n("Painter's Toolchest"));

    KConfigGroup grp =  KSharedConfig::openConfig()->group("krita").group("Toolbar BrushesAndStuff");
    int iconsize = grp.readEntry("IconSize", 32);

    if (!cfg.toolOptionsInDocker()) {
        m_toolOptionsPopupButton = new KisPopupButton(this);
        m_toolOptionsPopupButton->setIcon(KisIconUtils::loadIcon("configure"));
        m_toolOptionsPopupButton->setToolTip(i18n("Tool Settings"));
        m_toolOptionsPopupButton->setFixedSize(iconsize, iconsize);
    }

    m_brushEditorPopupButton = new KisPopupButton(this);
    m_brushEditorPopupButton->setIcon(KisIconUtils::loadIcon("paintop_settings_02"));
    m_brushEditorPopupButton->setToolTip(i18n("Edit brush settings"));
    m_brushEditorPopupButton->setFixedSize(iconsize, iconsize);

    m_presetSelectorPopupButton = new KisPopupButton(this);
    m_presetSelectorPopupButton->setIcon(KisIconUtils::loadIcon("paintop_settings_01"));
    m_presetSelectorPopupButton->setToolTip(i18n("Choose brush preset"));
    m_presetSelectorPopupButton->setFixedSize(iconsize, iconsize);

    m_eraseModeButton = new KisHighlightedToolButton(this);
    m_eraseModeButton->setFixedSize(iconsize, iconsize);
    m_eraseModeButton->setCheckable(true);

    m_eraseAction = m_viewManager->actionManager()->createAction("erase_action");
    m_eraseModeButton->setDefaultAction(m_eraseAction);

    m_reloadButton = new QToolButton(this);
    m_reloadButton->setFixedSize(iconsize, iconsize);

    m_reloadAction = m_viewManager->actionManager()->createAction("reload_preset_action");
    m_reloadButton->setDefaultAction(m_reloadAction);

    m_alphaLockButton = new KisHighlightedToolButton(this);
    m_alphaLockButton->setFixedSize(iconsize, iconsize);
    m_alphaLockButton->setCheckable(true);

    KisAction* alphaLockAction = m_viewManager->actionManager()->createAction("preserve_alpha");
    m_alphaLockButton->setDefaultAction(alphaLockAction);


    // pen pressure
    m_disablePressureButton = new KisHighlightedToolButton(this);
    m_disablePressureButton->setFixedSize(iconsize, iconsize);
    m_disablePressureButton->setCheckable(true);

    m_disablePressureAction = m_viewManager->actionManager()->createAction("disable_pressure");
    m_disablePressureButton->setDefaultAction(m_disablePressureAction);

    // horizontal and vertical mirror toolbar buttons

    // mirror tool options for the X Mirror
    QMenu *toolbarMenuXMirror = new QMenu();

    KisAction* hideCanvasDecorationsX = m_viewManager->actionManager()->createAction("mirrorX-hideDecorations");
    hideCanvasDecorationsX->setCheckable(true);
    hideCanvasDecorationsX->setText(i18n("Hide Mirror Line"));
    toolbarMenuXMirror->addAction(hideCanvasDecorationsX);

    KisAction* lockActionX = m_viewManager->actionManager()->createAction("mirrorX-lock");
    lockActionX->setText(i18n("Lock"));
    lockActionX->setCheckable(true);
    toolbarMenuXMirror->addAction(lockActionX);

    KisAction* moveToCenterActionX = m_viewManager->actionManager()->createAction("mirrorX-moveToCenter");
    moveToCenterActionX->setCheckable(false);
    moveToCenterActionX->setText(i18n("Move to Canvas Center"));
    toolbarMenuXMirror->addAction(moveToCenterActionX);



    // mirror tool options for the Y Mirror
    QMenu *toolbarMenuYMirror = new QMenu();

    KisAction* hideCanvasDecorationsY = m_viewManager->actionManager()->createAction("mirrorY-hideDecorations");
    hideCanvasDecorationsY->setCheckable(true);
    hideCanvasDecorationsY->setText(i18n("Hide Mirror Line"));
    toolbarMenuYMirror->addAction(hideCanvasDecorationsY);


    KisAction* lockActionY = m_viewManager->actionManager()->createAction("mirrorY-lock");
    lockActionY->setText(i18n("Lock"));
    lockActionY->setCheckable(true);
    toolbarMenuYMirror->addAction(lockActionY);

    KisAction* moveToCenterActionY = m_viewManager->actionManager()->createAction("mirrorY-moveToCenter");
    moveToCenterActionY->setCheckable(false);
    moveToCenterActionY->setText(i18n("Move to Canvas Center"));
    toolbarMenuYMirror->addAction(moveToCenterActionY);




    // create horizontal and vertical mirror buttons

    m_hMirrorButton = new KisHighlightedToolButton(this);
    int menuPadding = 10;
    m_hMirrorButton->setFixedSize(iconsize + menuPadding, iconsize);
    m_hMirrorButton->setCheckable(true);
    m_hMirrorAction = m_viewManager->actionManager()->createAction("hmirror_action");
    m_hMirrorButton->setDefaultAction(m_hMirrorAction);
    m_hMirrorButton->setMenu(toolbarMenuXMirror);
    m_hMirrorButton->setPopupMode(QToolButton::MenuButtonPopup);

    m_vMirrorButton = new KisHighlightedToolButton(this);
    m_vMirrorButton->setFixedSize(iconsize + menuPadding, iconsize);
    m_vMirrorButton->setCheckable(true);
    m_vMirrorAction = m_viewManager->actionManager()->createAction("vmirror_action");
    m_vMirrorButton->setDefaultAction(m_vMirrorAction);
    m_vMirrorButton->setMenu(toolbarMenuYMirror);
    m_vMirrorButton->setPopupMode(QToolButton::MenuButtonPopup);


    // add connections for horizontal and mirrror buttons
    connect(lockActionX, SIGNAL(toggled(bool)), this, SLOT(slotLockXMirrorToggle(bool)));
    connect(lockActionY, SIGNAL(toggled(bool)), this, SLOT(slotLockYMirrorToggle(bool)));

    connect(moveToCenterActionX, SIGNAL(triggered(bool)), this, SLOT(slotMoveToCenterMirrorX()));
    connect(moveToCenterActionY, SIGNAL(triggered(bool)), this, SLOT(slotMoveToCenterMirrorY()));

    connect(hideCanvasDecorationsX, SIGNAL(toggled(bool)), this, SLOT(slotHideDecorationMirrorX(bool)));
    connect(hideCanvasDecorationsY, SIGNAL(toggled(bool)), this, SLOT(slotHideDecorationMirrorY(bool)));

    const bool sliderLabels = cfg.sliderLabels();
    int sliderWidth;

    if (sliderLabels) {
        sliderWidth = 150 * logicalDpiX() / 96;
    }
    else {
        sliderWidth = 120 * logicalDpiX() / 96;
    }

    for (int i = 0; i < 3; ++i) {
        m_sliderChooser[i] = new KisWidgetChooser(i + 1);

        KisDoubleSliderSpinBox* slOpacity;
        KisDoubleSliderSpinBox* slFlow;
        KisDoubleSliderSpinBox* slSize;
        if (sliderLabels) {
            slOpacity = m_sliderChooser[i]->addWidget<KisDoubleSliderSpinBox>("opacity");
            slFlow    = m_sliderChooser[i]->addWidget<KisDoubleSliderSpinBox>("flow");
            slSize    = m_sliderChooser[i]->addWidget<KisDoubleSliderSpinBox>("size");
            slOpacity->setPrefix(QString("%1  ").arg(i18n("Opacity:")));
            slFlow->setPrefix(QString("%1  ").arg(i18n("Flow:")));
            slSize->setPrefix(QString("%1  ").arg(i18n("Size:")));
        }
        else {
            slOpacity = m_sliderChooser[i]->addWidget<KisDoubleSliderSpinBox>("opacity", i18n("Opacity:"));
            slFlow    = m_sliderChooser[i]->addWidget<KisDoubleSliderSpinBox>("flow", i18n("Flow:"));
            slSize    = m_sliderChooser[i]->addWidget<KisDoubleSliderSpinBox>("size", i18n("Size:"));
        }

        slOpacity->setRange(0.0, 1.0, 2);
        slOpacity->setValue(1.0);
        slOpacity->setSingleStep(0.05);
        slOpacity->setMinimumWidth(qMax(sliderWidth, slOpacity->sizeHint().width()));
        slOpacity->setFixedHeight(iconsize);
        slOpacity->setBlockUpdateSignalOnDrag(true);

        slFlow->setRange(0.0, 1.0, 2);
        slFlow->setValue(1.0);
        slFlow->setSingleStep(0.05);
        slFlow->setMinimumWidth(qMax(sliderWidth, slFlow->sizeHint().width()));
        slFlow->setFixedHeight(iconsize);
        slFlow->setBlockUpdateSignalOnDrag(true);

        slSize->setRange(0, 1000, 2);
        slSize->setValue(100);

        slSize->setSingleStep(1);
        slSize->setExponentRatio(3.0);
        slSize->setSuffix(i18n(" px"));
        slSize->setMinimumWidth(qMax(sliderWidth, slSize->sizeHint().width()));
        slSize->setFixedHeight(iconsize);
        slSize->setBlockUpdateSignalOnDrag(true);

        m_sliderChooser[i]->chooseWidget(cfg.toolbarSlider(i + 1));
    }

    m_cmbCompositeOp = new KisCompositeOpComboBox();
    m_cmbCompositeOp->setFixedHeight(iconsize);
    Q_FOREACH (KisAction * a, m_cmbCompositeOp->blendmodeActions()) {
        m_viewManager->actionManager()->addAction(a->text(), a);
    }

    m_workspaceWidget = new KisPopupButton(this);
    m_workspaceWidget->setIcon(KisIconUtils::loadIcon("view-choose"));
    m_workspaceWidget->setToolTip(i18n("Choose workspace"));
    m_workspaceWidget->setFixedSize(iconsize, iconsize);
    m_workspaceWidget->setPopupWidget(new KisWorkspaceChooser(view));

    QHBoxLayout* baseLayout = new QHBoxLayout(this);
    m_paintopWidget = new QWidget(this);
    baseLayout->addWidget(m_paintopWidget);
    baseLayout->setSpacing(4);
    baseLayout->setContentsMargins(0, 0, 0, 0);

    m_layout = new QHBoxLayout(m_paintopWidget);
    if (!cfg.toolOptionsInDocker()) {
        m_layout->addWidget(m_toolOptionsPopupButton);
    }
    m_layout->addWidget(m_brushEditorPopupButton);
    m_layout->addWidget(m_presetSelectorPopupButton);
    m_layout->setSpacing(4);
    m_layout->setContentsMargins(0, 0, 0, 0);

    QWidget* compositeActions = new QWidget(this);
    QHBoxLayout* compositeLayout = new QHBoxLayout(compositeActions);
    compositeLayout->addWidget(m_cmbCompositeOp);
    compositeLayout->addWidget(m_eraseModeButton);
    compositeLayout->addWidget(m_alphaLockButton);

    compositeLayout->setSpacing(4);
    compositeLayout->setContentsMargins(0, 0, 0, 0);

    compositeLayout->addWidget(m_reloadButton);

    QWidgetAction * action;

    action = new QWidgetAction(this);
    view->actionCollection()->addAction("composite_actions", action);
    action->setText(i18n("Brush composite"));
    action->setDefaultWidget(compositeActions);

    QWidget* compositePressure = new QWidget(this);
    QHBoxLayout* pressureLayout = new QHBoxLayout(compositePressure);
    pressureLayout->addWidget(m_disablePressureButton);
    pressureLayout->setSpacing(4);
    pressureLayout->setContentsMargins(0, 0, 0, 0);

    action = new QWidgetAction(this);
    view->actionCollection()->addAction("pressure_action", action);
    action->setText(i18n("Pressure usage (small button)"));
    action->setDefaultWidget(compositePressure);

    action = new QWidgetAction(this);
    KisActionRegistry::instance()->propertizeAction("brushslider1", action);
    view->actionCollection()->addAction("brushslider1", action);
    action->setDefaultWidget(m_sliderChooser[0]);
    connect(action, SIGNAL(triggered()), m_sliderChooser[0], SLOT(showPopupWidget()));
    connect(m_viewManager->mainWindow(), SIGNAL(themeChanged()), m_sliderChooser[0], SLOT(updateThemedIcons()));

    action = new QWidgetAction(this);
    KisActionRegistry::instance()->propertizeAction("brushslider2", action);
    view->actionCollection()->addAction("brushslider2", action);
    action->setDefaultWidget(m_sliderChooser[1]);
    connect(action, SIGNAL(triggered()), m_sliderChooser[1], SLOT(showPopupWidget()));
    connect(m_viewManager->mainWindow(), SIGNAL(themeChanged()), m_sliderChooser[1], SLOT(updateThemedIcons()));

    action = new QWidgetAction(this);
    KisActionRegistry::instance()->propertizeAction("brushslider3", action);
    view->actionCollection()->addAction("brushslider3", action);
    action->setDefaultWidget(m_sliderChooser[2]);
    connect(action, SIGNAL(triggered()), m_sliderChooser[2], SLOT(showPopupWidget()));
        connect(m_viewManager->mainWindow(), SIGNAL(themeChanged()), m_sliderChooser[2], SLOT(updateThemedIcons()));

    action = new QWidgetAction(this);
    KisActionRegistry::instance()->propertizeAction("next_favorite_preset", action);
    view->actionCollection()->addAction("next_favorite_preset", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotNextFavoritePreset()));

    action = new QWidgetAction(this);
    KisActionRegistry::instance()->propertizeAction("previous_favorite_preset", action);
    view->actionCollection()->addAction("previous_favorite_preset", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotPreviousFavoritePreset()));

    action = new QWidgetAction(this);
    KisActionRegistry::instance()->propertizeAction("previous_preset", action);
    view->actionCollection()->addAction("previous_preset", action);

    connect(action, SIGNAL(triggered()), this, SLOT(slotSwitchToPreviousPreset()));

    if (!cfg.toolOptionsInDocker()) {
        action = new QWidgetAction(this);
        KisActionRegistry::instance()->propertizeAction("show_tool_options", action);
        view->actionCollection()->addAction("show_tool_options", action);
        connect(action, SIGNAL(triggered()), m_toolOptionsPopupButton, SLOT(showPopupWidget()));
    }

    action = new QWidgetAction(this);
    KisActionRegistry::instance()->propertizeAction("show_brush_editor", action);
    view->actionCollection()->addAction("show_brush_editor", action);
    connect(action, SIGNAL(triggered()), m_brushEditorPopupButton, SLOT(showPopupWidget()));

    action = new QWidgetAction(this);
    KisActionRegistry::instance()->propertizeAction("show_brush_presets", action);
    view->actionCollection()->addAction("show_brush_presets", action);
    connect(action, SIGNAL(triggered()), m_presetSelectorPopupButton, SLOT(showPopupWidget()));

    QWidget* mirrorActions = new QWidget(this);
    QHBoxLayout* mirrorLayout = new QHBoxLayout(mirrorActions);
    mirrorLayout->addWidget(m_hMirrorButton);
    mirrorLayout->addWidget(m_vMirrorButton);
    mirrorLayout->setSpacing(4);
    mirrorLayout->setContentsMargins(0, 0, 0, 0);

    action = new QWidgetAction(this);
    KisActionRegistry::instance()->propertizeAction("mirror_actions", action);
    action->setDefaultWidget(mirrorActions);
    view->actionCollection()->addAction("mirror_actions", action);

    action = new QWidgetAction(this);
    KisActionRegistry::instance()->propertizeAction("workspaces", action);
    view->actionCollection()->addAction("workspaces", action);
    action->setDefaultWidget(m_workspaceWidget);

    if (!cfg.toolOptionsInDocker()) {
        m_toolOptionsPopup = new KisToolOptionsPopup();
        m_toolOptionsPopupButton->setPopupWidget(m_toolOptionsPopup);
        m_toolOptionsPopup->switchDetached(false);
    }

    m_presetsPopup = new KisPaintOpPresetsPopup(m_resourceProvider);
    m_brushEditorPopupButton->setPopupWidget(m_presetsPopup);
    m_presetsPopup->switchDetached(false);
    connect(m_presetsPopup, SIGNAL(brushEditorShown()), SLOT(slotUpdateOptionsWidget()));
    connect(m_viewManager->mainWindow(), SIGNAL(themeChanged()), m_presetsPopup, SLOT(updateThemedIcons()));

    m_presetsChooserPopup = new KisPaintOpPresetsChooserPopup();
    m_presetsChooserPopup->setFixedSize(500, 600);
    m_presetSelectorPopupButton->setPopupWidget(m_presetsChooserPopup);

    m_currCompositeOpID = KoCompositeOpRegistry::instance().getDefaultCompositeOp().id();

    slotNodeChanged(view->activeNode());
    // Get all the paintops
    QList<QString> keys = KisPaintOpRegistry::instance()->keys();
    QList<KisPaintOpFactory*> factoryList;

    Q_FOREACH (const QString & paintopId, keys) {
        factoryList.append(KisPaintOpRegistry::instance()->get(paintopId));
    }
    m_presetsPopup->setPaintOpList(factoryList);

    connect(m_presetsPopup       , SIGNAL(paintopActivated(QString))          , SLOT(slotSetPaintop(QString)));
    connect(m_presetsPopup       , SIGNAL(savePresetClicked())                , SLOT(slotSaveActivePreset()));
    connect(m_presetsPopup       , SIGNAL(defaultPresetClicked())             , SLOT(slotSetupDefaultPreset()));
    connect(m_presetsPopup       , SIGNAL(signalResourceSelected(KoResource*)), SLOT(resourceSelected(KoResource*)));
    connect(m_presetsPopup       , SIGNAL(reloadPresetClicked())              , SLOT(slotReloadPreset()));
    connect(m_presetsPopup       , SIGNAL(dirtyPresetToggled(bool))           , SLOT(slotDirtyPresetToggled(bool)));
    connect(m_presetsPopup       , SIGNAL(eraserBrushSizeToggled(bool))       , SLOT(slotEraserBrushSizeToggled(bool)));

    connect(m_presetsChooserPopup, SIGNAL(resourceSelected(KoResource*))      , SLOT(resourceSelected(KoResource*)));
    connect(m_resourceProvider   , SIGNAL(sigNodeChanged(const KisNodeSP))    , SLOT(slotNodeChanged(const KisNodeSP)));
    connect(m_cmbCompositeOp     , SIGNAL(currentIndexChanged(int))           , SLOT(slotSetCompositeMode(int)));
    connect(m_eraseAction          , SIGNAL(toggled(bool))                    , SLOT(slotToggleEraseMode(bool)));
    connect(alphaLockAction      , SIGNAL(toggled(bool))                    , SLOT(slotToggleAlphaLockMode(bool)));
    connect(m_disablePressureAction  , SIGNAL(toggled(bool))                    , SLOT(slotDisablePressureMode(bool)));

    m_disablePressureAction->setChecked(true);

    connect(m_hMirrorAction        , SIGNAL(toggled(bool))                    , SLOT(slotHorizontalMirrorChanged(bool)));
    connect(m_vMirrorAction        , SIGNAL(toggled(bool))                    , SLOT(slotVerticalMirrorChanged(bool)));




    connect(m_reloadAction         , SIGNAL(triggered())                        , SLOT(slotReloadPreset()));

    connect(m_sliderChooser[0]->getWidget<KisDoubleSliderSpinBox>("opacity"), SIGNAL(valueChanged(qreal)), SLOT(slotSlider1Changed()));
    connect(m_sliderChooser[0]->getWidget<KisDoubleSliderSpinBox>("flow")   , SIGNAL(valueChanged(qreal)), SLOT(slotSlider1Changed()));
    connect(m_sliderChooser[0]->getWidget<KisDoubleSliderSpinBox>("size")   , SIGNAL(valueChanged(qreal)), SLOT(slotSlider1Changed()));
    connect(m_sliderChooser[1]->getWidget<KisDoubleSliderSpinBox>("opacity"), SIGNAL(valueChanged(qreal)), SLOT(slotSlider2Changed()));
    connect(m_sliderChooser[1]->getWidget<KisDoubleSliderSpinBox>("flow")   , SIGNAL(valueChanged(qreal)), SLOT(slotSlider2Changed()));
    connect(m_sliderChooser[1]->getWidget<KisDoubleSliderSpinBox>("size")   , SIGNAL(valueChanged(qreal)), SLOT(slotSlider2Changed()));
    connect(m_sliderChooser[2]->getWidget<KisDoubleSliderSpinBox>("opacity"), SIGNAL(valueChanged(qreal)), SLOT(slotSlider3Changed()));
    connect(m_sliderChooser[2]->getWidget<KisDoubleSliderSpinBox>("flow")   , SIGNAL(valueChanged(qreal)), SLOT(slotSlider3Changed()));
    connect(m_sliderChooser[2]->getWidget<KisDoubleSliderSpinBox>("size")   , SIGNAL(valueChanged(qreal)), SLOT(slotSlider3Changed()));

    //Needed to connect canvas to favorite resource manager
    connect(m_viewManager->resourceProvider(), SIGNAL(sigFGColorChanged(KoColor)), SLOT(slotUnsetEraseMode()));

    m_favoriteResourceManager = new KisFavoriteResourceManager(this);
    connect(m_resourceProvider, SIGNAL(sigFGColorUsed(KoColor)), m_favoriteResourceManager, SLOT(slotAddRecentColor(KoColor)));

    connect(m_resourceProvider, SIGNAL(sigFGColorChanged(KoColor)), m_favoriteResourceManager, SLOT(slotChangeFGColorSelector(KoColor)));
    connect(m_resourceProvider, SIGNAL(sigBGColorChanged(KoColor)), m_favoriteResourceManager, SLOT(slotSetBGColor(KoColor)));
    // cold initialization
    m_favoriteResourceManager->slotChangeFGColorSelector(m_resourceProvider->fgColor());
    m_favoriteResourceManager->slotSetBGColor(m_resourceProvider->bgColor());

    connect(m_favoriteResourceManager, SIGNAL(sigSetFGColor(KoColor)), m_resourceProvider, SLOT(slotSetFGColor(KoColor)));
    connect(m_favoriteResourceManager, SIGNAL(sigSetBGColor(KoColor)), m_resourceProvider, SLOT(slotSetBGColor(KoColor)));
    connect(m_favoriteResourceManager, SIGNAL(sigEnableChangeColor(bool)), m_resourceProvider, SLOT(slotResetEnableFGChange(bool)));

    connect(view->mainWindow(), SIGNAL(themeChanged()), this, SLOT(slotUpdateSelectionIcon()));

    slotInputDeviceChanged(KoToolManager::instance()->currentInputDevice());
}

KisPaintopBox::~KisPaintopBox()
{
    KisConfig cfg;
    QMapIterator<TabletToolID, TabletToolData> iter(m_tabletToolMap);
    while (iter.hasNext()) {
        iter.next();
        if ((iter.key().pointer) == QTabletEvent::Eraser) {
            cfg.writeEntry(QString("LastEraser_%1").arg(iter.key().uniqueID) , iter.value().preset->name());
        }
        else {
            cfg.writeEntry(QString("LastPreset_%1").arg(iter.key().uniqueID) , iter.value().preset->name());
        }
    }
    // Do not delete the widget, since it it is global to the application, not owned by the view
    m_presetsPopup->setPaintOpSettingsWidget(0);
    qDeleteAll(m_paintopOptionWidgets);
    delete m_favoriteResourceManager;

    for (int i = 0; i < 3; ++i) {
        delete m_sliderChooser[i];
    }
}

void KisPaintopBox::restoreResource(KoResource* resource)
{
    KisPaintOpPreset* preset = dynamic_cast<KisPaintOpPreset*>(resource);
    if (preset) {
        setCurrentPaintop(preset->paintOp(), preset);

        m_presetsPopup->setPresetImage(preset->image());
        m_presetsPopup->resourceSelected(resource);
    }
}

void KisPaintopBox::newOptionWidgets(const QList<QPointer<QWidget> > &optionWidgetList)
{
    if (m_toolOptionsPopup) {
        m_toolOptionsPopup->newOptionWidgets(optionWidgetList);
    }
}

void KisPaintopBox::resourceSelected(KoResource* resource)
{
    KisPaintOpPreset* preset = dynamic_cast<KisPaintOpPreset*>(resource);
    if (preset) {
        if (!preset->settings()->isLoadable())
            return;

        setCurrentPaintopAndReload(preset->paintOp(), preset);
        m_presetsPopup->setPresetImage(preset->image());
        m_presetsPopup->resourceSelected(resource);
    }
}

void KisPaintopBox::setCurrentPaintopAndReload(const KoID& paintop, KisPaintOpPresetSP preset)
{
    if (!m_dirtyPresetsEnabled) {
        KisSignalsBlocker blocker(m_optionWidget);
        if (!preset->load()) {
            warnKrita << "failed to load the preset.";
        }
    }

    setCurrentPaintop(paintop, preset);
}

void KisPaintopBox::setCurrentPaintop(const KoID& paintop, KisPaintOpPresetSP preset)
{
    m_presetConnections.clear();

    if (m_resourceProvider->currentPreset()) {

        m_resourceProvider->setPreviousPaintOpPreset(m_resourceProvider->currentPreset());

        if (m_optionWidget) {
            m_optionWidget->hide();
        }

        m_paintOpPresetMap[m_resourceProvider->currentPreset()->paintOp()] = m_resourceProvider->currentPreset();
        m_tabletToolMap[m_currTabletToolID].preset = m_resourceProvider->currentPreset();
        m_tabletToolMap[m_currTabletToolID].paintOpID = m_resourceProvider->currentPreset()->paintOp();
    }

    preset = (!preset) ? activePreset(paintop) : preset;
    Q_ASSERT(preset && preset->settings());

    if (!m_paintopOptionWidgets.contains(paintop))
        m_paintopOptionWidgets[paintop] = KisPaintOpRegistry::instance()->get(paintop.id())->createConfigWidget(this);

    m_optionWidget = m_paintopOptionWidgets[paintop];

    KisSignalsBlocker b(m_optionWidget);

    preset->setOptionsWidget(m_optionWidget);

    m_optionWidget->setImage(m_viewManager->image());
    m_optionWidget->setNode(m_viewManager->activeNode());

    m_presetsPopup->setPaintOpSettingsWidget(m_optionWidget);

    m_resourceProvider->setPaintOpPreset(preset);

    Q_ASSERT(m_optionWidget && m_presetSelectorPopupButton);

    m_presetConnections.addConnection(m_optionWidget, SIGNAL(sigConfigurationUpdated()), this, SLOT(slotGuiChangedCurrentPreset()));
    m_presetConnections.addConnection(&m_presetUpdateCompressor, SIGNAL(timeout()), this, SLOT(slotUpdateOptionsWidget()));
    m_presetConnections.addConnection(m_optionWidget, SIGNAL(sigSaveLockedConfig(KisPropertiesConfiguration*)), this, SLOT(slotSaveLockedOptionToPreset(KisPropertiesConfiguration*)));
    m_presetConnections.addConnection(m_optionWidget, SIGNAL(sigDropLockedConfig(KisPropertiesConfiguration*)), this, SLOT(slotDropLockedOption(KisPropertiesConfiguration*)));


    // load the current brush engine icon for the brush editor toolbar button
    KisPaintOpFactory* paintOp = KisPaintOpRegistry::instance()->get(paintop.id());
    QString pixFilename = KoResourcePaths::findResource("kis_images", paintOp->pixmap());

    m_brushEditorPopupButton->setIcon(QIcon(pixFilename));
    m_presetsPopup->setCurrentPaintOp(paintop.id());

    if (m_presetsPopup->currentPaintOp() != paintop.id()) {
        // Must change the paintop as the current one is not supported
        // by the new colorspace.
        dbgKrita << "current paintop " << paintop.name() << " was not set, not supported by colorspace";
    }

    // preset -> compressor
    m_presetConnections.addConnection(
        preset->updateProxy(), SIGNAL(sigSettingsChanged()),
        &m_presetUpdateCompressor, SLOT(start()));
}

void KisPaintopBox::slotUpdateOptionsWidget()
{
    KisPaintOpPresetSP preset = m_resourceProvider->currentPreset();

    KIS_SAFE_ASSERT_RECOVER_RETURN(preset);
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_optionWidget);

    if (m_optionWidget->isVisible()) {
        m_optionWidget->setConfigurationSafe(preset->settings().data());
    }

    m_presetsPopup->resourceSelected(preset.data());
    m_presetsPopup->updateViewSettings();

}

KisPaintOpPresetSP KisPaintopBox::defaultPreset(const KoID& paintOp)
{
    QString defaultName = paintOp.id() + ".kpp";
    QString path = KoResourcePaths::findResource("kis_defaultpresets", defaultName);

    KisPaintOpPresetSP preset = new KisPaintOpPreset(path);

    if (!preset->load()) {
        preset = KisPaintOpRegistry::instance()->defaultPreset(paintOp);
    }

    Q_ASSERT(preset);
    Q_ASSERT(preset->valid());

    return preset;
}

KisPaintOpPresetSP KisPaintopBox::activePreset(const KoID& paintOp)
{
    if (m_paintOpPresetMap[paintOp] == 0) {
        m_paintOpPresetMap[paintOp] = defaultPreset(paintOp);
    }

    return m_paintOpPresetMap[paintOp];
}

void KisPaintopBox::updateCompositeOp(QString compositeOpID)
{
    if (!m_optionWidget) return;
    KisSignalsBlocker blocker(m_optionWidget);

    KisNodeSP node = m_resourceProvider->currentNode();

    if (node && node->paintDevice()) {
        if (!node->paintDevice()->colorSpace()->hasCompositeOp(compositeOpID))
            compositeOpID = KoCompositeOpRegistry::instance().getDefaultCompositeOp().id();

        {
            KisSignalsBlocker b1(m_cmbCompositeOp);
            m_cmbCompositeOp->selectCompositeOp(KoID(compositeOpID));
        }
        if (compositeOpID != m_currCompositeOpID) {
            m_currCompositeOpID = compositeOpID;
        }
    }
}

void KisPaintopBox::setWidgetState(int flags)
{
    if (flags & (ENABLE_COMPOSITEOP | DISABLE_COMPOSITEOP)) {
        m_cmbCompositeOp->setEnabled(flags & ENABLE_COMPOSITEOP);
        m_eraseModeButton->setEnabled(flags & ENABLE_COMPOSITEOP);
    }

    if (flags & (ENABLE_PRESETS | DISABLE_PRESETS)) {
        m_presetSelectorPopupButton->setEnabled(flags & ENABLE_PRESETS);
        m_brushEditorPopupButton->setEnabled(flags & ENABLE_PRESETS);
    }

    for (int i = 0; i < 3; ++i) {
        if (flags & (ENABLE_OPACITY | DISABLE_OPACITY))
            m_sliderChooser[i]->getWidget("opacity")->setEnabled(flags & ENABLE_OPACITY);

        if (flags & (ENABLE_FLOW | DISABLE_FLOW))
            m_sliderChooser[i]->getWidget("flow")->setEnabled(flags & ENABLE_FLOW);

        if (flags & (ENABLE_SIZE | DISABLE_SIZE))
            m_sliderChooser[i]->getWidget("size")->setEnabled(flags & ENABLE_SIZE);
    }
}

void KisPaintopBox::setSliderValue(const QString& sliderID, qreal value)
{
    for (int i = 0; i < 3; ++i) {
        KisDoubleSliderSpinBox* slider = m_sliderChooser[i]->getWidget<KisDoubleSliderSpinBox>(sliderID);
        KisSignalsBlocker b(slider);
        slider->setValue(value);
    }
}

void KisPaintopBox::slotSetPaintop(const QString& paintOpId)
{
    if (KisPaintOpRegistry::instance()->get(paintOpId) != 0) {
        KoID id(paintOpId, KisPaintOpRegistry::instance()->get(paintOpId)->name());
        setCurrentPaintop(id);
    }
}

void KisPaintopBox::slotInputDeviceChanged(const KoInputDevice& inputDevice)
{
    TabletToolMap::iterator toolData = m_tabletToolMap.find(inputDevice);

    if (toolData == m_tabletToolMap.end()) {
        KisConfig cfg;
        KisPaintOpPresetResourceServer *rserver = KisResourceServerProvider::instance()->paintOpPresetServer(false);
        KisPaintOpPresetSP preset;
        if (inputDevice.pointer() == QTabletEvent::Eraser) {
            preset = rserver->resourceByName(cfg.readEntry<QString>(QString("LastEraser_%1").arg(inputDevice.uniqueTabletId()), "Eraser_circle"));
        }
        else {
            preset = rserver->resourceByName(cfg.readEntry<QString>(QString("LastPreset_%1").arg(inputDevice.uniqueTabletId()), "Basic_tip_default"));
        }
        if (!preset) {
            preset = rserver->resourceByName("Basic_tip_default");
        }
        if (preset) {
            setCurrentPaintop(preset->paintOp(), preset);
        }
    }
    else {
        setCurrentPaintop(toolData->paintOpID, toolData->preset);
    }

    m_currTabletToolID = TabletToolID(inputDevice);
}

void KisPaintopBox::slotCanvasResourceChanged(int key, const QVariant &value)
{
    if (m_viewManager) {
        sender()->blockSignals(true);
        KisPaintOpPresetSP preset = m_viewManager->resourceProvider()->resourceManager()->resource(KisCanvasResourceProvider::CurrentPaintOpPreset).value<KisPaintOpPresetSP>();
        if (preset && m_resourceProvider->currentPreset()->name() != preset->name()) {
            QString compositeOp = preset->settings()->getString("CompositeOp");
            updateCompositeOp(compositeOp);
            resourceSelected(preset.data());
        }
        m_presetsChooserPopup->canvasResourceChanged(preset.data(), preset);

        if (key == KisCanvasResourceProvider::CurrentCompositeOp) {
            if (m_resourceProvider->currentCompositeOp() != m_currCompositeOpID) {
                updateCompositeOp(m_resourceProvider->currentCompositeOp());
            }
        }

        if (key == KisCanvasResourceProvider::Size) {
            setSliderValue("size", m_resourceProvider->size());
        }

        if (key == KisCanvasResourceProvider::Opacity) {
            setSliderValue("opacity", m_resourceProvider->opacity());
        }

        if (key == KisCanvasResourceProvider::Flow) {
            setSliderValue("flow", m_resourceProvider->flow());
        }

        if (key == KisCanvasResourceProvider::EraserMode) {
            m_eraseAction->setChecked(value.toBool());
        }

        if (key == KisCanvasResourceProvider::DisablePressure) {
            m_disablePressureAction->setChecked(value.toBool());
        }

        sender()->blockSignals(false);
    }
}

void KisPaintopBox::slotSaveActivePreset()
{
    KisPaintOpPresetSP curPreset = m_resourceProvider->currentPreset();

    if (!curPreset)
        return;

    m_favoriteResourceManager->setBlockUpdates(true);

    KisPaintOpPresetSP newPreset = curPreset->clone();
    KisPaintOpPresetResourceServer * rServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    QString saveLocation = rServer->saveLocation();
    QString presetName = m_presetsPopup->getPresetName();
    QString presetFilename = saveLocation + presetName + newPreset->defaultFileExtension();

    QStringList tags;
    KisPaintOpPresetSP resource = rServer->resourceByName(presetName);
    if (resource) {

        tags = rServer->assignedTagsList(resource.data());
        rServer->removeResourceAndBlacklist(resource);

    }

    newPreset->setImage(m_presetsPopup->cutOutOverlay());
    newPreset->setFilename(presetFilename);
    newPreset->setName(presetName);
    newPreset->setPresetDirty(false);

    rServer->addResource(newPreset);
    Q_FOREACH (const QString & tag, tags) {
        rServer->addTag(newPreset.data(), tag);
    }

    // HACK ALERT! the server does not notify the observers
    // automatically, so we need to call theupdate manually!
    rServer->tagCategoryMembersChanged();

    restoreResource(newPreset.data());

    m_favoriteResourceManager->setBlockUpdates(false);
}

void KisPaintopBox::slotUpdatePreset()
{
    if (!m_resourceProvider->currentPreset()) return;

    // block updates of avoid some over updating of the option widget
    m_blockUpdate = true;

    setSliderValue("size", m_resourceProvider->size());

    {
        qreal opacity = m_resourceProvider->currentPreset()->settings()->paintOpOpacity();
        m_resourceProvider->setOpacity(opacity);
        setSliderValue("opacity", opacity);
        setWidgetState(ENABLE_OPACITY);
    }

    {
        setSliderValue("flow", m_resourceProvider->currentPreset()->settings()->paintOpFlow());
        setWidgetState(ENABLE_FLOW);
    }

    {
        updateCompositeOp(m_resourceProvider->currentPreset()->settings()->paintOpCompositeOp());
        setWidgetState(ENABLE_COMPOSITEOP);
    }

    m_blockUpdate = false;
}

void KisPaintopBox::slotSetupDefaultPreset()
{
    KisPaintOpPresetSP preset = defaultPreset(m_resourceProvider->currentPreset()->paintOp());
    preset->setOptionsWidget(m_optionWidget);
    m_resourceProvider->setPaintOpPreset(preset);
}

void KisPaintopBox::slotNodeChanged(const KisNodeSP node)
{
    if (m_previousNode.isValid() && m_previousNode->paintDevice())
        disconnect(m_previousNode->paintDevice().data(), SIGNAL(colorSpaceChanged(const KoColorSpace*)), this, SLOT(slotColorSpaceChanged(const KoColorSpace*)));

    // Reconnect colorspace change of node
    if (node && node->paintDevice()) {
        connect(node->paintDevice().data(), SIGNAL(colorSpaceChanged(const KoColorSpace*)), this, SLOT(slotColorSpaceChanged(const KoColorSpace*)));
        m_resourceProvider->setCurrentCompositeOp(m_currCompositeOpID);
        m_previousNode = node;
        slotColorSpaceChanged(node->colorSpace());
    }

    if (m_optionWidget) {
        m_optionWidget->setNode(node);
    }
}

void KisPaintopBox::slotColorSpaceChanged(const KoColorSpace* colorSpace)
{
    m_cmbCompositeOp->validate(colorSpace);
}

void KisPaintopBox::slotToggleEraseMode(bool checked)
{
    const bool oldEraserMode = m_resourceProvider->eraserMode();
    m_resourceProvider->setEraserMode(checked);

    if (oldEraserMode != checked && m_eraserBrushSizeEnabled) {
        const qreal currentSize = m_resourceProvider->size();

        KisPaintOpSettingsSP settings = m_resourceProvider->currentPreset()->settings();

        // remember brush size. set the eraser size to the normal brush size if not set
        if (checked) {
            settings->setSavedBrushSize(currentSize);
            if (qFuzzyIsNull(settings->savedEraserSize())) {
                settings->setSavedEraserSize(currentSize);
            }
        } else {
            settings->setSavedEraserSize(currentSize);
            if (qFuzzyIsNull(settings->savedBrushSize())) {
                settings->setSavedBrushSize(currentSize);
            }
        }

        //update value in UI (this is the main place the value is 'stored' in memory)
        qreal newSize = checked ? settings->savedEraserSize() : settings->savedBrushSize();
        m_resourceProvider->setSize(newSize);
    }
}

void KisPaintopBox::slotSetCompositeMode(int index)
{
    Q_UNUSED(index);
    QString compositeOp = m_cmbCompositeOp->selectedCompositeOp().id();
    m_resourceProvider->setCurrentCompositeOp(compositeOp);
}

void KisPaintopBox::slotHorizontalMirrorChanged(bool value)
{
    m_resourceProvider->setMirrorHorizontal(value);
}

void KisPaintopBox::slotVerticalMirrorChanged(bool value)
{
    m_resourceProvider->setMirrorVertical(value);
}

void KisPaintopBox::sliderChanged(int n)
{
    if (!m_optionWidget) // widget will not exist if the are no documents open
        return;


    KisSignalsBlocker blocker(m_optionWidget);

    qreal opacity = m_sliderChooser[n]->getWidget<KisDoubleSliderSpinBox>("opacity")->value();
    qreal flow    = m_sliderChooser[n]->getWidget<KisDoubleSliderSpinBox>("flow")->value();
    qreal size    = m_sliderChooser[n]->getWidget<KisDoubleSliderSpinBox>("size")->value();


    setSliderValue("opacity", opacity);
    setSliderValue("flow"   , flow);
    setSliderValue("size"   , size);

    if (m_presetsEnabled) {
        // IMPORTANT: set the PaintOp size before setting the other properties
        //            it wont work the other way
        // TODO: why?!

        m_resourceProvider->setSize(size);
        m_resourceProvider->setOpacity(opacity);
        m_resourceProvider->setFlow(flow);


        KisLockedPropertiesProxy *propertiesProxy = KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(m_resourceProvider->currentPreset()->settings());
        propertiesProxy->setProperty("OpacityValue", opacity);
        propertiesProxy->setProperty("FlowValue", flow);
        delete propertiesProxy;
    } else {
        m_resourceProvider->setOpacity(opacity);
    }

    m_presetsPopup->resourceSelected(m_resourceProvider->currentPreset().data());
}

void KisPaintopBox::slotSlider1Changed()
{
    sliderChanged(0);
}

void KisPaintopBox::slotSlider2Changed()
{
    sliderChanged(1);
}

void KisPaintopBox::slotSlider3Changed()
{
    sliderChanged(2);
}

void KisPaintopBox::slotToolChanged(KoCanvasController* canvas, int toolId)
{
    Q_UNUSED(canvas);
    Q_UNUSED(toolId);

    if (!m_viewManager->canvasBase()) return;

    QString  id   = KoToolManager::instance()->activeToolId();
    KisTool* tool = dynamic_cast<KisTool*>(KoToolManager::instance()->toolById(m_viewManager->canvasBase(), id));

    if (tool) {
        int flags = tool->flags();

        if (flags & KisTool::FLAG_USES_CUSTOM_COMPOSITEOP) {
            setWidgetState(ENABLE_COMPOSITEOP | ENABLE_OPACITY);
        } else                                              {
            setWidgetState(DISABLE_COMPOSITEOP | DISABLE_OPACITY);
        }

        if (flags & KisTool::FLAG_USES_CUSTOM_PRESET) {
            setWidgetState(ENABLE_PRESETS | ENABLE_SIZE | ENABLE_FLOW);
            slotUpdatePreset();
            m_presetsEnabled = true;
        } else {
            setWidgetState(DISABLE_PRESETS | DISABLE_SIZE | DISABLE_FLOW);
            m_presetsEnabled = false;
        }
    } else setWidgetState(DISABLE_ALL);
}

void KisPaintopBox::slotPreviousFavoritePreset()
{
    if (!m_favoriteResourceManager) return;

    int i = 0;
    Q_FOREACH (KisPaintOpPresetSP preset, m_favoriteResourceManager->favoritePresetList()) {
        if (m_resourceProvider->currentPreset() && m_resourceProvider->currentPreset()->name() == preset->name()) {
            if (i > 0) {
                m_favoriteResourceManager->slotChangeActivePaintop(i - 1);
            } else {
                m_favoriteResourceManager->slotChangeActivePaintop(m_favoriteResourceManager->numFavoritePresets() - 1);
            }
            return;
        }
        i++;
    }

}

void KisPaintopBox::slotNextFavoritePreset()
{
    if (!m_favoriteResourceManager) return;

    int i = 0;
    Q_FOREACH (KisPaintOpPresetSP preset, m_favoriteResourceManager->favoritePresetList()) {
        if (m_resourceProvider->currentPreset()->name() == preset->name()) {
            if (i < m_favoriteResourceManager->numFavoritePresets() - 1) {
                m_favoriteResourceManager->slotChangeActivePaintop(i + 1);
            } else {
                m_favoriteResourceManager->slotChangeActivePaintop(0);
            }
            return;
        }
        i++;
    }
}

void KisPaintopBox::slotSwitchToPreviousPreset()
{
    if (m_resourceProvider->previousPreset()) {
        setCurrentPaintop(m_resourceProvider->previousPreset()->paintOp(), m_resourceProvider->previousPreset());
    }
}

void KisPaintopBox::slotUnsetEraseMode()
{
    m_eraseAction->setChecked(false);
}

void KisPaintopBox::slotToggleAlphaLockMode(bool checked)
{
    if (checked) {
        m_alphaLockButton->actions()[0]->setIcon(KisIconUtils::loadIcon("transparency-locked"));
    } else {
        m_alphaLockButton->actions()[0]->setIcon(KisIconUtils::loadIcon("transparency-unlocked"));
    }
    m_resourceProvider->setGlobalAlphaLock(checked);
}

void KisPaintopBox::slotDisablePressureMode(bool checked)
{
    if (checked) {
        m_disablePressureButton->actions()[0]->setIcon(KisIconUtils::loadIcon("transform_icons_penPressure"));
    } else {
        m_disablePressureButton->actions()[0]->setIcon(KisIconUtils::loadIcon("transform_icons_penPressure_locked"));
    }

    m_resourceProvider->setDisablePressure(checked);
}

void KisPaintopBox::slotReloadPreset()
{
    KisSignalsBlocker blocker(m_optionWidget);

    //Here using the name and fetching the preset from the server was the only way the load was working. Otherwise it was not loading.
    KisPaintOpPresetResourceServer * rserver = KisResourceServerProvider::instance()->paintOpPresetServer();
    KisPaintOpPresetSP preset = rserver->resourceByName(m_resourceProvider->currentPreset()->name());
    if (preset) {
        preset->load();
    }
}
void KisPaintopBox::slotGuiChangedCurrentPreset() // Called only when UI is changed and not when preset is changed
{
    m_optionWidget->writeConfigurationSafe(const_cast<KisPaintOpSettings*>(m_resourceProvider->currentPreset()->settings().data()));

    //m_presetsPopup->resourceSelected(m_resourceProvider->currentPreset().data());

    // TODO!!!!!!!!
    //m_presetsPopup->updateViewSettings();
}
void KisPaintopBox::slotSaveLockedOptionToPreset(KisPropertiesConfiguration* p)
{

    QMapIterator<QString, QVariant> i(p->getProperties());
    while (i.hasNext()) {
        i.next();
        m_resourceProvider->currentPreset()->settings()->setProperty(i.key(), QVariant(i.value()));
        if (m_resourceProvider->currentPreset()->settings()->hasProperty(i.key() + "_previous")) {
            m_resourceProvider->currentPreset()->settings()->removeProperty(i.key() + "_previous");
        }

    }
    slotGuiChangedCurrentPreset();

}

void KisPaintopBox::slotDropLockedOption(KisPropertiesConfiguration* p)
{
    KisSignalsBlocker blocker(m_optionWidget);
    KisPaintOpPresetSP preset = m_resourceProvider->currentPreset();

    {
        KisPaintOpPreset::DirtyStateSaver dirtySaver(preset.data());

        QMapIterator<QString, QVariant> i(p->getProperties());
        while (i.hasNext()) {
            i.next();
            if (preset->settings()->hasProperty(i.key() + "_previous")) {
                preset->settings()->setProperty(i.key(), preset->settings()->getProperty(i.key() + "_previous"));
                preset->settings()->removeProperty(i.key() + "_previous");
            }

        }
    }

    //slotUpdatePreset();
}
void KisPaintopBox::slotDirtyPresetToggled(bool value)
{
    if (!value) {
        slotReloadPreset();
        m_presetsPopup->resourceSelected(m_resourceProvider->currentPreset().data());
        m_presetsPopup->updateViewSettings();
    }
    m_dirtyPresetsEnabled = value;
    KisConfig cfg;
    cfg.setUseDirtyPresets(m_dirtyPresetsEnabled);

}
void KisPaintopBox::slotEraserBrushSizeToggled(bool value)
{
    m_eraserBrushSizeEnabled = value;
    KisConfig cfg;
    cfg.setUseEraserBrushSize(m_eraserBrushSizeEnabled);
}

void KisPaintopBox::slotUpdateSelectionIcon()
{
    m_hMirrorAction->setIcon(KisIconUtils::loadIcon("symmetry-horizontal"));
    m_vMirrorAction->setIcon(KisIconUtils::loadIcon("symmetry-vertical"));

    KisConfig cfg;
    if (!cfg.toolOptionsInDocker()) {
        m_toolOptionsPopupButton->setIcon(KisIconUtils::loadIcon("configure"));
    }

    m_presetSelectorPopupButton->setIcon(KisIconUtils::loadIcon("paintop_settings_01"));
    m_brushEditorPopupButton->setIcon(KisIconUtils::loadIcon("paintop_settings_02"));
    m_workspaceWidget->setIcon(KisIconUtils::loadIcon("view-choose"));

    m_eraseAction->setIcon(KisIconUtils::loadIcon("draw-eraser"));
    m_reloadAction->setIcon(KisIconUtils::loadIcon("view-refresh"));

    if (m_disablePressureAction->isChecked()) {
        m_disablePressureButton->setIcon(KisIconUtils::loadIcon("transform_icons_penPressure"));
    } else {
        m_disablePressureButton->setIcon(KisIconUtils::loadIcon("transform_icons_penPressure_locked"));
    }
}

void KisPaintopBox::slotLockXMirrorToggle(bool toggleLock) {
    m_resourceProvider->setMirrorHorizontalLock(toggleLock);
}

void KisPaintopBox::slotLockYMirrorToggle(bool toggleLock) {
    m_resourceProvider->setMirrorVerticalLock(toggleLock);
}

void KisPaintopBox::slotHideDecorationMirrorX(bool toggled) {
    m_resourceProvider->setMirrorHorizontalHideDecorations(toggled);
}

void KisPaintopBox::slotHideDecorationMirrorY(bool toggled) {
    m_resourceProvider->setMirrorVerticalHideDecorations(toggled);
}



void KisPaintopBox::slotMoveToCenterMirrorX() {
  m_resourceProvider->mirrorHorizontalMoveCanvasToCenter();
}

void KisPaintopBox::slotMoveToCenterMirrorY() {
  m_resourceProvider->mirrorVerticalMoveCanvasToCenter();
}

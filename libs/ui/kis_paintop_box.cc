/*
 *  kis_paintop_box.cc - part of KImageShop/Krayon/Krita
 *
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt (boud@valdyas.org)
 *  SPDX-FileCopyrightText: 2009-2011 Sven Langkamp (sven.langkamp@gmail.com)
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *  SPDX-FileCopyrightText: 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *  SPDX-FileCopyrightText: 2014 Mohit Goyal <mohit.bits2011@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_paintop_box.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QPixmap>
#include <QWidgetAction>
#include <QApplication>
#include <QMenu>
#include <QTime>

#include <kis_debug.h>
#include <kis_types.h>

#include <kactioncollection.h>
#include <kacceleratormanager.h>
#include <QKeySequence>

#include <kis_icon.h>
#include <KoColorSpace.h>
#include <KoCompositeOpRegistry.h>
#include <KoToolManager.h>
#include <KoColorSpaceRegistry.h>

#include <KoResource.h>
#include <KisResourceDirtyStateSaver.h>

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
#include "KisResourceServerProvider.h"
#include "kis_favorite_resource_manager.h"
#include "kis_config.h"

#include "KisPopupButton.h"
#include "widgets/kis_iconwidget.h"
#include "widgets/kis_tool_options_popup.h"
#include "widgets/kis_paintop_presets_popup.h"
#include "widgets/kis_paintop_presets_chooser_popup.h"
#include "widgets/kis_workspace_chooser.h"
#include "widgets/kis_paintop_list_widget.h"
#include "kis_slider_spin_box.h"
#include "widgets/kis_multipliers_double_slider_spinbox.h"
#include "widgets/kis_cmb_composite.h"
#include "widgets/kis_widget_chooser.h"
#include "tool/kis_tool.h"
#include "kis_signals_blocker.h"
#include "kis_action_manager.h"
#include "KisHighlightedToolButton.h"
#include <KisGlobalResourcesInterface.h>

KisPaintopBox::KisPaintopBox(KisViewManager *view, QWidget *parent, const char *name)
    : QWidget(parent)
    , m_resourceProvider(view->canvasResourceProvider())
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
    , m_eraserBrushOpacityEnabled(false)
{
    Q_ASSERT(view != 0);


    setObjectName(name);
    KisConfig cfg(true);
    m_dirtyPresetsEnabled = cfg.useDirtyPresets();
    m_eraserBrushSizeEnabled = cfg.useEraserBrushSize();
    m_eraserBrushOpacityEnabled = cfg.useEraserBrushOpacity();

    KAcceleratorManager::setNoAccel(this);

    setWindowTitle(i18n("Painter's Toolchest"));

    m_favoriteResourceManager = new KisFavoriteResourceManager(this);


    KConfigGroup grp =  KSharedConfig::openConfig()->group("krita").group("Toolbar BrushesAndStuff");
    int iconsize = grp.readEntry("IconSize", 22);
    // NOTE: buttonsize should be the same value as the one used in ktoolbar for all QToolButton
    int buttonsize = grp.readEntry("ButtonSize", 32);

    if (!cfg.toolOptionsInDocker()) {
        m_toolOptionsPopupButton = new KisPopupButton(this);
        m_toolOptionsPopupButton->setIcon(KisIconUtils::loadIcon("view-choose"));
        m_toolOptionsPopupButton->setToolTip(i18n("Tool Settings"));
        m_toolOptionsPopupButton->setFixedSize(buttonsize, buttonsize);
        m_toolOptionsPopupButton->setIconSize(QSize(iconsize, iconsize));
        m_toolOptionsPopupButton->setFlat(true);
    }

    m_brushEditorPopupButton = new KisIconWidget(this);
    m_brushEditorPopupButton->setIcon(KisIconUtils::loadIcon("paintop_settings_02"));
    m_brushEditorPopupButton->setToolTip(i18n("Edit brush settings"));
    m_brushEditorPopupButton->setFixedSize(buttonsize, buttonsize);
    m_brushEditorPopupButton->setIconSize(QSize(iconsize, iconsize));
    m_brushEditorPopupButton->setFlat(true);

    m_presetSelectorPopupButton = new KisPopupButton(this);
    m_presetSelectorPopupButton->setIcon(KisIconUtils::loadIcon("paintop_settings_01"));
    m_presetSelectorPopupButton->setToolTip(i18n("Choose brush preset"));
    m_presetSelectorPopupButton->setFixedSize(buttonsize, buttonsize);
    m_presetSelectorPopupButton->setIconSize(QSize(iconsize, iconsize));
    m_presetSelectorPopupButton->setFlat(true);
    m_presetSelectorPopupButton->setArrowVisible(false);

    m_eraseModeButton = new KisHighlightedToolButton(this);
    m_eraseModeButton->setFixedSize(buttonsize, buttonsize);
    m_eraseModeButton->setIconSize(QSize(iconsize, iconsize));
    m_eraseModeButton->setCheckable(true);
    m_eraseModeButton->setAutoRaise(true);


    m_eraseAction = m_viewManager->actionManager()->createAction("erase_action");
    m_eraseModeButton->setDefaultAction(m_eraseAction);

    m_reloadButton = new QToolButton(this);
    m_reloadButton->setFixedSize(buttonsize, buttonsize);
    m_reloadButton->setIconSize(QSize(iconsize, iconsize));
    m_reloadButton->setAutoRaise(true); // make button flat


    m_reloadAction = m_viewManager->actionManager()->createAction("reload_preset_action");
    m_reloadButton->setDefaultAction(m_reloadAction);

    m_alphaLockButton = new KisHighlightedToolButton(this);
    m_alphaLockButton->setFixedSize(buttonsize, buttonsize);
    m_alphaLockButton->setIconSize(QSize(iconsize, iconsize));
    m_alphaLockButton->setCheckable(true);
    m_alphaLockButton->setAutoRaise(true);

    KisAction* alphaLockAction = m_viewManager->actionManager()->createAction("preserve_alpha");
    m_alphaLockButton->setDefaultAction(alphaLockAction);

    // horizontal and vertical mirror toolbar buttons

    // mirror tool options for the X Mirror
    QMenu *toolbarMenuXMirror = new QMenu();

    hideCanvasDecorationsX = m_viewManager->actionManager()->createAction("mirrorX-hideDecorations");
    toolbarMenuXMirror->addAction(hideCanvasDecorationsX);

    lockActionX = m_viewManager->actionManager()->createAction("mirrorX-lock");
    toolbarMenuXMirror->addAction(lockActionX);

    moveToCenterActionX = m_viewManager->actionManager()->createAction("mirrorX-moveToCenter");
    toolbarMenuXMirror->addAction(moveToCenterActionX);


    // mirror tool options for the Y Mirror
    QMenu *toolbarMenuYMirror = new QMenu();

    hideCanvasDecorationsY = m_viewManager->actionManager()->createAction("mirrorY-hideDecorations");
    toolbarMenuYMirror->addAction(hideCanvasDecorationsY);


    lockActionY = m_viewManager->actionManager()->createAction("mirrorY-lock");
    toolbarMenuYMirror->addAction(lockActionY);

    moveToCenterActionY = m_viewManager->actionManager()->createAction("mirrorY-moveToCenter");
    toolbarMenuYMirror->addAction(moveToCenterActionY);



    // create horizontal and vertical mirror buttons

    m_hMirrorButton = new KisHighlightedToolButton(this);
    int menuPadding = 20;
    m_hMirrorButton->setFixedSize(buttonsize + menuPadding, buttonsize);
    m_hMirrorButton->setIconSize(QSize(iconsize, iconsize));
    m_hMirrorButton->setCheckable(true);
    m_hMirrorAction = m_viewManager->actionManager()->createAction("hmirror_action");
    m_hMirrorButton->setDefaultAction(m_hMirrorAction);
    m_hMirrorButton->setMenu(toolbarMenuXMirror);
    m_hMirrorButton->setPopupMode(QToolButton::MenuButtonPopup);
    m_hMirrorButton->setAutoRaise(true);

    m_vMirrorButton = new KisHighlightedToolButton(this);
    m_vMirrorButton->setFixedSize(buttonsize + menuPadding, buttonsize);
    m_vMirrorButton->setIconSize(QSize(iconsize, iconsize));
    m_vMirrorButton->setCheckable(true);
    m_vMirrorAction = m_viewManager->actionManager()->createAction("vmirror_action");
    m_vMirrorButton->setDefaultAction(m_vMirrorAction);
    m_vMirrorButton->setMenu(toolbarMenuYMirror);
    m_vMirrorButton->setPopupMode(QToolButton::MenuButtonPopup);
    m_vMirrorButton->setAutoRaise(true);

    QAction *wrapAroundAction = m_viewManager->actionManager()->createAction("wrap_around_mode");

    m_wrapAroundButton = new KisHighlightedToolButton(this);
    m_wrapAroundButton->setFixedSize(buttonsize, buttonsize);
    m_wrapAroundButton->setIconSize(QSize(iconsize, iconsize));
    m_wrapAroundButton->setDefaultAction(wrapAroundAction);
    m_wrapAroundButton->setCheckable(true);
    m_wrapAroundButton->setAutoRaise(true);

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

    for (int i = 0; i < 4; ++i) {
        m_sliderChooser[i] = new KisWidgetChooser(i + 1);

        KisDoubleSliderSpinBox* slOpacity;
        KisDoubleSliderSpinBox* slFlow;
        KisDoubleSliderSpinBox* slSize;
        KisMultipliersDoubleSliderSpinBox* slPatternSize;
        if (sliderLabels) {
            slOpacity     = m_sliderChooser[i]->addWidget<KisDoubleSliderSpinBox>("opacity");
            slFlow        = m_sliderChooser[i]->addWidget<KisDoubleSliderSpinBox>("flow");
            slSize        = m_sliderChooser[i]->addWidget<KisDoubleSliderSpinBox>("size");
            slPatternSize = m_sliderChooser[i]->addWidget<KisMultipliersDoubleSliderSpinBox>("patternsize");
            slOpacity->setPrefix(QString("%1 ").arg(i18n("Opacity:")));
            slFlow->setPrefix(QString("%1 ").arg(i18n("Flow:")));
            slSize->setPrefix(QString("%1 ").arg(i18n("Size:")));
            slPatternSize->setPrefix(QString("%1 ").arg(i18n("Pattern Scale:")));
        }
        else {
            slOpacity = m_sliderChooser[i]->addWidget<KisDoubleSliderSpinBox>("opacity", i18n("Opacity:"));
            slFlow    = m_sliderChooser[i]->addWidget<KisDoubleSliderSpinBox>("flow", i18n("Flow:"));
            slSize    = m_sliderChooser[i]->addWidget<KisDoubleSliderSpinBox>("size", i18n("Size:"));
            slPatternSize = m_sliderChooser[i]->addWidget<KisMultipliersDoubleSliderSpinBox>("patternsize", i18n("Pattern Scale:"));
        }

        slOpacity->setRange(0, 100, 0);
        slOpacity->setValue(100);
        slOpacity->setSingleStep(5);
        slOpacity->setSuffix(i18n("%"));
        slOpacity->setMinimumWidth(qMax(sliderWidth, slOpacity->sizeHint().width()));
        slOpacity->setFixedHeight(buttonsize);
        slOpacity->setBlockUpdateSignalOnDrag(true);

        slFlow->setRange(0, 100, 0);
        slFlow->setValue(100);
        slFlow->setSingleStep(5);
        slFlow->setSuffix(i18n("%"));
        slFlow->setMinimumWidth(qMax(sliderWidth, slFlow->sizeHint().width()));
        slFlow->setFixedHeight(buttonsize);
        slFlow->setBlockUpdateSignalOnDrag(true);

        slSize->setRange(0.01, cfg.readEntry("maximumBrushSize", 1000), 2);
        slSize->setValue(100);

        slSize->setSingleStep(1);
        slSize->setExponentRatio(3.0);
        slSize->setSuffix(i18n(" px"));
        slSize->setMinimumWidth(qMax(sliderWidth, slSize->sizeHint().width()));
        slSize->setFixedHeight(buttonsize);
        slSize->setBlockUpdateSignalOnDrag(true);

        slPatternSize->setRange(0.0, 2.0, 2);
        slPatternSize->setValue(1.0);
        slPatternSize->addMultiplier(0.1);
        slPatternSize->addMultiplier(2);
        slPatternSize->addMultiplier(10);

        slPatternSize->setSingleStep(.01);
        slPatternSize->setSuffix(i18n("x"));
        slPatternSize->setMinimumWidth(qMax(sliderWidth, slPatternSize->sizeHint().width()));
        slPatternSize->setFixedHeight(buttonsize);
        slPatternSize->setBlockUpdateSignalOnDrag(true);

        m_sliderChooser[i]->setMinimumWidth(qMax(sliderWidth, slPatternSize->sizeHint().width()));

        m_sliderChooser[i]->chooseWidget(cfg.toolbarSlider(i + 1));
    }

    m_cmbCompositeOp = new KisCompositeOpComboBox();
    m_cmbCompositeOp->setFixedHeight(buttonsize);
    m_cmbCompositeOp->connectBlendmodeActions(m_viewManager->actionManager());

    m_workspaceWidget = new KisPopupButton(this);
    m_workspaceWidget->setIcon(KisIconUtils::loadIcon("workspace-chooser"));
    m_workspaceWidget->setToolTip(i18n("Choose workspace"));
    m_workspaceWidget->setFixedSize(buttonsize, buttonsize);
    m_workspaceWidget->setIconSize(QSize(iconsize, iconsize));
    m_workspaceWidget->setPopupWidget(new KisWorkspaceChooser(view));
    m_workspaceWidget->setFlat(true);
    m_workspaceWidget->setArrowVisible(false);

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
    KisActionRegistry::instance()->propertizeAction("brushslider4", action);
    view->actionCollection()->addAction("brushslider4", action);
    action->setDefaultWidget(m_sliderChooser[3]);
    connect(action, SIGNAL(triggered()), m_sliderChooser[3], SLOT(showPopupWidget()));
    connect(m_viewManager->mainWindow(), SIGNAL(themeChanged()), m_sliderChooser[3], SLOT(updateThemedIcons()));

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
    mirrorLayout->addWidget(m_wrapAroundButton);
    mirrorLayout->setSpacing(4);
    mirrorLayout->setContentsMargins(0, 0, 0, 0);



    action = new QWidgetAction(this);
    KisActionRegistry::instance()->propertizeAction("mirror_actions", action);
    action->setDefaultWidget(mirrorActions);
    view->actionCollection()->addAction("mirror_actions", action);

    action = new QWidgetAction(this);
    KisActionRegistry::instance()->propertizeAction(ResourceType::Workspaces, action);
    view->actionCollection()->addAction(ResourceType::Workspaces, action);
    action->setDefaultWidget(m_workspaceWidget);

    if (!cfg.toolOptionsInDocker()) {
        m_toolOptionsPopup = new KisToolOptionsPopup();
        m_toolOptionsPopupButton->setPopupWidget(m_toolOptionsPopup);
        m_toolOptionsPopup->switchDetached(false);
    }


    m_savePresetWidget = new KisPresetSaveWidget(this);

    m_presetsPopup = new KisPaintOpPresetsPopup(m_resourceProvider, m_favoriteResourceManager, m_savePresetWidget);
    m_brushEditorPopupButton->setPopupWidget(m_presetsPopup);
    m_presetsPopup->parentWidget()->setWindowTitle(i18n("Brush Editor"));


    connect(m_presetsPopup, SIGNAL(brushEditorShown()), SLOT(slotUpdateOptionsWidgetPopup()));
    connect(m_viewManager->mainWindow(), SIGNAL(themeChanged()), m_presetsPopup, SLOT(updateThemedIcons()));

    m_presetsChooserPopup = new KisPaintOpPresetsChooserPopup();
    m_presetsChooserPopup->setMinimumHeight(550);
    m_presetsChooserPopup->setMinimumWidth(450);
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
    connect(m_presetsPopup       , SIGNAL(defaultPresetClicked())             , SLOT(slotSetupDefaultPreset()));
    connect(m_presetsPopup       , SIGNAL(signalResourceSelected(KoResourceSP )), SLOT(resourceSelected(KoResourceSP )));
    connect(m_presetsPopup       , SIGNAL(reloadPresetClicked())              , SLOT(slotReloadPreset()));
    connect(m_presetsPopup       , SIGNAL(dirtyPresetToggled(bool))           , SLOT(slotDirtyPresetToggled(bool)));
    connect(m_presetsPopup       , SIGNAL(eraserBrushSizeToggled(bool))       , SLOT(slotEraserBrushSizeToggled(bool)));
    connect(m_presetsPopup       , SIGNAL(eraserBrushOpacityToggled(bool))       , SLOT(slotEraserBrushOpacityToggled(bool)));

    connect(m_presetsPopup, SIGNAL(createPresetFromScratch(QString)), this, SLOT(slotCreatePresetFromScratch(QString)));

    connect(m_presetsChooserPopup, SIGNAL(resourceSelected(KoResourceSP ))      , SLOT(resourceSelected(KoResourceSP )));
    connect(m_presetsChooserPopup, SIGNAL(resourceClicked(KoResourceSP ))      , SLOT(resourceSelected(KoResourceSP )));

    connect(m_resourceProvider   , SIGNAL(sigNodeChanged(KisNodeSP))    , SLOT(slotNodeChanged(KisNodeSP)));
    connect(m_cmbCompositeOp     , SIGNAL(currentIndexChanged(int))           , SLOT(slotSetCompositeMode(int)));
    connect(m_eraseAction          , SIGNAL(toggled(bool))                    , SLOT(slotToggleEraseMode(bool)));
    connect(alphaLockAction      , SIGNAL(toggled(bool))                    , SLOT(slotToggleAlphaLockMode(bool)));

    m_disablePressureAction = m_viewManager->actionManager()->createAction("disable_pressure");
    connect(m_disablePressureAction  , SIGNAL(toggled(bool))                    , SLOT(slotDisablePressureMode(bool)));
    m_disablePressureAction->setChecked(true);

    connect(m_hMirrorAction        , SIGNAL(toggled(bool))                    , SLOT(slotHorizontalMirrorChanged(bool)));
    connect(m_vMirrorAction        , SIGNAL(toggled(bool))                    , SLOT(slotVerticalMirrorChanged(bool)));
    connect(m_reloadAction         , SIGNAL(triggered())                        , SLOT(slotReloadPreset()));

    connect(m_sliderChooser[0]->getWidget<KisDoubleSliderSpinBox>("opacity")               , SIGNAL(valueChanged(qreal)), SLOT(slotSlider1Changed()));
    connect(m_sliderChooser[0]->getWidget<KisDoubleSliderSpinBox>("flow")                  , SIGNAL(valueChanged(qreal)), SLOT(slotSlider1Changed()));
    connect(m_sliderChooser[0]->getWidget<KisDoubleSliderSpinBox>("size")                  , SIGNAL(valueChanged(qreal)), SLOT(slotSlider1Changed()));
    connect(m_sliderChooser[0]->getWidget<KisMultipliersDoubleSliderSpinBox>("patternsize"), SIGNAL(valueChanged(qreal)), SLOT(slotSlider1Changed()));
    connect(m_sliderChooser[1]->getWidget<KisDoubleSliderSpinBox>("opacity")               , SIGNAL(valueChanged(qreal)), SLOT(slotSlider2Changed()));
    connect(m_sliderChooser[1]->getWidget<KisDoubleSliderSpinBox>("flow")                  , SIGNAL(valueChanged(qreal)), SLOT(slotSlider2Changed()));
    connect(m_sliderChooser[1]->getWidget<KisDoubleSliderSpinBox>("size")                  , SIGNAL(valueChanged(qreal)), SLOT(slotSlider2Changed()));
    connect(m_sliderChooser[1]->getWidget<KisMultipliersDoubleSliderSpinBox>("patternsize"), SIGNAL(valueChanged(qreal)), SLOT(slotSlider2Changed()));
    connect(m_sliderChooser[2]->getWidget<KisDoubleSliderSpinBox>("opacity")               , SIGNAL(valueChanged(qreal)), SLOT(slotSlider3Changed()));
    connect(m_sliderChooser[2]->getWidget<KisDoubleSliderSpinBox>("flow")                  , SIGNAL(valueChanged(qreal)), SLOT(slotSlider3Changed()));
    connect(m_sliderChooser[2]->getWidget<KisDoubleSliderSpinBox>("size")                  , SIGNAL(valueChanged(qreal)), SLOT(slotSlider3Changed()));
    connect(m_sliderChooser[2]->getWidget<KisMultipliersDoubleSliderSpinBox>("patternsize"), SIGNAL(valueChanged(qreal)), SLOT(slotSlider3Changed()));
    connect(m_sliderChooser[3]->getWidget<KisDoubleSliderSpinBox>("opacity")               , SIGNAL(valueChanged(qreal)), SLOT(slotSlider4Changed()));
    connect(m_sliderChooser[3]->getWidget<KisDoubleSliderSpinBox>("flow")                  , SIGNAL(valueChanged(qreal)), SLOT(slotSlider4Changed()));
    connect(m_sliderChooser[3]->getWidget<KisDoubleSliderSpinBox>("size")                  , SIGNAL(valueChanged(qreal)), SLOT(slotSlider4Changed()));
    connect(m_sliderChooser[3]->getWidget<KisMultipliersDoubleSliderSpinBox>("patternsize"), SIGNAL(valueChanged(qreal)), SLOT(slotSlider4Changed()));

    connect(m_resourceProvider, SIGNAL(sigFGColorUsed(KoColor)), m_favoriteResourceManager, SLOT(slotAddRecentColor(KoColor)));

    connect(m_resourceProvider, SIGNAL(sigFGColorChanged(KoColor)), m_favoriteResourceManager, SLOT(slotChangeFGColorSelector(KoColor)));
    connect(m_resourceProvider, SIGNAL(sigBGColorChanged(KoColor)), m_favoriteResourceManager, SLOT(slotSetBGColor(KoColor)));
    // cold initialization
    m_favoriteResourceManager->slotChangeFGColorSelector(m_resourceProvider->fgColor());
    m_favoriteResourceManager->slotSetBGColor(m_resourceProvider->bgColor());

    connect(m_favoriteResourceManager, SIGNAL(sigSetFGColor(KoColor)), m_resourceProvider, SLOT(slotSetFGColor(KoColor)));
    connect(m_favoriteResourceManager, SIGNAL(sigSetBGColor(KoColor)), m_resourceProvider, SLOT(slotSetBGColor(KoColor)));

    connect(view->mainWindow(), SIGNAL(themeChanged()), this, SLOT(slotUpdateSelectionIcon()));
    connect(m_resourceProvider->resourceManager(), SIGNAL(canvasResourceChanged(int,QVariant)),
            this, SLOT(slotCanvasResourceChanged(int,QVariant)));
    connect(m_resourceProvider->resourceManager(), SIGNAL(canvasResourceChangeAttempted(int,QVariant)),
            this, SLOT(slotCanvasResourceChangeAttempted(int,QVariant)));

    slotInputDeviceChanged(KoToolManager::instance()->currentInputDevice());

    findDefaultPresets();
}


KisPaintopBox::~KisPaintopBox()
{
    KisConfig cfg(false);
    QMapIterator<TabletToolID, TabletToolData> iter(m_tabletToolMap);
    while (iter.hasNext()) {
        iter.next();
        if ((iter.key().pointer) == QTabletEvent::Eraser) {
            cfg.writeEntry(QString("LastEraser_%1").arg(iter.key().uniqueTabletId), iter.value().preset->name());
        }
        else {
            cfg.writeEntry(QString("LastPreset_%1").arg(iter.key().uniqueTabletId), iter.value().preset->name());
        }
    }
    // Do not delete the widget, since it is global to the application, not owned by the view
    m_presetsPopup->setPaintOpSettingsWidget(0);
    qDeleteAll(m_paintopOptionWidgets);
    delete m_favoriteResourceManager;

    for (int i = 0; i < 3; ++i) {
        delete m_sliderChooser[i];
    }
}

void KisPaintopBox::restoreResource(KoResourceSP resource)
{
    KisPaintOpPresetSP preset = resource.dynamicCast<KisPaintOpPreset>();

    if (preset) {
        setCurrentPaintop(preset);

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

void KisPaintopBox::resourceSelected(KoResourceSP resource)
{
    // This happens if no storages were available on startup
    if (!m_optionWidget) {
        KisPaintOpPresetSP preset = resource.dynamicCast<KisPaintOpPreset>();
        setCurrentPaintop(preset);
        return;
    }

    m_presetsPopup->setCreatingBrushFromScratch(false); // show normal UI elements when we are not creating

    KisPaintOpPresetSP preset = resource.dynamicCast<KisPaintOpPreset>();

    if (preset && preset->valid() && preset != m_resourceProvider->currentPreset()) {
        if (!m_dirtyPresetsEnabled) {
            KisSignalsBlocker blocker(m_optionWidget);
            Q_UNUSED(blocker);

            KisPaintOpPresetResourceServer *rserver = KisResourceServerProvider::instance()->paintOpPresetServer();

            if (!rserver->reloadResource(preset)) {
                qWarning() << "failed to reload the preset.";
            }
        }

        dbgResources << "resourceSelected: preset" << preset << (preset ? QString("%1").arg(preset->valid()) : "");
        setCurrentPaintop(preset);

        m_presetsPopup->setPresetImage(preset->image());
        m_presetsPopup->resourceSelected(resource);
    }
}

void KisPaintopBox::setCurrentPaintop(const KoID& paintop)
{
    KisPaintOpPresetSP preset = activePreset(paintop);
    Q_ASSERT(preset && preset->settings());
    setCurrentPaintop(preset);
}

void KisPaintopBox::setCurrentPaintop(KisPaintOpPresetSP preset)
{
    if (preset == m_resourceProvider->currentPreset()) {

        if (preset == m_tabletToolMap[m_currTabletToolID].preset) {
            return;
        }
    }
    Q_ASSERT(preset);
    const KoID& paintop = preset->paintOp();
    m_presetConnections.clear();

    if (m_resourceProvider->currentPreset()) {

        m_resourceProvider->setPreviousPaintOpPreset(m_resourceProvider->currentPreset());

        if (m_optionWidget) {
            m_optionWidget->hide();
        }

    }

    if (!m_paintopOptionWidgets.contains(paintop)) {
        m_paintopOptionWidgets[paintop] = KisPaintOpRegistry::instance()->get(paintop.id())->createConfigWidget(this);
        m_paintopOptionWidgets[paintop]->setResourcesInterface(KisGlobalResourcesInterface::instance());
        m_paintopOptionWidgets[paintop]->setCanvasResourcesInterface(m_viewManager->canvasResourceProvider()->resourceManager()->canvasResourcesInterface());
    }

    m_optionWidget = m_paintopOptionWidgets[paintop];

    KisSignalsBlocker b(m_optionWidget);

    preset->setOptionsWidget(m_optionWidget);

    m_optionWidget->setImage(m_viewManager->image());
    m_optionWidget->setNode(m_viewManager->activeNode());

    m_presetsPopup->setPaintOpSettingsWidget(m_optionWidget);

    m_resourceProvider->setPaintOpPreset(preset);

    Q_ASSERT(m_optionWidget && m_presetSelectorPopupButton);

    m_presetConnections.addConnection(m_optionWidget, SIGNAL(sigConfigurationUpdated()), this, SLOT(slotGuiChangedCurrentPreset()));
    m_presetConnections.addConnection(m_optionWidget, SIGNAL(sigSaveLockedConfig(KisPropertiesConfigurationSP)), this, SLOT(slotSaveLockedOptionToPreset(KisPropertiesConfigurationSP)));
    m_presetConnections.addConnection(m_optionWidget, SIGNAL(sigDropLockedConfig(KisPropertiesConfigurationSP)), this, SLOT(slotDropLockedOption(KisPropertiesConfigurationSP)));


    // load the current brush engine icon for the brush editor toolbar button
    m_brushEditorPopupButton->setThumbnail(preset->image());
    m_presetsPopup->setCurrentPaintOpId(paintop.id());


    //o() << "\tsetting the new preset for" << m_currTabletToolID.uniqueID << "to" << preset->name();
    m_paintOpPresetMap[m_resourceProvider->currentPreset()->paintOp()] = preset;
    m_tabletToolMap[m_currTabletToolID].preset = preset;
    m_tabletToolMap[m_currTabletToolID].paintOpID = preset->paintOp();


    if (m_presetsPopup->currentPaintOpId() != paintop.id()) {
        // Must change the paintop as the current one is not supported
        // by the new colorspace.
        dbgKrita << "current paintop " << paintop.name() << " was not set, not supported by colorspace";
    }

    m_currCompositeOpID = preset->settings()->paintOpCompositeOp();
    updateCompositeOp(m_currCompositeOpID);

    if (preset->settings()->hasPatternSettings()) {
        setMultiplierSliderValue("patternsize", preset->settings()->paintOpPatternSize());
        setWidgetState(ENABLE_PATTERNSIZE);
    }
    else {
        setWidgetState(DISABLE_PATTERNSIZE);
    }
}

void KisPaintopBox::slotUpdateOptionsWidgetPopup()
{
    KisPaintOpPresetSP preset = m_resourceProvider->currentPreset();

    // This happens when we have a new brush engine for which no default preset exists yet.
    if (!preset) return;

    KIS_SAFE_ASSERT_RECOVER_RETURN(preset);
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_optionWidget);

    m_optionWidget->setConfigurationSafe(preset->settings());

    m_presetsPopup->resourceSelected(preset);
    m_presetsPopup->updateViewSettings();

    // the m_viewManager->image() is set earlier, but the reference will be missing when the stamp button is pressed
    // need to later do some research on how and when we should be using weak shared pointers (WSP) that creates this situation
    m_optionWidget->setImage(m_viewManager->image());
}

KisPaintOpPresetSP KisPaintopBox::defaultPreset(const KoID& paintOp)
{
    QString path = ":/presets/" + paintOp.id() + ".kpp";
    dbgResources << "Getting default presets from qrc resources" << path;

    KisPaintOpPresetSP preset(new KisPaintOpPreset(path));

    if (!preset->load(KisGlobalResourcesInterface::instance())) {
        preset = KisPaintOpRegistry::instance()->defaultPreset(paintOp, KisGlobalResourcesInterface::instance());
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
        if (compositeOpID == COMPOSITE_ERASE || m_resourceProvider->eraserMode()) {
            m_eraseModeButton->setChecked(true);
        }
        else {
            m_eraseModeButton->setChecked(false);
        }
    }
    else if (!node) {
        KisSignalsBlocker b1(m_cmbCompositeOp);
        m_cmbCompositeOp->selectCompositeOp(KoID(compositeOpID));
        m_currCompositeOpID = compositeOpID;

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

    for (int i = 0; i < 4; ++i) {
        if (flags & (ENABLE_OPACITY | DISABLE_OPACITY))
            m_sliderChooser[i]->getWidget("opacity")->setEnabled(flags & ENABLE_OPACITY);

        if (flags & (ENABLE_FLOW | DISABLE_FLOW))
            m_sliderChooser[i]->getWidget("flow")->setEnabled(flags & ENABLE_FLOW);

        if (flags & (ENABLE_SIZE | DISABLE_SIZE))
            m_sliderChooser[i]->getWidget("size")->setEnabled(flags & ENABLE_SIZE);

        if (flags & (ENABLE_PATTERNSIZE | DISABLE_PATTERNSIZE))
            m_sliderChooser[i]->getWidget("patternsize")->setEnabled(flags & ENABLE_PATTERNSIZE);
    }
}

void KisPaintopBox::setSliderValue(const QString& sliderID, qreal value)
{
    for (int i = 0; i < 4; ++i) {
        KisDoubleSliderSpinBox* slider = m_sliderChooser[i]->getWidget<KisDoubleSliderSpinBox>(sliderID);
        KisSignalsBlocker b(slider);

        if (sliderID == "opacity" || sliderID == "flow") { // opacity and flows UI stored at 0-100%
            slider->setValue(value*100);
        } else {
            slider->setValue(value); // brush size
        }


    }
}

void KisPaintopBox::setMultiplierSliderValue(const QString& sliderID, qreal value)
{
    for (int i = 0; i < 4; ++i) {
        KisMultipliersDoubleSliderSpinBox* slider = m_sliderChooser[i]->getWidget<KisMultipliersDoubleSliderSpinBox>(sliderID);
        if (!slider) continue;
        KisSignalsBlocker b(slider);

        slider->setValue(value); // brush pattern size
    }
}

void KisPaintopBox::slotSetPaintop(const QString& paintOpId)
{
    if (KisPaintOpRegistry::instance()->get(paintOpId) != 0) {
        KoID id(paintOpId, KisPaintOpRegistry::instance()->get(paintOpId)->name());
        //qDebug() << "slotsetpaintop" << id;
        setCurrentPaintop(id);
    }
}

void KisPaintopBox::slotInputDeviceChanged(const KoInputDevice& inputDevice)
{
    TabletToolMap::iterator toolData = m_tabletToolMap.find(inputDevice);

    //qDebug() << "slotInputDeviceChanged()" << inputDevice.device() << inputDevice.uniqueTabletId();

    m_currTabletToolID = TabletToolID(inputDevice);

    if (toolData == m_tabletToolMap.end()) {
        KisConfig cfg(true);
        KisPaintOpPresetResourceServer *rserver = KisResourceServerProvider::instance()->paintOpPresetServer();
        KisPaintOpPresetSP preset;
        if (inputDevice.pointer() == QTabletEvent::Eraser) {
            preset = rserver->resourceByName(cfg.readEntry<QString>(QString("LastEraser_%1").arg(inputDevice.uniqueTabletId()), m_eraserName));
        }
        else {
            preset = rserver->resourceByName(cfg.readEntry<QString>(QString("LastPreset_%1").arg(inputDevice.uniqueTabletId()), m_defaultPresetName));
            //if (preset)
            //qDebug() << "found stored preset " << preset->name() << "for" << inputDevice.uniqueTabletId();
            //else
            //qDebug() << "no preset found for" << inputDevice.uniqueTabletId();
        }
        if (!preset) {
            preset = rserver->resourceByName(m_defaultPresetName);
        }
        if (preset) {
            //qDebug() << "inputdevicechanged 1" << preset;
            setCurrentPaintop(preset);
        }
    }
    else {
        if (toolData->preset) {
            //qDebug() << "inputdevicechanged 2" << toolData->preset;
            setCurrentPaintop(toolData->preset);
        }
        else {
            //qDebug() << "inputdevicechanged 3" << toolData->paintOpID;
            setCurrentPaintop(toolData->paintOpID);
        }
    }
}

void KisPaintopBox::slotCreatePresetFromScratch(QString paintop)
{
    //First try to select an available default preset for that engine. If it doesn't exist, then
    //manually set the engine to use a new preset.
    KoID id(paintop, KisPaintOpRegistry::instance()->get(paintop)->name());
    KisPaintOpPresetSP preset = defaultPreset(id);

    slotSetPaintop(paintop);  // change the paintop settings area and update the UI

    if (!preset) {
        m_presetsPopup->setCreatingBrushFromScratch(true); // disable UI elements while creating from scratch
        preset = m_resourceProvider->currentPreset();
    } else {
        m_resourceProvider->setPaintOpPreset(preset);
        preset->setOptionsWidget(m_optionWidget);
    }
    m_presetsPopup->resourceSelected(preset);  // this helps update the UI on the brush editor
}

void KisPaintopBox::slotCanvasResourceChangeAttempted(int key, const QVariant &value)
{
    Q_UNUSED(value);

    if (key == KoCanvasResource::ForegroundColor) {
        slotUnsetEraseMode();
    }
}

void KisPaintopBox::slotCanvasResourceChanged(int key, const QVariant &value)
{
    if (m_viewManager) {
        sender()->blockSignals(true);
        KisPaintOpPresetSP preset = m_viewManager->canvasResourceProvider()->resourceManager()->resource(KoCanvasResource::CurrentPaintOpPreset).value<KisPaintOpPresetSP>();
        if (preset && m_resourceProvider->currentPreset()->name() != preset->name()) {
            QString compositeOp = preset->settings()->getString("CompositeOp");
            updateCompositeOp(compositeOp);
            resourceSelected(preset);
        }

        if (key == KoCanvasResource::CurrentPaintOpPreset) {
            /**
             * Update currently selected preset in both the popup widgets
             */
            m_presetsChooserPopup->canvasResourceChanged(preset);
            m_presetsPopup->currentPresetChanged(preset);
        }

        if (key == KoCanvasResource::CurrentCompositeOp) {
            if (m_resourceProvider->currentCompositeOp() != m_currCompositeOpID) {
                updateCompositeOp(m_resourceProvider->currentCompositeOp());
            }
        }

        if (key == KoCanvasResource::Size) {
            setSliderValue("size", m_resourceProvider->size());
        }

        if (key == KoCanvasResource::PatternSize) {
            setMultiplierSliderValue("patternsize", m_resourceProvider->patternSize());
        }

        if (key == KoCanvasResource::Opacity) {
            setSliderValue("opacity", m_resourceProvider->opacity());
        }

        if (key == KoCanvasResource::Flow) {
            setSliderValue("flow", m_resourceProvider->flow());
        }

        if (key == KoCanvasResource::EraserMode) {
            m_eraseAction->setChecked(value.toBool());
        }

        if (key == KoCanvasResource::DisablePressure) {
            m_disablePressureAction->setChecked(value.toBool());
        }

        if (key == KoCanvasResource::MirrorHorizontal) {
            m_hMirrorAction->setChecked(value.toBool());
        }

        if (key == KoCanvasResource::MirrorVertical) {
            m_vMirrorAction->setChecked(value.toBool());
        }

        sender()->blockSignals(false);
    }
}

void KisPaintopBox::slotSetupDefaultPreset()
{
    KisPaintOpPresetSP preset = defaultPreset(m_resourceProvider->currentPreset()->paintOp());
    preset->setOptionsWidget(m_optionWidget);
    m_resourceProvider->setPaintOpPreset(preset);

    // tell the brush editor that the resource has changed
    // so it can update everything
    m_presetsPopup->resourceSelected(preset);
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
    if (oldEraserMode != checked && m_eraserBrushOpacityEnabled) {
        const qreal currentOpacity = m_resourceProvider->opacity();

        KisPaintOpSettingsSP settings = m_resourceProvider->currentPreset()->settings();

        // remember brush opacity. set the eraser opacity to the normal brush opacity if not set
        if (checked) {
            settings->setSavedBrushOpacity(currentOpacity);
            if (qFuzzyIsNull(settings->savedEraserOpacity())) {
                settings->setSavedEraserOpacity(currentOpacity);
            }
        } else {
            settings->setSavedEraserOpacity(currentOpacity);
            if (qFuzzyIsNull(settings->savedBrushOpacity())) {
                settings->setSavedBrushOpacity(currentOpacity);
            }
        }

        //update value in UI (this is the main place the value is 'stored' in memory)
        qreal newOpacity = checked ? settings->savedEraserOpacity() : settings->savedBrushOpacity();
        m_resourceProvider->setOpacity(newOpacity);
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

    // flow and opacity are shown as 0-100% on the UI, but their data is actually 0-1. Convert those two values
    // back for further work
    qreal opacity = m_sliderChooser[n]->getWidget<KisDoubleSliderSpinBox>("opacity")->value()/100;
    qreal flow    = m_sliderChooser[n]->getWidget<KisDoubleSliderSpinBox>("flow")->value()/100;
    qreal size    = m_sliderChooser[n]->getWidget<KisDoubleSliderSpinBox>("size")->value();
    qreal patternsize = m_sliderChooser[n]->getWidget<KisMultipliersDoubleSliderSpinBox>("patternsize")->value();


    setSliderValue("opacity", opacity);
    setSliderValue("flow"   , flow);
    setSliderValue("size"   , size);
    setMultiplierSliderValue("patternsize", patternsize);

    if (m_presetsEnabled) {
        // IMPORTANT: set the PaintOp size before setting the other properties
        //            it won't work the other way
        // TODO: why?!

        m_resourceProvider->setSize(size);
        m_resourceProvider->setPatternSize(patternsize);
        m_resourceProvider->setOpacity(opacity);
        m_resourceProvider->setFlow(flow);


        KisLockedPropertiesProxySP propertiesProxy = KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(m_resourceProvider->currentPreset()->settings());
        propertiesProxy->setProperty("OpacityValue", opacity);
        propertiesProxy->setProperty("FlowValue", flow);
        propertiesProxy->setProperty("Texture/Pattern/Scale", patternsize);
        m_optionWidget->setConfigurationSafe(m_resourceProvider->currentPreset()->settings().data());
    } else {
        m_resourceProvider->setOpacity(opacity);
    }

    m_presetsPopup->resourceSelected(m_resourceProvider->currentPreset());
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

void KisPaintopBox::slotSlider4Changed()
{
    sliderChanged(3);
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
            setWidgetState(ENABLE_COMPOSITEOP | ENABLE_OPACITY | ENABLE_PATTERNSIZE);
        } else                                              {
            setWidgetState(DISABLE_COMPOSITEOP | DISABLE_OPACITY | DISABLE_PATTERNSIZE);
        }

        if (flags & KisTool::FLAG_USES_CUSTOM_PRESET) {
            setWidgetState(ENABLE_PRESETS);
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
                setMultiplierSliderValue("patternsize", m_resourceProvider->currentPreset()->settings()->paintOpPatternSize());
                setWidgetState(ENABLE_PATTERNSIZE);
            }

            {
                updateCompositeOp(m_resourceProvider->currentPreset()->settings()->paintOpCompositeOp());
                setWidgetState(ENABLE_COMPOSITEOP);
            }

            m_blockUpdate = false;

            m_presetsEnabled = true;
        } else {
            setWidgetState(DISABLE_PRESETS);
            m_presetsEnabled = false;
        }

        if (flags & KisTool::FLAG_USES_CUSTOM_SIZE) {
            setWidgetState(ENABLE_SIZE | ENABLE_FLOW | ENABLE_PATTERNSIZE);
        } else {
            setWidgetState(DISABLE_SIZE | DISABLE_FLOW| DISABLE_PATTERNSIZE);
        }

    } else setWidgetState(DISABLE_ALL);
}

void KisPaintopBox::slotPreviousFavoritePreset()
{
    if (!m_favoriteResourceManager) return;

    QVector<QString> presets = m_favoriteResourceManager->favoritePresetNamesList();
    for (int i=0; i < presets.size(); ++i) {
        if (m_resourceProvider->currentPreset() &&
                m_resourceProvider->currentPreset()->name() == presets[i]) {
            if (i > 0) {
                m_favoriteResourceManager->slotChangeActivePaintop(i - 1);
            } else {
                m_favoriteResourceManager->slotChangeActivePaintop(m_favoriteResourceManager->numFavoritePresets() - 1);
            }
            //floating message should have least 2 lines, otherwise
            //preset thumbnail will be too small to distinguish
            //(because size of image on floating message depends on amount of lines in msg)
            m_viewManager->showFloatingMessage(
                        i18n("%1\nselected",
                             m_resourceProvider->currentPreset()->name()),
                        QIcon(QPixmap::fromImage(m_resourceProvider->currentPreset()->image())));

            return;
        }
    }
}

void KisPaintopBox::slotNextFavoritePreset()
{
    if (!m_favoriteResourceManager) return;

    QVector<QString> presets = m_favoriteResourceManager->favoritePresetNamesList();
    for(int i = 0; i < presets.size(); ++i) {
        if (m_resourceProvider->currentPreset()->name() == presets[i]) {
            if (i < m_favoriteResourceManager->numFavoritePresets() - 1) {
                m_favoriteResourceManager->slotChangeActivePaintop(i + 1);
            } else {
                m_favoriteResourceManager->slotChangeActivePaintop(0);
            }
            m_viewManager->showFloatingMessage(
                        i18n("%1\nselected",
                             m_resourceProvider->currentPreset()->name()),
                        QIcon(QPixmap::fromImage(m_resourceProvider->currentPreset()->image())));

            return;
        }
    }
}

void KisPaintopBox::slotSwitchToPreviousPreset()
{
    if (m_resourceProvider->previousPreset()) {
        setCurrentPaintop(m_resourceProvider->previousPreset());
        m_viewManager->showFloatingMessage(
                    i18n("%1\nselected",
                         m_resourceProvider->currentPreset()->name()),
                    QIcon(QPixmap::fromImage(m_resourceProvider->currentPreset()->image())));
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
        m_disablePressureAction->setIcon(KisIconUtils::loadIcon("transform_icons_penPressure"));
    } else {
        m_disablePressureAction->setIcon(KisIconUtils::loadIcon("transform_icons_penPressure_locked"));
    }

    m_resourceProvider->setDisablePressure(checked);
}

void KisPaintopBox::slotReloadPreset()
{
    KisSignalsBlocker blocker(m_optionWidget);

    KisPaintOpPresetResourceServer *rserver = KisResourceServerProvider::instance()->paintOpPresetServer();
    QSharedPointer<KisPaintOpPreset> preset = m_resourceProvider->currentPreset();

    if (preset) {
        const bool result = rserver->reloadResource(preset);
        KIS_SAFE_ASSERT_RECOVER_NOOP(result && "couldn't reload preset");
    }
}
void KisPaintopBox::slotGuiChangedCurrentPreset() // Called only when UI is changed and not when preset is changed
{
    KisPaintOpPresetSP preset = m_resourceProvider->currentPreset();

    {
        /**
         * Here we postpone all the settings updates events until the entire writing
         * operation will be finished. As soon as it is finished, the updates will be
         * emitted happily (if there were any).
         */

        KisPaintOpPreset::UpdatedPostponer postponer(preset);


        QStringList preserveProperties;
        preserveProperties << "lodUserAllowed";
        preserveProperties << "lodSizeThreshold";

        // clear all the properties before dumping the stuff into the preset,
        // some of the options add the values incrementally
        // (e.g. KisPaintOpUtils::RequiredBrushFilesListTag), therefore they
        // may add up if we pass the same preset multiple times
        preset->settings()->resetSettings(preserveProperties);

        m_optionWidget->writeConfigurationSafe(const_cast<KisPaintOpSettings*>(preset->settings().data()));
    }

    // we should also update the preset strip to update the status of the "dirty" mark
    m_presetsPopup->resourceSelected(m_resourceProvider->currentPreset());

    // TODO!!!!!!!!
    //m_presetsPopup->updateViewSettings();
}
void KisPaintopBox::slotSaveLockedOptionToPreset(KisPropertiesConfigurationSP p)
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

void KisPaintopBox::slotDropLockedOption(KisPropertiesConfigurationSP p)
{
    KisSignalsBlocker blocker(m_optionWidget);
    KisPaintOpPresetSP preset = m_resourceProvider->currentPreset();

    {
        KisResourceDirtyStateSaver dirtySaver(preset);

        QMapIterator<QString, QVariant> i(p->getProperties());
        while (i.hasNext()) {
            i.next();
            if (preset->settings()->hasProperty(i.key() + "_previous")) {
                preset->settings()->setProperty(i.key(), preset->settings()->getProperty(i.key() + "_previous"));
                preset->settings()->removeProperty(i.key() + "_previous");
            }

        }
    }

}
void KisPaintopBox::slotDirtyPresetToggled(bool value)
{
    if (!value) {
        slotReloadPreset();
        m_presetsPopup->resourceSelected(m_resourceProvider->currentPreset());
        m_presetsPopup->updateViewSettings();
    }
    m_dirtyPresetsEnabled = value;
    KisConfig cfg(false);
    cfg.setUseDirtyPresets(m_dirtyPresetsEnabled);

}
void KisPaintopBox::slotEraserBrushSizeToggled(bool value)
{
    m_eraserBrushSizeEnabled = value;
    KisConfig cfg(false);
    cfg.setUseEraserBrushSize(m_eraserBrushSizeEnabled);
}

void KisPaintopBox::slotEraserBrushOpacityToggled(bool value)
{
    m_eraserBrushOpacityEnabled = value;
    KisConfig cfg(false);
    cfg.setUseEraserBrushOpacity(m_eraserBrushOpacityEnabled);
}

void KisPaintopBox::slotUpdateSelectionIcon()
{
    m_hMirrorAction->setIcon(KisIconUtils::loadIcon("symmetry-horizontal"));
    m_vMirrorAction->setIcon(KisIconUtils::loadIcon("symmetry-vertical"));

    KisConfig cfg(true);
    if (!cfg.toolOptionsInDocker() && m_toolOptionsPopupButton) {
        m_toolOptionsPopupButton->setIcon(KisIconUtils::loadIcon("configure"));
    }

    m_presetSelectorPopupButton->setIcon(KisIconUtils::loadIcon("paintop_settings_01"));
    m_brushEditorPopupButton->setIcon(KisIconUtils::loadIcon("paintop_settings_02"));
    m_workspaceWidget->setIcon(KisIconUtils::loadIcon("workspace-chooser"));

    m_eraseAction->setIcon(KisIconUtils::loadIcon("draw-eraser"));
    m_reloadAction->setIcon(KisIconUtils::loadIcon("reload-preset"));

    if (m_disablePressureAction->isChecked()) {
        m_disablePressureAction->setIcon(KisIconUtils::loadIcon("transform_icons_penPressure"));
    } else {
        m_disablePressureAction->setIcon(KisIconUtils::loadIcon("transform_icons_penPressure_locked"));
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

void KisPaintopBox::findDefaultPresets()
{
    KisPaintOpPresetResourceServer *rserver = KisResourceServerProvider::instance()->paintOpPresetServer();
    m_eraserName = "eraser_circle";
    m_defaultPresetName = "basic_tip_default";

    KisResourceModel *resourceModel = rserver->resourceModel();

    for (int i = 0; i < resourceModel->rowCount(); i++) {

        QModelIndex idx = resourceModel->index(i, 0);

        QString resourceName = idx.data(Qt::UserRole + KisAbstractResourceModel::Name).toString().toLower();
        QString fileName = idx.data(Qt::UserRole + KisAbstractResourceModel::Filename).toString().toLower();

        if (resourceName.contains("eraser_circle")) {
            m_eraserName = resourceName;
        }
        else if (resourceName.contains("eraser") || fileName.contains("eraser")) {
            m_eraserName = resourceName;
        }

        if (resourceName.contains("basic_tip_default")) {
            m_defaultPresetName = resourceName;
        }
        else if (resourceName.contains("default") || fileName.contains("default")) {
            m_defaultPresetName = resourceName;
        }
    }
}

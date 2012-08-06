/*
 *  kis_paintop_box.cc - part of KImageShop/Krayon/Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
 *  Copyright (c) 2009-2011 Sven Langkamp (sven.langkamp@gmail.com)
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
 *  Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
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
#include <QAction>
#include <QPixmap>

#include <kactioncollection.h>
#include <kis_debug.h>
#include <kacceleratormanager.h>
#include <kseparator.h>

#include <KoIcon.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include <KoResourceSelector.h>
#include <KoResourceServerAdapter.h>
#include <KoToolManager.h>
#include <QTemporaryFile>

#include <kis_paint_device.h>
#include <kis_paintop_registry.h>
#include <kis_paintop_preset.h>
#include <kis_paintop_settings.h>
#include <kis_config_widget.h>
#include <kis_image.h>
#include <kis_node.h>
#include <kis_paintop_settings_widget.h>

#include "kis_canvas2.h"
#include "kis_node_manager.h"
#include "kis_view2.h"
#include "kis_factory2.h"
#include "kis_canvas_resource_provider.h"
#include "kis_resource_server_provider.h"
#include "ko_favorite_resource_manager.h"

#include "widgets/kis_popup_button.h"
#include "widgets/kis_paintop_presets_popup.h"
#include "widgets/kis_paintop_presets_chooser_popup.h"
#include "widgets/kis_workspace_chooser.h"
#include "widgets/kis_paintop_list_widget.h"
#include "widgets/kis_slider_spin_box.h"
#include "widgets/kis_cmb_composite.h"
#include "widgets/kis_widget_chooser.h"
#include "tool/kis_tool.h"

KisPaintopBox::KisPaintopBox(KisView2 * view, QWidget *parent, const char * name)
    : QWidget(parent)
    , m_resourceProvider(view->resourceProvider())
    , m_optionWidget(0)
    , m_settingsWidget(0)
    , m_presetWidget(0)
    , m_brushChooser(0)
    , m_view(view)
    , m_activePreset(0)
    , m_previousNode(0)
    , m_currTabletToolID(KoToolManager::instance()->currentInputDevice())
    , m_presetsEnabled(true)
    , m_blockUpdate(false)
{
    Q_ASSERT(view != 0);

    KGlobal::mainComponent().dirs()->addResourceType("kis_defaultpresets", "data", "krita/defaultpresets/");

    setObjectName(name);

    KAcceleratorManager::setNoAccel(this);

    setWindowTitle(i18n("Painter's Toolchest"));

    m_settingsWidget = new KisPopupButton(this);
    m_settingsWidget->setIcon(koIcon("paintop_settings_02"));
    m_settingsWidget->setToolTip(i18n("Edit brush settings"));
    m_settingsWidget->setFixedSize(32, 32);

    m_presetWidget = new KisPopupButton(this);
    m_presetWidget->setIcon(koIcon("paintop_settings_01"));
    m_presetWidget->setToolTip(i18n("Choose brush preset"));
    m_presetWidget->setFixedSize(32, 32);

    m_eraseModeButton = new QToolButton(this);
    m_eraseModeButton->setFixedSize(32, 32);
    m_eraseModeButton->setCheckable(true);
    KAction* eraseAction = new KAction(i18n("Set eraser mode"), m_eraseModeButton);
    eraseAction->setIcon(koIcon("draw-eraser"));
    eraseAction->setShortcut(Qt::Key_E);
    eraseAction->setCheckable(true);
    m_eraseModeButton->setDefaultAction(eraseAction);
    m_view->actionCollection()->addAction("erase_action", eraseAction);

    QToolButton* hMirrorButton = new QToolButton(this);
    hMirrorButton->setFixedSize(32, 32);
    hMirrorButton->setCheckable(true);
    KAction* hMirrorAction = new KAction(i18n("Set horizontal mirror mode"), hMirrorButton);
    hMirrorAction->setIcon(koIcon("object-flip-horizontal"));
    hMirrorAction->setCheckable(true);
    hMirrorButton->setDefaultAction(hMirrorAction);
    m_view->actionCollection()->addAction("hmirror_action", hMirrorAction);

    QToolButton* vMirrorButton = new QToolButton(this);
    vMirrorButton->setFixedSize(32, 32);
    vMirrorButton->setCheckable(true);
    KAction* vMirrorAction = new KAction(i18n("Set vertical mirror mode"), vMirrorButton);
    vMirrorAction->setIcon(koIcon("object-flip-vertical"));
    vMirrorAction->setCheckable(true);
    vMirrorButton->setDefaultAction(vMirrorAction);
    m_view->actionCollection()->addAction("vmirror_action", vMirrorAction);

    for(int i=0; i<2; ++i) {
        m_sliderChooser[i] = new KisWidgetChooser();
        KisDoubleSliderSpinBox* slOpacity = m_sliderChooser[i]->addWidget<KisDoubleSliderSpinBox>("opacity", i18n("Opacity:"));
        KisDoubleSliderSpinBox* slFlow    = m_sliderChooser[i]->addWidget<KisDoubleSliderSpinBox>("flow"   , i18n("Flow:"));
        KisDoubleSliderSpinBox* slSize    = m_sliderChooser[i]->addWidget<KisDoubleSliderSpinBox>("size"   , i18n("Size:"));
        
        slOpacity->setRange(0.0, 1.0, 2);
        slOpacity->setValue(1.0);
        slOpacity->setSingleStep(0.05);
        slOpacity->setMinimumWidth(120);
        
        slFlow->setRange(0.0, 1.0, 2);
        slFlow->setValue(1.0);
        slFlow->setSingleStep(0.05);
        slFlow->setMinimumWidth(120);
        
        slSize->setRange(0.0, 1000.0, 2);
        slSize->setValue(100.0);
        slSize->setSingleStep(0.05);
        slSize->setExponentRatio(3.0);
        slSize->setMinimumWidth(120);
    }
    
    m_sliderChooser[0]->chooseWidget("opacity");
    m_sliderChooser[1]->chooseWidget("flow");

    QLabel* labelMode = new QLabel(i18n("Mode: "), this);
    labelMode->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

    m_cmbCompositeOp = new KisCompositeOpComboBox();
    m_cmbCompositeOp->setFixedHeight(30);

    m_paletteButton = new QPushButton(i18n("Save to Palette"));
    m_paletteButton->setFixedHeight(30);

    m_workspaceWidget = new KisPopupButton(view);
    m_workspaceWidget->setIcon(koIcon("document-multiple"));
    m_workspaceWidget->setToolTip(i18n("Choose workspace"));
    m_workspaceWidget->setFixedSize(32, 32);
    m_workspaceWidget->setPopupWidget(new KisWorkspaceChooser(view));

    QHBoxLayout* baseLayout = new QHBoxLayout(this);
    m_paintopWidget = new QWidget(this);
    baseLayout->addWidget(m_paintopWidget);
    baseLayout->setContentsMargins(0, 0, 0, 0);

    KAction* action;
    m_layout = new QHBoxLayout(m_paintopWidget);
    m_layout->addWidget(m_settingsWidget);
    m_layout->addWidget(m_presetWidget);
    m_layout->setContentsMargins(0, 0, 0, 0);

    QWidget* compositeActions = new QWidget(this);
    QHBoxLayout* compositeLayout = new QHBoxLayout(compositeActions);
    compositeLayout->addWidget(labelMode);
    compositeLayout->addWidget(m_cmbCompositeOp);
    compositeLayout->addWidget(m_eraseModeButton);
    compositeLayout->setContentsMargins(0, 0, 0, 0);
    action = new KAction(i18n("Brush composite"), this);
    view->actionCollection()->addAction("composite_actions", action);
    action->setDefaultWidget(compositeActions);

    action = new KAction(i18n("Brush option slider 1"), this);
    view->actionCollection()->addAction("brushslider1", action);
    action->setDefaultWidget(m_sliderChooser[0]);
    connect(action, SIGNAL(triggered()), m_sliderChooser[0], SLOT(showPopupWidget()));

    action = new KAction(i18n("Brush option slider 2"), this);
    view->actionCollection()->addAction("brushslider2", action);
    action->setDefaultWidget(m_sliderChooser[1]);
    connect(action, SIGNAL(triggered()), m_sliderChooser[1], SLOT(showPopupWidget()));

    QWidget* mirrorActions = new QWidget(this);
    QHBoxLayout* mirrorLayout = new QHBoxLayout(mirrorActions);
    mirrorLayout->addWidget(hMirrorButton);
    mirrorLayout->addWidget(vMirrorButton);
    mirrorLayout->setContentsMargins(0, 0, 0, 0);
    action = new KAction(i18n("Mirror"), this);
    view->actionCollection()->addAction("mirror_actions", action);
    action->setDefaultWidget(mirrorActions);

    action = new KAction(i18n("Add to palette"), this);
    view->actionCollection()->addAction("palette_manager", action);
    action->setDefaultWidget(m_paletteButton);

    action = new KAction(i18n("Workspaces"), this);
    view->actionCollection()->addAction("workspaces", action);
    action->setDefaultWidget(m_workspaceWidget);

    m_presetsPopup = new KisPaintOpPresetsPopup(m_resourceProvider);
    m_settingsWidget->setPopupWidget(m_presetsPopup);
    m_presetsPopup->switchDetached();

    m_presetsChooserPopup = new KisPaintOpPresetsChooserPopup();
    m_presetsChooserPopup->setFixedSize(500, 500);
    m_presetWidget->setPopupWidget(m_presetsChooserPopup);

    m_prevCompositeOpID = KoCompositeOpRegistry::instance().getDefaultCompositeOp().id();
    m_currCompositeOpID = KoCompositeOpRegistry::instance().getDefaultCompositeOp().id();

    slotNodeChanged(view->activeNode());
    updatePaintops(view->image()->colorSpace());
    setCurrentPaintop(defaultPaintOp());

    connect(m_presetsPopup       , SIGNAL(paintopActivated(QString))          , SLOT(slotSetPaintop(QString)));
    connect(m_presetsPopup       , SIGNAL(savePresetClicked())                , SLOT(slotSaveActivePreset()));
    connect(m_presetsPopup       , SIGNAL(defaultPresetClicked())             , SLOT(slotSetupDefaultPreset()));
    connect(m_presetsPopup       , SIGNAL(presetNameLineEditChanged(QString)) , SLOT(slotWatchPresetNameLineEdit(QString)));
    connect(m_presetsPopup       , SIGNAL(signalResourceSelected(KoResource*)), SLOT(resourceSelected(KoResource*)));
    connect(m_presetsChooserPopup, SIGNAL(resourceSelected(KoResource*))      , SLOT(resourceSelected(KoResource*)));
    connect(m_resourceProvider   , SIGNAL(sigNodeChanged(const KisNodeSP))    , SLOT(slotNodeChanged(const KisNodeSP)));
    connect(m_paletteButton      , SIGNAL(clicked())                          , SLOT(slotSaveToFavouriteBrushes()));
    connect(m_cmbCompositeOp     , SIGNAL(activated(int))                     , SLOT(slotSetCompositeMode(int)));
    connect(eraseAction          , SIGNAL(triggered(bool))                    , SLOT(slotToggleEraseMode(bool)));
    connect(hMirrorAction        , SIGNAL(triggered(bool))                    , SLOT(slotHorizontalMirrorChanged(bool)));
    connect(vMirrorAction        , SIGNAL(triggered(bool))                    , SLOT(slotVerticalMirrorChanged(bool)));
    
    connect(m_sliderChooser[0]->getWidget<KisDoubleSliderSpinBox>("opacity"), SIGNAL(valueChanged(qreal)), SLOT(slotSlider1Changed()));
    connect(m_sliderChooser[0]->getWidget<KisDoubleSliderSpinBox>("flow")   , SIGNAL(valueChanged(qreal)), SLOT(slotSlider1Changed()));
    connect(m_sliderChooser[0]->getWidget<KisDoubleSliderSpinBox>("size")   , SIGNAL(valueChanged(qreal)), SLOT(slotSlider1Changed()));
    connect(m_sliderChooser[1]->getWidget<KisDoubleSliderSpinBox>("opacity"), SIGNAL(valueChanged(qreal)), SLOT(slotSlider2Changed()));
    connect(m_sliderChooser[1]->getWidget<KisDoubleSliderSpinBox>("flow")   , SIGNAL(valueChanged(qreal)), SLOT(slotSlider2Changed()));
    connect(m_sliderChooser[1]->getWidget<KisDoubleSliderSpinBox>("size")   , SIGNAL(valueChanged(qreal)), SLOT(slotSlider2Changed()));

    //Needed to connect canvas to favorite resource manager
    m_view->canvasBase()->createFavoriteResourceManager(this);
    connect(m_view->resourceProvider(), SIGNAL(sigOpacityChanged(qreal)), SLOT(slotOpacityChanged(qreal)));
}

KisPaintopBox::~KisPaintopBox()
{
    // Do not delete the widget, since it it is global to the application, not owned by the view
    m_presetsPopup->setPaintOpSettingsWidget(0);
    qDeleteAll(m_paintopOptionWidgets);
}

KisPaintOpPresetSP KisPaintopBox::paintOpPresetSP(KoID* paintop)
{
    if(paintop == 0)
        return m_activePreset->clone();

    return activePreset(*paintop);
}

void KisPaintopBox::updatePaintops(const KoColorSpace* colorSpace)
{
    /* get the list of the factories*/
    QList<QString> keys = KisPaintOpRegistry::instance()->keys();
    QList<KisPaintOpFactory*> factoryList;

    foreach(const QString & paintopId, keys) {
        KisPaintOpFactory * factory = KisPaintOpRegistry::instance()->get(paintopId);
        if (KisPaintOpRegistry::instance()->userVisible(KoID(factory->id(), factory->name()), colorSpace)){
            factoryList.append(factory);
        }
    }

    m_presetsPopup->setPaintOpList(factoryList);
}

void KisPaintopBox::resourceSelected(KoResource* resource)
{
    KisPaintOpPreset* preset = dynamic_cast<KisPaintOpPreset*>(resource);
    if (preset) {
        if(!preset->settings()->isLoadable())
            return;

        setCurrentPaintop(preset->paintOp(), preset->clone());
        m_presetsPopup->setPresetImage(preset->image());
        m_presetsPopup->resourceSelected(resource);
    }
}

QPixmap KisPaintopBox::paintopPixmap(const KoID& paintop)
{
    QString pixmapName = KisPaintOpRegistry::instance()->pixmap(paintop);

    if(pixmapName.isEmpty())
        return QPixmap();

    return QPixmap(KisFactory2::componentData().dirs()->findResource("kis_images", pixmapName));
}

KoID KisPaintopBox::currentPaintop()
{
    return m_activePreset->paintOp();
}

void KisPaintopBox::setCurrentPaintop(const KoID& paintop, KisPaintOpPresetSP preset)
{
    if(m_activePreset) {
        if(m_optionWidget) {
            m_optionWidget->writeConfiguration(const_cast<KisPaintOpSettings*>(m_activePreset->settings().data()));
            m_optionWidget->disconnect(this);
            m_optionWidget->hide();
        }

        m_paintOpPresetMap[m_activePreset->paintOp()] = m_activePreset->clone();
        m_tabletToolMap[m_currTabletToolID].preset    = m_activePreset->clone();
        m_tabletToolMap[m_currTabletToolID].paintOpID = m_activePreset->paintOp();
    }

    preset = (!preset) ? activePreset(paintop) : preset;

    Q_ASSERT(preset && preset->settings());

    if(!m_paintopOptionWidgets.contains(paintop))
        m_paintopOptionWidgets[paintop] = KisPaintOpRegistry::instance()->get(paintop.id())->createSettingsWidget(this);

    m_optionWidget = m_paintopOptionWidgets[paintop];
    m_optionWidget->setImage(m_view->image());
    m_optionWidget->setConfiguration(preset->settings());

    preset->settings()->setOptionsWidget(m_optionWidget);
    preset->settings()->setNode(m_resourceProvider->currentNode());

    m_presetsPopup->setPaintOpSettingsWidget(m_optionWidget);
    m_presetsChooserPopup->setPresetFilter(paintop);

    Q_ASSERT(m_optionWidget && m_presetWidget);
    connect(m_optionWidget, SIGNAL(sigConfigurationUpdated()), this, SLOT(slotUpdatePreset()));

    KisPaintOpFactory* paintOp     = KisPaintOpRegistry::instance()->get(paintop.id());
    QString            pixFilename = KisFactory2::componentData().dirs()->findResource("kis_images", paintOp->pixmap());

    m_settingsWidget->setIcon(QIcon(pixFilename));
    m_resourceProvider->setPaintOpPreset(preset);
    m_presetsPopup->setCurrentPaintOp(paintop.id());

    if (m_presetsPopup->currentPaintOp() != paintop.id()){
        // Must change the paintop as the current one is not supported
        // by the new colorspace.
        kWarning() << "current paintop " << paintop.name() << " was not set, not supported by colorspace";
    }

    m_activePreset = preset;
    emit signalPaintopChanged(preset);
}

KoID KisPaintopBox::defaultPaintOp()
{
    return KoID("paintbrush");
}

KisPaintOpPresetSP KisPaintopBox::defaultPreset(const KoID& paintOp)
{
    QString defaultName = paintOp.id() + ".kpp";
    QString path = KGlobal::mainComponent().dirs()->findResource("kis_defaultpresets", defaultName);

    KisPaintOpPresetSP preset = new KisPaintOpPreset(path);

    if (!preset->load()) {
        preset = KisPaintOpRegistry::instance()->defaultPreset(paintOp, m_view->image());
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
    KisNodeSP node = m_resourceProvider->currentNode();
    
    if(node && node->paintDevice()) {
        if(!node->paintDevice()->colorSpace()->hasCompositeOp(compositeOpID))
            compositeOpID = KoCompositeOpRegistry::instance().getDefaultCompositeOp().id();
        
        int index = m_cmbCompositeOp->indexOf(KoID(compositeOpID));
        
        m_cmbCompositeOp->blockSignals(true);
        m_cmbCompositeOp->setCurrentIndex(index);
        m_cmbCompositeOp->blockSignals(false);
        
        m_eraseModeButton->blockSignals(true);
        m_eraseModeButton->setChecked(compositeOpID == COMPOSITE_ERASE);
        m_eraseModeButton->blockSignals(false);
        
        if(compositeOpID != m_currCompositeOpID) {
            m_activePreset->settings()->setProperty("CompositeOp", compositeOpID);
            m_optionWidget->setConfiguration(m_activePreset->settings().data());
            m_resourceProvider->setCurrentCompositeOp(compositeOpID);
            m_prevCompositeOpID = m_currCompositeOpID;
            m_currCompositeOpID = compositeOpID;
        }
    }
}

void KisPaintopBox::setWidgetState(int flags)
{
    if(flags & (ENABLE_COMPOSITEOP|DISABLE_COMPOSITEOP)) {
        m_cmbCompositeOp->setEnabled(flags & ENABLE_COMPOSITEOP);
        m_eraseModeButton->setEnabled(flags & ENABLE_COMPOSITEOP);
    }
    
    if(flags & (ENABLE_PRESETS|DISABLE_PRESETS)) {
        m_presetWidget->setEnabled(flags & ENABLE_PRESETS);
        m_settingsWidget->setEnabled(flags & ENABLE_PRESETS);
    }
    
    for(int i=0; i<2; ++i) {
        if(flags & (ENABLE_OPACITY|DISABLE_OPACITY))
            m_sliderChooser[i]->getWidget("opacity")->setEnabled(flags & ENABLE_OPACITY);
        
        if(flags & (ENABLE_FLOW|DISABLE_FLOW))
            m_sliderChooser[i]->getWidget("flow")->setEnabled(flags & ENABLE_FLOW);
        
        if(flags & (ENABLE_SIZE|DISABLE_SIZE))
            m_sliderChooser[i]->getWidget("size")->setEnabled(flags & ENABLE_SIZE);
    }
}

void KisPaintopBox::setSliderValue(const QString& sliderID, qreal value)
{
    for(int i=0; i<2; ++i) {
        KisDoubleSliderSpinBox* slider = m_sliderChooser[i]->getWidget<KisDoubleSliderSpinBox>(sliderID);
        slider->blockSignals(true);
        slider->setValue(value);
        slider->blockSignals(false);
    }
}

void KisPaintopBox::slotSetPaintop(const QString& paintOpId)
{
    if(KisPaintOpRegistry::instance()->get(paintOpId) != 0) {
        KoID id(paintOpId, KisPaintOpRegistry::instance()->get(paintOpId)->name());
        setCurrentPaintop(id);
    }
}

void KisPaintopBox::slotInputDeviceChanged(const KoInputDevice& inputDevice)
{
    TabletToolMap::iterator toolData = m_tabletToolMap.find(inputDevice);
    
    if(toolData == m_tabletToolMap.end())
        setCurrentPaintop(currentPaintop());
    else
        setCurrentPaintop(toolData->paintOpID, toolData->preset);
    
    m_currTabletToolID = TabletToolID(inputDevice);
}

void KisPaintopBox::slotCurrentNodeChanged(KisNodeSP node)
{
    for(TabletToolMap::iterator itr=m_tabletToolMap.begin(); itr!=m_tabletToolMap.end(); ++itr) {
        if(itr->preset && itr->preset->settings())
            itr->preset->settings()->setNode(node);
    }
}

void KisPaintopBox::slotSaveActivePreset()
{
    KisPaintOpPresetSP curPreset = m_resourceProvider->currentPreset();

    if (!curPreset)
        return;

    m_view->canvasBase()->favoriteResourceManager()->setBlockUpdates(true);

    KisPaintOpPreset* newPreset = curPreset->clone();
    KoResourceServer<KisPaintOpPreset>* rServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    QString saveLocation = rServer->saveLocation();
    QString name = m_presetsPopup->getPresetName();

    QStringList tags;
    KisPaintOpPreset* resource = rServer->getResourceByName(name);
    if (resource) {
        tags = rServer->getAssignedTagsList(resource);
        rServer->removeResource(resource);
    }

    newPreset->setImage(m_presetsPopup->cutOutOverlay());
    newPreset->setFilename(saveLocation + name + newPreset->defaultFileExtension());
    newPreset->setName(name);

    m_presetsPopup->changeSavePresetButtonText(true);

    rServer->addResource(newPreset);
    foreach(const QString& tag, tags) {
        rServer->addTag(newPreset, tag);
    }
    m_view->canvasBase()->favoriteResourceManager()->setBlockUpdates(false);
}

void KisPaintopBox::slotUpdatePreset()
{
    // block updates of avoid some over updating of the option widget
    m_blockUpdate = true;
    m_optionWidget->writeConfiguration(const_cast<KisPaintOpSettings*>(m_activePreset->settings().data()));
    
    setSliderValue("size", m_activePreset->settings()->paintOpSize().width());

    if(m_activePreset->settings()->hasProperty("OpacityValue")) {
        qreal opacity = m_activePreset->settings()->getDouble("OpacityValue");
        m_resourceProvider->setOpacity(opacity);
        setSliderValue("opacity", opacity);
        setWidgetState(ENABLE_OPACITY);
    }
    else {
        m_resourceProvider->setOpacity(1.0);
        setSliderValue("opacity", 1.0);
        setWidgetState(DISABLE_OPACITY);
    }
    
    if(m_activePreset->settings()->hasProperty("FlowValue")) {
        setSliderValue("flow", m_activePreset->settings()->getDouble("FlowValue"));
        setWidgetState(ENABLE_FLOW);
    }
    else {
        setSliderValue("flow", 1.0);
        setWidgetState(DISABLE_FLOW);
    }

    if(m_activePreset->settings()->hasProperty("CompositeOp")) {
        updateCompositeOp(m_activePreset->settings()->getString("CompositeOp"));
        setWidgetState(ENABLE_COMPOSITEOP);
    }
    else {
        updateCompositeOp(KoCompositeOpRegistry::instance().getDefaultCompositeOp().id());
        setWidgetState(DISABLE_COMPOSITEOP);
    }
    m_blockUpdate = false;
}

void KisPaintopBox::slotSetupDefaultPreset()
{
    KisPaintOpPresetSP preset = defaultPreset(m_activePreset->paintOp());
    preset->settings()->setNode(m_activePreset->settings()->node());
    preset->settings()->setOptionsWidget(m_optionWidget);
    m_optionWidget->setConfiguration(preset->settings());
    m_optionWidget->writeConfiguration(const_cast<KisPaintOpSettings*>(preset->settings().data()));
}

void KisPaintopBox::slotNodeChanged(const KisNodeSP node)
{
    // Deconnect colorspace change of previous node
    if (m_previousNode && m_previousNode->paintDevice())
        disconnect(m_previousNode->paintDevice().data(), SIGNAL(colorSpaceChanged(const KoColorSpace*)), this, SLOT(slotColorSpaceChanged(const KoColorSpace*)));

    // Reconnect colorspace change of node
    if(node && node->paintDevice())
    {
        connect(node->paintDevice().data(), SIGNAL(colorSpaceChanged(const KoColorSpace*)), this, SLOT(slotColorSpaceChanged(const KoColorSpace*)));
        m_resourceProvider->setCurrentCompositeOp(m_currCompositeOpID);
        m_previousNode = node;
        slotColorSpaceChanged(node->colorSpace());
    }

    for(TabletToolMap::iterator itr=m_tabletToolMap.begin(); itr!=m_tabletToolMap.end(); ++itr) {
        if(itr->preset && itr->preset->settings())
            itr->preset->settings()->setNode(node);
    }
}

void KisPaintopBox::slotColorSpaceChanged(const KoColorSpace* colorSpace)
{
    m_cmbCompositeOp->getModel()->validateCompositeOps(colorSpace);
}

void KisPaintopBox::slotToggleEraseMode(bool checked)
{
    if(checked)
        updateCompositeOp(COMPOSITE_ERASE);
    else
        updateCompositeOp(m_prevCompositeOpID);
}

void KisPaintopBox::slotSetCompositeMode(int index)
{
    if(m_activePreset->settings()->hasProperty("CompositeOp")) {
        KoID compositeOp;

        if(m_cmbCompositeOp->entryAt(compositeOp, index))
            updateCompositeOp(compositeOp.id());
    }
}

void KisPaintopBox::slotSaveToFavouriteBrushes()
{
    if(!m_view->canvasBase()->favoriteResourceManager())
        m_view->canvasBase()->createFavoriteResourceManager(this);
    else
        m_view->canvasBase()->favoriteResourceManager()->showPaletteManager();
}

void KisPaintopBox::slotWatchPresetNameLineEdit(const QString& text)
{
    KoResourceServer<KisPaintOpPreset>* rServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    m_presetsPopup->changeSavePresetButtonText(rServer->getResourceByName(text) != 0);
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
    qreal opacity = m_sliderChooser[n]->getWidget<KisDoubleSliderSpinBox>("opacity")->value();
    qreal flow    = m_sliderChooser[n]->getWidget<KisDoubleSliderSpinBox>("flow")->value();
    qreal size    = m_sliderChooser[n]->getWidget<KisDoubleSliderSpinBox>("size")->value();
    
    setSliderValue("opacity", opacity);
    setSliderValue("flow"   , flow   );
    setSliderValue("size"   , size   );

    if(m_presetsEnabled) {
        // IMPORTANT: set the PaintOp size before setting the other properties
        //            it wont work the other way
        qreal sizeDiff = size - m_activePreset->settings()->paintOpSize().width();
        m_activePreset->settings()->changePaintOpSize(sizeDiff, 0);

        if(m_activePreset->settings()->hasProperty("OpacityValue"))
            m_activePreset->settings()->setProperty("OpacityValue", opacity);

        if(m_activePreset->settings()->hasProperty("FlowValue"))
            m_activePreset->settings()->setProperty("FlowValue", flow);

        m_optionWidget->setConfiguration(m_activePreset->settings().data());
    }
    else m_resourceProvider->setOpacity(opacity);
}

void KisPaintopBox::slotSlider1Changed()
{
    sliderChanged(0);
}

void KisPaintopBox::slotSlider2Changed()
{
    sliderChanged(1);
}

void KisPaintopBox::slotToolChanged(KoCanvasController* canvas, int toolId)
{
    Q_UNUSED(canvas);
    Q_UNUSED(toolId);
    
    QString  id   = KoToolManager::instance()->activeToolId();
    KisTool* tool = dynamic_cast<KisTool*>(KoToolManager::instance()->toolById(m_view->canvasBase(), id));
    
    if(tool) {
        int flags = tool->flags();
        
        if(flags & KisTool::FLAG_USES_CUSTOM_COMPOSITEOP) { setWidgetState(ENABLE_COMPOSITEOP|ENABLE_OPACITY);   }
        else                                              { setWidgetState(DISABLE_COMPOSITEOP|DISABLE_OPACITY); }
        
        if(flags & KisTool::FLAG_USES_CUSTOM_PRESET) {
            setWidgetState(ENABLE_PRESETS|ENABLE_SIZE|ENABLE_FLOW);
            slotUpdatePreset();
            m_presetsEnabled = true;
        }
        else {
            setWidgetState(DISABLE_PRESETS|DISABLE_SIZE|DISABLE_FLOW);
            m_presetsEnabled = false;
        }
    }
    else setWidgetState(DISABLE_ALL);
}

void KisPaintopBox::slotOpacityChanged(qreal opacity)
{
    if (m_blockUpdate) {
        return;
    }
    m_blockUpdate = true;

    for (int i = 0; i < 2; ++i) {
        KisDoubleSliderSpinBox *opacitySlider = m_sliderChooser[i]->getWidget<KisDoubleSliderSpinBox>("opacity");
        opacitySlider->blockSignals(true);
        opacitySlider->setValue(opacity);
        opacitySlider->blockSignals(false);
    }
    if(m_presetsEnabled) {
        if(m_activePreset->settings()->hasProperty("OpacityValue"))
            m_activePreset->settings()->setProperty("OpacityValue", opacity);
        m_optionWidget->setConfiguration(m_activePreset->settings().data());
    }
    m_blockUpdate = false;
}

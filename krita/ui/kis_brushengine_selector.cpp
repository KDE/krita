/*
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_brushengine_selector.h"

#include <QMenu>
#include <QWidget>
#include <QString>
#include <QPixmap>
#include <QLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>

#include <kactioncollection.h>
#include <kis_debug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kacceleratormanager.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include <KoToolManager.h>
#include <KoColorSpace.h>
#include <KoResourceSelector.h>
#include <KoResourceServerAdapter.h>

#include <kis_paint_device.h>
#include <kis_cmb_composite.h>
#include <kis_paintop_registry.h>
#include <kis_canvas_resource_provider.h>
#include <kis_painter.h>
#include <kis_paintop.h>
#include <kis_layer.h>
#include <kis_resource_server_provider.h>
#include <kis_paintop_preset.h>
#include <kis_paintop_settings.h>
#include <kis_config_widget.h>
#include <kis_image.h>
#include <kis_node.h>
#include <kis_paintop_settings_widget.h>

#include "kis_config.h"
#include "kis_node_manager.h"
#include "kis_layer_manager.h"
#include "kis_view2.h"
#include "kis_factory2.h"
#include "widgets/kis_popup_button.h"
#include "widgets/kis_paintop_presets_popup.h"
#include "widgets/kis_paintop_presets_chooser_popup.h"
#include "widgets/kis_slider_spin_box.h"
#include "widgets/kis_scratch_pad.h"
#include "kis_brushengine_selector.h"
#include "ko_favorite_resource_manager.h"
#include <kis_paintop_presets_chooser_popup.h>


KisBrushEngineSelector::KisBrushEngineSelector(KisView2 * view, QWidget *parent)
    : QWidget(parent)
    , m_view(view)
    , m_resourceProvider(view->resourceProvider())
{
    KGlobal::mainComponent().dirs()->addResourceType("kis_defaultpresets", "data", "krita/defaultpresets/");

    KisConfig cfg;
    m_detached = cfg.paintopPopupDetached();
    if (parentWidget()) {
        if (m_detached) {
            parentWidget()->setWindowFlags(Qt::Tool);
            parentWidget()->show();
        }
        else {
            parentWidget()->setWindowFlags(Qt::Popup);
        }
    }
    QHBoxLayout *layout = new QHBoxLayout(this);

    m_cmbPaintops = new QListWidget(this);
    m_cmbPaintops->setIconSize(QSize(32,32));
    m_cmbPaintops->setViewMode(QListView::IconMode);
    m_cmbPaintops->setMovement(QListView::Static);
    m_cmbPaintops->setMaximumWidth(150);
    m_cmbPaintops->setMovement(QListView::Static);
    m_cmbPaintops->setGridSize(QSize(150,50));

    m_cmbPaintops->setToolTip(i18n("Brush Engines"));
    m_cmbPaintops->setMovement(QListView::Static);

    layout->addWidget(m_cmbPaintops);

    QTabWidget* tabWidget = new QTabWidget(this);
    layout->addWidget(tabWidget, 100);

    m_presetsPopup = new KisPaintOpPresetsPopup(m_resourceProvider);
    tabWidget->addTab(m_presetsPopup, i18n("Brush Editor"));

    m_presetsChooserPopup = new KisPaintOpPresetsChooserPopup();
    tabWidget->addTab(m_presetsChooserPopup, i18n("Presets"));

//    KisScratchPad *scratchPad = m_presetsPopup->scratchPad();
//    scratchPad->layout()->removeWidget(scratchPad);
//    scratchPad->setParent(this);
//    layout->addWidget(scratchPad);

    QList<KoID> keys = KisPaintOpRegistry::instance()->listKeys();
    for (QList<KoID>::Iterator it = keys.begin(); it != keys.end(); ++it) {
        // add all paintops, and show/hide them afterwards
        m_paintops.append(*it);
    }

    m_colorspace = view->image()->colorSpace();
    updatePaintops();
    setCurrentPaintop(defaultPaintop(KoToolManager::instance()->currentInputDevice()));

    connect(m_cmbPaintops, SIGNAL(currentRowChanged(int)), this, SLOT(slotItemSelected(int)));

    connect(m_presetsPopup, SIGNAL(savePresetClicked()), this, SLOT(slotSaveActivePreset()));

    connect(m_presetsPopup, SIGNAL(defaultPresetClicked()), this, SLOT(slotSetupDefaultPreset()));

    connect(m_presetsChooserPopup, SIGNAL(resourceSelected(KoResource*)),
            this, SLOT(resourceSelected(KoResource*)));

    connect(m_resourceProvider, SIGNAL(sigNodeChanged(const KisNodeSP)),
            this, SLOT(nodeChanged(const KisNodeSP)));



}

KisBrushEngineSelector::~KisBrushEngineSelector()
{
    // Do not delete the widget, since it it is global to the application, not owned by the view
    m_presetsPopup->setPaintOpSettingsWidget(0);
    qDeleteAll(m_paintopOptionWidgets);
}

KisPaintOpPresetSP KisBrushEngineSelector::paintOpPresetSP(KoID* paintop)
{
    if (paintop == 0)
        return m_activePreset->clone();
    else if (!QString::compare(paintop->id(), "eraser", Qt::CaseInsensitive))
        return activePreset(*paintop, KoInputDevice::eraser());
    else
        return activePreset(*paintop, KoToolManager::instance()->currentInputDevice());
}

void KisBrushEngineSelector::slotItemSelected(int index)
{
    if (index < m_displayedOps.count()) {
        KoID paintop = m_displayedOps.at(index);
        setCurrentPaintop(paintop);
    }
}

void KisBrushEngineSelector::colorSpaceChanged(const KoColorSpace *cs)
{
    if (cs != m_colorspace) {
        m_colorspace = cs;
        updatePaintops();

        // ensure the the right paintop is selected
        int index = m_displayedOps.indexOf(currentPaintop());
        if (index == -1) {
            // Must change the paintop as the current one is not supported
            // by the new colorspace.
            index = 0;
        }
        m_cmbPaintops->setCurrentRow(index);
        slotItemSelected(index);
    }
}

void KisBrushEngineSelector::updatePaintops()
{
    m_displayedOps.clear();
    m_cmbPaintops->clear();

    QStringList paintopNames;
    QMap<QString, KoID> paintopMap;
    foreach(const KoID & paintopId, m_paintops) {
        paintopNames << paintopId.name();
        paintopMap[paintopId.name()] = paintopId;
    }
    qSort(paintopNames);

    foreach(QString paintopName, paintopNames) {
        KoID paintop = paintopMap[paintopName];
        if (KisPaintOpRegistry::instance()->userVisible(paintop, m_colorspace)) {
            QPixmap pm = paintopPixmap(paintop);

            if (pm.isNull()) {
                pm = QPixmap(16, 16);
                pm.fill();
            }
            pm = pm.scaled(QSize(32,32), Qt::KeepAspectRatio, Qt::SmoothTransformation);

            QListWidgetItem *item =  new QListWidgetItem(QIcon(pm), paintopName);
            item->setToolTip(paintopName);
            m_cmbPaintops->addItem(item);
            m_displayedOps.append(paintop);
        }
    }
}

void KisBrushEngineSelector::resourceSelected(KoResource* resource)
{
    KisPaintOpPreset* preset = static_cast<KisPaintOpPreset*>(resource);
    dbgUI << "preset " << preset->name() << "selected";

    if(preset->paintOp() != currentPaintop()) {
        setCurrentPaintop(preset->paintOp());
    }

    m_optionWidget->setConfiguration(preset->settings());
    slotUpdatePreset();
}

QPixmap KisBrushEngineSelector::paintopPixmap(const KoID & paintop)
{
    QString pixmapName = KisPaintOpRegistry::instance()->pixmap(paintop);

    if (pixmapName.isEmpty()) {
        return QPixmap();
    }

    QString fname = KisFactory2::componentData().dirs()->findResource("kis_images", pixmapName);

    return QPixmap(fname);
}

void KisBrushEngineSelector::slotInputDeviceChanged(const KoInputDevice & inputDevice)
{
    KoID paintop;
    InputDevicePaintopMap::iterator it = m_currentID.find(inputDevice);

    if (it == m_currentID.end()) {
        paintop = defaultPaintop(inputDevice);
    } else {
        paintop = (*it);
    }

    int index = m_displayedOps.indexOf(paintop);

    if (index == -1) {
        // Must change the paintop as the current one is not supported
        // by the new colorspace.
        index = 0;
        paintop = m_displayedOps.at(index);
    }

    m_cmbPaintops->setCurrentRow(index);
    setCurrentPaintop(paintop);
}

void KisBrushEngineSelector::slotCurrentNodeChanged(KisNodeSP node)
{
    for (InputDevicePresetsMap::iterator it = m_inputDevicePresets.begin();
         it != m_inputDevicePresets.end();
         ++it)
    {
        foreach(const KisPaintOpPresetSP & preset, it.value()) {
            if (preset && preset->settings()) {
                preset->settings()->setNode(node);
            }
        }
    }
}


const KoID& KisBrushEngineSelector::currentPaintop()
{
    KoID id = m_currentID[KoToolManager::instance()->currentInputDevice()];
    return m_currentID[KoToolManager::instance()->currentInputDevice()];
}


void KisBrushEngineSelector::setCurrentPaintop(const KoID & paintop)
{
    if (m_activePreset && m_optionWidget) {
        m_optionWidget->writeConfiguration(const_cast<KisPaintOpSettings*>(m_activePreset->settings().data()));
        m_optionWidget->hide();
    }

    m_currentID[KoToolManager::instance()->currentInputDevice()] = paintop;

    KisPaintOpPresetSP preset =
            activePreset(currentPaintop(), KoToolManager::instance()->currentInputDevice());

    if (preset != 0 && preset->settings()) {
        if (!m_paintopOptionWidgets.contains(paintop)) {
            m_paintopOptionWidgets[paintop] = KisPaintOpRegistry::instance()->get(paintop.id())->createSettingsWidget(this);
        }
        m_optionWidget = m_paintopOptionWidgets[paintop];
        m_optionWidget->setImage(m_view->image());
        m_optionWidget->writeConfiguration(const_cast<KisPaintOpSettings*>(preset->settings().data()));
        preset->settings()->setOptionsWidget(m_optionWidget);

        if (!preset->settings()->getProperties().isEmpty()) {
            m_optionWidget->setConfiguration(preset->settings());
        }
        m_presetsPopup->setPaintOpSettingsWidget(m_optionWidget);
        m_presetsChooserPopup->setPresetFilter(paintop);
        Q_ASSERT(m_optionWidget);
        connect(m_optionWidget, SIGNAL(sigConfigurationUpdated()), this, SLOT(slotUpdatePreset()));
        m_presetsPopup->setPreset(preset);
    } else {
        m_presetsPopup->setPaintOpSettingsWidget(0);
    }

    preset->settings()->setNode(m_resourceProvider->currentNode());
    m_resourceProvider->slotPaintOpPresetActivated(preset);

    int index = m_displayedOps.indexOf(paintop);

    if (index == -1) {
        // Must change the paintop as the current one is not supported
        // by the new colorspace.
        index = 0;
    }

    m_cmbPaintops->setCurrentRow(index);
    m_activePreset = preset;

    emit signalPaintopChanged(preset);
}

KoID KisBrushEngineSelector::defaultPaintop(const KoInputDevice & inputDevice)
{
    if (inputDevice == KoInputDevice::eraser()) {
        return KoID("eraser", "");
    } else {
        return KoID("paintbrush", "");
    }
}

KisPaintOpPresetSP KisBrushEngineSelector::activePreset(const KoID & paintop, const KoInputDevice & inputDevice)
{
    QHash<QString, KisPaintOpPresetSP> settingsArray;

    if (!m_inputDevicePresets.contains(inputDevice)) {
        foreach(const KoID& paintop, KisPaintOpRegistry::instance()->listKeys()) {
            settingsArray[paintop.id()] =
                    KisPaintOpRegistry::instance()->defaultPreset(paintop, m_view->image());
        }
        m_inputDevicePresets[inputDevice] = settingsArray;
    } else {
        settingsArray = m_inputDevicePresets[inputDevice];
    }

    if (settingsArray.contains(paintop.id())) {
        KisPaintOpPresetSP preset = settingsArray[paintop.id()];
        return preset;
    } else {
        warnKrita << "Could not get paintop preset for paintop " << paintop.name() << ", return default";
        return KisPaintOpRegistry::instance()->defaultPreset(paintop, m_view->image());
    }
}

void KisBrushEngineSelector::slotSaveActivePreset()
{
    KisPaintOpPresetSP preset = m_resourceProvider->currentPreset();
    if (!preset)
        return;

    KisPaintOpPreset* newPreset = preset->clone();
    newPreset->setImage(m_presetsPopup->cutOutOverlay());

    KoResourceServer<KisPaintOpPreset>* rServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    QString saveLocation = rServer->saveLocation();

    QString name = m_presetsPopup->getPresetName();

    int i = 1;
    QFileInfo fileInfo;
    do {
        fileInfo.setFile(saveLocation + name + QString("%1.kpp").arg(i));
        i++;
    } while (fileInfo.exists());
    newPreset->setFilename(fileInfo.filePath());
    newPreset->setName(name);

    rServer->addResource(newPreset);
}

void KisBrushEngineSelector::slotUpdatePreset()
{
    m_optionWidget->writeConfiguration(const_cast<KisPaintOpSettings*>(m_activePreset->settings().data()));
}

void KisBrushEngineSelector::slotSetupDefaultPreset(){
    QString defaultName = m_activePreset->paintOp().id() + ".kpp";
    QString path = KGlobal::mainComponent().dirs()->findResource("kis_defaultpresets", defaultName);
    KisPaintOpPresetSP preset = new KisPaintOpPreset(path);

    if ( !preset->load() ){
        kWarning() << preset->filename() << "could not be found.";
        kWarning() << "I was looking for " << defaultName;
        return;
    }

    preset->settings()->setNode( m_activePreset->settings()->node() );
    preset->settings()->setOptionsWidget(m_optionWidget);
    m_optionWidget->setConfiguration(preset->settings());
    m_optionWidget->writeConfiguration(const_cast<KisPaintOpSettings*>( preset->settings().data() ));
}

void KisBrushEngineSelector::contextMenuEvent(QContextMenuEvent *e) {

    QMenu menu(this);
    QAction* action = menu.addAction(m_detached ? i18n("Attach to Toolbar") : i18n("Detach from Toolbar"));
    connect(action, SIGNAL(triggered()), this, SLOT(switchDetached()));
    menu.exec(e->globalPos());
}

void KisBrushEngineSelector::switchDetached()
{
    if (parentWidget()) {
        if (m_detached) {
            m_detached = !m_detached;
            parentWidget()->setWindowFlags(Qt::Tool);
            parentWidget()->show();
        }
        else {
            parentWidget()->setWindowFlags(Qt::Popup);
        }

        KisConfig cfg;
        cfg.setPaintopPopupDetached(m_detached);
    }
}

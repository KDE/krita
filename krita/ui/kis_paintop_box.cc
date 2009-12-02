/*
 *  kis_paintop_box.cc - part of KImageShop/Krayon/Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
 *  Copyright (c) 2009 Sven Langkamp (sven.langkamp@gmail.com)
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
#include <QWidget>
#include <QString>
#include <QPixmap>
#include <QLayout>
#include <QHBoxLayout>

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

#include <kis_paintop_registry.h>
#include <kis_canvas_resource_provider.h>
#include <kis_painter.h>
#include <kis_paintop.h>
#include <kis_layer.h>
#include <kis_resource_server_provider.h>
#include <kis_paintop_settings.h>
#include <kis_config_widget.h>
#include <kis_image.h>
#include <kis_node.h>
#include "kis_node_manager.h"
#include "kis_layer_manager.h"
#include "kis_view2.h"
#include "kis_factory2.h"
#include "widgets/kis_preset_widget.h"
#include "widgets/kis_paintop_presets_popup.h"
#include <kis_paintop_settings_widget.h>

#include "ko_favorite_resource_manager.h"

KisPaintopBox::KisPaintopBox(KisView2 * view, QWidget *parent, const char * name)
        : QWidget(parent)
        , m_resourceProvider(view->resourceProvider())
        , m_optionWidget(0)
        , m_presetWidget(0)
        , m_view(view)
        , m_activePreset(0)
{
    Q_ASSERT(view != 0);

    setObjectName(name);

    KAcceleratorManager::setNoAccel(this);

    setWindowTitle(i18n("Painter's Toolchest"));

    m_cmbPaintops = new KComboBox(this);
    m_cmbPaintops->setObjectName("KisPaintopBox::m_cmbPaintops");
    m_cmbPaintops->setMinimumWidth(150);
    m_cmbPaintops->setMaxVisibleItems(20);
    m_cmbPaintops->setToolTip(i18n("Artist's materials"));

#ifdef Q_WS_MAC
    m_cmbPaintops->setAttribute(Qt::WA_MacSmallSize, true);
#endif

    m_presetWidget = new KisPresetWidget(this, "presetwidget");
    m_presetWidget->setToolTip(i18n("Edit brush preset"));
    m_presetWidget->setFixedSize(120, 26);

    m_layout = new QHBoxLayout(this);
    m_layout->addWidget(m_cmbPaintops);
    m_layout->addWidget(m_presetWidget);

    m_presetsPopup = new KisPaintOpPresetsPopup();
    m_presetWidget->setPopupWidget(m_presetsPopup);

    QList<KoID> keys = KisPaintOpRegistry::instance()->listKeys();
    for (QList<KoID>::Iterator it = keys.begin(); it != keys.end(); ++it) {
        // add all paintops, and show/hide them afterwards
        m_paintops.append(*it);
    }
    m_colorspace = view->image()->colorSpace();
    updatePaintops();
    setCurrentPaintop(defaultPaintop(KoToolManager::instance()->currentInputDevice()));

    connect(m_cmbPaintops, SIGNAL(activated(int)), this, SLOT(slotItemSelected(int)));

    connect(m_presetsPopup, SIGNAL(savePresetClicked()), this, SLOT(slotSaveActivePreset()));
}

KisPaintopBox::~KisPaintopBox()
{
    // Do not delete the widget, since it it is global to the application, not owned by the view
    m_presetsPopup->setPaintOpSettingsWidget(0);
}

KisPaintOpPresetSP KisPaintopBox::paintOpPresetSP(KoID* paintop)
{
    if (paintop == 0)
        return m_activePreset->clone();
    else if (!QString::compare(paintop->id(), "eraser", Qt::CaseInsensitive))
        return activePreset(*paintop, KoInputDevice::eraser());
    else
        return activePreset(*paintop, KoToolManager::instance()->currentInputDevice());
}

void KisPaintopBox::slotItemSelected(int index)
{
    if (index < m_displayedOps.count()) {
        KoID paintop = m_displayedOps.at(index);
        setCurrentPaintop(paintop);
    }
}

void KisPaintopBox::colorSpaceChanged(const KoColorSpace *cs)
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
        m_cmbPaintops->setCurrentIndex(index);
        slotItemSelected(index);

    }
}

void KisPaintopBox::updatePaintops()
{
    m_displayedOps.clear();
    m_cmbPaintops->clear();

    foreach(const KoID & paintopId, m_paintops) {
        if (KisPaintOpRegistry::instance()->userVisible(paintopId, m_colorspace)) {
            QPixmap pm = paintopPixmap(paintopId);

            if (pm.isNull()) {
                pm = QPixmap(16, 16);
                pm.fill();
            }

            m_cmbPaintops->addItem(QIcon(pm), paintopId.name());
            m_displayedOps.append(paintopId);
        }
    }

}

QPixmap KisPaintopBox::paintopPixmap(const KoID & paintop)
{
    QString pixmapName = KisPaintOpRegistry::instance()->pixmap(paintop);

    if (pixmapName.isEmpty()) {
        return QPixmap();
    }

    QString fname = KisFactory2::componentData().dirs()->findResource("kis_images", pixmapName);

    return QPixmap(fname);
}

void KisPaintopBox::slotInputDeviceChanged(const KoInputDevice & inputDevice)
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

    m_cmbPaintops->setCurrentIndex(index);
    setCurrentPaintop(paintop);
}

void KisPaintopBox::slotCurrentNodeChanged(KisNodeSP node)
{
    for (InputDevicePresetsMap::iterator it = m_inputDevicePresets.begin();
            it != m_inputDevicePresets.end();
            ++it) {
        foreach(const KisPaintOpPresetSP & preset, it.value()) {
            if (preset && preset->settings()) {
                preset->settings()->setNode(node);
            }
        }
    }
}


const KoID& KisPaintopBox::currentPaintop()
{
    KoID id = m_currentID[KoToolManager::instance()->currentInputDevice()];
    return m_currentID[KoToolManager::instance()->currentInputDevice()];
}


void KisPaintopBox::setCurrentPaintop(const KoID & paintop)
{
    if (m_activePreset && m_optionWidget) {
        m_optionWidget->writeConfiguration(const_cast<KisPaintOpSettings*>(m_activePreset->settings().data()));
        m_optionWidget->disconnect(m_presetWidget);
        m_optionWidget->disconnect(m_presetsPopup->presetPreview());
        m_presetsPopup->setPaintOpSettingsWidget(0);
        m_optionWidget->hide();
        const_cast<KisPaintOpSettings*>(m_activePreset->settings().data())->setOptionsWidget(0);
    }

    m_currentID[KoToolManager::instance()->currentInputDevice()] = paintop;

    KisPaintOpPresetSP preset =
        activePreset(currentPaintop(), KoToolManager::instance()->currentInputDevice());

    if (preset != 0 && preset->settings()) {
        if (!m_paintopOptionWidgets.contains(paintop)) {
        m_paintopOptionWidgets[paintop] = KisPaintOpRegistry::instance()->get(paintop.id())->createSettingsWidget(this);
        }
        m_optionWidget = m_paintopOptionWidgets[paintop];
        // XXX: Hack!
        const_cast<KisPaintOpSettings*>(preset->settings().data())->setOptionsWidget(m_optionWidget);
        m_optionWidget->setImage(m_view->image());

        if (!preset->settings()->getProperties().isEmpty()) {
            m_optionWidget->setConfiguration(preset->settings());
        }
        m_presetsPopup->setPaintOpSettingsWidget(m_optionWidget);
        Q_ASSERT(m_optionWidget);
        Q_ASSERT(m_presetWidget);
        connect(m_optionWidget, SIGNAL(sigConfigurationUpdated()), this, SLOT(slotUpdatePreset()));
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

    m_cmbPaintops->setCurrentIndex(index);
    m_activePreset = preset;
    m_presetWidget->setPreset(m_activePreset);
    m_presetsPopup->presetPreview()->setPreset(m_activePreset);

    emit signalPaintopChanged();
}

KoID KisPaintopBox::defaultPaintop(const KoInputDevice & inputDevice)
{
    if (inputDevice == KoInputDevice::eraser()) {
        return KoID("eraser", "");
    } else {
        return KoID("paintbrush", "");
    }
}

KisPaintOpPresetSP KisPaintopBox::activePreset(const KoID & paintop, const KoInputDevice & inputDevice)
{
    QHash<QString, KisPaintOpPresetSP> settingsArray;

    if (!m_inputDevicePresets.contains(inputDevice)) {
        foreach(const KoID& paintop, KisPaintOpRegistry::instance()->listKeys()) {
            settingsArray[paintop.id()] =
                KisPaintOpRegistry::instance()->defaultPreset(paintop, inputDevice, m_view->image());
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
        return KisPaintOpRegistry::instance()->defaultPreset(paintop, inputDevice, m_view->image());
    }
}

void KisPaintopBox::slotSaveActivePreset()
{
    KisPaintOpPresetSP preset = m_resourceProvider->currentPreset();
    if (!preset)
        return;

    KisPaintOpPreset* newPreset = preset->clone();

    KoResourceServer<KisPaintOpPreset>* rServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    QString saveLocation = rServer->saveLocation();

    int i = 1;
    QFileInfo fileInfo;
    do {
        fileInfo.setFile(saveLocation + QString("PaintOpPreset%1.kpp").arg(i));
        i++;
    } while (fileInfo.exists());
    newPreset->setFilename(fileInfo.filePath());

    rServer->addResource(newPreset);
}

void KisPaintopBox::slotUpdatePreset()
{
    m_optionWidget->writeConfiguration(const_cast<KisPaintOpSettings*>(m_activePreset->settings().data()));
    m_presetWidget->updatePreview();
    m_presetsPopup->presetPreview()->updatePreview();
}

#include "kis_paintop_box.moc"

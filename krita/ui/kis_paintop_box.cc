/*
 *  kis_paintop_box.cc - part of KImageShop/Krayon/Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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


KisPaintopBox::KisPaintopBox(KisView2 * view, QWidget *parent, const char * name)
        : QWidget(parent),
        m_resourceProvider(view->resourceProvider()), m_view(view), m_optionWidget(0), m_activePreset(0)
{
    Q_ASSERT(view != 0);

    setObjectName(name);

    KAcceleratorManager::setNoAccel(this);

    setWindowTitle(i18n("Painter's Toolchest"));

    m_cmbPaintops = new KComboBox(this);
    m_cmbPaintops->setObjectName("KisPaintopBox::m_cmbPaintops");
    m_cmbPaintops->setMinimumWidth(150);
    m_cmbPaintops->setToolTip(i18n("Artist's materials"));

    m_cmbPaintopPresets = new KComboBox(this);
    m_cmbPaintopPresets->setObjectName("KisPaintopBox::m_cmbPaintopPresets");
    m_cmbPaintopPresets->setMinimumWidth(150);
    m_cmbPaintopPresets->setToolTip(i18n("Brush presets"));

    m_layout = new QHBoxLayout(this);
    m_layout->setMargin(1);
    m_layout->setSpacing(1);
    m_layout->addWidget(m_cmbPaintops);
    m_layout->addWidget(m_cmbPaintopPresets);

    KisPresetWidget * m_presetWidget = new KisPresetWidget(this, "presetwidget");
    m_presetWidget->setToolTip(i18n("Edit brush preset"));
    m_presetWidget->setFixedSize(120, 26);
    m_layout->addWidget(m_presetWidget);

    m_presetsPopup = new KisPaintOpPresetsPopup();
    m_presetWidget->setPopupWidget(m_presetsPopup);

    // XXX: Let's see... Are all paintops loaded and ready?
    QList<KoID> keys = KisPaintOpRegistry::instance()->listKeys();
    for (QList<KoID>::Iterator it = keys.begin(); it != keys.end(); ++it) {
        // add all paintops, and show/hide them afterwards
        addItem(*it);
    }

    setCurrentPaintop(defaultPaintop(KoToolManager::instance()->currentInputDevice()));

    connect(m_cmbPaintops, SIGNAL(activated(int)), this, SLOT(slotItemSelected(int)));

    connect(view->layerManager(), SIGNAL(currentColorSpaceChanged(const KoColorSpace*)),
            this, SLOT(colorSpaceChanged(const KoColorSpace*)));

    connect(view->nodeManager(), SIGNAL(sigNodeActivated(KisNodeSP)),
            SLOT(slotCurrentNodeChanged(KisNodeSP)));

    connect(KoToolManager::instance(), SIGNAL(inputDeviceChanged(const KoInputDevice &)),
            SLOT(slotInputDeviceChanged(const KoInputDevice &)));



}

KisPaintopBox::~KisPaintopBox()
{
    // XXX???
    m_presetsPopup->setPaintOpSettingsWidget(0);
}

void KisPaintopBox::addItem(const KoID & paintop, const QString & /*category*/)
{
    m_paintops.append(paintop);
}

void KisPaintopBox::slotItemSelected(int index)
{
    if (index < m_displayedOps.count()) {
        KoID paintop = m_displayedOps.at(index);

        setCurrentPaintop(paintop);

        m_cmbPaintopPresets->clear();
        KisPaintOpPresetSP preset =
            activePreset(currentPaintop(), KoToolManager::instance()->currentInputDevice());
        m_cmbPaintopPresets->addItem(preset->name(), QVariant(preset));

        foreach(KisPaintOpPreset* preset,
                KisResourceServerProvider::instance()->paintOpPresetServer()->resources()) {
            if (preset->paintOp() == paintop) {
                m_cmbPaintopPresets->addItem(preset->name(), QVariant(preset));
            }
        }
    }
}

void KisPaintopBox::colorSpaceChanged(const KoColorSpace *cs)
{
    m_displayedOps.clear();
    m_cmbPaintops->clear();

    foreach(const KoID & paintopId, m_paintops) {
        if (KisPaintOpRegistry::instance()->userVisible(paintopId, cs)) {
            QPixmap pm = paintopPixmap(paintopId);

            if (pm.isNull()) {
                pm = QPixmap(16, 16);
                pm.fill();
            }

            m_cmbPaintops->addItem(QIcon(pm), paintopId.name());
            m_displayedOps.append(paintopId);
        }
    }

    int index = m_displayedOps.indexOf(currentPaintop());

    if (index == -1) {
        // Must change the paintop as the current one is not supported
        // by the new colorspace.
        index = 0;
    }

    m_cmbPaintops->setCurrentIndex(index);
    slotItemSelected(index);
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
    dbgKrita << "Change input device : " << inputDevice.device() << " " << inputDevice.pointer() << " " << inputDevice.uniqueTabletId() ;
    dbgKrita << KoToolManager::instance()->currentInputDevice().device() << " " << KoToolManager::instance()->currentInputDevice().pointer() << " " << KoToolManager::instance()->currentInputDevice().uniqueTabletId() ;
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
    return m_currentID[KoToolManager::instance()->currentInputDevice()];
}

void KisPaintopBox::setCurrentPaintop(const KoID & paintop)
{

    if ( m_activePreset && m_optionWidget ) {
        m_optionWidget->writeConfiguration( const_cast<KisPaintOpSettings*>( m_activePreset->settings().data() ) );
    }

    m_currentID[KoToolManager::instance()->currentInputDevice()] = paintop;

    KisPaintOpPresetSP preset =
        activePreset(currentPaintop(), KoToolManager::instance()->currentInputDevice());
    dbgUI << "active preset for paintop " << paintop.id() << " is " << preset;

    if (preset != 0 && preset->settings() && preset->settings()->widget()) {
        m_optionWidget = preset->settings()->widget();
        if ( !preset->settings()->getProperties().isEmpty() ) {
            m_optionWidget->setConfiguration( preset->settings() );
        }
        m_presetsPopup->setPaintOpSettingsWidget(m_optionWidget);
    } else {
        m_presetsPopup->setPaintOpSettingsWidget(0);
    }

    m_resourceProvider->slotPaintOpPresetActivated(preset);

    m_activePreset = preset.data();
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

    if ( !m_inputDevicePresets.contains( inputDevice ) ) {
        foreach( const KoID& paintop, KisPaintOpRegistry::instance()->listKeys() ) {
            settingsArray[paintop.id()] =
                KisPaintOpRegistry::instance()->defaultPreset(paintop, 0, inputDevice, m_view->image());
        }
        m_inputDevicePresets[inputDevice] = settingsArray;
    }
    else {
        settingsArray = m_inputDevicePresets[inputDevice];
    }

    if ( settingsArray.contains( paintop.id() ) ) {
        KisPaintOpPresetSP preset = settingsArray[paintop.id()];
        return preset;
    }
    else {
        warnKrita << "Could not get paintop preset for paintop " << paintop.name() << ", return default";
        return KisPaintOpRegistry::instance()->defaultPreset(paintop, 0, inputDevice, m_view->image());
    }
}


#include "kis_paintop_box.moc"


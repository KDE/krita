/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_paintop_config_widget.h"

#include <brushengine/kis_paintop_settings.h>

#include <KisResourcesInterface.h>
#include <KoCanvasResourcesInterface.h>


KisPaintOpConfigWidget::KisPaintOpConfigWidget(QWidget * parent, Qt::WindowFlags f)
    : KisConfigWidget(parent, f, 100),
      m_isInsideUpdateCall(0)
{
}

KisPaintOpConfigWidget::~KisPaintOpConfigWidget() {
}


void KisPaintOpConfigWidget::writeConfigurationSafe(KisPropertiesConfigurationSP config) const
{
    if (m_isInsideUpdateCall) return;

    m_isInsideUpdateCall++;
    writeConfiguration(config);
    m_isInsideUpdateCall--;
}

void KisPaintOpConfigWidget::setConfigurationSafe(const KisPropertiesConfigurationSP config)
{
    if (m_isInsideUpdateCall) return;

    m_isInsideUpdateCall++;
    setConfiguration(config);
    m_isInsideUpdateCall--;
}

void KisPaintOpConfigWidget::setImage(KisImageWSP image) {
    m_image = image;
}

void KisPaintOpConfigWidget::setNode(KisNodeWSP node) {
    m_node = node;
}

void KisPaintOpConfigWidget::setResourcesInterface(KisResourcesInterfaceSP resourcesInterface)
{
    m_resourcesInterface = resourcesInterface;
}

void KisPaintOpConfigWidget::setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    m_canvasResourcesInterface = canvasResourcesInterface;
}

KisResourcesInterfaceSP KisPaintOpConfigWidget::resourcesInterface() const
{
    return m_resourcesInterface;
}

KoCanvasResourcesInterfaceSP KisPaintOpConfigWidget::canvasResourcesInterface() const
{
    return m_canvasResourcesInterface;
}

bool KisPaintOpConfigWidget::supportScratchBox() {
    return true;
}

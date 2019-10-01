/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_paintop_config_widget.h"

#include <brushengine/kis_paintop_settings.h>

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

bool KisPaintOpConfigWidget::presetIsValid() {
    return true;
}

bool KisPaintOpConfigWidget::supportScratchBox() {
    return true;
}

/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_liquify_properties.h"

#include <kglobal.h>
#include <kglobalsettings.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include "kis_debug.h"


QString liquifyModeString(KisLiquifyProperties::LiquifyMode mode)
{
    QString result;

    switch (mode) {
    case KisLiquifyProperties::MOVE:
        result = "Move";
        break;
    case KisLiquifyProperties::SCALE:
        result = "Scale";
        break;
    case KisLiquifyProperties::ROTATE:
        result = "Rotate";
        break;
    case KisLiquifyProperties::OFFSET:
        result = "Offset";
        break;
    case KisLiquifyProperties::UNDO:
        result = "Undo";
        break;
    }

    return QString("LiquifyTool/%1").arg(result);
}

void KisLiquifyProperties::saveMode() const
{
    KConfigGroup cfg =
        KGlobal::config()->group(liquifyModeString(m_mode));

    cfg.writeEntry("size", m_size);
    cfg.writeEntry("amount", m_amount);
    cfg.writeEntry("spacing", m_spacing);
    cfg.writeEntry("sizeHasPressure", m_sizeHasPressure);
    cfg.writeEntry("amountHasPressure", m_amountHasPressure);
    cfg.writeEntry("reverseDirection", m_reverseDirection);
    cfg.writeEntry("useWashMode", m_useWashMode);
    cfg.writeEntry("flow", m_flow);

    KConfigGroup globalCfg = KGlobal::config()->group("LiquifyTool");
    globalCfg.writeEntry("mode", (int)m_mode);
}

void KisLiquifyProperties::loadMode()
{
    KConfigGroup cfg =
        KGlobal::config()->group(liquifyModeString(m_mode));

    m_size = cfg.readEntry("size", m_size);
    m_amount = cfg.readEntry("amount", m_amount);
    m_spacing = cfg.readEntry("spacing", m_spacing);
    m_sizeHasPressure = cfg.readEntry("sizeHasPressure", m_sizeHasPressure);
    m_amountHasPressure = cfg.readEntry("amountHasPressure", m_amountHasPressure);
    m_reverseDirection = cfg.readEntry("reverseDirection", m_reverseDirection);
    m_useWashMode = cfg.readEntry("useWashMode", m_useWashMode);
    m_flow = cfg.readEntry("flow", m_flow);
}

void KisLiquifyProperties::loadAndResetMode()
{
    loadMode();

    KConfigGroup globalCfg = KGlobal::config()->group("LiquifyTool");
    m_mode = (LiquifyMode) globalCfg.readEntry("mode", (int)m_mode);
}

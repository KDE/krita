/* This file is part of the KDE project
 * Copyright (C) Timothée Giet <animtim@gmail.com>, (C) 2014
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "kis_pressure_flow_option.h"
#include <klocale.h>
#include <kis_painter.h>
#include <KoColor.h>

KisPressureFlowOption::KisPressureFlowOption()
    : KisCurveOption(i18n("Flow"), "Flow", KisPaintOpOption::generalCategory(), true)
{
    m_checkable = false;
    setMinimumLabel(i18n("0%"));
    setMaximumLabel(i18n("100%"));
}

void KisPressureFlowOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    KisCurveOption::writeOptionSetting(setting);
    setting->setProperty("FlowValue", m_flow);
}

void KisPressureFlowOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    KisCurveOption::readOptionSetting(setting);
    setFlow(setting->getDouble("FlowValue", 1.0));
}


void KisPressureFlowOption::apply(KisPainter *painter, const KisPaintInformation & /*info*/) const
{
    painter->setFlow(quint8(getFlow() * 255.0));
}
 
void KisPressureFlowOption::setFlow(qreal flow)
{
    m_flow = qBound(qreal(0), flow, qreal(1));
}

qreal KisPressureFlowOption::getFlow() const
{
    return m_flow;
}


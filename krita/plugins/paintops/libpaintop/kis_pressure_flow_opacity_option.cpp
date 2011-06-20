/* 
 * Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
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

#include "kis_pressure_flow_opacity_option.h"
#include "kis_paint_action_type_option.h"

#include <klocale.h>

#include <kis_painter.h>
#include <kis_paint_information.h>
#include <widgets/kis_curve_widget.h>

KisFlowOpacityOption::KisFlowOpacityOption():
    KisCurveOption(i18n("Opacity"), "Opacity", KisPaintOpOption::brushCategory(), true, 1.0, 0.0, 1.0, true, true),
    m_flow(1.0)
{
    m_checkable = false;
    setMinimumLabel(i18n("Transparent"));
    setMaximumLabel(i18n("Opaque"));
}

void KisFlowOpacityOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    KisCurveOption::writeOptionSetting(setting);
    setting->setProperty("FlowValue", m_flow);
}

void KisFlowOpacityOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    KisCurveOption::readOptionSetting(setting);
    setFlow(setting->getDouble("FlowValue", 1.0));
    m_paintActionType = setting->getInt("PaintOpAction", BUILDUP);
}

qreal KisFlowOpacityOption::getFlow() const
{
    return m_flow;
}

qreal KisFlowOpacityOption::getStaticOpacity() const
{
    return value();
}

qreal KisFlowOpacityOption::getDynamicOpacity(const KisPaintInformation& info) const
{
    return computeValue(info);
}

void KisFlowOpacityOption::setFlow(qreal flow)
{
    m_flow = qBound(qreal(0), flow, qreal(1));
}

void KisFlowOpacityOption::setOpacity(qreal opacity)
{
    setValue(opacity);
}

void KisFlowOpacityOption::apply(KisPainter* painter, const KisPaintInformation& info)
{
    if(m_paintActionType == WASH)
        painter->setOpacity(quint8(getDynamicOpacity(info) * 255.0));
    else
        painter->setOpacity(quint8(getStaticOpacity() * getDynamicOpacity(info) * 255.0));
    
    painter->setFlow(quint8(getFlow() * 255.0));
}

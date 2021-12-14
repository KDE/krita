/*
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_pressure_flow_opacity_option.h"
#include "kis_paint_action_type_option.h"

#include <klocalizedstring.h>

#include <kis_painter.h>
#include <brushengine/kis_paint_information.h>
#include <kis_indirect_painting_support.h>
#include <kis_node.h>
#include <widgets/kis_curve_widget.h>

KisFlowOpacityOption::KisFlowOpacityOption(KisNodeSP currentNode)
    : KisCurveOption(KoID("Opacity", i18n("Opacity")), KisPaintOpOption::GENERAL, true, 1.0, 0.0, 1.0)
    , m_flow(1.0)
{
    setCurveUsed(true);
    setSeparateCurveValue(true);

    m_checkable = false;

    m_nodeHasIndirectPaintingSupport =
        currentNode &&
        dynamic_cast<KisIndirectPaintingSupport*>(currentNode.data());
}

void KisFlowOpacityOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisCurveOption::writeOptionSetting(setting);
    setting->setProperty("FlowValue", m_flow);
}

void KisFlowOpacityOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
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
    return computeSizeLikeValue(info);
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
    quint8 opacity = OPACITY_OPAQUE_U8;
    quint8 flow = OPACITY_OPAQUE_U8;

    apply(info, &opacity, &flow);

    painter->setOpacityUpdateAverage(opacity);
    painter->setFlow(flow);
}

void KisFlowOpacityOption::apply(const KisPaintInformation &info, quint8 *opacity, quint8 *flow)
{
    if (m_paintActionType == WASH && m_nodeHasIndirectPaintingSupport) {
        *opacity = quint8(getDynamicOpacity(info) * 255.0);
    } else {
        *opacity = quint8(getStaticOpacity() * getDynamicOpacity(info) * 255.0);
    }

    *flow = quint8(getFlow() * 255.0);
}

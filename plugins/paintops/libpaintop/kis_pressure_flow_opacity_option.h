/*
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_PRESSURE_FLOW_OPACITY_OPTION_H
#define KIS_PRESSURE_FLOW_OPACITY_OPTION_H

#include "kis_curve_option.h"
#include <kritapaintop_export.h>
#include <kis_types.h>
#include <brushengine/kis_paintop_settings.h>

class KisPaintInformation;
class KisPainter;

class PAINTOP_EXPORT KisFlowOpacityOption: public KisCurveOption
{
public:
    KisFlowOpacityOption(KisNodeSP currentNode);
    ~KisFlowOpacityOption() override { }

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

    void setFlow(qreal flow);
    void setOpacity(qreal opacity);
    void apply(KisPainter* painter, const KisPaintInformation& info);
    void apply(const KisPaintInformation& info, quint8 *opacity, quint8 *flow);

    qreal getFlow() const;
    qreal getStaticOpacity() const;
    qreal getDynamicOpacity(const KisPaintInformation& info) const;

protected:
    qreal m_flow;
    int   m_paintActionType;
    bool  m_nodeHasIndirectPaintingSupport;
};

#endif //KIS_PRESSURE_FLOW_OPACITY_OPTION_H

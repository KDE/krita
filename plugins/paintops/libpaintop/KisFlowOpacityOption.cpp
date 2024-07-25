/*
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisFlowOpacityOption.h"


#include <kis_properties_configuration.h>
#include <kis_painter.h>
#include <kis_node.h>
#include <kis_indirect_painting_support.h>


KisFlowOpacityOption2::KisFlowOpacityOption2(const KisPropertiesConfiguration *setting, KisNodeSP currentNode)
    : m_opacityOption(setting),
      m_flowOption(setting)
{
    if (currentNode &&
        setting->getString(KisPropertiesConfiguration::extractedPrefixKey()).isEmpty()) {

        KisIndirectPaintingSupport *indirect =
            dynamic_cast<KisIndirectPaintingSupport*>(currentNode.data());
        m_indirectPaintingActive = indirect && indirect->hasTemporaryTarget();
    }
}

void KisFlowOpacityOption2::apply(KisPainter* painter, const KisPaintInformation& info)
{
    quint8 opacity = OPACITY_OPAQUE_U8;
    quint8 flow = OPACITY_OPAQUE_U8;

    apply(info, &opacity, &flow);

    painter->setOpacityUpdateAverage(opacity);
    painter->setFlow(flow);
}

void KisFlowOpacityOption2::apply(const KisPaintInformation &info, quint8 *opacity, quint8 *flow)
{
    if (m_opacityOption.isChecked()) {
        *opacity = quint8(m_opacityOption.computeSizeLikeValue(info, !m_indirectPaintingActive) * 255.0);
    }
    *flow = quint8(m_flowOption.apply(info) * 255.0);
}




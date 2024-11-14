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
    qreal opacity = OPACITY_OPAQUE_F;
    qreal flow = OPACITY_OPAQUE_F;

    apply(info, &opacity, &flow);

    painter->setOpacityUpdateAverage(opacity);
    painter->setFlow(flow);
}

void KisFlowOpacityOption2::apply(const KisPaintInformation &info, qreal *opacity, qreal *flow)
{
    if (m_opacityOption.isChecked()) {
        *opacity = m_opacityOption.computeSizeLikeValue(info, !m_indirectPaintingActive);
    }
    *flow = m_flowOption.apply(info);
}




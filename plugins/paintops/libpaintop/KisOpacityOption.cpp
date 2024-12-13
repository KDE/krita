/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisOpacityOption.h"

#include <kis_properties_configuration.h>
#include <kis_painter.h>
#include <kis_node.h>
#include <kis_indirect_painting_support.h>


KisOpacityOption::KisOpacityOption(const KisPropertiesConfiguration *setting, KisNodeSP currentNode)
    : BaseClass(setting)
{
    if (currentNode &&
        setting->getString(KisPropertiesConfiguration::extractedPrefixKey()).isEmpty()) {

        KisIndirectPaintingSupport *indirect =
            dynamic_cast<KisIndirectPaintingSupport*>(currentNode.data());
        m_indirectPaintingActive = indirect && indirect->hasTemporaryTarget();
    }
}

void KisOpacityOption::apply(KisPainter* painter, const KisPaintInformation& info) const
{
    const qreal opacity = isChecked() ? computeSizeLikeValue(info, !m_indirectPaintingActive) : 1.0;
    painter->setOpacityUpdateAverage(opacity);
}

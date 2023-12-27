/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISFLOWOPACITYOPTION_H
#define KISFLOWOPACITYOPTION_H

#include "KisStandardOptions.h"
#include "KisOpacityOption.h"

class PAINTOP_EXPORT KisFlowOpacityOption2
{
public:
    KisFlowOpacityOption2(const KisPropertiesConfiguration *setting, KisNodeSP currentNode);

    void apply(KisPainter* painter, const KisPaintInformation& info);
    void apply(const KisPaintInformation &info, quint8 *opacity, quint8 *flow);

private:
    KisOpacityOption m_opacityOption;
    KisFlowOption m_flowOption;
    bool m_indirectPaintingActive {false};
};

#endif // KISFLOWOPACITYOPTION_H

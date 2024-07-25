/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDynamicSensorFade.h"

#include <KisDynamicSensorIds.h>

#include <kis_paint_information.h>
#include <KisCurveOptionData.h>


KisDynamicSensorFade::KisDynamicSensorFade(const KisSensorWithLengthData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor(FadeId, data, curveOverride)
    , m_periodic(data.isPeriodic)
    , m_length(data.length)
{
}

qreal KisDynamicSensorFade::value(const KisPaintInformation &pi) const
{
    if (pi.isHoveringMode()) return 1.0;

    const int currentValue =
        m_periodic ?
        pi.currentDabSeqNo() % m_length :
        qMin(pi.currentDabSeqNo(), m_length);

    return qreal(currentValue) / m_length;
}

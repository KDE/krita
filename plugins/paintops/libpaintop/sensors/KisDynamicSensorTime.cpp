/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDynamicSensorTime.h"

#include <KisDynamicSensorIds.h>

#include <kis_paint_information.h>
#include <KisCurveOptionData.h>


KisDynamicSensorTime::KisDynamicSensorTime(const KisSensorWithLengthData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor(TimeId, data, curveOverride)
    , m_periodic(data.isPeriodic)
    , m_length(data.length)
{
}

qreal KisDynamicSensorTime::value(const KisPaintInformation &pi) const
{
    if (pi.isHoveringMode()) return 1.0;

    const qreal currentTime =
        m_periodic ?
        std::fmod(pi.currentTime(), m_length) :
        qMin(pi.currentTime(), qreal(m_length));

    return currentTime / qreal(m_length);
}

/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDynamicSensorDistance.h"

#include <KisDynamicSensorIds.h>

#include <kis_paint_information.h>
#include <KisCurveOptionData.h>


KisDynamicSensorDistance::KisDynamicSensorDistance(const KisSensorWithLengthData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor(DistanceId, data, curveOverride)
    , m_periodic(data.isPeriodic)
    , m_length(data.length)
{
}

qreal KisDynamicSensorDistance::value(const KisPaintInformation &pi) const
{
    if (pi.isHoveringMode()) return 1.0;

    const qreal distance =
        m_periodic ?
        fmod(pi.totalStrokeLength(), m_length) :
        qMin(pi.totalStrokeLength(), (qreal)m_length);

    return distance / m_length;
}

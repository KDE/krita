/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDynamicSensor.h"

#include <kis_algebra_2d.h>
#include <KisSensorData.h>

KisDynamicSensor::KisDynamicSensor(const KoID &id,
                                     const KisSensorData &data,
                                     std::optional<KisCubicCurve> curveOverride)
    : m_id(id),
      m_curve(curveOverride ?
                  *curveOverride :
                  KisCubicCurve(data.curve))
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(id == data.id);

    if (m_curve->isIdentity()) {
        m_curve = std::nullopt;
    }
}

KisDynamicSensor::~KisDynamicSensor()
{
}

KoID KisDynamicSensor::id() const
{
    return m_id;
}

qreal KisDynamicSensor::parameter(const KisPaintInformation &info) const
{
    const qreal val = value(info);
    if (m_curve) {
        qreal scaledVal = isAdditive() ? additiveToScaling(val) :
                          isAbsoluteRotation() ? KisAlgebra2D::wrapValue(val + 0.5, 0.0, 1.0) : val;

        const QVector<qreal> transfer = m_curve->floatTransfer(256);
        scaledVal = KisCubicCurve::interpolateLinear(scaledVal, transfer);

        return isAdditive() ? scalingToAdditive(scaledVal) :
               isAbsoluteRotation() ? KisAlgebra2D::wrapValue(scaledVal + 0.5, 0.0, 1.0) : scaledVal;
    }
    else {
        return val;
    }
}

bool KisDynamicSensor::isAdditive() const
{
    return false;
}

bool KisDynamicSensor::isAbsoluteRotation() const
{
    return false;
}

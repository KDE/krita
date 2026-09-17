/*
 * SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisCanvasNavigationActionStrategy.h"

#include <kis_algebra_2d.h>

namespace
{
qreal angleForSnapping(qreal angle)
{
    if (angle < 0) {
        return std::fmod(angle - 2, 45) + 2;
    } else {
        return std::fmod(angle + 2, 45) - 2;
    }
}
} // namespace

qreal KisCanvasNavigationActionStrategy::canvasRotationAngleContinuous(qreal currentRelativeRotation, qreal currectCanvasRotation, SnappedRotationData &data)
{
    if (!data.previousAngle) {
        data.previousAngle = currentRelativeRotation;
        return 0;
    }
    qreal rotationAngle = (180 / M_PI) * (currentRelativeRotation - data.previousAngle);
    data.previousAngle = currentRelativeRotation;

    const qreal canvasAnglePostRotation = currectCanvasRotation + rotationAngle;
    const qreal snapDelta = angleForSnapping(canvasAnglePostRotation);
    // we snap the canvas to an angle that is a multiple of 45
    if (abs(snapDelta) <= 2 && abs(data.accumRotationAngle) <= 2) {
        // accumulate the relative angle of finger from the point when we started snapping
        data.accumRotationAngle += rotationAngle;
        rotationAngle = rotationAngle - snapDelta;
    } else {
        // snap the canvas out using the accumulated angle
        rotationAngle += data.accumRotationAngle;
        data.accumRotationAngle = 0;
    }

    return rotationAngle;
}

qreal KisCanvasNavigationActionStrategy::canvasRotationAngleDescrete(qreal currentRelativeRotation, SnappedRotationData &data)
{
    if (!data.initialReferenceAngle) {
        data.initialReferenceAngle = currentRelativeRotation;
        return 0;
    }
    qreal rotationAngle = 0;
    const qreal relativeAngle = (180 / M_PI) * (currentRelativeRotation - data.initialReferenceAngle);
    const qreal rotationThreshold = 15;

    // if the canvas is moved in either direction with an angle greater than the threshold, we rotate the canvas in
    // that direction by 15°.
    if (std::abs(relativeAngle) >= rotationThreshold && std::abs(relativeAngle) <= (360 - rotationThreshold)) {
        // set reference as currentAngle to check if we go beyond the threshold next time
        data.initialReferenceAngle = currentRelativeRotation;

        if (std::abs(relativeAngle) <= 180) {
            rotationAngle = KisAlgebra2D::copysign(15.0, relativeAngle);
        } else {
            // if we're over 180, it means the canvas has to be rotated in the opposite direction of the current
            // angle. E.g if the relative angle is +341° then we move the canvas by -15° (because the actual effect
            // is 341 - 360 = -19°  on the original theta).
            rotationAngle = KisAlgebra2D::copysign(15.0, -relativeAngle);
        }
    }
    return rotationAngle;
}

KisCanvasNavigationActionStrategy::KisCanvasNavigationActionStrategy(const Flags &flags)
    : m_flags(flags)
{
}

KisCanvasNavigationActionStrategy::Flags KisCanvasNavigationActionStrategy::flags() const
{
    return m_flags;
}
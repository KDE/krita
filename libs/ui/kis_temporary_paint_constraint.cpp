/*
 *  SPDX-FileCopyrightText: 2026 Ayanami Kaine <personal@ayanamikaine.com>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_temporary_paint_constraint.h"

#include <cmath>

#include <QtGlobal>

#include <kis_algebra_2d.h>

namespace {
constexpr qreal minLineLengthSquared = 1e-6;
constexpr qreal snapAngleStepRadians = 3.14159265358979323846 / 12.0;

QPointF snappedEndpoint(const QPointF &anchor, const QPointF &point)
{
    const QPointF delta = point - anchor;
    const qreal length = KisAlgebra2D::norm(delta);
    if (qFuzzyIsNull(length)) {
        return point;
    }

    const qreal angle = std::atan2(delta.y(), delta.x());
    const qreal snappedAngle = std::round(angle / snapAngleStepRadians) * snapAngleStepRadians;
    return anchor + QPointF(length * std::cos(snappedAngle), length * std::sin(snappedAngle));
}
}

void KisTemporaryPaintConstraint::setInteractionMode(InteractionMode mode)
{
    m_interactionMode = mode;
}

KisTemporaryPaintConstraint::InteractionMode KisTemporaryPaintConstraint::interactionMode() const
{
    return m_interactionMode;
}

void KisTemporaryPaintConstraint::beginLine(const QPointF &anchor)
{
    m_anchor = anchor;
    m_secondPoint = anchor;
    m_hasLine = true;
    m_locked = false;
    m_angleSnapped = false;
}

void KisTemporaryPaintConstraint::updateLine(const QPointF &secondPoint, bool snapToAngle)
{
    if (m_locked) {
        return;
    }

    if (!m_hasLine) {
        beginLine(secondPoint);
        return;
    }

    if (snapToAngle) {
        m_secondPoint = snappedEndpoint(m_anchor, secondPoint);
        m_angleSnapped = true;
    } else if (!m_angleSnapped) {
        m_secondPoint = secondPoint;
    }
}

bool KisTemporaryPaintConstraint::hasLine() const
{
    return m_hasLine;
}

void KisTemporaryPaintConstraint::setLocked(bool locked)
{
    m_locked = locked && hasValidLine();
}

bool KisTemporaryPaintConstraint::isLocked() const
{
    return m_locked;
}

bool KisTemporaryPaintConstraint::hasValidLine() const
{
    return m_hasLine && KisAlgebra2D::normSquared(m_secondPoint - m_anchor) > minLineLengthSquared;
}

QPointF KisTemporaryPaintConstraint::lineAnchor() const
{
    return m_anchor;
}

QPointF KisTemporaryPaintConstraint::lineSecondPoint() const
{
    return m_secondPoint;
}

QPointF KisTemporaryPaintConstraint::adjustPosition(const QPointF &point,
                                                    const QPointF &strokeBegin,
                                                    bool checkForInitialMovement,
                                                    qreal moveThresholdPt) const
{
    if (!m_locked || !hasValidLine()) {
        return point;
    }

    if (checkForInitialMovement && KisAlgebra2D::norm(point - strokeBegin) < moveThresholdPt) {
        return strokeBegin;
    }

    const QPointF delta = m_secondPoint - m_anchor;
    const qreal squareLength = KisAlgebra2D::normSquared(delta);
    if (qFuzzyIsNull(squareLength)) {
        return point;
    }

    const qreal t = KisAlgebra2D::dotProduct(point - m_anchor, delta) / squareLength;
    return m_anchor + delta * t;
}

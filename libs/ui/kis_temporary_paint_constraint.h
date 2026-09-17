/*
 *  SPDX-FileCopyrightText: 2026 Ayanami Kaine <personal@ayanamikaine.com>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TEMPORARY_PAINT_CONSTRAINT_H
#define KIS_TEMPORARY_PAINT_CONSTRAINT_H

#include <QPointF>

#include "kritaui_export.h"

class KRITAUI_EXPORT KisTemporaryPaintConstraint
{
public:
    enum class InteractionMode {
        Normal,
        SnapToAngle
    };

    void setInteractionMode(InteractionMode mode);
    InteractionMode interactionMode() const;

    void beginLine(const QPointF &anchor);
    void updateLine(const QPointF &secondPoint, bool snapToAngle = false);
    void setLocked(bool locked);

    bool hasLine() const;
    bool isLocked() const;
    bool hasValidLine() const;

    QPointF lineAnchor() const;
    QPointF lineSecondPoint() const;

    QPointF adjustPosition(const QPointF &point,
                           const QPointF &strokeBegin,
                           bool checkForInitialMovement,
                           qreal moveThresholdPt) const;

private:
    QPointF m_anchor;
    QPointF m_secondPoint;
    InteractionMode m_interactionMode {InteractionMode::Normal};
    bool m_hasLine {false};
    bool m_locked {false};
    bool m_angleSnapped {false};
};

#endif // KIS_TEMPORARY_PAINT_CONSTRAINT_H

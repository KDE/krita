/*
 * SPDX-FileCopyrightText: 2023 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SVG_MOVE_TEXT_STRATEGY_H
#define SVG_MOVE_TEXT_STRATEGY_H

#include <KoInteractionStrategy.h>

#include <QPointF>

class KoSvgTextShape;

class SvgMoveTextStrategy : public KoInteractionStrategy
{
public:
    SvgMoveTextStrategy(KoToolBase *tool, KoSvgTextShape *shape, const QPointF &clicked);
    ~SvgMoveTextStrategy() override = default;

    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers) override;
    KUndo2Command *createCommand() override;
    void cancelInteraction() override;
    void finishInteraction(Qt::KeyboardModifiers modifiers) override;

private:
    KoSvgTextShape *m_shape;
    QPointF m_dragStart;
    QPointF m_initialPosition;
    QPointF m_finalPosition;
    QPointF m_anchorOffset;
};

#endif /* SVG_MOVE_TEXT_STRATEGY_H */

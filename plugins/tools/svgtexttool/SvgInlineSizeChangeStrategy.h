/*
 * SPDX-FileCopyrightText: 2023 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SVG_INLINE_SIZE_CHANGE_STRATEGY_H
#define SVG_INLINE_SIZE_CHANGE_STRATEGY_H

#include <KoInteractionStrategy.h>

#include <QPointF>

class KoSvgTextShape;

namespace SvgInlineSizeHelper
{
enum class VisualAnchor;
enum class Side;
} // namespace SvgInlineSizeHelper


class SvgInlineSizeChangeStrategy : public KoInteractionStrategy
{
public:
    SvgInlineSizeChangeStrategy(KoToolBase *tool, KoSvgTextShape *shape, const QPointF &clicked, bool start);
    ~SvgInlineSizeChangeStrategy() override = default;

    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers) override;
    KUndo2Command *createCommand() override;
    void cancelInteraction() override;
    void finishInteraction(Qt::KeyboardModifiers modifiers) override;

private:
    KoSvgTextShape *m_shape;
    double m_initialInlineSize;
    double m_finalInlineSize;
    QPointF m_dragStart;
    int m_originalAnchor;
    int m_finalAnchor;
    QPointF m_initialPosition;
    QPointF m_finalPos;
    QPointF m_anchorOffset;
    QPointF m_snapDelta;
    SvgInlineSizeHelper::VisualAnchor m_anchor;
    SvgInlineSizeHelper::Side m_handleSide;
    bool m_startHandle;
};

#endif /* SVG_INLINE_SIZE_CHANGE_STRATEGY_H */

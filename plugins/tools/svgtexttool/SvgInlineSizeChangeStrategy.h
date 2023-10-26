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
    SvgInlineSizeChangeStrategy(KoToolBase *tool, KoSvgTextShape *shape, const QPointF &clicked);
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
    SvgInlineSizeHelper::VisualAnchor m_anchor;
    SvgInlineSizeHelper::Side m_handleSide;
};

#endif /* SVG_INLINE_SIZE_CHANGE_STRATEGY_H */

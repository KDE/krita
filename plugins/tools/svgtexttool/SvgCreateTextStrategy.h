/*
 * SPDX-FileCopyrightText: 2023 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SVG_CREATE_TEXT_STRATEGY_H
#define SVG_CREATE_TEXT_STRATEGY_H

#include <KoInteractionStrategy.h>

#include <QPointF>
#include <QSizeF>

class SvgTextTool;

class KoSvgTextShape;
class KoShape;

class SvgCreateTextStrategy : public KoInteractionStrategy
{
public:
    SvgCreateTextStrategy(SvgTextTool *tool, const QPointF &clicked, KoShape *shape = nullptr);
    ~SvgCreateTextStrategy() override = default;

    void paint(QPainter &painter, const KoViewConverter &converter) override;
    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers) override;
    KUndo2Command *createCommand() override;
    void cancelInteraction() override;
    void finishInteraction(Qt::KeyboardModifiers modifiers) override;

    bool draggingInlineSize();
    bool hasWrappingShape();

private:
    QPointF m_dragStart;
    QPointF m_dragEnd;
    QSizeF m_minSizeInline;
    KoShape *m_flowShape;
    Qt::KeyboardModifiers m_modifiers;
};

#endif /* SVG_CREATE_TEXT_STRATEGY_H */

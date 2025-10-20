/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef SVGCHANGETEXTPADDINGMARGINSTRATEGY_H
#define SVGCHANGETEXTPADDINGMARGINSTRATEGY_H

#include <KoInteractionStrategy.h>

#include <KoSvgTextShape.h>
#include <optional>

class SvgTextTool;
class KoPathShape;

class SvgChangeTextPaddingMarginStrategy : public KoInteractionStrategy
{
public:
    SvgChangeTextPaddingMarginStrategy(SvgTextTool *tool, KoSvgTextShape *shape, const QPointF &clicked);
    ~SvgChangeTextPaddingMarginStrategy();


    /**
     * @brief hitTest
     * Tests whether the current mouse position is over a text wrapping area,
     * and if so, will return the angle vector at that point.
     * @param shape -- shape to test for.
     * @param mousePos -- mousePos to test against.
     * @param grabSensitivityInPts -- grabSensitivity in Points
     * @return -- std::optional containing the angle vector.
     */
    static std::optional<QPointF> hitTest(KoSvgTextShape *shape, const QPointF &mousePos, const qreal grabSensitivityInPts);
private:
    KoSvgTextShape *m_shape;
    KoPathShape *m_referenceShape;
    bool m_isPadding;
    QPointF m_lastMousePos;

    // KoInteractionStrategy interface
public:
    void paint(QPainter &painter, const KoViewConverter &converter) override;
    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers) override;
    KUndo2Command *createCommand() override;
    void finishInteraction(Qt::KeyboardModifiers modifiers) override;
};

#endif // SVGCHANGETEXTPADDINGMARGINSTRATEGY_H

/*
 * SPDX-FileCopyrightText: 2023 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SVG_INLINE_SIZE_HELPER_H
#define SVG_INLINE_SIZE_HELPER_H

#include "KoSvgText.h"
#include "KoSvgTextProperties.h"
#include "KoSvgTextShape.h"

#include <optional>

namespace SvgInlineSizeHelper
{

[[nodiscard]] static inline double getInlineSizePt(const KoSvgTextShape *const shape)
{
    const KoSvgText::AutoValue inlineSizeProp =
        shape->textProperties().property(KoSvgTextProperties::InlineSizeId).value<KoSvgText::AutoValue>();
    if (!inlineSizeProp.isAuto) {
        return inlineSizeProp.customValue;
    }
    return 0.0;
}

enum class VisualAnchor {
    LeftOrTop,
    Mid,
    RightOrBottom,
};

enum class Side {
    LeftOrTop,
    RightOrBottom,
};

struct Q_DECL_HIDDEN InlineSizeInfo {
    double inlineSize;
    /// Baseline coord along the block-flow direction
    double baseline;
    /// Left coord (vertical mode) or top coord (horizontal mode)
    double left;
    /// Right coord (vertical mode) or bottom coord (horizontal mode)
    double right;
    /// Top coord along the block-flow direction (right for h-rl, left for h-lr)
    double top;
    /// Bottom coord along the block-flow direction (left for h-rl, right for h-lr)
    double bottom;
    VisualAnchor anchor;
    /// Transformation from inline-size editor (writing-mode transformation) to shape
    QTransform editorTransform;
    /// Transformation from shape local to document
    QTransform shapeTransform;

    [[nodiscard]] static inline std::optional<InlineSizeInfo> fromShape(KoSvgTextShape *const shape)
    {
        const double inlineSize = getInlineSizePt(shape);
        if (inlineSize <= 0) {
            return {};
        }
        KoSvgTextProperties props = shape->propertiesForPos(-1);

        const KoSvgText::WritingMode writingMode = KoSvgText::WritingMode(
            props.propertyOrDefault(KoSvgTextProperties::WritingModeId).toInt());
        const KoSvgText::Direction direction =
            KoSvgText::Direction(props.propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());
        const KoSvgText::TextAnchor textAnchor =
            KoSvgText::TextAnchor(props.propertyOrDefault(KoSvgTextProperties::TextAnchorId).toInt());

        VisualAnchor anchor{};
        switch (textAnchor) {
        case KoSvgText::TextAnchor::AnchorStart:
        default:
            if (direction == KoSvgText::Direction::DirectionLeftToRight) {
                anchor = VisualAnchor::LeftOrTop;
            } else {
                anchor = VisualAnchor::RightOrBottom;
            }
            break;
        case KoSvgText::TextAnchor::AnchorMiddle:
            anchor = VisualAnchor::Mid;
            break;
        case KoSvgText::TextAnchor::AnchorEnd:
            if (direction == KoSvgText::Direction::DirectionLeftToRight) {
                anchor = VisualAnchor::RightOrBottom;
            } else {
                anchor = VisualAnchor::LeftOrTop;
            }
            break;
        }
        const double xPos = shape->initialTextPosition().x();
        const double yPos = shape->initialTextPosition().y();

        const double baseline = yPos;
        double left{};
        double right{};
        double top{};
        double bottom{};
        switch (anchor) {
        case VisualAnchor::LeftOrTop:
            left = xPos;
            right = xPos + inlineSize;
            break;
        case VisualAnchor::Mid:
            left = xPos - inlineSize * 0.5;
            right = xPos + inlineSize * 0.5;
            break;
        case VisualAnchor::RightOrBottom:
            left = xPos - inlineSize;
            right = xPos;
            break;
        };

        // We piggyback on the shape transformation that we already need to
        // deal with to also handle the different orientations of writing-mode.
        // We default to the "default caret size"  when the textShape (and thus outline) is empty.
        QLineF caret;
        shape->cursorForPos(0, caret);
        const QRectF outline = shape->outlineRect().isEmpty()? QRectF(caret.p2(), caret.p1()): shape->outlineRect();
        QTransform editorTransform;
        switch (writingMode) {
        case KoSvgText::WritingMode::HorizontalTB:
        default:
            top = outline.top();
            bottom = outline.bottom();
            break;
        case KoSvgText::WritingMode::VerticalRL:
            editorTransform.rotate(90.0);
            top = -outline.right();
            bottom = -outline.left();
            break;
        case KoSvgText::WritingMode::VerticalLR:
            editorTransform.rotate(-90.0);
            editorTransform.scale(-1.0, 1.0);
            top = outline.left();
            bottom = outline.right();
            break;
        }
        InlineSizeInfo ret{inlineSize,
                           baseline,
                           left,
                           right,
                           top,
                           bottom,
                           anchor,
                           editorTransform,
                           shape->absoluteTransformation()};
        return {ret};
    }

private:
    [[nodiscard]] inline QLineF leftLineRaw() const
    {
        return {left, top, left, bottom};
    }

    [[nodiscard]] inline QLineF rightLineRaw() const
    {
        return {right, top, right, bottom};
    }

    [[nodiscard]] inline QRectF boundingRectRaw() const
    {
        return {QPointF(left, top), QPointF(right, bottom)};
    }

    [[nodiscard]] inline QVector<QLineF> generateLineDashes(const QLineF line, qreal toPtMultiplier = 1.0, qreal dashLengthPixels = 4.0) const
    {
        QVector <QLineF> dashes;

        qreal dashL = dashLengthPixels * toPtMultiplier;
        int pattern = 3;
        int totalDashes = 3 * pattern;

        QPointF start = line.p2();
        for (int i = 0; i <= totalDashes; i++) {
            QLineF dash = line;
            dash.setLength(line.length() + dashL * i);
            dash.setP1(start);
            if (i % pattern == 0) {
                dashes.append(dash);
            }
            start = dash.p2();
        }
        return dashes;
    }

public:
    /**
     * @brief Gets a shape-local line representing the first line baseline. This
     * always goes from left to right by the inline-base direction, then mapped
     * by the editor transformation.
     * @return QLineF
     */
    [[nodiscard]] inline QLineF baselineLineLocal() const
    {
        return editorTransform.map(QLineF{left, baseline, right, baseline});
    }

    /**
     * @brief Gets a line representing the first line baseline. This always
     * goes from left to right by the inline-base direction, then mapped by the
     * editor and the shape transformation.
     * @return QLineF
     */
    [[nodiscard]] inline QLineF baselineLine() const
    {
        return shapeTransform.map(baselineLineLocal());
    }

    [[nodiscard]] inline Side endLineSide() const
    {
        switch (anchor) {
        case VisualAnchor::LeftOrTop:
        case VisualAnchor::Mid:
        default:
            return Side::RightOrBottom;
        case VisualAnchor::RightOrBottom:
            return Side::LeftOrTop;
        }
    }

    [[nodiscard]] inline QLineF endLineLocal() const
    {
        switch (endLineSide()) {
        case Side::LeftOrTop:
            return editorTransform.map(leftLineRaw());
        case Side::RightOrBottom:
        default:
            return editorTransform.map(rightLineRaw());
        }
    }

    [[nodiscard]] inline QVector<QLineF> endLineDashes(qreal pixelToPt) const
    {
        switch (endLineSide()) {
        case Side::LeftOrTop:
            return generateLineDashes(editorTransform.map(leftLineRaw()), pixelToPt);
        case Side::RightOrBottom:
        default:
            return generateLineDashes(editorTransform.map(rightLineRaw()), pixelToPt);
        }
    }

    [[nodiscard]] inline QLineF endLine() const
    {
        return shapeTransform.map(endLineLocal());
    }

    [[nodiscard]] inline Side startLineSide() const
    {
        switch (anchor) {
        case VisualAnchor::LeftOrTop:
        case VisualAnchor::Mid:
        default:
            return Side::LeftOrTop;
        case VisualAnchor::RightOrBottom:
            return Side::RightOrBottom;
        }
    }

    [[nodiscard]] inline QLineF startLineLocal() const
    {
        switch (endLineSide()) {
        case Side::LeftOrTop:
            return editorTransform.map(rightLineRaw());
        case Side::RightOrBottom:
        default:
            return editorTransform.map(leftLineRaw());
        }
    }

    [[nodiscard]] inline QVector<QLineF> startLineDashes(qreal pixelToPt) const
    {
        switch (endLineSide()) {
        case Side::LeftOrTop:
            return generateLineDashes(editorTransform.map(rightLineRaw()), pixelToPt);
        case Side::RightOrBottom:
        default:
            return generateLineDashes(editorTransform.map(leftLineRaw()), pixelToPt);
        }
    }

    [[nodiscard]] inline QLineF startLine() const
    {
        return shapeTransform.map(startLineLocal());
    }

    [[nodiscard]] inline QPolygonF endLineGrabRect(double grabThreshold) const
    {
        QLineF endLine;
        switch (endLineSide()) {
        case Side::LeftOrTop:
            endLine = leftLineRaw();
            break;
        case Side::RightOrBottom:
        default:
            endLine = rightLineRaw();
            break;
        }
        const QRectF rect{endLine.x1() - grabThreshold,
                          top - grabThreshold,
                          grabThreshold * 2,
                          bottom - top + grabThreshold * 2};
        const QPolygonF poly(rect);
        return shapeTransform.map(editorTransform.map(poly));
    }

    [[nodiscard]] inline QPolygonF startLineGrabRect(double grabThreshold) const
    {
        QLineF startLine;
        switch (endLineSide()) {
        case Side::LeftOrTop:
            startLine = rightLineRaw();
            break;
        case Side::RightOrBottom:
        default:
            startLine = leftLineRaw();
            break;
        }
        const QRectF rect{startLine.x1() - grabThreshold,
                          top - grabThreshold,
                          grabThreshold * 2,
                          bottom - top + grabThreshold * 2};
        const QPolygonF poly(rect);
        return shapeTransform.map(editorTransform.map(poly));
    }

    [[nodiscard]] inline QRectF boundingRect() const
    {
        return shapeTransform.mapRect(editorTransform.mapRect(boundingRectRaw()));
    }
};

} // namespace SvgInlineSizeHelper

#endif /* SVG_INLINE_SIZE_HELPER_H */

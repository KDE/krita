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
        const double xPos = 0;
        const double yPos = 0;

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
        const QRectF outline = shape->outlineRect();
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
        bottom += 4.0;
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

    [[nodiscard]] inline Side editLineSide() const
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

    [[nodiscard]] inline QLineF editLineLocal() const
    {
        switch (editLineSide()) {
        case Side::LeftOrTop:
            return editorTransform.map(leftLineRaw());
        case Side::RightOrBottom:
        default:
            return editorTransform.map(rightLineRaw());
        }
    }

    [[nodiscard]] inline QLineF editLine() const
    {
        return shapeTransform.map(editLineLocal());
    }

    [[nodiscard]] inline QLineF nonEditLineLocal() const
    {
        switch (editLineSide()) {
        case Side::LeftOrTop:
            return editorTransform.map(rightLineRaw());
        case Side::RightOrBottom:
        default:
            return editorTransform.map(leftLineRaw());
        }
    }

    [[nodiscard]] inline QLineF nonEditLine() const
    {
        return shapeTransform.map(nonEditLineLocal());
    }

    [[nodiscard]] inline QPolygonF editLineGrabRect(double grabThreshold) const
    {
        QLineF editLine;
        switch (editLineSide()) {
        case Side::LeftOrTop:
            editLine = leftLineRaw();
            break;
        case Side::RightOrBottom:
        default:
            editLine = rightLineRaw();
            break;
        }
        const QRectF rect{editLine.x1() - grabThreshold,
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

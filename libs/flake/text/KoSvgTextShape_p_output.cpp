/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoSvgTextShape.h"
#include "KoSvgTextShape_p.h"

#include "KoSvgTextProperties.h"

#include <KoClipMaskPainter.h>
#include <KoColorBackground.h>
#include <KoPathShape.h>
#include <KoShapeStroke.h>

#include <QPainter>
#include <QtMath>

#include <variant>

void inheritPaintProperties(KisForest<KoSvgTextContentElement>::composition_iterator it,
                       KoShapeStrokeModelSP &stroke,
                       QSharedPointer<KoShapeBackground> &background,
                       QVector<KoShape::PaintOrder> &paintOrder) {
    for (auto parentIt = KisForestDetail::hierarchyBegin(siblingCurrent(it)); parentIt != KisForestDetail::hierarchyEnd(siblingCurrent(it)); parentIt++) {
        if (parentIt->properties.hasProperty(KoSvgTextProperties::StrokeId)) {
            stroke = parentIt->properties.stroke();
            break;
        }
    }
    for (auto parentIt = KisForestDetail::hierarchyBegin(siblingCurrent(it)); parentIt != KisForestDetail::hierarchyEnd(siblingCurrent(it)); parentIt++) {
        if (parentIt->properties.hasProperty(KoSvgTextProperties::FillId)) {
            background = parentIt->properties.background();
            break;
        }
    }
    for (auto parentIt = KisForestDetail::hierarchyBegin(siblingCurrent(it)); parentIt != KisForestDetail::hierarchyEnd(siblingCurrent(it)); parentIt++) {
        if (parentIt->properties.hasProperty(KoSvgTextProperties::PaintOrder)) {
            paintOrder = parentIt->properties.propertyOrDefault(KoSvgTextProperties::PaintOrder).value<QVector<KoShape::PaintOrder>>();
            break;
        }
    }
}


// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void KoSvgTextShape::Private::paintPaths(QPainter &painter,
                                         const QPainterPath &rootOutline,
                                         const KoShape *rootShape,
                                         const QVector<CharacterResult> &result,
                                         QPainterPath &chunk,
                                         int &currentIndex)
{

    KoShapeStrokeModelSP stroke = rootShape->stroke();
    QSharedPointer<KoShapeBackground> background = rootShape->background();
    QVector<KoShape::PaintOrder> paintOrder = rootShape->paintOrder();

    for (auto it = compositionBegin(textData); it != compositionEnd(textData); it++) {
        inheritPaintProperties(it, stroke, background, paintOrder);

        if (it.state() == KisForestDetail::Enter) {
            QMap<KoSvgText::TextDecoration, QPainterPath> textDecorations = it->textDecorations;
            QColor textDecorationColor = it->properties.propertyOrDefault(KoSvgTextProperties::TextDecorationColorId).value<QColor>();
            if (textDecorations.contains(KoSvgText::DecorationUnderline)) {
                Q_FOREACH(const KoShape::PaintOrder p, paintOrder) {
                    if (p == KoShape::Fill) {
                        if (background && !textDecorationColor.isValid() && textDecorationColor != Qt::transparent) {
                            background->paint(painter, textDecorations.value(KoSvgText::DecorationUnderline));
                        } else if (textDecorationColor.isValid()) {
                            painter.fillPath(textDecorations.value(KoSvgText::DecorationUnderline), textDecorationColor);
                        }
                    } else if (p == KoShape::Stroke) {
                        if (stroke) {
                            QScopedPointer<KoShape> shape(KoPathShape::createShapeFromPainterPath(textDecorations.value(KoSvgText::DecorationUnderline)));
                            stroke->paint(shape.data(), painter);
                        }
                    }
                }

            }
            if (textDecorations.contains(KoSvgText::DecorationOverline)) {
                Q_FOREACH(const KoShape::PaintOrder p, paintOrder) {
                    if (p == KoShape::Fill) {
                        if (background && !textDecorationColor.isValid() && textDecorationColor != Qt::transparent) {
                            background->paint(painter, textDecorations.value(KoSvgText::DecorationOverline));
                        } else if (textDecorationColor.isValid()) {
                            painter.fillPath(textDecorations.value(KoSvgText::DecorationOverline), textDecorationColor);
                        }
                    } else if (p == KoShape::Stroke) {
                        if (stroke) {
                            QScopedPointer<KoShape> shape(KoPathShape::createShapeFromPainterPath(textDecorations.value(KoSvgText::DecorationOverline)));
                            stroke->paint(shape.data(), painter);
                        }
                    }
                }
            }

            if (childCount(siblingCurrent(it)) == 0) {
                const int j = currentIndex + it->numChars(true);

                const QRect shapeGlobalClipRect = painter.transform().mapRect(it->associatedOutline.boundingRect()).toAlignedRect();

                if (shapeGlobalClipRect.isValid()) {
                    KoClipMaskPainter fillPainter(&painter, shapeGlobalClipRect);
                    if (background) {
                        background->paint(*fillPainter.shapePainter(), rootOutline);
                        fillPainter.maskPainter()->fillPath(rootOutline, Qt::black);
                        if (textRendering != OptimizeSpeed && painter.testRenderHint(QPainter::Antialiasing)) {
                            // also apply antialiasing only if antialiasing is active on provided target QPainter
                            fillPainter.maskPainter()->setRenderHint(QPainter::Antialiasing, true);
                            fillPainter.maskPainter()->setRenderHint(QPainter::SmoothPixmapTransform, true);
                        } else {
                            fillPainter.maskPainter()->setRenderHint(QPainter::Antialiasing, false);
                            fillPainter.maskPainter()->setRenderHint(QPainter::SmoothPixmapTransform, false);
                        }
                    }
                    QPainterPath textDecorationsRest;
                    textDecorationsRest.setFillRule(Qt::WindingFill);

                    for (int i = currentIndex; i < j; i++) {
                        if (result.at(i).addressable && !result.at(i).hidden) {
                            const QTransform tf = result.at(i).finalTransform();

                            /**
                     * Make sure the character touches the painter's clip rect,
                     * otherwise we can just skip it
                     */
                            const QRectF boundingRect = tf.mapRect(result.at(i).boundingBox);
                            const QRectF clipRect = painter.clipBoundingRect();
                            if (boundingRect.isEmpty() ||
                                    (!clipRect.contains(boundingRect) &&
                                     !clipRect.intersects(boundingRect))) continue;

                            /**
                     * There's an annoying problem here that officially speaking
                     * the chunks need to be unified into one single path before
                     * drawing, so there's no weirdness with the stroke, but
                     * QPainterPath's union function will frequently lead to
                     * reduced quality of the paths because of 'numerical
                     * instability'.
                     */

                            if (const auto *colorGlyph = std::get_if<Glyph::ColorLayers>(&result.at(i).glyph)) {
                                for (int c = 0; c < colorGlyph->paths.size(); c++) {
                                    QBrush color = colorGlyph->colors.at(c);
                                    bool replace = colorGlyph->replaceWithForeGroundColor.at(c);
                                    // In theory we can use the pattern or gradient as well
                                    // for ColorV0 fonts, but ColorV1 fonts can have
                                    // gradients, so I am hesitant.
                                    KoColorBackground *b = dynamic_cast<KoColorBackground *>(background.data());
                                    if (b && replace) {
                                        color = b->brush();
                                    }
                                    painter.fillPath(tf.map(colorGlyph->paths.at(c)), color);
                                }
                            } else if (const auto *outlineGlyph = std::get_if<Glyph::Outline>(&result.at(i).glyph)) {
                                chunk.addPath(tf.map(outlineGlyph->path));
                            } else if (const auto *bitmapGlyph = std::get_if<Glyph::Bitmap>(&result.at(i).glyph)) {
                                if (bitmapGlyph->image.isGrayscale() || bitmapGlyph->image.format() == QImage::Format_Mono) {
                                    fillPainter.maskPainter()->save();
                                    fillPainter.maskPainter()->translate(result.at(i).finalPosition.x(), result.at(i).finalPosition.y());
                                    fillPainter.maskPainter()->rotate(qRadiansToDegrees(result.at(i).rotate));
                                    fillPainter.maskPainter()->setCompositionMode(QPainter::CompositionMode_Plus);
                                    fillPainter.maskPainter()->drawImage(bitmapGlyph->drawRect, bitmapGlyph->image);
                                    fillPainter.maskPainter()->restore();
                                } else {
                                    painter.save();
                                    painter.translate(result.at(i).finalPosition.x(), result.at(i).finalPosition.y());
                                    painter.rotate(qRadiansToDegrees(result.at(i).rotate));
                                    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
                                    painter.drawImage(bitmapGlyph->drawRect, bitmapGlyph->image);
                                    painter.restore();
                                }
                            }
                        }
                    }
                    Q_FOREACH(const KoShape::PaintOrder p, paintOrder) {
                        if (p == KoShape::Fill) {
                            if (background) {
                                chunk.setFillRule(Qt::WindingFill);
                                fillPainter.maskPainter()->fillPath(chunk, Qt::white);
                            }
                            if (!textDecorationsRest.isEmpty()) {
                                fillPainter.maskPainter()->fillPath(textDecorationsRest.simplified(), Qt::white);
                            }
                            fillPainter.renderOnGlobalPainter();
                        } else if (p == KoShape::Stroke) {
                            KoShapeStrokeSP maskStroke;
                            if (stroke) {
                                KoShapeStrokeSP strokeSP = qSharedPointerDynamicCast<KoShapeStroke>(stroke);

                                if (strokeSP) {
                                    if (strokeSP->lineBrush().gradient()) {
                                        KoClipMaskPainter strokePainter(&painter, shapeGlobalClipRect);
                                        strokePainter.shapePainter()->fillRect(rootOutline.boundingRect(), strokeSP->lineBrush());
                                        maskStroke = KoShapeStrokeSP(new KoShapeStroke(*strokeSP.data()));
                                        maskStroke->setColor(Qt::white);
                                        maskStroke->setLineBrush(Qt::white);
                                        strokePainter.maskPainter()->fillPath(rootOutline, Qt::black);
                                        if (textRendering != OptimizeSpeed && painter.testRenderHint(QPainter::Antialiasing)) {
                                            // also apply antialiasing only if antialiasing is active on provided target QPainter
                                            strokePainter.maskPainter()->setRenderHint(QPainter::Antialiasing, true);
                                        } else {
                                            strokePainter.maskPainter()->setRenderHint(QPainter::Antialiasing, false);
                                        }
                                        {
                                            QScopedPointer<KoShape> shape(KoPathShape::createShapeFromPainterPath(chunk));
                                            maskStroke->paint(shape.data(), *strokePainter.maskPainter());
                                        }
                                        if (!textDecorationsRest.isEmpty()) {
                                            QScopedPointer<KoShape> shape(KoPathShape::createShapeFromPainterPath(textDecorationsRest));
                                            maskStroke->paint(shape.data(), *strokePainter.maskPainter());
                                        }
                                        strokePainter.renderOnGlobalPainter();
                                    } else {
                                        {
                                            QScopedPointer<KoShape> shape(KoPathShape::createShapeFromPainterPath(chunk));
                                            stroke->paint(shape.data(), painter);
                                        }
                                        if (!textDecorationsRest.isEmpty()) {
                                            QScopedPointer<KoShape> shape(KoPathShape::createShapeFromPainterPath(textDecorationsRest));
                                            stroke->paint(shape.data(), painter);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                chunk = QPainterPath();
                currentIndex = j;
            }
        }

        if (it.state() == KisForestDetail::Leave) {
            inheritPaintProperties(it, stroke, background, paintOrder);
            QMap<KoSvgText::TextDecoration, QPainterPath> textDecorations = it->textDecorations;
            QColor textDecorationColor = it->properties.propertyOrDefault(KoSvgTextProperties::TextDecorationColorId).value<QColor>();
            if (textDecorations.contains(KoSvgText::DecorationLineThrough)) {
                Q_FOREACH(const KoShape::PaintOrder p, paintOrder) {
                    if (p == KoShape::Fill) {
                        if (background && !textDecorationColor.isValid() && textDecorationColor != Qt::transparent) {
                            background->paint(painter, textDecorations.value(KoSvgText::DecorationLineThrough));
                        } else if (textDecorationColor.isValid()) {
                            painter.fillPath(textDecorations.value(KoSvgText::DecorationLineThrough), textDecorationColor);
                        }
                    } else if (p == KoShape::Stroke) {
                        if (stroke) {
                            QScopedPointer<KoShape> shape(KoPathShape::createShapeFromPainterPath(textDecorations.value(KoSvgText::DecorationLineThrough)));
                            stroke->paint(shape.data(), painter);
                        }
                    }
                }
            }
        }
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
QList<KoShape *>
KoSvgTextShape::Private::collectPaths(const KoShape *rootShape, QVector<CharacterResult> &result, int &currentIndex)
{

    QList<KoShape *> shapes;
    KoShapeStrokeModelSP stroke = rootShape->stroke();
    QSharedPointer<KoShapeBackground> background = rootShape->background();
    QVector<KoShape::PaintOrder> paintOrder = rootShape->paintOrder();

    for (auto it = compositionBegin(textData); it != compositionEnd(textData); it++) {
        bool hasPaintOrder = it->properties.hasProperty(KoSvgTextProperties::PaintOrder);
        inheritPaintProperties(it, stroke, background, paintOrder);
        QMap<KoSvgText::TextDecoration, QPainterPath> textDecorations = it->textDecorations;
        QColor textDecorationColor = it->properties.propertyOrDefault(KoSvgTextProperties::TextDecorationColorId).value<QColor>();
        QSharedPointer<KoShapeBackground> decorationColor = background;
        if (textDecorationColor.isValid()) {
            decorationColor = QSharedPointer<KoColorBackground>(new KoColorBackground(textDecorationColor));
        }

        KoShape::PaintOrder first = paintOrder.at(0);
        KoShape::PaintOrder second = paintOrder.at(1);

        if (it.state() == KisForestDetail::Enter) {


            if (textDecorations.contains(KoSvgText::DecorationUnderline)) {
                KoPathShape *shape = KoPathShape::createShapeFromPainterPath(textDecorations.value(KoSvgText::DecorationUnderline));
                shape->setBackground(decorationColor);
                shape->setStroke(stroke);
                shape->setZIndex(shapes.size());
                shape->setFillRule(Qt::WindingFill);
                if (hasPaintOrder)
                    shape->setPaintOrder(first, second);
                shapes.append(shape);
            }
            if (textDecorations.contains(KoSvgText::DecorationOverline)) {
                KoPathShape *shape = KoPathShape::createShapeFromPainterPath(textDecorations.value(KoSvgText::DecorationOverline));
                shape->setBackground(decorationColor);
                shape->setStroke(stroke);
                shape->setZIndex(shapes.size());
                shape->setFillRule(Qt::WindingFill);
                if (hasPaintOrder)
                    shape->setPaintOrder(first, second);
                shapes.append(shape);
            }

            if (childCount(siblingCurrent(it)) == 0) {
                QPainterPath chunk;

                const int j = currentIndex + it->numChars(true);
                for (int i = currentIndex; i < j; i++) {
                    if (result.at(i).addressable && !result.at(i).hidden) {
                        const QTransform tf = result.at(i).finalTransform();
                        if (const auto *colorGlyph = std::get_if<Glyph::ColorLayers>(&result.at(i).glyph)) {
                            for (int c = 0; c < colorGlyph->paths.size(); c++) {
                                QBrush color = colorGlyph->colors.at(c);
                                bool replace = colorGlyph->replaceWithForeGroundColor.at(c);
                                // In theory we can use the pattern or gradient as well
                                // for ColorV0 fonts, but ColorV1 fonts can have
                                // gradients, so I am hesitant.
                                KoColorBackground *b = dynamic_cast<KoColorBackground *>(background.data());
                                if (b && replace) {
                                    color = b->brush();
                                }
                                KoPathShape *shape = KoPathShape::createShapeFromPainterPath(tf.map(colorGlyph->paths.at(c)));
                                shape->setBackground(QSharedPointer<KoColorBackground>(new KoColorBackground(color.color())));
                                shape->setZIndex(shapes.size());
                                shape->setFillRule(Qt::WindingFill);
                                shape->setPaintOrder(first, second);
                                shapes.append(shape);
                            }
                        } else if (const auto *outlineGlyph = std::get_if<Glyph::Outline>(&result.at(i).glyph)) {
                            chunk.addPath(tf.map(outlineGlyph->path));
                        }
                    }
                }
                KoPathShape *shape = KoPathShape::createShapeFromPainterPath(chunk);
                shape->setBackground(background);
                shape->setStroke(stroke);
                shape->setZIndex(shapes.size());
                shape->setFillRule(Qt::WindingFill);
                if (hasPaintOrder)
                    shape->setPaintOrder(first, second);
                shapes.append(shape);
                currentIndex = j;

            }
        }
        if (it.state() ==KisForestDetail::Leave) {
            inheritPaintProperties(it, stroke, background, paintOrder);
            if (textDecorations.contains(KoSvgText::DecorationLineThrough)) {
                KoPathShape *shape = KoPathShape::createShapeFromPainterPath(textDecorations.value(KoSvgText::DecorationLineThrough));
                shape->setBackground(decorationColor);
                shape->setStroke(stroke);
                shape->setZIndex(shapes.size());
                shape->setFillRule(Qt::WindingFill);
                shape->setPaintOrder(first, second);
                shapes.append(shape);
            }
        }
    }
    return shapes;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void KoSvgTextShape::Private::paintDebug(QPainter &painter,
                                         const QVector<CharacterResult> &result,
                                         int &currentIndex)
{

    for (auto it = textData.depthFirstTailBegin(); it != textData.depthFirstTailEnd(); it++) {
        const int j = currentIndex + it->numChars(true);

        const QRect shapeGlobalClipRect = painter.transform().mapRect(it->associatedOutline.boundingRect()).toAlignedRect();

        painter.save();

        QFont font(QFont(), painter.device());
        font.setPointSizeF(16.0);

        if (shapeGlobalClipRect.isValid() && childCount(siblingCurrent(it)) == 0) {
            for (int i = currentIndex; i < j; i++) {
                if (result.at(i).addressable && !result.at(i).hidden) {
                    const QTransform tf = result.at(i).finalTransform();

#if 1 // Debug: draw character bounding boxes
                    painter.setBrush(Qt::transparent);
                    QPen pen(QColor(0, 0, 0, 50));
                    pen.setCosmetic(true);
                    pen.setWidth(2);
                    painter.setPen(pen);
                    if (const auto *bitmapGlyph = std::get_if<Glyph::Bitmap>(&result.at(i).glyph)) {
                        painter.drawPolygon(tf.map(bitmapGlyph->drawRect));
                    } else if (const auto *colorGlyph = std::get_if<Glyph::ColorLayers>(&result.at(i).glyph)) {
                        QRectF boundingRect;
                        Q_FOREACH (const QPainterPath &p, colorGlyph->paths) {
                            boundingRect |= p.boundingRect();
                        }
                        painter.drawPolygon(tf.map(boundingRect));
                    } else if (const auto *outlineGlyph = std::get_if<Glyph::Outline>(&result.at(i).glyph)) {
                        painter.drawPolygon(tf.map(outlineGlyph->path.boundingRect()));
                    }
                    QColor penColor = result.at(i).anchored_chunk ? result.at(i).isHanging ? Qt::red : Qt::magenta
                        : result.at(i).lineEnd == LineEdgeBehaviour::NoChange ? Qt::cyan
                                                                              : Qt::yellow;
                    penColor.setAlpha(192);
                    pen.setColor(penColor);
                    painter.setPen(pen);
                    painter.drawPolygon(tf.map(result.at(i).boundingBox));

                    penColor.setAlpha(96);
                    pen.setColor(penColor);
                    pen.setWidth(1);
                    pen.setStyle(Qt::DotLine);
                    painter.setPen(pen);
                    painter.drawPolygon(tf.map(result.at(i).lineHeightBox));

                    pen.setWidth(2);
                    pen.setStyle(Qt::SolidLine);

                    const QPointF center = tf.mapRect(result.at(i).boundingBox).center();
                    QString text = "#";
                    text += QString::number(i);
                    {
                        // Find the range of this typographic character
                        int end = i + 1;
                        while (end < result.size() && result[end].middle) {
                            end++;
                        }
                        end--;
                        if (end != i) {
                            text += "~";
                            text += QString::number(end);
                        }
                    }
                    text += QString("\n(%1)").arg(result.at(i).plaintTextIndex);
                    painter.setWorldMatrixEnabled(false);
                    painter.setPen(Qt::red);
                    painter.drawText(QRectF(painter.transform().map(center), QSizeF(0, 64)).adjusted(-128, 0, 128, 0),
                                     Qt::AlignHCenter | Qt::AlignTop,
                                     text);
                    painter.setWorldMatrixEnabled(true);

                    pen.setWidth(6);
                    const BreakType breakType = result.at(i).breakType;
                    if (breakType == BreakType::SoftBreak || breakType == BreakType::HardBreak) {
                        if (breakType == BreakType::SoftBreak) {
                            penColor = Qt::blue;
                        } else if (breakType == BreakType::HardBreak) {
                            penColor = Qt::red;
                        }
                        penColor.setAlpha(128);
                        pen.setColor(penColor);
                        painter.setPen(pen);
                        painter.drawPoint(center);
                    }
                    //ligature carets
                    penColor = Qt::darkGreen;
                    penColor.setAlpha(192);
                    pen.setColor(penColor);
                    painter.setPen(pen);
                    QVector<QPointF> offset = result.at(i).cursorInfo.offsets;
                    for (int k=0; k<offset.size(); k++) {
                        painter.drawPoint(tf.map(offset.at(k)));
                    }
                    // Finalpos
                    penColor = Qt::red;
                    penColor.setAlpha(192);
                    pen.setColor(penColor);
                    painter.setPen(pen);
                    painter.drawPoint(result.at(i).finalPosition);
#endif
                }
            }
        }
        painter.restore();
        currentIndex = j;
    }
}

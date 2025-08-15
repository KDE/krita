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
#include <KoGradientBackground.h>
#include <KoPathShape.h>
#include <KoShapeStroke.h>
#include <KoShapeRegistry.h>
#include <KoShapeFactoryBase.h>
#include <KoProperties.h>
#include <KoClipMask.h>
#include <KoInsets.h>
#include <KoShapeGroup.h>
#include <KoShapeGroupCommand.h>

#include <kis_algebra_2d.h>

#include <QPainter>
#include <QtMath>

#include <variant>

static void inheritPaintProperties(const KisForest<KoSvgTextContentElement>::composition_iterator it,
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

void setRenderHints(QPainter &painter, const KoSvgText::TextRendering textRendering, const bool testAntialiasing) {
    if (textRendering != KoSvgText::RenderingOptimizeSpeed && testAntialiasing) {
        // also apply antialiasing only if antialiasing is active on provided target QPainter
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    } else {
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    }
}

void KoSvgTextShape::Private::paintTextDecoration(QPainter &painter,
                                                  const QPainterPath &rootOutline,
                                                  const KoShape *rootShape,
                                                  const KoSvgText::TextDecoration type,
                                                  const KoSvgText::TextRendering rendering)
{
    KoShapeStrokeModelSP stroke = rootShape->stroke();
    QSharedPointer<KoShapeBackground> background = rootShape->background();
    QVector<KoShape::PaintOrder> paintOrder = rootShape->paintOrder();

    for (auto it = compositionBegin(textData); it != compositionEnd(textData); it++) {
        if (it.state() == KisForestDetail::Leave) continue;

        inheritPaintProperties(it, stroke, background, paintOrder);


        KoInsets insets;
        if (stroke) {
            stroke->strokeInsets(rootShape, insets);
        }
        QMap<KoSvgText::TextDecoration, QPainterPath> textDecorations = it->textDecorations;

        if (textDecorations.isEmpty() || !textDecorations.contains(type)) continue;

        const QPainterPath decorPath = textDecorations.value(type);
        const QRect shapeGlobalClipRect = painter.transform().mapRect(decorPath.boundingRect().adjusted(-insets.left, -insets.top, insets.right, insets.bottom)).toAlignedRect();

        if (!shapeGlobalClipRect.isValid()) continue;

        const QRectF clipRect = painter.clipBoundingRect();
        if (!clipRect.contains(decorPath.boundingRect()) &&
                 !clipRect.intersects(decorPath.boundingRect())) continue;
        const QColor textDecorationColor = it->properties.propertyOrDefault(KoSvgTextProperties::TextDecorationColorId).value<QColor>();
        const bool colorValid = textDecorationColor.isValid() && it->properties.hasProperty(KoSvgTextProperties::TextDecorationColorId) && textDecorationColor != Qt::transparent;

        Q_FOREACH(const KoShape::PaintOrder p, paintOrder) {
            if (p == KoShape::Fill) {

                if (background && !colorValid) {
                    KoClipMaskPainter fillPainter(&painter, shapeGlobalClipRect);
                    setRenderHints(*fillPainter.maskPainter(), rendering, painter.testRenderHint(QPainter::Antialiasing));
                    background->paint(*fillPainter.shapePainter(), rootOutline);
                    fillPainter.maskPainter()->fillPath(rootOutline, Qt::black);
                    fillPainter.maskPainter()->fillPath(decorPath, Qt::white);
                    fillPainter.renderOnGlobalPainter();
                } else if (colorValid) {
                    painter.fillPath(decorPath, textDecorationColor);
                }
            } else if (p == KoShape::Stroke) {
                if (stroke) {
                    KoShapeStrokeSP strokeSP = qSharedPointerDynamicCast<KoShapeStroke>(stroke);

                    if (strokeSP) {
                        if (strokeSP->lineBrush().gradient()) {
                            KoClipMaskPainter strokePainter(&painter, shapeGlobalClipRect);
                            QPainterPath strokeOutline;
                            strokeOutline.addRect(rootOutline.boundingRect().adjusted(-insets.left, -insets.top, insets.right, insets.bottom));
                            strokePainter.shapePainter()->fillRect(strokeOutline.boundingRect(), strokeSP->lineBrush());
                            strokePainter.maskPainter()->fillRect(strokeOutline.boundingRect(), Qt::black);

                            KoShapeStrokeSP maskStroke = KoShapeStrokeSP(new KoShapeStroke(*strokeSP.data()));
                            maskStroke->setColor(Qt::white);
                            maskStroke->setLineBrush(Qt::white);


                            setRenderHints(*strokePainter.maskPainter(), rendering, painter.testRenderHint(QPainter::Antialiasing));
                            {
                                QScopedPointer<KoShape> shape(KoPathShape::createShapeFromPainterPath(decorPath));
                                maskStroke->paint(shape.data(), *strokePainter.maskPainter());
                            }
                            strokePainter.renderOnGlobalPainter();
                        } else {
                            {
                                QScopedPointer<KoShape> shape(KoPathShape::createShapeFromPainterPath(decorPath));
                                stroke->paint(shape.data(), painter);
                            }
                        }
                    }
                }
            }
        }

    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void KoSvgTextShape::Private::paintPaths(QPainter &painter,
                                         const QPainterPath &rootOutline,
                                         const KoShape *rootShape,
                                         const QVector<CharacterResult> &result, const KoSvgText::TextRendering rendering,
                                         QPainterPath &chunk,
                                         int &currentIndex)
{

    KoShapeStrokeModelSP stroke = rootShape->stroke();
    QSharedPointer<KoShapeBackground> background = rootShape->background();
    QVector<KoShape::PaintOrder> paintOrder = rootShape->paintOrder();

    for (auto it = compositionBegin(textData); it != compositionEnd(textData); it++) {
        inheritPaintProperties(it, stroke, background, paintOrder);

        KoInsets insets;
        if (stroke) {
            stroke->strokeInsets(rootShape, insets);
        }

        if (it.state() == KisForestDetail::Enter) {

            if (childCount(siblingCurrent(it)) == 0) {
                const int j = currentIndex + it->numChars(true);

                const QRect shapeGlobalClipRect = painter.transform().mapRect(it->associatedOutline.boundingRect().adjusted(-insets.left, -insets.top, insets.right, insets.bottom)).toAlignedRect();

                if (shapeGlobalClipRect.isValid()) {
                    KoClipMaskPainter fillPainter(&painter, shapeGlobalClipRect);
                    if (background) {
                        background->paint(*fillPainter.shapePainter(), rootOutline);
                        fillPainter.maskPainter()->fillPath(rootOutline, Qt::black);
                        setRenderHints(*fillPainter.maskPainter(), rendering, painter.testRenderHint(QPainter::Antialiasing));
                    }
                    QPainterPath textDecorationsRest;
                    textDecorationsRest.setFillRule(Qt::WindingFill);

                    for (int i = currentIndex; i < j; i++) {
                        if (result.at(i).addressable && !result.at(i).hidden) {
                            const QTransform tf = result.at(i).finalTransform();

                            /**
                     * Make sure the character touches the painter's clip rect,
                     * otherwise we can just skip it. Adding insets to ensure the outline will not be skipped.
                     */
                            const QRectF boundingRect = tf.mapRect(result.at(i).inkBoundingBox).adjusted(-insets.left, -insets.top, insets.right, insets.bottom);
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
                                for (int b = 0; b < bitmapGlyph->images.size(); b++) {
                                    QImage img = bitmapGlyph->images.at(b);
                                    QRectF rect = bitmapGlyph->drawRects.value(b, QRectF(0, 0, img.width(), img.height()));
                                    if (img.format() == QImage::Format_Grayscale8 || img.format() == QImage::Format_Mono) {
                                        fillPainter.maskPainter()->save();
                                        fillPainter.maskPainter()->translate(result.at(i).finalPosition.x(), result.at(i).finalPosition.y());
                                        fillPainter.maskPainter()->rotate(qRadiansToDegrees(result.at(i).rotate));
                                        fillPainter.maskPainter()->setCompositionMode(QPainter::CompositionMode_Plus);
                                        fillPainter.maskPainter()->drawImage(rect, img);
                                        fillPainter.maskPainter()->restore();
                                    } else {
                                        painter.save();
                                        painter.translate(result.at(i).finalPosition.x(), result.at(i).finalPosition.y());
                                        painter.rotate(qRadiansToDegrees(result.at(i).rotate));
                                        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
                                        painter.drawImage(rect, img);
                                        painter.restore();
                                    }
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
                                        QPainterPath strokeOutline;
                                        strokeOutline.addRect(rootOutline.boundingRect().adjusted(-insets.left, -insets.top, insets.right, insets.bottom));
                                        strokePainter.shapePainter()->fillRect(strokeOutline.boundingRect(), strokeSP->lineBrush());
                                        strokePainter.maskPainter()->fillRect(strokeOutline.boundingRect(), Qt::black);
                                        maskStroke = KoShapeStrokeSP(new KoShapeStroke(*strokeSP.data()));
                                        maskStroke->setColor(Qt::white);
                                        maskStroke->setLineBrush(Qt::white);
                                        setRenderHints(*strokePainter.maskPainter(), rendering, painter.testRenderHint(QPainter::Antialiasing));
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
    }
}

QGradient *cloneAndTransformGradient(const QGradient *grad, const QTransform &tf) {
    QGradient *newGrad = KoFlake::cloneGradient(grad);

    if (newGrad->type() == QGradient::LinearGradient) {
        QLinearGradient *lgradient = static_cast<QLinearGradient*>(newGrad);
        lgradient->setStart(tf.map(lgradient->start()));
        lgradient->setFinalStop(tf.map(lgradient->finalStop()));
    } else if (newGrad->type() == QGradient::RadialGradient) {
        QRadialGradient *rgradient = static_cast<QRadialGradient*>(newGrad);
        rgradient->setFocalPoint(tf.map(rgradient->focalPoint()));
        rgradient->setCenter(tf.map(rgradient->center()));
    }
    return newGrad;
}

QSharedPointer<KoShapeBackground> transformBackgroundToBounds(QSharedPointer<KoShapeBackground> bg, const QRectF &oldBounds, const QRectF &newBounds) {
    KoGradientBackground *g = dynamic_cast<KoGradientBackground *>(bg.data());

    if (g) {
        QRectF relative = KisAlgebra2D::absoluteToRelative(oldBounds, newBounds);
        QTransform newTf = QTransform::fromTranslate(relative.x(), relative.y());
        newTf.scale(relative.width(), relative.height());

        return QSharedPointer<KoGradientBackground>(new KoGradientBackground(cloneAndTransformGradient(g->gradient(), newTf), g->transform()));
    }

    // assume bg is KoColorBackground.
    return bg;
}

KoShapeStrokeModelSP transformStrokeBgToNewBounds(KoShapeStrokeModelSP stroke, const QRectF &oldBounds, const QRectF &newBounds, bool calcInsets = true) {
    KoShapeStrokeSP s = qSharedPointerDynamicCast<KoShapeStroke>(stroke);
    if (s) {
        QBrush b = s->lineBrush();
        if (b.gradient()) {
            QRectF nb = newBounds;
            if (calcInsets) {
                KoInsets insets;
                s->strokeInsets(nullptr, insets);
                nb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
            }

            QRectF relative = KisAlgebra2D::absoluteToRelative(oldBounds, nb);
            KoShapeStrokeSP newStroke(new KoShapeStroke(*s.data()));
            QTransform newTf = QTransform::fromTranslate(relative.x(), relative.y());
            newTf.scale(relative.width(), relative.height());
            QBrush newBrush = *cloneAndTransformGradient(b.gradient(), newTf);
            newStroke->setLineBrush(newBrush);
            return newStroke;
        }
    }
    return stroke;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
KoShape *
KoSvgTextShape::Private::collectPaths(const KoShape *rootShape, QVector<CharacterResult> &result, int &currentIndex)
{

    QList<KoShape *> shapes;

    KoShapeFactoryBase *imageFactory = KoShapeRegistry::instance()->value("ImageShape");
    const QString imageProp = "image";
    const QString imageViewTransformProp = "viewboxTransform";
    KoShapeFactoryBase *rectangleFactory = KoShapeRegistry::instance()->value("RectangleShape");

    KoShapeStrokeModelSP stroke = rootShape->stroke();
    QSharedPointer<KoShapeBackground> background = rootShape->background();
    QVector<KoShape::PaintOrder> paintOrder = rootShape->paintOrder();

    bool currentNodeInheritsBg = false;
    bool currentNodeInheritsStroke = false;

    for (auto it = compositionBegin(textData); it != compositionEnd(textData); it++) {
        bool hasPaintOrder = it->properties.hasProperty(KoSvgTextProperties::PaintOrder);
        inheritPaintProperties(it, stroke, background, paintOrder);
        QMap<KoSvgText::TextDecoration, QPainterPath> textDecorations = it->textDecorations;
        QColor textDecorationColor = it->properties.propertyOrDefault(KoSvgTextProperties::TextDecorationColorId).value<QColor>();
        QSharedPointer<KoShapeBackground> decorationColor = background;
        if (textDecorationColor.isValid() && it->properties.hasProperty(KoSvgTextProperties::TextDecorationColorId)) {
            decorationColor = QSharedPointer<KoColorBackground>(new KoColorBackground(textDecorationColor));
        }
        KoInsets insets;
        if (stroke) {
            stroke->strokeInsets(rootShape, insets);
        }

        KoShape::PaintOrder first = paintOrder.at(0);
        KoShape::PaintOrder second = paintOrder.at(1);

        if (it != compositionBegin(textData)) {
            currentNodeInheritsBg = currentNodeInheritsBg? !it->properties.hasProperty(KoSvgTextProperties::FillId): false;
            currentNodeInheritsStroke = currentNodeInheritsStroke? !it->properties.hasProperty(KoSvgTextProperties::StrokeId): false;
        }

        if (it.state() == KisForestDetail::Enter) {


            if (textDecorations.contains(KoSvgText::DecorationUnderline)) {
                KoPathShape *shape = KoPathShape::createShapeFromPainterPath(textDecorations.value(KoSvgText::DecorationUnderline));
                shape->setBackground(transformBackgroundToBounds(decorationColor,
                                                                 rootShape->outlineRect(),
                                                                 shape->outlineRect()));
                shape->setStroke(stroke);
                shape->setZIndex(shapes.size());
                shape->setFillRule(Qt::WindingFill);
                if (hasPaintOrder)
                    shape->setPaintOrder(first, second);
                shapes.append(shape);
                if (currentNodeInheritsBg && !textDecorationColor.isValid()) {
                    shape->setInheritBackground(true);
                }
                shape->setInheritStroke(currentNodeInheritsStroke);
            }
            if (textDecorations.contains(KoSvgText::DecorationOverline)) {
                KoPathShape *shape = KoPathShape::createShapeFromPainterPath(textDecorations.value(KoSvgText::DecorationOverline));
                shape->setBackground(transformBackgroundToBounds(decorationColor,
                                                                 rootShape->outlineRect(),
                                                                 shape->outlineRect()));
                shape->setStroke(transformStrokeBgToNewBounds(stroke,
                                                              rootShape->outlineRect(),
                                                              shape->outlineRect()));
                shape->setZIndex(shapes.size());
                shape->setFillRule(Qt::WindingFill);
                if (hasPaintOrder)
                    shape->setPaintOrder(first, second);
                shapes.append(shape);
                if (currentNodeInheritsBg && !textDecorationColor.isValid()) {
                    shape->setInheritBackground(true);
                }
                shape->setInheritStroke(currentNodeInheritsStroke);
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
                                if (replace && currentNodeInheritsBg) {
                                    shape->setInheritBackground(true);
                                }
                                shape->setInheritStroke(currentNodeInheritsStroke);
                            }
                        } else if (const auto *outlineGlyph = std::get_if<Glyph::Outline>(&result.at(i).glyph)) {
                            chunk.addPath(tf.map(outlineGlyph->path));
                        } else if (const auto *bitmapGlyph = std::get_if<Glyph::Bitmap>(&result.at(i).glyph)) {

                            for (int b = 0; b < bitmapGlyph->images.size(); b++) {
                                QImage img = bitmapGlyph->images.at(b);
                                QRectF drawRect = bitmapGlyph->drawRects.at(b);
                                KoProperties params;
                                QTransform imageTf = QTransform::fromTranslate(drawRect.x(), drawRect.y());
                                QTransform viewBox = QTransform::fromScale(drawRect.width()/img.width(),
                                                                           drawRect.height()/img.height());
                                params.setProperty(imageProp, img);
                                params.setProperty(imageViewTransformProp, viewBox);
                                KoShape *shape = imageFactory->createShape(&params);
                                if (img.format() == QImage::Format_Grayscale8 || img.format() == QImage::Format_Mono) {
                                    KoShape *rect = rectangleFactory->createDefaultShape();
                                    shape->setSize(drawRect.size());
                                    rect->setSize(drawRect.size());
                                    rect->setStroke(nullptr);
                                    KoClipMask *mask = new KoClipMask();
                                    mask->setShapes({shape});
                                    rect->setClipMask(mask);
                                    rect->setZIndex(shapes.size());
                                    rect->setBackground(transformBackgroundToBounds(background,
                                                                                    rootShape->outlineRect(),
                                                                                    tf.mapRect(drawRect)));
                                    rect->setTransformation(imageTf*tf);

                                    shapes.append(rect);
                                    rect->setInheritBackground(currentNodeInheritsBg);
                                    rect->setInheritStroke(currentNodeInheritsStroke);
                                } else {
                                    shape->setSize(drawRect.size());
                                    shape->setTransformation(imageTf*tf);
                                    shape->setZIndex(shapes.size());
                                    shapes.append(shape);
                                }
                            }
                        }
                    }
                }
                KoPathShape *shape = KoPathShape::createShapeFromPainterPath(chunk);
                shape->setBackground(transformBackgroundToBounds(background,
                                                                 rootShape->outlineRect(),
                                                                 shape->outlineRect()));

                shape->setStroke(transformStrokeBgToNewBounds(stroke,
                                                              rootShape->outlineRect().adjusted(-insets.left, -insets.top, insets.right, insets.bottom),
                                                              shape->outlineRect()));
                shape->setZIndex(shapes.size());
                shape->setFillRule(Qt::WindingFill);
                if (hasPaintOrder)
                    shape->setPaintOrder(first, second);
                shapes.append(shape);
                shape->setInheritBackground(currentNodeInheritsBg);
                shape->setInheritStroke(currentNodeInheritsStroke);
                currentIndex = j;

            }
            if (it == compositionBegin(textData)) {
                // We don't want the root to inherit stroke or fill, but after
                // that it should, so we turn both to inherit at the end of the
                // 'enter' code block for the root.
                currentNodeInheritsBg = true;
                currentNodeInheritsStroke = true;
            }
        }
        if (it.state() ==KisForestDetail::Leave) {
            inheritPaintProperties(it, stroke, background, paintOrder);
            if (textDecorations.contains(KoSvgText::DecorationLineThrough)) {
                KoPathShape *shape = KoPathShape::createShapeFromPainterPath(textDecorations.value(KoSvgText::DecorationLineThrough));
                shape->setBackground(transformBackgroundToBounds(decorationColor,
                                                                 rootShape->outlineRect(),
                                                                 shape->outlineRect()));
                shape->setStroke(transformStrokeBgToNewBounds(stroke,
                                                              rootShape->outlineRect(),
                                                              shape->outlineRect()));
                shape->setZIndex(shapes.size());
                shape->setFillRule(Qt::WindingFill);
                if (hasPaintOrder)
                    shape->setPaintOrder(first, second);
                shapes.append(shape);
                if (currentNodeInheritsBg && !textDecorationColor.isValid()) {
                    shape->setInheritBackground(true);
                }
                shape->setInheritStroke(currentNodeInheritsStroke);
            }
        }
    }

    KoShape *parentShape = new KoPathShape();
    if (shapes.size() == 1) {
        parentShape = shapes.first();
    } else if (shapes.size() > 1) {
        KoShapeGroup *group = new KoShapeGroup();
        KoShapeGroupCommand cmd(group, shapes, false);
        cmd.redo();

        group->setBackground(rootShape->background());
        group->setStroke(rootShape->stroke());
        group->setPaintOrder(rootShape->paintOrder().first(), rootShape->paintOrder().at(1));
        group->setInheritPaintOrder(rootShape->inheritPaintOrder());

        parentShape = group;
    }
    parentShape->setZIndex(rootShape->zIndex());
    parentShape->setTransformation(rootShape->absoluteTransformation());
    parentShape->setName(rootShape->name());

    return parentShape;
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
                        Q_FOREACH(const QRectF drawRect, bitmapGlyph->drawRects) {
                            painter.drawPolygon(tf.map(drawRect));
                        }
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
                    painter.drawPolygon(tf.map(result.at(i).layoutBox()));

                    penColor.setAlpha(96);
                    pen.setColor(penColor);
                    pen.setWidth(1);
                    pen.setStyle(Qt::DotLine);
                    painter.setPen(pen);
                    painter.drawPolygon(tf.map(result.at(i).lineHeightBox()));

                    pen.setStyle(Qt::SolidLine);
                    pen.setWidth(2);

                    penColor.setAlpha(192);
                    pen.setColor(penColor);
                    painter.setPen(pen);
                    painter.drawLine(tf.map(result.at(i).cursorInfo.caret));


                    const QPointF center = tf.mapRect(result.at(i).layoutBox()).center();
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

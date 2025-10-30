/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoSvgTextShapeLayoutFunc.h"

#include "KoPolygonUtils.h"
#include "KoSvgTextProperties.h"

#include <KoClipMaskPainter.h>
#include <KoColorBackground.h>
#include <KoPathShape.h>
#include <KoShapeStroke.h>

#include <kis_global.h>

#include <QPainter>
#include <QtMath>

namespace KoSvgTextShapeLayoutFunc {

QList<QPainterPath>
getShapes(QList<KoShape *> shapesInside, QList<KoShape *> shapesSubtract, const KoSvgTextProperties &properties)
{
    // the boost polygon method requires (and gives best result) on a inter-based polygon,
    // so we need to scale up. The scale selected here is the size freetype coordinates give to a single pixel.
    qreal scale = 64.0;
    QTransform precisionTF = QTransform::fromScale(scale, scale);

    qreal shapePadding = scale * properties.propertyOrDefault(KoSvgTextProperties::ShapePaddingId).toReal();
    qreal shapeMargin = scale * properties.propertyOrDefault(KoSvgTextProperties::ShapeMarginId).toReal();

    QPainterPath subtract;
    Q_FOREACH(const KoShape *shape, shapesSubtract) {
        const KoPathShape *path = dynamic_cast<const KoPathShape*>(shape);
        if (path) {
            QPainterPath p = path->transformation().map(path->outline());
            p.setFillRule(path->fillRule());
            // grow each polygon here with the shape margin size.
            if (shapeMargin > 0) {
                QList<QPolygon> subpathPolygons;
                Q_FOREACH(QPolygonF subPath, p.toSubpathPolygons()) {
                    subpathPolygons.append(precisionTF.map(subPath).toPolygon());
                }
                subpathPolygons = KoPolygonUtils::offsetPolygons(subpathPolygons, shapeMargin);
                p.clear();
                Q_FOREACH (const QPolygon poly, subpathPolygons) {
                    p.addPolygon(poly);
                }
            } else {
                p = precisionTF.map(p);
            }
            subtract.addPath(p);
        }
    }

    QList<QPainterPath> shapes;
    Q_FOREACH(const KoShape *shape, shapesInside) {
        const KoPathShape *path = dynamic_cast<const KoPathShape*>(shape);
        if (path) {
            QPainterPath p = path->transformation().map(path->outline());
            p.setFillRule(path->fillRule());
            QPainterPath p2;
            p2.setFillRule(path->fillRule());

            QList<QPolygon> subpathPolygons;
            Q_FOREACH(QPolygonF subPath, p.toSubpathPolygons()) {
                subpathPolygons.append(precisionTF.map(subPath).toPolygon());
            }
            subpathPolygons = KoPolygonUtils::offsetPolygons(subpathPolygons, -shapePadding);

            for (int i=0; i < subpathPolygons.size(); i++) {
                QPolygonF subpathPoly = subpathPolygons.at(i);
                Q_FOREACH(QPolygonF subtractPoly, subtract.toSubpathPolygons()) {
                    if (subpathPoly.intersects(subtractPoly)) {
                        subpathPoly = subpathPoly.subtracted(subtractPoly);
                    }
                }
                p2.addPolygon(subpathPoly);
            }
            shapes.append(precisionTF.inverted().map(p2));
        }
    }
    return shapes;
}

static bool getFirstPosition(QPointF &firstPoint,
                             QPainterPath p,
                             QRectF wordBox,
                             QPointF terminator,
                             KoSvgText::WritingMode writingMode,
                             bool ltr)
{
    if (wordBox.isEmpty()) {
        // line-height: 0 will give us empty rects, make them slightly tall/wide
        // to avoid issues with complex QRectF operations.
        if (writingMode == KoSvgText::HorizontalTB) {
            wordBox.setHeight(1e-3);
        } else {
            wordBox.setWidth(1e-3);
        }
    }

    /// Precision of fitting the word box into the polygon to account for
    /// floating-point precision error.

    QVector<QPointF> candidatePositions;
    QRectF word = wordBox.normalized();
    word.translate(-wordBox.topLeft());
    // Slightly shrink the box to account for precision error.
    word.adjust(SHAPE_PRECISION, SHAPE_PRECISION, -SHAPE_PRECISION, -SHAPE_PRECISION);

    QPointF terminatorAdjusted = terminator;
    Q_FOREACH(const QPolygonF polygon, p.toFillPolygons()) {
        QVector<QLineF> offsetPoly;
        for(int i = 0; i < polygon.size()-1; i++) {
            QLineF line;
            line.setP1(polygon.at(i));
            line.setP2(polygon.at(i+1));

            if (line.angle() == 0.0 || line.angle() == 180.0) {
                qreal offset = word.center().y();
                offsetPoly.append(line.translated(0, offset));
                offsetPoly.append(line.translated(0, -offset));
            } else if (line.angle() == 90.0 || line.angle() == 270.0) {
                qreal offset = word.center().x();
                offsetPoly.append(line.translated(offset, 0));
                offsetPoly.append(line.translated(-offset, 0));
            } else {
                qreal tAngle = fmod(line.angle(), 180.0);
                QPointF cPos = tAngle > 90? line.center() + QPointF(-word.center().x(), word.center().y()): line.center() + word.center();
                qreal offset = kisDistanceToLine(cPos, line);
                const QPointF vectorT(qCos(qDegreesToRadians(tAngle)), -qSin(qDegreesToRadians(tAngle)));
                QPointF vectorN(-vectorT.y(), vectorT.x());
                QPointF offsetP = QPointF() - (0.0 * vectorT) + (offset * vectorN);
                offsetPoly.append(line.translated(offsetP));
                offsetPoly.append(line.translated(-offsetP));
            }
        }
        if (writingMode == KoSvgText::HorizontalTB) {
            terminatorAdjusted = terminator + word.center();
            QLineF top(polygon.boundingRect().topLeft(), polygon.boundingRect().topRight());
            offsetPoly.append(top.translated(0, terminatorAdjusted.y()));
        } else if (writingMode == KoSvgText::VerticalRL) {
            terminatorAdjusted = terminator - word.center();
            QLineF top(terminatorAdjusted.x(), polygon.boundingRect().top(),
                       terminatorAdjusted.x(), polygon.boundingRect().bottom());
            offsetPoly.append(top);
        } else{
            terminatorAdjusted = terminator + word.center();
            QLineF top(terminatorAdjusted.x(), polygon.boundingRect().top(),
                       terminatorAdjusted.x(), polygon.boundingRect().bottom());
            offsetPoly.append(top);
        }
        for (int i=0; i < offsetPoly.size(); i++) {
            QLineF line = offsetPoly.at(i);
            for (int j=i; j< offsetPoly.size(); j++){
                QLineF line2 = offsetPoly.at(j);
                QPointF intersectPoint;
                QLineF::IntersectType intersect = line.intersects(line2, &intersectPoint);
                if (intersect != QLineF::NoIntersection) {
                    // should proly handle 'reflex' vertices better.
                    if (!p.contains(intersectPoint)) {
                        continue;
                    }
                    if(!p.contains(word.translated(intersectPoint-word.center()))) {
                        continue;
                    }
                    if (!candidatePositions.contains(intersectPoint)) {
                        candidatePositions.append(intersectPoint);
                    }
                }
            }
        }
    }
    if (candidatePositions.isEmpty()) {
        return false;
    }

    QPointF firstPointC = writingMode == KoSvgText::VerticalRL? p.boundingRect().bottomLeft(): p.boundingRect().bottomRight();
    Q_FOREACH(const QPointF candidate, candidatePositions) {
        if (writingMode == KoSvgText::HorizontalTB) {
            if (terminatorAdjusted.y() - candidate.y() < SHAPE_PRECISION) {

                if (firstPointC.y() - candidate.y() > SHAPE_PRECISION) {
                    firstPointC = candidate;
                } else if (firstPointC.y() - candidate.y() > -SHAPE_PRECISION) {
                    if (ltr) {
                        if (candidate.x() < firstPointC.x()) {
                            firstPointC = candidate;
                        }
                    } else {
                        if (candidate.x() > firstPointC.x()) {
                            firstPointC = candidate;
                        }
                    }
                }
            }
        } else if (writingMode == KoSvgText::VerticalRL) {
            if (terminatorAdjusted.x() - candidate.x() >= -SHAPE_PRECISION) {

                if (firstPointC.x() - candidate.x() < -SHAPE_PRECISION) {
                    firstPointC = candidate;
                } else if (firstPointC.x() - candidate.x() < SHAPE_PRECISION) {
                    if (ltr) {
                        if (candidate.y() < firstPointC.y()) {
                            firstPointC = candidate;
                        }
                    } else {
                        if (candidate.y() > firstPointC.y()) {
                            firstPointC = candidate;
                        }
                    }
                }
            }
        } else {
            if (terminatorAdjusted.x() - candidate.x() < SHAPE_PRECISION) {

                if (firstPointC.x() - candidate.x() > SHAPE_PRECISION) {
                    firstPointC = candidate;
                } else if (firstPointC.x() - candidate.x() > -SHAPE_PRECISION) {
                    if (ltr) {
                        if (candidate.y() < firstPointC.y()) {
                            firstPointC = candidate;
                        }
                    } else {
                        if (candidate.y() > firstPointC.y()) {
                            firstPointC = candidate;
                        }
                    }
                }
            }
        }
    }
    if (!p.contains(firstPointC)) {
        return false;
    }
    firstPointC -= word.center();
    firstPointC -= wordBox.topLeft();
    firstPoint = firstPointC;

    return true;
}

static bool pointLessThan(const QPointF &a, const QPointF &b)
{
    return a.x() < b.x();
}

static bool pointLessThanVertical(const QPointF &a, const QPointF &b)
{
    return a.y() < b.y();
}

static QVector<QLineF>
findLineBoxesForFirstPos(QPainterPath shape, QPointF firstPos, const QRectF wordBox, KoSvgText::WritingMode writingMode)
{
    QVector<QLineF> lines;

    QLineF baseLine;
    QPointF lineTop;
    QPointF lineBottom;
    QRectF word = wordBox.normalized();
    word.adjust(SHAPE_PRECISION, SHAPE_PRECISION, -SHAPE_PRECISION, -SHAPE_PRECISION);

    if (writingMode == KoSvgText::HorizontalTB) {
        baseLine = QLineF(shape.boundingRect().left()-5, firstPos.y(), shape.boundingRect().right()+5, firstPos.y());
        lineTop = QPointF(0, word.top());
        lineBottom = QPointF(0, word.bottom());
    } else {
        baseLine = QLineF(firstPos.x(), shape.boundingRect().top()-5, firstPos.x(), shape.boundingRect().bottom()+5);
        if (writingMode == KoSvgText::VerticalRL) {
            lineTop = QPointF(word.left(), 0);
            lineBottom = QPointF(word.right(), 0);
        } else {
            lineTop = QPointF(word.right(), 0);
            lineBottom = QPointF(word.left(), 0);
        }
    }

    QPolygonF polygon = shape.toFillPolygon();
    QList<QPointF> intersects;
    QLineF topLine = baseLine.translated(lineTop);
    QLineF bottomLine = baseLine.translated(lineBottom);
    for(int i = 0; i < polygon.size()-1; i++) {
        QLineF line(polygon.at(i), polygon.at(i+1));
        bool addedA = false;
        QPointF intersectA;
        QPointF intersectB;
        QPointF intersect;
        if (topLine.intersects(line, &intersect) == QLineF::BoundedIntersection) {
            intersectA = intersect-lineTop;
            intersects.append(intersectA);
            addedA = true;
        }
        if (bottomLine.intersects(line, &intersect) == QLineF::BoundedIntersection) {
            intersectB = intersect-lineBottom;
            if (intersectA != intersectB || !addedA) {
                intersects.append(intersectB);
            }
        }
    }
    if (!intersects.isEmpty()) {
        intersects.append(baseLine.p1());
        intersects.append(baseLine.p2());
    }
    if (writingMode == KoSvgText::HorizontalTB) {
        std::sort(intersects.begin(), intersects.end(), pointLessThan);
    } else {
        std::sort(intersects.begin(), intersects.end(), pointLessThanVertical);
    }


    for (int i = 0; i< intersects.size()-1; i++) {
        QLineF line(intersects.at(i), intersects.at(i+1));

        if (!(shape.contains(line.translated(lineTop).center())
              && shape.contains(line.translated(lineBottom).center()))
                || line.length() == 0) {
            continue;
        }

        QRectF lineBox = QRectF(line.p1() + lineTop, line.p2() + lineBottom).normalized();

        QVector<QPointF> relevant;
        for(int i = 0; i < polygon.size()-1; i++) {

            QLineF edgeLine(polygon.at(i), polygon.at(i+1));

            if (lineBox.contains(polygon.at(i))) {
                relevant.append(polygon.at(i));
            }
        }
        qreal start = writingMode == KoSvgText::HorizontalTB? lineBox.left(): lineBox.top();
        qreal end = writingMode == KoSvgText::HorizontalTB? lineBox.right(): lineBox.bottom();
        for(int j = 0; j < relevant.size(); j++) {
            QPointF current = relevant.at(j);

            if (writingMode == KoSvgText::HorizontalTB) {
                if (current.x() < line.center().x()) {
                    start = qMax(current.x(), start);
                } else if (current.x() > line.center().x()) {
                    end = qMin(current.x(), end);
                }
            } else {
                if (current.y() < line.center().y()) {
                    start = qMax(current.y(), start);
                } else if (current.y() > line.center().y()) {
                    end = qMin(current.y(), end);
                }
            }
        }
        if (writingMode == KoSvgText::HorizontalTB) {

            QLineF newLine(start, line.p1().y(), end, line.p2().y());
            if (!lines.isEmpty()) {
                if (lines.last().p2() == intersects.at(i)) {
                    newLine.setP1(lines.last().p1());
                    lines.removeLast();
                }
            }
            lines.append(newLine);
        } else {
            QLineF newLine(line.p1().x(), start, line.p2().x(), end);
            if (!lines.isEmpty()) {
                if (lines.last().p2() == intersects.at(i)) {
                    newLine.setP1(lines.last().p1());
                    lines.removeLast();
                }
            }
            lines.append(newLine);
        }
    }

    return lines;
}

/**
 * @brief getEstimatedHeight
 * Adjust the wordbox with the estimated height.
 */

static void getEstimatedHeight(QVector<CharacterResult> &result,
                               const int index,
                               QRectF &wordBox,
                               const QRectF boundingBox,
                               KoSvgText::WritingMode writingMode)
{
    bool isHorizontal = writingMode == KoSvgText::HorizontalTB;
    QPointF totalAdvance = wordBox.bottomRight() - wordBox.topLeft();
    qreal maxAscent = isHorizontal? wordBox.top(): wordBox.right();
    qreal maxDescent = isHorizontal? wordBox.bottom(): wordBox.left();

    for (int i=index; i<result.size(); i++) {
        if (!result.at(i).addressable || result.at(i).hidden) {
            continue;
        }
        totalAdvance += result.at(i).advance;
        if ((totalAdvance.x() > boundingBox.width() && isHorizontal) ||
                (totalAdvance.y() > boundingBox.height() && !isHorizontal)) {
            break;
        }
        calculateLineHeight(result.at(i), maxAscent, maxDescent, isHorizontal, true);
    }
    if (writingMode == KoSvgText::HorizontalTB) {
        wordBox.setTop(maxAscent);
        wordBox.setBottom(maxDescent);
    } else {
        // vertical lr has top at the right even though block flow is also to the right.
        wordBox.setRight(maxAscent);
        wordBox.setLeft(maxDescent);
    }
}

static KoSvgText::TextAnchor
textAnchorForTextAlign(KoSvgText::TextAlign align, KoSvgText::TextAlign alignLast, bool ltr)
{
    KoSvgText::TextAlign compare = align;
    if (align == KoSvgText::AlignJustify) {
        compare = alignLast;
    }
    if (compare == KoSvgText::AlignStart) {
        return KoSvgText::AnchorStart;
    } else if (compare == KoSvgText::AlignCenter) {
        return KoSvgText::AnchorMiddle;
    } else if (compare == KoSvgText::AlignEnd) {
        return KoSvgText::AnchorEnd;
    } else if (compare == KoSvgText::AlignLeft) {
        return ltr? KoSvgText::AnchorStart: KoSvgText::AnchorEnd;
    } else if (compare == KoSvgText::AlignRight) {
        return ltr? KoSvgText::AnchorEnd: KoSvgText::AnchorStart;
    } else if (align == KoSvgText::AlignJustify) {
        return KoSvgText::AnchorMiddle;
    }
    return KoSvgText::AnchorStart;
}

QVector<LineBox> flowTextInShapes(const KoSvgTextProperties &properties,
                                  const QMap<int, int> &logicalToVisual,
                                  QVector<CharacterResult> &result,
                                  QList<QPainterPath> shapes,
                                  QPointF &startPos,
                                  const KoSvgText::ResolutionHandler &resHandler)
{
    QVector<LineBox> lineBoxes;
    KoSvgText::WritingMode writingMode = KoSvgText::WritingMode(properties.propertyOrDefault(KoSvgTextProperties::WritingModeId).toInt());
    KoSvgText::Direction direction = KoSvgText::Direction(properties.propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());
    bool ltr = direction == KoSvgText::DirectionLeftToRight;
    bool isHorizontal = writingMode == KoSvgText::HorizontalTB;
    KoSvgText::TextAlign align = KoSvgText::TextAlign(properties.propertyOrDefault(KoSvgTextProperties::TextAlignAllId).toInt());
    KoSvgText::TextAlign alignLast = KoSvgText::TextAlign(properties.propertyOrDefault(KoSvgTextProperties::TextAlignLastId).toInt());
    KoSvgText::TextAnchor anchor = textAnchorForTextAlign(align, alignLast, ltr);

    QPointF textIndent; ///< The textIndent.
    KoSvgText::TextIndentInfo textIndentInfo = properties.propertyOrDefault(KoSvgTextProperties::TextIndentId).value<KoSvgText::TextIndentInfo>();

    QVector<int> wordIndices; ///< 'word' in this case meaning characters
                              ///< inbetween softbreaks.
    QRectF wordBox; ///< Approximated box of the current word;
    QPointF wordAdvance;

    LineBox currentLine;
    bool firstLine = true; ///< First line will be created proper after we get our first wordbox, this tracks if it's the first.
    bool indentLine = true;

    QPointF currentPos = writingMode == KoSvgText::VerticalRL? shapes.at(0).boundingRect().topRight()
                                                             :shapes.at(0).boundingRect().topLeft(); ///< Current position with advances of each character.
    QPointF lineOffset = currentPos; ///< Current line offset.

    QListIterator<int> it(logicalToVisual.keys());
    QListIterator<QPainterPath> shapesIt(shapes);
    if (shapes.isEmpty()) {
        return lineBoxes;
    }
    {
        // Calculate the default pos.
        qreal fontSize = properties.fontSize().value;
        QRectF wordBox = isHorizontal? QRectF(0, fontSize * -0.8, SHAPE_PRECISION, fontSize)
                                     : QRectF(fontSize * -0.5, 0, fontSize, SHAPE_PRECISION);
        getFirstPosition(startPos, shapes.first(), wordBox, currentPos, writingMode, ltr);
    }
    QPainterPath currentShape;
    while (it.hasNext()) {
        int index = it.next();
        result[index].calculateAndApplyTabsize(wordAdvance + currentPos, isHorizontal, resHandler);
        CharacterResult charResult = result.at(index);
        if (!charResult.addressable) {
            continue;
        }

        bool softBreak = false; ///< Whether to break a line.
        bool doNotCountAdvance =
            ((charResult.lineEnd != LineEdgeBehaviour::NoChange)
             && !(currentLine.isEmpty() && wordIndices.isEmpty()));
        if (!doNotCountAdvance) {
            if (wordIndices.isEmpty()) {
                wordBox = charResult.lineHeightBox().translated(charResult.totalBaselineOffset());
                wordAdvance = charResult.advance;
            } else {
                wordBox |= charResult.lineHeightBox().translated(wordAdvance+charResult.totalBaselineOffset());
                wordAdvance += charResult.advance;
            }
        }
        wordIndices.append(index);
        currentLine.lastLine = !it.hasNext();
        if (currentLine.lastLine) {
            currentLine.justifyLine = alignLast == KoSvgText::AlignJustify;
        }

        if (charResult.breakType != BreakType::NoBreak || currentLine.lastLine) {
            if (currentLine.chunks.isEmpty() || currentLine.lastLine) {
                softBreak = true;
            }

            for (int i = currentLine.currentChunk; i < currentLine.chunks.size(); i++) {
                if (i == -1) {
                    currentLine.currentChunk = 0;
                    i = 0;
                }
                QLineF line = currentLine.chunks.value(i).length;
                qreal lineLength = isHorizontal ? (currentPos - line.p1() + wordAdvance).x()
                                                : (currentPos - line.p1() + wordAdvance).y();
                if (qRound((abs(lineLength) - line.length())) > 0) {
                    if (i == currentLine.chunks.size()-1) {
                        softBreak = true;
                        break;
                    } else {
                        QLineF nextLine = currentLine.chunks.value(i+1).length;
                        if (isHorizontal) {
                            currentPos.setX(ltr? qMax(nextLine.p1().x(), currentPos.x()):
                                                 qMin(nextLine.p1().x(), currentPos.x()));
                        } else {
                            currentPos.setY(nextLine.p1().y());
                        }
                    }
                } else {
                    currentLine.currentChunk = i;
                    addWordToLine(result, currentPos, wordIndices, currentLine, ltr, isHorizontal);
                    break;
                }
            }
        }

        if (softBreak) {
            if (!currentLine.isEmpty()) {
                finalizeLine(result, currentPos, currentLine, lineOffset, anchor, writingMode, ltr, true, true, resHandler);
                lineBoxes.append(currentLine);
                firstLine = false;
                indentLine = false;
            }
            // Not adding indent to the (first) word box means it'll overflow if there's no room,
            // but being too strict might end with the whole text disappearing. Given Krita's text layout is
            // in an interactive context, ugly result might be more communicative than all text disappearing.
            bool ind = textIndentInfo.hanging? !indentLine: indentLine;
            QPointF indent = ind? textIndent: QPointF();
            bool foundFirst = false;
            bool needNewLine = true;
            // add text indent to wordbox.
            if (!currentShape.isEmpty()) {
                // we're going to try and get an offset line first before trying get first pos.
                // This gives more stable results on curved shapes.
                getEstimatedHeight(result, index, wordBox, currentShape.boundingRect(), writingMode);
                currentPos -= writingMode == KoSvgText::VerticalRL? wordBox.topRight(): wordBox.topLeft();

                currentLine = LineBox(findLineBoxesForFirstPos(currentShape, currentPos, wordBox, writingMode), ltr, indent, resHandler);
                qreal length = isHorizontal? wordBox.width(): wordBox.height();
                for (int i = 0; i < currentLine.chunks.size(); i++) {
                    if (currentLine.chunks.at(i).length.length() > length) {
                        currentLine.currentChunk = i;
                        foundFirst = true;
                        needNewLine = false;
                        break;
                    }
                }
            }
            /*
             * In theory we could have overflow wrap for shapes, but it'd require either generalizing
             * the line-filling portion above and this new line seeking portion, or somehow reverting
             * the iterator over the results to be on the last fitted glyph (which'd still require
             * generalizing the line-filling portion about), and I am unsure how to do that.
             * Either way, this place here is where you'd check for overflow wrap.
             */
            while(!foundFirst) {
                foundFirst = getFirstPosition(currentPos, currentShape, wordBox, lineOffset, writingMode, ltr);
                if (foundFirst || !shapesIt.hasNext()) {
                    break;
                }
                currentShape = shapesIt.next();
                getEstimatedHeight(result, index, wordBox, currentShape.boundingRect(), writingMode);
                bool indentPercent = textIndentInfo.length.unit == KoSvgText::CssLengthPercentage::Percentage;
                qreal textIdentValue = textIndentInfo.length.value;
                if (isHorizontal) {
                    if (indentPercent) {
                        textIdentValue *= currentShape.boundingRect().width();
                    }
                    textIndent = resHandler.adjust(QPointF(textIdentValue, 0));
                } else {
                    if (indentPercent) {
                        textIdentValue *= currentShape.boundingRect().height();
                    }
                    textIndent = resHandler.adjust(QPointF(0, textIdentValue));
                }
                bool ind = textIndentInfo.hanging? !indentLine: indentLine;
                indent = ind? textIndent: QPointF();
                currentPos = writingMode == KoSvgText::VerticalRL? currentShape.boundingRect().topRight(): currentShape.boundingRect().topLeft();
                lineOffset = currentPos;
            }

            bool lastDitch = false;
            if (!foundFirst && firstLine && !wordIndices.isEmpty() && !currentShape.isEmpty()) {
                // Last-ditch attempt to get any kind of positioning to happen.
                // This typically happens when wrapping has been disabled.
                wordBox = result[wordIndices.first()].lineHeightBox().translated(result[wordIndices.first()].totalBaselineOffset());
                foundFirst = getFirstPosition(currentPos, currentShape, wordBox, lineOffset, writingMode, ltr);
                lastDitch = true;
            }

            if (foundFirst) {

                if (needNewLine) {
                    currentLine = LineBox(findLineBoxesForFirstPos(currentShape, currentPos, wordBox, writingMode), ltr, indent, resHandler);
                    // We could set this to find the first fitting width, but it's better to try and improve the precision of the firstpos algorithm,
                    // as this gives more stable results.
                    currentLine.setCurrentChunkForPos(currentPos, isHorizontal);
                }
                const qreal expectedLineT = isHorizontal? wordBox.top():
                                                          writingMode == KoSvgText::VerticalRL? wordBox.right(): wordBox.left();

                currentLine.firstLine = firstLine;
                currentLine.expectedLineTop = expectedLineT;
                currentLine.justifyLine = align == KoSvgText::AlignJustify;
                currentPos = currentLine.chunk().length.p1() + indent;
                lineOffset = currentPos;

                if (lastDitch) {
                    QVector<int> wordNew;
                    QPointF advance = currentPos;
                    Q_FOREACH(const int i, wordIndices) {
                        advance += result[i].advance;
                        if (currentShape.contains(advance)) {
                            wordNew.append(i);
                        } else {
                            result[i].hidden = true;
                            result[i].cssPosition = advance-result[i].advance;
                        }
                        wordIndices = wordNew;
                    }
                }
                addWordToLine(result, currentPos, wordIndices, currentLine, ltr, isHorizontal);
            } else {
                currentLine = LineBox();
                QPointF advance = currentPos;
                Q_FOREACH (const int j, wordIndices) {
                    result[j].cssPosition = advance;
                    advance += result[j].advance;
                    result[j].hidden = true;
                }
            }
        }

        if (charResult.breakType == BreakType::HardBreak) {
            finalizeLine(result, currentPos, currentLine, lineOffset, anchor, writingMode, ltr, true, true, resHandler);
            lineBoxes.append(currentLine);
            currentLine = LineBox();
            indentLine = textIndentInfo.hanging? false: textIndentInfo.eachLine;
        }
    }
    finalizeLine(result, currentPos, currentLine, lineOffset, anchor, writingMode, ltr, true, true, resHandler);
    lineBoxes.append(currentLine);
    return lineBoxes;
}

} // namespace KoSvgTextShapeLayoutFunc

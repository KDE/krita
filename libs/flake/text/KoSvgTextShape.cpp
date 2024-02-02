/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoSvgTextShape.h"
#include "KoSvgTextShape_p.h"

#include <QTextLayout>

#include <klocalizedstring.h>

#include "KoSvgTextProperties.h"
#include "KoSvgTextChunkShapeLayoutInterface.h"
#include <KoDocumentResourceManager.h>
#include <KoShapeContainer_p.h>
#include <KoShapeController.h>
#include <text/KoCssTextUtils.h>
#include <text/KoFontRegistry.h>
#include <text/KoSvgTextChunkShape_p.h>
#include <text/KoSvgTextShapeMarkupConverter.h>
#include <text/KoPolygonUtils.h>

#include <kis_global.h>

#include <KoClipMaskPainter.h>
#include <KoColorBackground.h>
#include <KoIcon.h>
#include <KoPathShape.h>
#include <KoProperties.h>
#include <KoShapeLoadingContext.h>
#include <KoXmlNS.h>
#include <KoInsets.h>

#include <SvgLoadingContext.h>
#include <SvgGraphicContext.h>
#include <SvgUtil.h>
#include <SvgStyleWriter.h>

#include <QPainter>
#include <QPainterPath>
#include <QtMath>

#include <FlakeDebug.h>


KoSvgTextShape::KoSvgTextShape()
    : KoSvgTextChunkShape()
    , d(new Private)
{
    setShapeId(KoSvgTextShape_SHAPEID);
    d->textData.insert(d->textData.childBegin(), KoSvgTextContentElement());
}

KoSvgTextShape::KoSvgTextShape(const KoSvgTextShape &rhs)
    : KoSvgTextChunkShape(rhs)
    , d(new Private(*rhs.d))
{
    setShapeId(KoSvgTextShape_SHAPEID);
}

KoSvgTextShape::~KoSvgTextShape()
{
}

const QString &KoSvgTextShape::defaultPlaceholderText()
{
    static const QString s_placeholderText = i18nc("Default text for the text shape", "Placeholder Text");
    return s_placeholderText;
}

KoShape *KoSvgTextShape::cloneShape() const
{
    return new KoSvgTextShape(*this);
}

void KoSvgTextShape::shapeChanged(ChangeType type, KoShape *shape)
{
    KoSvgTextChunkShape::shapeChanged(type, shape);

    if (type == StrokeChanged || type == BackgroundChanged || type == ContentChanged) {
        relayout();
    }
}

void KoSvgTextShape::setResolution(qreal xRes, qreal yRes)
{
    int roundedX = qRound(xRes);
    int roundedY = qRound(yRes);
    if (roundedX != d->xRes || roundedY != d->yRes) {
        d->xRes = roundedX;
        d->yRes = roundedY;
        relayout();
    }
}

int KoSvgTextShape::posLeft(int pos, bool visual)
{
    KoSvgText::WritingMode mode = writingMode();
    KoSvgText::Direction direction = KoSvgText::Direction(this->textProperties().propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());
    if (mode == KoSvgText::VerticalRL) {
        return nextLine(pos);
    } else if (mode == KoSvgText::VerticalLR) {
        return previousLine(pos);
    } else {
        if (direction == KoSvgText::DirectionRightToLeft) {
            return nextPos(pos, visual);
        } else {
            return previousPos(pos, visual);
        }
    }
}

int KoSvgTextShape::posRight(int pos, bool visual)
{
    KoSvgText::WritingMode mode = writingMode();
    KoSvgText::Direction direction = KoSvgText::Direction(this->textProperties().propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());

    if (mode == KoSvgText::VerticalRL) {
        return previousLine(pos);
    } else if (mode == KoSvgText::VerticalLR) {
        return nextLine(pos);
    } else {
        if (direction == KoSvgText::DirectionRightToLeft) {
            return previousPos(pos, visual);
        } else {
            return nextPos(pos, visual);
        }
    }
}

int KoSvgTextShape::posUp(int pos, bool visual)
{
    KoSvgText::WritingMode mode = writingMode();
    KoSvgText::Direction direction = KoSvgText::Direction(this->textProperties().propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());
    if (mode == KoSvgText::VerticalRL || mode == KoSvgText::VerticalLR) {
        if (direction == KoSvgText::DirectionRightToLeft) {
            return nextPos(pos, visual);
        } else {
            return previousPos(pos, visual);
        }
    } else {
        return previousLine(pos);
    }
}

int KoSvgTextShape::posDown(int pos, bool visual)
{
    KoSvgText::WritingMode mode = writingMode();
    KoSvgText::Direction direction = KoSvgText::Direction(this->textProperties().propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());
    if (mode == KoSvgText::VerticalRL || mode == KoSvgText::VerticalLR) {
        if (direction == KoSvgText::DirectionRightToLeft) {
            return previousPos(pos, visual);
        } else {
            return nextPos(pos, visual);
        }
    } else {
        return nextLine(pos);
    }
}

int KoSvgTextShape::lineStart(int pos)
{
    if (pos < 0 || d->cursorPos.isEmpty() || d->result.isEmpty()) {
        return pos;
    }
    CursorPos p = d->cursorPos.at(pos);
    if (d->result.at(p.cluster).anchored_chunk && p.offset == 0) {
        return pos;
    }
    int candidate = 0;
    for (int i = 0; i < pos; i++) {
        CursorPos p2 = d->cursorPos.at(i);
        if (d->result.at(p2.cluster).anchored_chunk && p2.offset == 0) {
            candidate = i;
        }
    }
    return candidate;
}

int KoSvgTextShape::lineEnd(int pos)
{
    if (pos < 0 || d->cursorPos.isEmpty() || d->result.isEmpty()) {
        return pos;
    }
    if (pos > d->cursorPos.size() - 1) {
        return d->cursorPos.size() - 1;
    }
    int candidate = 0;
    int posCluster = d->cursorPos.at(pos).cluster;
    for (int i = pos; i < d->cursorPos.size(); i++) {
        CursorPos p = d->cursorPos.at(i);
        if (d->result.at(p.cluster).anchored_chunk && i > pos && posCluster != p.cluster) {
            break;
        }
        candidate = i;
    }
    return candidate;
}

int KoSvgTextShape::wordLeft(int pos, bool visual)
{
    //TODO: figure out preferred behaviour for wordLeft in RTL && visual.
    Q_UNUSED(visual)
    if (pos < 0 || pos > d->cursorPos.size()-1 || d->result.isEmpty() || d->cursorPos.isEmpty()) {
        return pos;
    }
    KoSvgText::Direction direction = KoSvgText::Direction(this->textProperties().propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());
    if (direction == KoSvgText::DirectionRightToLeft) {
        return wordEnd(pos);
    }
    return wordStart(pos);
}

int KoSvgTextShape::wordRight(int pos, bool visual)
{
    Q_UNUSED(visual)
    if (pos < 0 || pos > d->cursorPos.size()-1 || d->result.isEmpty() || d->cursorPos.isEmpty()) {
        return pos;
    }
    KoSvgText::Direction direction = KoSvgText::Direction(this->textProperties().propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());
    if (direction == KoSvgText::DirectionRightToLeft) {
        return wordStart(pos);
    }
    return wordEnd(pos);
}

int KoSvgTextShape::nextIndex(int pos)
{
    if (d->cursorPos.isEmpty()) {
        return pos;
    }
    int currentIndex = d->cursorPos.at(pos).index;

    for (int i = pos; i < d->cursorPos.size(); i++) {
        if (d->cursorPos.at(i).index > currentIndex) {
            return i;
        }
    }
    return pos;
}

int KoSvgTextShape::previousIndex(int pos)
{
    if (d->cursorPos.isEmpty()) {
        return pos;
    }
    int currentIndex = d->cursorPos.at(pos).index;

    for (int i = pos; i >= 0; i--) {
        if (d->cursorPos.at(i).index < currentIndex) {
            return i;
        }
    }
    return pos;
}

int KoSvgTextShape::nextPos(int pos, bool visual)
{
    if (d->cursorPos.isEmpty()) {
        return -1;
    }

    if(visual) {
        int visualIndex = d->logicalToVisualCursorPos.value(pos);
        return d->logicalToVisualCursorPos.key(qMin(visualIndex + 1, d->cursorPos.size() - 1), d->cursorPos.size() - 1);
    }

    return qMin(pos + 1, d->cursorPos.size() - 1);
}

int KoSvgTextShape::previousPos(int pos, bool visual)
{
    if (d->cursorPos.isEmpty()) {
        return -1;
    }

    if(visual) {
        int visualIndex = d->logicalToVisualCursorPos.value(pos);
        return d->logicalToVisualCursorPos.key(qMax(visualIndex - 1, 0), 0);
    }

    return qMax(pos - 1, 0);
}

int KoSvgTextShape::nextLine(int pos)
{
    if (pos < 0 || pos > d->cursorPos.size()-1 || d->result.isEmpty() || d->cursorPos.isEmpty()) {
        return pos;
    }

    int nextLineStart = lineEnd(pos)+1;
    int nextLineEnd = lineEnd(nextLineStart);
    CursorPos cursorPos = d->cursorPos.at(pos);
    if (!this->shapesInside().isEmpty()) {
        LineBox nextLineBox;
        for (int i = 0; i < d->lineBoxes.size(); ++i) {
            for (int j = 0; j < d->lineBoxes.at(i).chunks.size(); ++j) {
                if (d->lineBoxes.at(i).chunks.at(j).chunkIndices.contains(cursorPos.cluster)) {
                    nextLineBox = d->lineBoxes.at(qMin(i + 1, d->lineBoxes.size()-1));
                }
            }
        }
        if (nextLineBox.chunks.size()>0) {
            int first = -1;
            int last = -1;
            Q_FOREACH(LineChunk chunk, nextLineBox.chunks) {
                Q_FOREACH (int i, chunk.chunkIndices) {
                    if (d->result.at(i).addressable) {
                        if (first < 0) {
                            first = d->result.at(i).cursorInfo.graphemeIndices.first();
                        }
                        last = d->result.at(i).cursorInfo.graphemeIndices.last();
                    }
                }
            }
            if (first > -1 && last > -1) {
                nextLineStart = posForIndex(first);
                nextLineEnd = posForIndex(last);
            }
        }
    }


    if (nextLineStart > d->cursorPos.size()-1) {
        return d->cursorPos.size()-1;
    }

    CharacterResult res = d->result.at(cursorPos.cluster);
    QPointF currentPoint = res.finalPosition;
    currentPoint += res.cursorInfo.offsets.value(cursorPos.offset, res.advance);
    int candidate = posForPoint(currentPoint, nextLineStart, nextLineEnd+1);
    if (candidate < 0) {
        return pos;
    }

    return candidate;
}

int KoSvgTextShape::previousLine(int pos)
{
    if (pos < 0 || pos > d->cursorPos.size()-1 || d->result.isEmpty() || d->cursorPos.isEmpty()) {
        return pos;
    }
    int currentLineStart = lineStart(pos);
    if (currentLineStart - 1 < 0) {
        return 0;
    }
    int previousLineStart = lineStart(currentLineStart-1);

    CursorPos cursorPos = d->cursorPos.at(pos);
    if (!this->shapesInside().isEmpty()) {
        LineBox previousLineBox;
        for (int i = 0; i < d->lineBoxes.size(); ++i) {
            for (int j = 0; j < d->lineBoxes.at(i).chunks.size(); ++j) {
                if (d->lineBoxes.at(i).chunks.at(j).chunkIndices.contains(cursorPos.cluster)) {
                    previousLineBox = d->lineBoxes.at(qMax(i - 1, 0));
                }
            }
        }
        if (previousLineBox.chunks.size()>0) {
            int first = -1;
            int last = -1;
            Q_FOREACH(LineChunk chunk, previousLineBox.chunks) {
                Q_FOREACH (int i, chunk.chunkIndices) {
                    if (d->result.at(i).addressable) {
                        if (first < 0) {
                            first = d->result.at(i).cursorInfo.graphemeIndices.first();
                        }
                        last = d->result.at(i).cursorInfo.graphemeIndices.last();
                    }
                }
            }
            if (first > -1 && last > -1) {
                previousLineStart = posForIndex(first);
                currentLineStart = posForIndex(last);
            }
        }
    }

    CharacterResult res = d->result.at(cursorPos.cluster);
    QPointF currentPoint = res.finalPosition;
    currentPoint += res.cursorInfo.offsets.value(cursorPos.offset, res.advance);
    int candidate = posForPoint(currentPoint, previousLineStart, currentLineStart);
    if (candidate < 0) {
        return pos;
    }

    return candidate;
}

int KoSvgTextShape::wordEnd(int pos)
{
    if (pos < 0 || pos > d->cursorPos.size()-1 || d->result.isEmpty() || d->cursorPos.isEmpty()) {
        return pos;
    }
    int currentLineEnd = lineEnd(pos);
    if (pos == lineStart(pos) || pos == currentLineEnd) {
        return pos;
    }

    int wordEnd = pos;
    for (int i = pos; i<= currentLineEnd; i++) {
        wordEnd = i;
        CursorPos cursorPos = d->cursorPos.at(i);
        if (d->result.at(cursorPos.cluster).cursorInfo.isWordBoundary && cursorPos.offset == 0) {
            break;
        }

    }

    return wordEnd;
}

int KoSvgTextShape::wordStart(int pos)
{
    if (pos < 0 || pos > d->cursorPos.size()-1 || d->result.isEmpty() || d->cursorPos.isEmpty()) {
        return pos;
    }
    int currentLineStart = lineStart(pos);
    if (pos == currentLineStart || pos == lineEnd(pos)) {
        return pos;
    }

    int wordStart = pos;
    bool breakNext = false;
    for (int i = pos; i >= currentLineStart; i--) {
        if (breakNext) break;
        CursorPos cursorPos = d->cursorPos.at(i);
        if (d->result.at(cursorPos.cluster).cursorInfo.isWordBoundary && cursorPos.offset == 0) {
            breakNext = true;
        }
        wordStart = i;
    }

    return wordStart;
}

QPainterPath KoSvgTextShape::defaultCursorShape()
{
    KoSvgText::WritingMode mode = writingMode();
    double fontSize = this->textProperties().propertyOrDefault(KoSvgTextProperties::FontSizeId).toReal();
    QPainterPath p;
    if (mode == KoSvgText::HorizontalTB) {
        p.moveTo(0, fontSize*0.2);
        p.lineTo(0, -fontSize);
    } else {
        p.moveTo(-fontSize * 0.5, 0);
        p.lineTo(fontSize, 0);
    }
    p.translate(d->initialTextPosition);

    return p;
}

QPainterPath KoSvgTextShape::cursorForPos(int pos, QLineF &caret, QColor &color, double bidiFlagSize)
{
    if (d->result.isEmpty() || d->cursorPos.isEmpty() || pos < 0 || pos >= d->cursorPos.size()) {
        return defaultCursorShape();
    }
    QPainterPath p;

    CursorPos cursorPos = d->cursorPos.at(pos);

    CharacterResult res = d->result.at(cursorPos.cluster);

    const QTransform tf = res.finalTransform();
    color = res.cursorInfo.color;
    caret = res.cursorInfo.caret;
    caret.translate(res.cursorInfo.offsets.value(cursorPos.offset, QPointF()));

    p.moveTo(tf.map(caret.p1()));
    p.lineTo(tf.map(caret.p2()));
    if (d->isBidi && bidiFlagSize > 0) {
        int sign = res.cursorInfo.rtl ? -1 : 1;
        double bidiFlagHalf = bidiFlagSize * 0.5;
        QPointF point3;
        QPointF point4;
        if (writingMode() == KoSvgText::HorizontalTB) {
            qreal slope = bidiFlagHalf * (caret.dx()/ caret.dy());
            point3 = QPointF(caret.p2().x() + slope + (sign * bidiFlagHalf), caret.p2().y() + bidiFlagHalf);
            point4 = QPointF(point3.x() + slope - (sign * bidiFlagHalf),point3.y() + bidiFlagHalf);
        } else {
            qreal slope = bidiFlagHalf * (caret.dy()/ caret.dx());
            point3 = QPointF(caret.p2().x() - bidiFlagHalf, caret.p2().y() - slope + (sign * bidiFlagHalf));
            point4 = QPointF(point3.x() - bidiFlagHalf, point3.y() - slope - (sign * bidiFlagHalf));
        }
        p.lineTo(tf.map(point3));
        p.lineTo(tf.map(point4));
    }
    caret = tf.map(caret);

    return p;
}

QPainterPath KoSvgTextShape::selectionBoxes(int pos, int anchor)
{
    int start = qMin(pos, anchor);
    int end = qMax(pos, anchor);

    if (start == end || start < 0 || end >= d->cursorPos.size()) {
        return QPainterPath();
    }

    QPainterPath p;
    p.setFillRule(Qt::WindingFill);
    for (int i = start+1; i <= end; i++) {
        CursorPos cursorPos = d->cursorPos.at(i);
        CharacterResult res = d->result.at(cursorPos.cluster);
        const QTransform tf = res.finalTransform();
        QLineF first = res.cursorInfo.caret;
        QLineF last = first;
        if (res.cursorInfo.rtl) {
            last.translate(res.cursorInfo.offsets.value(cursorPos.offset-1,  res.advance));
            first.translate(res.cursorInfo.offsets.value(cursorPos.offset, QPointF()));
        } else {
            first.translate(res.cursorInfo.offsets.value(cursorPos.offset-1,  QPointF()));
            last.translate(res.cursorInfo.offsets.value(cursorPos.offset, res.advance));
        }
        QPolygonF poly;
        poly << first.p1() << first.p2() << last.p2() << last.p1() << first.p1();
        p.addPolygon(tf.map(poly));
    }

    return p;
}

QPainterPath KoSvgTextShape::underlines(int pos, int anchor, KoSvgText::TextDecorations decor, KoSvgText::TextDecorationStyle style, qreal minimum, bool thick)
{
    int start = qMin(pos, anchor);
    int end = qMax(pos, anchor);

    if (start == end || start < 0 || end >= d->cursorPos.size()) {
        return QPainterPath();
    }

    QPainterPathStroker stroker;
    qreal width = qMax(minimum, this->layoutInterface()->getTextDecorationWidth(KoSvgText::DecorationUnderline));
    if (thick) {
        width *= 2;
    }
    stroker.setWidth(width);
    KoSvgText::WritingMode mode = KoSvgText::WritingMode(this->textProperties().propertyOrDefault(KoSvgTextProperties::WritingModeId).toInt());
    stroker.setCapStyle(Qt::FlatCap);
    if (style == KoSvgText::Solid) {
        stroker.setDashPattern(Qt::SolidLine);
    } else if (style == KoSvgText::Dashed) {
        stroker.setDashPattern(Qt::DashLine);
    } else if (style == KoSvgText::Dotted) {
        stroker.setDashPattern(Qt::DotLine);
    } else {
        stroker.setDashPattern(Qt::SolidLine);
    }

    QPainterPath underPath;
    QPainterPath overPath;
    QPainterPath middlePath;
    QPointF inset = mode == KoSvgText::HorizontalTB? QPointF(width*0.5, 0): QPointF(0, width*0.5);
    for (int i = start+1; i <= end; i++) {
        CursorPos pos = d->cursorPos.at(i);
        CharacterResult res = d->result.at(pos.cluster);
        const QTransform tf = res.finalTransform();
        QPointF first = res.cursorInfo.caret.p1();
        QPointF last = first;
        if (res.cursorInfo.rtl) {
            last  += res.cursorInfo.offsets.value(pos.offset-1,  res.advance);
            first += res.cursorInfo.offsets.value(pos.offset, QPointF());
            if (i == start+1) {
                first -= inset;
            }
            if (i == end) {
                last += inset;
            }
        } else {
            first += res.cursorInfo.offsets.value(pos.offset-1,  QPointF());
            last  += res.cursorInfo.offsets.value(pos.offset, res.advance);
            if (i == start+1) {
                first += inset;
            }
            if (i == end) {
                last -= inset;
            }
        }

        if (decor.testFlag(KoSvgText::DecorationUnderline)){
            underPath.moveTo(tf.map(first));
            underPath.lineTo(tf.map(last));
        }
        QPointF diff = res.cursorInfo.caret.p2() - res.cursorInfo.caret.p1();
        if (decor.testFlag(KoSvgText::DecorationOverline)){
            overPath.moveTo(tf.map(first+diff));
            overPath.lineTo(tf.map(last+diff));
        }
        if (decor.testFlag(KoSvgText::DecorationLineThrough)){
            middlePath.moveTo(tf.map(first+(diff*0.5)));
            middlePath.lineTo(tf.map(last+(diff*0.5)));
        }
    }

    QPainterPath final;
    if (decor.testFlag(KoSvgText::DecorationUnderline)){
        final.addPath(stroker.createStroke(underPath));
    }
    if (decor.testFlag(KoSvgText::DecorationOverline)){
        final.addPath(stroker.createStroke(overPath));
    }
    if (decor.testFlag(KoSvgText::DecorationLineThrough)){
        final.addPath(stroker.createStroke(middlePath));
    }

    return final;
}

int KoSvgTextShape::posForPoint(QPointF point, int start, int end, bool *overlaps)
{
    int a = 0;
    int b = d->cursorPos.size();
    if (start >= 0 && end >= 0) {
        a = qMax(start, a);
        b = qMin(end, b);
    }
    double closest = std::numeric_limits<double>::max();
    int candidate = -1;
    for (int i = a; i < b; i++) {
        CursorPos pos = d->cursorPos.at(i);
        CharacterResult res = d->result.at(pos.cluster);
        QPointF cursorStart = res.finalPosition;
        cursorStart += res.cursorInfo.offsets.value(pos.offset, res.advance);
        double distance = kisDistance(cursorStart, point);
        if (distance < closest) {
            candidate = i;
            closest = distance;
        }
        if (res.finalTransform().map(res.boundingBox).containsPoint(point, Qt::WindingFill) && overlaps) {
            *overlaps = true;
        }
    }
    return candidate;
}

int KoSvgTextShape::posForPointLineSensitive(QPointF point)
{
    bool overlaps = false;
    int initialPos = posForPoint(point, -1, -1, &overlaps);

    if (overlaps) {
        return initialPos;
    }

    KoSvgText::WritingMode mode = writingMode();

    int candidateLineStart = -1;
    double closest = std::numeric_limits<double>::max();
    for (int i = 0; i < d->cursorPos.size(); i++) {
        CursorPos pos = d->cursorPos.at(i);
        CharacterResult res = d->result.at(pos.cluster);
        if (res.anchored_chunk) {
            QLineF caret = res.cursorInfo.caret;
            caret.translate(res.finalPosition);
            QPointF cursorStart = res.finalPosition;
            cursorStart += res.cursorInfo.offsets.value(pos.offset, res.advance);
            double distance = kisDistance(cursorStart, point);
            if (mode == KoSvgText::HorizontalTB) {
                if (caret.p1().y() > point.y() && caret.p2().y() <= point.y() && closest > distance) {
                    candidateLineStart = i;
                    closest = distance;
                }
            } else {
                if (caret.p2().x() > point.x() && caret.p1().x() <= point.x() && closest > distance) {
                    candidateLineStart = i;
                    closest = distance;
                }
            }
        }
    }

    if (candidateLineStart > -1) {
        int end = lineEnd(candidateLineStart);
        initialPos = posForPoint(point, candidateLineStart, qMin(end + 1, d->cursorPos.size()));
    }

    return initialPos;
}

int KoSvgTextShape::posForIndex(int index, bool firstIndex, bool skipSynthetic)
{
    int pos = -1;
    if (d->cursorPos.isEmpty() || index < 0) {
        return pos;
    }
    for (int i = 0; i< d->cursorPos.size(); i++) {
        if (skipSynthetic && d->cursorPos.at(i).synthetic) {
            continue;
        }
        if (d->cursorPos.at(i).index <= index) {
            pos = i;
            if (d->cursorPos.at(i).index == index && firstIndex) {
                break;
            }
        } else if (d->cursorPos.at(i).index > index) {
            break;
        }
    }

    return pos;
}

int KoSvgTextShape::indexForPos(int pos)
{
    if (d->cursorPos.isEmpty() || pos < 0) {
        return -1;
    }

    return d->cursorPos.at(qMin(d->cursorPos.size()-1, pos)).index;
}

QPointF KoSvgTextShape::initialTextPosition() const
{
    return d->initialTextPosition;
}

KisForest<KoSvgTextContentElement>::depth_first_tail_iterator findTextChunkForIndex(KisForest<KoSvgTextContentElement> &tree, int &currentIndex, int sought, bool skipZeroWidth = false)
{
    auto it = tree.depthFirstTailBegin();
    for (; it != tree.depthFirstTailEnd(); it++) {
        if (std::distance(KisForestDetail::childBegin(it), KisForestDetail::childEnd(it)) > 0) {
            continue;
        }
        int length = it->numChars(false);
        if (length == 0 && skipZeroWidth) {
            continue;
        }

        if (sought == currentIndex || (sought > currentIndex && sought < currentIndex + length)) {
            break;
        } else {
            currentIndex += length;
        }
    }
    return it;
}

bool KoSvgTextShape::insertText(int pos, QString text)
{
    bool succes = false;
    int currentIndex = 0;
    int index = 0;
    int oldIndex = 0;
    if (pos > -1 && !d->cursorPos.isEmpty()) {
        CursorPos cursorPos = d->cursorPos.at(pos);
        CharacterResult res = d->result.at(cursorPos.cluster);
        index = res.plaintTextIndex;
        oldIndex = cursorPos.index;
        index = qMin(index, d->result.size()-1);
    }
    auto it = findTextChunkForIndex(d->textData, currentIndex, index);
    if (it.node()) {
        int offset = oldIndex - currentIndex;
        it->insertText(offset, text);
        notifyChanged();
        shapeChangedPriv(ContentChanged);
        succes = true;
    }
    return succes;
}

bool KoSvgTextShape::removeText(int &index, int &length)
{
    bool succes = false;
    if (index < -1 || d->cursorPos.isEmpty()) {
        return succes;
    }
    int currentLength = length;
    int endLength = 0;
    while (currentLength > 0) {
        int currentIndex = 0;
        auto it = findTextChunkForIndex(d->textData, currentIndex, index, true);
        if (it.node()) {
            int offset = index > currentIndex? index - currentIndex: 0;
            int size = it->numChars(false);
            it->removeText(offset, currentLength);
            int diff = size - it->numChars(false);
            currentLength -= diff;
            endLength += diff;

            if (index >= currentIndex) {
                index = currentIndex + offset;
            }
            succes = true;
        } else {
            currentLength = -1;
        }
    }
    if (succes) {
        length = endLength;
        notifyChanged();
        shapeChangedPriv(ContentChanged);
    }
    return succes;
}

KoSvgTextProperties KoSvgTextShape::propertiesForPos(int pos)
{
    KoSvgTextProperties props = KisForestDetail::size(d->textData)? d->textData.childBegin()->properties: KoSvgTextProperties();
    if (pos < 0 || d->cursorPos.isEmpty()) {
        return props;
    }
    CursorPos cursorPos = d->cursorPos.at(pos);
    CharacterResult res = d->result.at(cursorPos.cluster);
    int currentIndex = 0;
    auto it = findTextChunkForIndex(d->textData, currentIndex, res.plaintTextIndex);
    if (it.node()) {
        props = it->properties;
    }

    return props;
}

void KoSvgTextShape::setPropertiesAtPos(int pos, KoSvgTextProperties properties)
{
    if (pos < 0 || d->cursorPos.isEmpty()) {
        if (KisForestDetail::size(d->textData)) {
            d->textData.childBegin()->properties = properties;
        }
        notifyChanged();
        shapeChangedPriv(ContentChanged);
        return;
    }
    CursorPos cursorPos = d->cursorPos.at(pos);
    CharacterResult res = d->result.at(cursorPos.cluster);
    int currentIndex = 0;
    auto it = findTextChunkForIndex(d->textData, currentIndex, res.plaintTextIndex);
    if (it.node()) {
        it->properties = properties;
        notifyChanged();
        shapeChangedPriv(ContentChanged);
    }
}

KoSvgTextProperties KoSvgTextShape::textProperties() const
{
    return d->textData.childBegin().node()? d->textData.childBegin()->properties: KoSvgTextProperties();
}

QSharedPointer<KoShapeBackground> KoSvgTextShape::background() const
{
    QSharedPointer<KoShapeBackground> bg(new KoColorBackground(Qt::black));
    KoSvgTextProperties props = KisForestDetail::size(d->textData)? d->textData.childBegin()->properties: KoSvgTextProperties();
    if (props.hasProperty(KoSvgTextProperties::FillId)) {
        return props.property(KoSvgTextProperties::FillId).value<KoSvgText::BackgroundProperty>().property;
    }
    return QSharedPointer<KoShapeBackground>(new KoColorBackground(Qt::black));
}

void KoSvgTextShape::setBackground(QSharedPointer<KoShapeBackground> background)
{
    qDebug() << Q_FUNC_INFO;
    if (KisForestDetail::size(d->textData) == 0) {
        d->textData.insert(d->textData.childBegin(), KoSvgTextContentElement());
    }
    d->textData.childBegin()->properties.setProperty(KoSvgTextProperties::FillId,
                                                     QVariant::fromValue(KoSvgText::BackgroundProperty(background)));

    shapeChangedPriv(BackgroundChanged);
    notifyChanged();
}

KoShapeStrokeModelSP KoSvgTextShape::stroke() const
{
    KoSvgTextProperties props = KisForestDetail::size(d->textData)? d->textData.childBegin()->properties: KoSvgTextProperties();
    return props.property(KoSvgTextProperties::StrokeId).value<KoSvgText::StrokeProperty>().property;
}

void KoSvgTextShape::setStroke(KoShapeStrokeModelSP stroke)
{
    if (KisForestDetail::size(d->textData) == 0) {
        d->textData.insert(d->textData.childBegin(), KoSvgTextContentElement());
    }
    d->textData.childBegin()->properties.setProperty(KoSvgTextProperties::StrokeId,
                                                     QVariant::fromValue(KoSvgText::StrokeProperty(stroke)));
    shapeChangedPriv(StrokeChanged);
    notifyChanged();
}

QVector<KoShape::PaintOrder> KoSvgTextShape::paintOrder() const
{
    KoSvgTextProperties props = KisForestDetail::size(d->textData)? d->textData.childBegin()->properties: KoSvgTextProperties();
    if (props.hasProperty(KoSvgTextProperties::PaintOrder)) {
        return props.property(KoSvgTextProperties::PaintOrder).value<QVector<KoShape::PaintOrder>>();
    }
    return KoShape::paintOrder();
}

void KoSvgTextShape::setPaintOrder(KoShape::PaintOrder first, KoShape::PaintOrder second)
{
    if (KisForestDetail::size(d->textData) == 0) {
        d->textData.insert(d->textData.childBegin(), KoSvgTextContentElement());
    }
    KIS_SAFE_ASSERT_RECOVER_RETURN(first != second);
    QVector<PaintOrder> order = {Fill, Stroke, Markers};

    if (first != Fill) {
        if (order.at(1) == first) {
            order[1] = order[0];
            order[0] = first;
        } else if (order.at(2) == first) {
            order[2] = order[0];
            order[0] = first;
        }
    }
    if (second != first && second != Stroke) {
        if (order.at(2) == second) {
            order[2] = order[1];
            order[1] = second;
        }
    }
    d->textData.childBegin()->properties.setProperty(KoSvgTextProperties::PaintOrder,
                                                    QVariant::fromValue(order));
    setInheritPaintOrder(false);
}

QString KoSvgTextShape::plainText()
{
    return d->plainText;
}

KoSvgText::WritingMode KoSvgTextShape::writingMode() const
{
    return KoSvgText::WritingMode(this->textProperties().propertyOrDefault(KoSvgTextProperties::WritingModeId).toInt());
}

void KoSvgTextShape::notifyCursorPosChanged(int pos, int anchor)
{
    Q_FOREACH (KoShape::ShapeChangeListener *listener, listeners()) {
        TextCursorChangeListener *cursorListener = dynamic_cast<TextCursorChangeListener*>(listener);
        if (cursorListener) {
            cursorListener->notifyCursorPosChanged(pos, anchor);
        }
    }
}

bool KoSvgTextShape::loadSvg(const QDomElement &element, SvgLoadingContext &context)
{

    KoSvgTextContentElement textNode;
    textNode.loadSvg(element, context);
    qDebug() << Q_FUNC_INFO;
    if (KisForestDetail::size(d->textData) == 0) {
        KoSvgTextChunkShape::loadSvg(element, context);
        d->currentNode = d->textData.insert(d->textData.childEnd(), textNode);
    } else {
        d->currentNode = d->textData.insert(childEnd(KisForestDetail::parent(d->currentNode)), textNode);
    }

    return true;
}

bool KoSvgTextShape::loadSvgTextIntoNewLeaf(const QDomElement &parent, const QDomText &text, SvgLoadingContext &context) {
    KoSvgTextContentElement textNode;
    textNode.loadSvg(parent, context);
    textNode.localTransformations.clear();
    textNode.textLength = KoSvgText::AutoValue();
    d->currentNode = d->textData.insert(childEnd(KisForestDetail::parent(d->currentNode)), textNode);
    return loadSvgText(text, context);
}

bool KoSvgTextShape::loadSvgText(const QDomText &text, SvgLoadingContext &context)
{
    if (d->currentNode.node()) {
        qDebug() << "loading text";
        d->currentNode.node()->value.loadSvgTextNode(text, context);
    } else {
        qDebug() << "new text";
        KoSvgTextContentElement textNode;
        textNode.loadSvgTextNode(text, context);
        d->currentNode = d->textData.insert(childEnd(KisForestDetail::parent(d->currentNode)), textNode);
    }
    return true;
}

void KoSvgTextShape::setStyleInfo(KoShape *s)
{
    if (d->currentNode.node()) {
        // find closest parent stroke and fill so we can check for inheritance.
        QSharedPointer<KoShapeBackground> parentBg;
        KoShapeStrokeModelSP parentStroke;
        for (auto it = KisForestDetail::hierarchyBegin(d->currentNode); it != KisForestDetail::hierarchyEnd(d->currentNode); it++) {
            if (it->properties.hasProperty(KoSvgTextProperties::FillId)) {
                parentBg = it->properties.background();
                break;
            }
        }
        for (auto it = KisForestDetail::hierarchyBegin(d->currentNode); it != KisForestDetail::hierarchyEnd(d->currentNode); it++) {
            if (it->properties.hasProperty(KoSvgTextProperties::StrokeId)) {
                parentStroke = it->properties.stroke();
                break;
            }
        }

        if ((parentBg && !parentBg->compareTo(s->background().data()))
                || (!parentBg && s->background())) {
            d->currentNode->properties.setProperty(KoSvgTextProperties::FillId,
                                                   QVariant::fromValue(KoSvgText::BackgroundProperty(s->background())));
        }
        if ((parentStroke && !parentStroke->compareFillTo(s->stroke().data()) && !parentStroke->compareStyleTo(s->stroke().data()))
                || (!parentStroke && s->stroke())) {
            qDebug() << "setting stroke";
            d->currentNode->properties.setProperty(KoSvgTextProperties::StrokeId,
                                                   QVariant::fromValue(KoSvgText::StrokeProperty(s->stroke())));
        }
        d->currentNode->properties.setProperty(KoSvgTextProperties::Opacity,
                                               s->transparency());
        d->currentNode->properties.setProperty(KoSvgTextProperties::Visiblity,
                                               s->isVisible());
        if (!s->inheritPaintOrder()) {
            d->currentNode->properties.setProperty(KoSvgTextProperties::PaintOrder,
                                                   QVariant::fromValue(s->paintOrder()));
        }
    }
}

void KoSvgTextShape::setTextPathOnCurrentNode(KoShape *s)
{
    if (d->currentNode.node()) {
        d->currentNode.node()->value.textPath.reset(s);
    }
}

#include "KoXmlWriter.h"
bool KoSvgTextShape::saveSvg(SvgSavingContext &context)
{
    bool success = false;
    QList<KoSvgTextProperties> parentProps = {KoSvgTextProperties::defaultProperties()};
    for (auto it = d->textData.compositionBegin(); it != d->textData.compositionEnd(); it++) {
        if (it.state() == KisForestDetail::Enter) {
            bool isTextPath = false;
            QMap<QString, QString> shapeSpecificStyles;
            if (it->textPath) {
                isTextPath = true;
            }
            if (it == d->textData.compositionBegin()) {
                context.shapeWriter().startElement("text", false);

                if (!context.strippedTextMode()) {
                    context.shapeWriter().addAttribute("id", context.getID(this));

                    context.shapeWriter().addAttribute("text-rendering", textRenderingString());

                    // save the version to distinguish from the buggy Krita version
                    // 2: Wrong font-size.
                    // 3: Wrong font-size-adjust.
                    context.shapeWriter().addAttribute("krita:textVersion", 3);

                    SvgUtil::writeTransformAttributeLazy("transform", transformation(), context.shapeWriter());
                    SvgStyleWriter::saveSvgStyle(this, context);
                } else {
                    context.shapeWriter().addAttribute("text-rendering", textRenderingString());
                    SvgStyleWriter::saveSvgFill(this->background(), false, this->outlineRect(), this->size(), this->absoluteTransformation(), context);
                    SvgStyleWriter::saveSvgStroke(this->stroke(), context);
                }
                shapeSpecificStyles = this->shapeTypeSpecificStyles(context);
            } else {
                if (isTextPath) {
                    context.shapeWriter().startElement("textPath", false);
                } else {
                    context.shapeWriter().startElement("tspan", false);
                }
                if (!context.strippedTextMode()) {
                    SvgStyleWriter::saveSvgBasicStyle(it->properties.property(KoSvgTextProperties::Visiblity).toBool(),
                                                      it->properties.property(KoSvgTextProperties::Opacity).toReal(),
                                                      it->properties.property(KoSvgTextProperties::PaintOrder).value<QVector<KoShape::PaintOrder>>(),
                                                      !it->properties.hasProperty(KoSvgTextProperties::PaintOrder), context);
                }
            }

            success = it->saveSvg(context,
                                  parentProps.last(),
                                  it == d->textData.compositionBegin(),
                                  d->childCount(siblingCurrent(it)) == 0,
                                  shapeSpecificStyles);
            parentProps.append(it.node()->value.properties);
        } else {
            parentProps.pop_back();
            context.shapeWriter().endElement();
        }
    }
    return success;
}

void KoSvgTextShape::enterNodeSubtree()
{
    qDebug() << Q_FUNC_INFO;
    if (!d->currentNode.node()) {
        d->currentNode = d->textData.insert(childEnd(KisForestDetail::parent(d->currentNode)), KoSvgTextContentElement());
        qDebug() << KisForestDetail::depth(d->textData);
    }
    d->currentNode = childEnd(d->currentNode);
}

void KoSvgTextShape::leaveNodeSubtree()
{
    d->currentNode = KisForestDetail::parent(d->currentNode);
}

void KoSvgTextShape::resetParsing()
{
    qDebug() << Q_FUNC_INFO;
    d->textData.erase(d->textData.childBegin(), d->textData.childEnd());
    d->currentNode = d->textData.childEnd();
}

void KoSvgTextShape::debugParsing()
{
    qDebug() << "testing parsing" << KisForestDetail::size(d->textData);
    QString spaces;
    QList<KoSvgTextProperties> parentProps = {KoSvgTextProperties::defaultProperties()};
    for (auto it = compositionBegin(d->textData); it != compositionEnd(d->textData); it++) {
        if (it.state() == KisForestDetail::Enter) {

            qDebug() << QString(spaces + "+") << it->text;
            qDebug() << QString(spaces + "|") << it->properties.ownProperties(parentProps.last()).convertToSvgTextAttributes();
            qDebug() << QString(spaces + "| Fill set: ") << it->properties.hasProperty(KoSvgTextProperties::FillId);
            qDebug() << QString(spaces + "| Stroke set: ") << it->properties.hasProperty(KoSvgTextProperties::StrokeId);
            qDebug() << QString(spaces + "| Opacity: ") << it->properties.property(KoSvgTextProperties::Opacity);
            qDebug() << QString(spaces + "| PaintOrder: ") << it->properties.hasProperty(KoSvgTextProperties::PaintOrder);
            qDebug() << QString(spaces + "| Visibility set: ") << it->properties.hasProperty(KoSvgTextProperties::Visiblity);
            qDebug() << QString(spaces + "| TextPath set: ") << (it->textPath);
            qDebug() << QString(spaces + "| Transforms set: ") << it->localTransformations;
            spaces.append(" ");
            parentProps.append(it.node()->value.properties);
        }

        if (it.state() == KisForestDetail::Leave) {
            spaces.chop(1);
            parentProps.pop_back();
        }
    }
}

void KoSvgTextShape::paintComponent(QPainter &painter) const
{
    painter.save();
    if (d->textRendering == OptimizeSpeed) {
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    } else {
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    }

    QPainterPath chunk;
    int currentIndex = 0;
    if (!d->result.isEmpty()) {
        QPainterPath rootBounds;
        rootBounds.addRect(this->outline().boundingRect());
        d->paintPaths(painter, rootBounds, this, d->result, chunk, currentIndex);
    }
#if 0 // Debug
    Q_FOREACH (KoShape *child, this->shapes()) {
        const KoSvgTextChunkShape *textPathChunk = dynamic_cast<const KoSvgTextChunkShape *>(child);
        if (textPathChunk) {
            painter.save();
            painter.setPen(Qt::magenta);
            painter.setOpacity(0.5);
            if (textPathChunk->layoutInterface()->textPath()) {
                QPainterPath p = textPathChunk->layoutInterface()->textPath()->outline();
                p = textPathChunk->layoutInterface()->textPath()->transformation().map(p);
                painter.strokePath(p, QPen(Qt::green));
                painter.drawPoint(p.pointAtPercent(0));
                painter.drawPoint(p.pointAtPercent(p.percentAtLength(p.length() * 0.5)));
                painter.drawPoint(p.pointAtPercent(1.0));
            }
            painter.restore();
        }
    }
#endif
#if 0 // Debug
    Q_FOREACH (KoShape *shapeInside, d->shapesInside) {
        QPainterPath p = shapeInside->outline();
        p = shapeInside->transformation().map(p);
        painter.strokePath(p, QPen(Qt::green));
    }
    Q_FOREACH (KoShape *shapeInside, d->shapesSubtract) {
        QPainterPath p = shapeInside->outline();
        p = shapeInside->transformation().map(p);
        painter.strokePath(p, QPen(Qt::red));
    }
#endif

    painter.restore();
}

void KoSvgTextShape::paintStroke(QPainter &painter) const
{
    Q_UNUSED(painter);
    // do nothing! everything is painted in paintComponent()
}

QPainterPath KoSvgTextShape::outline() const {
    QPainterPath result;
    for (auto it = d->textData.depthFirstTailBegin(); it != d->textData.depthFirstTailEnd(); it++) {
        result.addPath(it->associatedOutline);
        for (int i = 0; i < it->textDecorations.values().size(); ++i) {
            result.addPath(it->textDecorations.values().at(i));
        }

    }
    return result;
}
QRectF KoSvgTextShape::outlineRect() const
{
    return outline().boundingRect();
}

QRectF KoSvgTextShape::boundingRect() const
{
    QRectF result;
    KoShapeStrokeModelSP stroke = nullptr;
    for (auto it = d->textData.depthFirstTailBegin(); it != d->textData.depthFirstTailEnd(); it++) {
        if (it->properties.hasProperty(KoSvgTextProperties::StrokeId)) {
            stroke = it->properties.property(KoSvgTextProperties::StrokeId).value<KoSvgText::StrokeProperty>().property;
        }
        if (stroke) {
            QRectF bb = outlineRect();
            KoInsets insets;
            stroke->strokeInsets(this, insets);
            result |= bb.adjusted(-insets.left, -insets.top, insets.right, insets.bottom);
        } else {
            result |= outlineRect();
        }
    }
    return this->absoluteTransformation().mapRect(result);
}

void KoSvgTextShape::paintDebug(QPainter &painter, const DebugElements elements) const
{
    if (elements & DebugElement::CharBbox) {
        int currentIndex = 0;
        if (!d->result.isEmpty()) {
            QPainterPath rootBounds;
            rootBounds.addRect(this->outline().boundingRect());
            d->paintDebug(painter, d->result, currentIndex);
        }
    }

    if (elements & DebugElement::LineBox) {
        Q_FOREACH (LineBox lineBox, d->lineBoxes) {
            Q_FOREACH (const LineChunk &chunk, lineBox.chunks) {
                QPen pen;
                pen.setCosmetic(true);
                pen.setWidth(2);
                painter.setBrush(QBrush(Qt::transparent));
                pen.setColor(QColor(0, 128, 255, 128));
                painter.setPen(pen);
                painter.drawLine(chunk.length);
                pen.setColor(QColor(255, 128, 0, 128));
                painter.setPen(pen);
                painter.drawRect(chunk.boundingBox);
            }
        }
    }
}

QList<KoShape *> KoSvgTextShape::textOutline() const
{
    QList<KoShape *> shapes;
    int currentIndex = 0;
    if (!d->result.empty()) {
        shapes = d->collectPaths(this, d->result, currentIndex);
    }

    return shapes;
}

void KoSvgTextShape::setTextRenderingFromString(const QString &textRendering)
{
    if (textRendering == "optimizeSpeed") {
        d->textRendering = OptimizeSpeed;
    } else if (textRendering == "optimizeLegibility") {
        d->textRendering = OptimizeLegibility;
    } else if (textRendering == "geometricPrecision") {
        d->textRendering = GeometricPrecision;
    } else {
        d->textRendering = Auto;
    }
}

QString KoSvgTextShape::textRenderingString() const
{
    if (d->textRendering == OptimizeSpeed) {
        return "optimizeSpeed";
    } else if (d->textRendering == OptimizeLegibility) {
        return "optimizeLegibility";
    } else if (d->textRendering == GeometricPrecision) {
        return "geometricPrecision";
    } else {
        return "auto";
    }
}

void KoSvgTextShape::setShapesInside(QList<KoShape *> shapesInside)
{
    d->shapesInside = shapesInside;
}

QList<KoShape *> KoSvgTextShape::shapesInside() const
{
    return d->shapesInside;
}

void KoSvgTextShape::setShapesSubtract(QList<KoShape *> shapesSubtract)
{
    d->shapesSubtract = shapesSubtract;
}

QList<KoShape *> KoSvgTextShape::shapesSubtract() const
{
    return d->shapesSubtract;
}

QMap<QString, QString> KoSvgTextShape::shapeTypeSpecificStyles(SvgSavingContext &context) const
{
    QMap<QString, QString> map = this->textProperties().convertParagraphProperties();
    if (!d->shapesInside.isEmpty()) {
        QStringList shapesInsideList;
        Q_FOREACH(KoShape* shape, d->shapesInside) {
            QString id = SvgStyleWriter::embedShape(shape, context);
            shapesInsideList.append(QString("url(#%1)").arg(id));
        }
        map.insert("shape-inside", shapesInsideList.join(" "));
    }
    if (!d->shapesSubtract.isEmpty()) {
        QStringList shapesInsideList;
        Q_FOREACH(KoShape* shape, d->shapesSubtract) {
            QString id = SvgStyleWriter::embedShape(shape, context);
            shapesInsideList.append(QString("url(#%1)").arg(id));
        }
        map.insert("shape-subtract", shapesInsideList.join(" "));
    }

    return map;
}

void KoSvgTextShape::resetTextShape()
{
    KoSvgTextChunkShape::resetTextShape();
    relayout();
}

void KoSvgTextShape::relayout() const
{
    d->relayout();
}

bool KoSvgTextShape::isRootTextNode() const
{
    return true;
}

KoSvgTextShapeFactory::KoSvgTextShapeFactory()
    : KoShapeFactoryBase(KoSvgTextShape_SHAPEID, i18nc("Text label in SVG Text Tool", "Text"))
{
    setToolTip(i18n("SVG Text Shape"));
    setIconName(koIconNameCStr("x-shape-text"));
    setLoadingPriority(5);
    setXmlElementNames(KoXmlNS::svg, QStringList("text"));

    KoShapeTemplate t;
    t.name = i18n("SVG Text");
    t.iconName = koIconName("x-shape-text");
    t.toolTip = i18n("SVG Text Shape");
    addTemplate(t);
}

KoShape *KoSvgTextShapeFactory::createDefaultShape(KoDocumentResourceManager *documentResources) const
{
    debugFlake << "Create default svg text shape";

    KoSvgTextShape *shape = new KoSvgTextShape();
    shape->setShapeId(KoSvgTextShape_SHAPEID);

    KoSvgTextShapeMarkupConverter converter(shape);
    converter.convertFromSvg(i18nc("Default text for the text shape", "<text>Placeholder Text</text>"),
                             "<defs/>",
                             QRectF(0, 0, 200, 60),
                             documentResources->documentResolution());

    debugFlake << converter.errors() << converter.warnings();

    return shape;
}

KoShape *KoSvgTextShapeFactory::createShape(const KoProperties *params, KoDocumentResourceManager *documentResources) const
{
    KoSvgTextShape *shape = new KoSvgTextShape();
    shape->setShapeId(KoSvgTextShape_SHAPEID);

    QString svgText = params->stringProperty("svgText", i18nc("Default text for the text shape", "<text>Placeholder Text</text>"));
    QString defs = params->stringProperty("defs" , "<defs/>");
    QRectF shapeRect = QRectF(0, 0, 200, 60);
    QVariant rect = params->property("shapeRect");
    QVariant origin = params->property("origin");

    if (rect.type()==QVariant::RectF) {
        shapeRect = rect.toRectF();
    }

    KoSvgTextShapeMarkupConverter converter(shape);
    converter.convertFromSvg(svgText,
                             defs,
                             shapeRect,
                             documentResources->documentResolution());
    if (origin.type() == QVariant::PointF) {
        shape->setPosition(origin.toPointF());
    } else {
        shape->setPosition(shapeRect.topLeft());
    }
    shape->setPaintOrder(KoShape::Stroke, KoShape::Fill);

    return shape;
}

bool KoSvgTextShapeFactory::supports(const QDomElement &/*e*/, KoShapeLoadingContext &/*context*/) const
{
    return false;
}

void KoSvgTextShape::TextCursorChangeListener::notifyShapeChanged(KoShape::ChangeType type, KoShape *shape)
{
    Q_UNUSED(type);
    Q_UNUSED(shape);
}

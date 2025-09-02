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
#include <KoDocumentResourceManager.h>
#include <KoShapeContainer_p.h>
#include <KoShapeController.h>
#include <text/KoCssTextUtils.h>
#include <text/KoFontRegistry.h>
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

// Memento pointer to hold data for Undo commands.

class KoSvgTextShapeMementoImpl : public KoSvgTextShapeMemento
{
public:
    KoSvgTextShapeMementoImpl(const KisForest<KoSvgTextContentElement> &textData,
                              const QVector<CharacterResult> &result,
                              const QVector<LineBox> &lineBoxes,
                              const QVector<CursorPos> &cursorPos,
                              const QMap<int, int> &logicalToVisualCursorPos,
                              const QString &plainText,
                              const bool &isBidi,
                              const QPointF &initialTextPosition)
        : KoSvgTextShapeMemento()
        , textData(textData)
        , result(result)
        , lineBoxes(lineBoxes)
        , cursorPos(cursorPos)
        , logicalToVisualCursorPos(logicalToVisualCursorPos)
        , plainText(plainText)
        , isBidi(isBidi)
        , initialTextPosition(initialTextPosition)
    {
    }

    ~KoSvgTextShapeMementoImpl() {}

private:
    friend class KoSvgTextShape;
    KisForest<KoSvgTextContentElement> textData;
    QVector<CharacterResult> result;
    QVector<LineBox> lineBoxes;

    QVector<CursorPos> cursorPos;
    QMap<int, int> logicalToVisualCursorPos;

    QString plainText;
    bool isBidi = false;
    QPointF initialTextPosition = QPointF();
};


KoSvgTextShape::KoSvgTextShape()
    : KoShape()
    , d(new Private)
{
    setShapeId(KoSvgTextShape_SHAPEID);
    d->textData.insert(d->textData.childBegin(), KoSvgTextContentElement());
}

KoSvgTextShape::KoSvgTextShape(const KoSvgTextShape &rhs)
    : KoShape(rhs)
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
    if (d->isLoading) {
        return;
    }
    KoShape::shapeChanged(type, shape);

    if (type == ContentChanged) {
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
    double fontSize = this->textProperties().fontSize().value;
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
    end = qMin(d->cursorPos.size()-1, end);

    if (start == end || start < 0) {
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
    qint32 strokeWidth = 0;
    QPointF inset = mode == KoSvgText::HorizontalTB? QPointF(minimum*0.5, 0): QPointF(0, minimum*0.5);
    for (int i = start+1; i <= end; i++) {
        CursorPos pos = d->cursorPos.at(i);
        CharacterResult res = d->result.at(pos.cluster);
        strokeWidth += res.metrics.underlineThickness;
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

    const qreal freetypePixelsToPt = (1.0 / 64.0) * (72. / qMin(d->xRes, d->yRes));
    const qreal width = strokeWidth > 0 ? qMax(qreal(strokeWidth/qMax(1, end-(start+1)))*freetypePixelsToPt, minimum): minimum;

    stroker.setWidth(thick? width*2: width);

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
    int candidate = 0;
    for (int i = a; i < b; i++) {
        CursorPos pos = d->cursorPos.at(i);
        CharacterResult res = d->result.at(pos.cluster);
        QPointF cursorStart = res.finalPosition;
        cursorStart += res.cursorInfo.offsets.value(pos.offset, res.advance);
        double distance = kisDistance(cursorStart, point);
        if (distance < closest) {
            candidate = i;
            closest = distance;
            if (overlaps) {
               *overlaps = res.finalTransform().map(res.layoutBox()).containsPoint(point, Qt::WindingFill);
            }
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

    int candidateLineStart = 0;
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

int KoSvgTextShape::posForIndex(int index, bool firstIndex, bool skipSynthetic) const
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

int KoSvgTextShape::indexForPos(int pos) const
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

bool KoSvgTextShape::insertText(int pos, QString text)
{
    bool success = false;
    int currentIndex = 0; ///< Current position of the search.

    /// The distinction between element and insertion index allows us to insert text at
    /// the start or end of a content element, without ambiguity on whether we're
    /// inserting into the next element.
    int elementIndex = 0;
    int insertionIndex = 0;
    if (pos > -1 && !d->cursorPos.isEmpty()) {
        CursorPos cursorPos = d->cursorPos.at(pos);
        CharacterResult res = d->result.at(cursorPos.cluster);
        elementIndex = res.plaintTextIndex;
        insertionIndex = cursorPos.index;
        elementIndex = qMin(elementIndex, d->result.size()-1);
    }
    auto it = d->findTextContentElementForIndex(d->textData, currentIndex, elementIndex);
    if (it != d->textData.depthFirstTailEnd()) {
        const int offset = insertionIndex - currentIndex;
        it->insertText(offset, text);

        d->insertTransforms(d->textData, insertionIndex, text.size(), (elementIndex == insertionIndex));
        notifyChanged();
        shapeChangedPriv(ContentChanged);
        success = true;
    }
    return success;
}

bool KoSvgTextShape::removeText(int &index, int &length)
{
    bool success = false;
    if (index < -1) {
        return success;
    }
    int currentLength = length;
    int endLength = 0;
    while (currentLength > 0) {
        int currentIndex = 0;

        auto it = d->findTextContentElementForIndex(d->textData, currentIndex, index, true);
        if (it != d->textData.depthFirstTailEnd()) {
            int offset = index > currentIndex? index - currentIndex: 0;
            int size = it->numChars(false);
            it->removeText(offset, currentLength);
            int diff = size - it->numChars(false);
            currentLength -= diff;
            endLength += diff;

            if (index >= currentIndex) {
                index = currentIndex + offset;
            }

            d->removeTransforms(d->textData, index, endLength);

            success = true;
        } else {
            currentLength = -1;
        }
    }
    if (success) {
        length = endLength;
        notifyChanged();
        shapeChangedPriv(ContentChanged);
    }
    return success;
}

KoSvgTextProperties KoSvgTextShape::propertiesForPos(const int pos, bool inherited) const
{
    return propertiesForRange(pos, pos, inherited).value(0, KoSvgTextProperties());
}

KoSvgTextProperties inheritProperties(KisForest<KoSvgTextContentElement>::depth_first_tail_iterator it)  {
    KoSvgTextProperties props = it->properties;
    for (auto parentIt = KisForestDetail::hierarchyBegin(siblingCurrent(it));
         parentIt != KisForestDetail::hierarchyEnd(siblingCurrent(it)); parentIt++) {
         KoSvgTextProperties parentProps = parentIt->properties;
         parentProps.setAllButNonInheritableProperties(props);
         props = parentProps;
    }
    return props;
}

QList<KoSvgTextProperties> KoSvgTextShape::propertiesForRange(const int startPos, const int endPos, bool inherited) const
{
    QList<KoSvgTextProperties> props;

    /// Sometimes this gets called when xml data is uploaded into the shape, at which point the tree is empty.
    if (d->isLoading) return props;

    if (((startPos < 0 || startPos >= d->cursorPos.size()) && startPos == endPos) || d->cursorPos.isEmpty()) {
        props = {KisForestDetail::size(d->textData)? d->textData.childBegin()->properties: KoSvgTextProperties()};
        return props;
    }
    const int finalPos = d->cursorPos.size() - 1;
    const int startIndex = d->cursorPos.at(qBound(0, startPos, finalPos)).index;
    const int endIndex = d->cursorPos.at(qBound(0, endPos, finalPos)).index;
    int sought = startIndex;
    if (startIndex == endIndex) {
        int currentIndex = 0;
        auto it = d->findTextContentElementForIndex(d->textData, currentIndex, sought);
        if (it != d->textData.depthFirstTailEnd()) {
            if (inherited) {
                props.append(inheritProperties(it));
            } else {
                props.append(it->properties);
            }
        } else {
            currentIndex = 0;
            it = d->findTextContentElementForIndex(d->textData, currentIndex, sought - 1);
            if (it != d->textData.depthFirstTailEnd()) {
                if (inherited) {
                    props.append(inheritProperties(it));
                } else {
                    props.append(it->properties);
                }
            }
        }
    } else {
        while(sought < endIndex) {
            int currentIndex = 0;
            auto it = d->findTextContentElementForIndex(d->textData, currentIndex, sought);
            if (KisForestDetail::siblingCurrent(it) == d->textData.childBegin()) {
                // If there's a selection and the search algorithm only returns the root, return empty.
                // The root text properties should be retrieved explicitely (either by using -1 as pos, or by calling textProperties()).
                props = {KoSvgTextProperties()};
                return props;
            } else if (it != d->textData.depthFirstTailEnd()) {
                if (inherited) {
                    props.append(inheritProperties(it));
                } else {
                    props.append(it->properties);
                }
            }
            sought = currentIndex + it->numChars(false);
        }
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
    auto it = d->findTextContentElementForIndex(d->textData, currentIndex, res.plaintTextIndex);
    if (it != d->textData.depthFirstTailEnd()) {
        it->properties = properties;
        notifyChanged();
        shapeChangedPriv(ContentChanged);
    }
}

void KoSvgTextShape::mergePropertiesIntoRange(const int startPos,
                                              const int endPos,
                                              const KoSvgTextProperties properties,
                                              const QSet<KoSvgTextProperties::PropertyId> removeProperties)
{
    if ((startPos < 0 && startPos == endPos) || d->cursorPos.isEmpty()) {
        if (KisForestDetail::size(d->textData)) {
            Q_FOREACH(KoSvgTextProperties::PropertyId p, properties.properties()) {
                d->textData.childBegin()->properties.setProperty(p, properties.property(p));
            }
            Q_FOREACH(KoSvgTextProperties::PropertyId p, removeProperties) {
                d->textData.childBegin()->properties.removeProperty(p);
            }
        }
        notifyChanged();
        shapeChangedPriv(ContentChanged);
        return;
    }
    const int startIndex = d->cursorPos.at(startPos).index;
    const int endIndex = d->cursorPos.at(endPos).index;
    if (startIndex != endIndex) {
        KoSvgTextShape::Private::splitContentElement(d->textData, startIndex);
        KoSvgTextShape::Private::splitContentElement(d->textData, endIndex);
    }
    bool changed = false;
    int currentIndex = 0;
    KoSvgText::AutoValue inlineSize = d->textData.childBegin()->properties.propertyOrDefault(KoSvgTextProperties::InlineSizeId).value<KoSvgText::AutoValue>();
    bool isWrapping = !d->shapesInside.isEmpty() || !inlineSize.isAuto;
    for (auto it = d->textData.depthFirstTailBegin(); it != d->textData.depthFirstTailEnd(); it++) {
        if (KoSvgTextShape::Private::childCount(siblingCurrent(it)) > 0) {
            continue;
        }

        if (currentIndex >= startIndex && currentIndex < endIndex) {
            Q_FOREACH(KoSvgTextProperties::PropertyId p, removeProperties) {
                if (KoSvgTextProperties::propertyIsBlockOnly(p)) {
                    d->textData.childBegin()->properties.removeProperty(p);
                } else {
                    it->properties.removeProperty(p);
                }
            }
            Q_FOREACH(KoSvgTextProperties::PropertyId p, properties.properties()) {
                if (KoSvgTextProperties::propertyIsBlockOnly(p) || (p == KoSvgTextProperties::TextAnchorId && isWrapping)) {
                    d->textData.childBegin()->properties.setProperty(p, properties.property(p));
                } else {
                    it->properties.setProperty(p, properties.property(p));
                }
            }

            changed = true;
        }
        currentIndex += it->numChars(false);
    }

    if (changed){
        KoSvgTextShape::Private::cleanUp(d->textData);
        notifyChanged();
        shapeChangedPriv(ContentChanged);
        if (properties.hasProperty(KoSvgTextProperties::FillId)) {
            shapeChangedPriv(BackgroundChanged);
        }
        if (properties.hasProperty(KoSvgTextProperties::StrokeId)) {
            shapeChangedPriv(StrokeChanged);
        }
    }
}

std::unique_ptr<KoSvgTextShape> KoSvgTextShape::copyRange(int index, int length) const
{
    KoSvgTextShape *clone = new KoSvgTextShape(*this);
    int zero = 0;
    int endRange = index + length;
    int size = KoSvgTextShape::Private::numChars(clone->d->textData.childBegin(), false) - endRange;
    clone->removeText(endRange, size);
    clone->removeText(zero, index);
    KoSvgTextShape::Private::cleanUp(clone->d->textData);
    return std::unique_ptr<KoSvgTextShape>(clone);
}

bool KoSvgTextShape::insertRichText(int pos, const KoSvgTextShape *richText)
{
    bool success = false;
    int currentIndex = 0;
    int elementIndex = 0;
    int insertionIndex = 0;

    if (isEnd(richText->d->textData.childBegin())) {
        // rich text is empty.
        return success;
    }

    if (pos > -1 && !d->cursorPos.isEmpty()) {
        CursorPos cursorPos = d->cursorPos.at(qMin(d->cursorPos.size()-1, pos));
        CharacterResult res = d->result.at(cursorPos.cluster);
        elementIndex = res.plaintTextIndex;
        insertionIndex = cursorPos.index;
        elementIndex = qMin(elementIndex, d->result.size()-1);
    }

    KoSvgTextShape::Private::splitContentElement(this->d->textData, insertionIndex);

    auto it = d->findTextContentElementForIndex(d->textData, currentIndex, insertionIndex);
    if (it != d->textData.depthFirstTailEnd()) {
        d->textData.move(richText->d->textData.childBegin(), siblingCurrent(it));
        success = true;
    } else {
        currentIndex = 0;
        it = d->findTextContentElementForIndex(d->textData, currentIndex, elementIndex);
        if (it != d->textData.depthFirstTailEnd()) {
            d->textData.move(richText->d->textData.childBegin(), siblingEnd(siblingCurrent(it)));
            success = true;
        }
    }

    if (success) {
        notifyChanged();
        shapeChangedPriv(ContentChanged);
    }
    return success;
}

void KoSvgTextShape::cleanUp()
{
    KoSvgTextShape::Private::cleanUp(d->textData);
    notifyChanged();
    shapeChangedPriv(ContentChanged);
}

QVector<int> findTreeIndexForPropertyIdImpl(KisForest<KoSvgTextContentElement>::child_iterator parent, KoSvgTextProperties::PropertyId propertyId) {
    int count = 0;
    for (auto child = KisForestDetail::childBegin(parent); child != KisForestDetail::childEnd(parent); child++) {
        if (child->properties.hasProperty(propertyId)) {
            return QVector<int>({count});
        } else if (KisForestDetail::childBegin(child) != KisForestDetail::childEnd(child)) {
            QVector<int> current = QVector<int>({count});
            QVector<int> childIndex = findTreeIndexForPropertyIdImpl(child, propertyId);
            if (!childIndex.isEmpty()) {
                current.append(childIndex);
                return current;
            }
        }
        count += 1;
    }
    return QVector<int>();
}

QVector<int> KoSvgTextShape::findTreeIndexForPropertyId(KoSvgTextProperties::PropertyId propertyId)
{
    QVector<int> treeIndex;

    for (auto it = d->textData.childBegin(); it != d->textData.childEnd(); it++) {
        if (it->properties.hasProperty(propertyId)) {
            return QVector<int>({0});
        } else {
            treeIndex = findTreeIndexForPropertyIdImpl(it, propertyId);
            if (!treeIndex.isEmpty()) {
                treeIndex.insert(0, 0);
            }
        }
    }

    return treeIndex;
}

KoSvgTextProperties KoSvgTextShape::propertiesForTreeIndex(const QVector<int> &treeIndex) const
{
    if (treeIndex.isEmpty()) return KoSvgTextProperties();
    QVector<int> idx = treeIndex;
    idx.removeFirst();
    for (auto it = d->textData.childBegin(); it != d->textData.childEnd(); it++) {
        if (idx.isEmpty()) {
            if (it != d->textData.childEnd()) {
                return it->properties;
            } else {
                return KoSvgTextProperties();
            }
        }
        auto child = d->iteratorForTreeIndex(idx, it);
        if (child != d->textData.childEnd()) {
            return child->properties;
        }
    }
    return KoSvgTextProperties();
}

bool KoSvgTextShape::setPropertiesAtTreeIndex(const QVector<int> treeIndex, const KoSvgTextProperties props)
{
    QVector<int> idx = treeIndex;
    bool success = false;
    if (treeIndex.isEmpty()) return success;
    idx.removeFirst();
    for (auto it = d->textData.childBegin(); it != d->textData.childEnd(); it++) {
        if (idx.isEmpty()) {
            if (it != d->textData.childEnd()) {
                it->properties = props;
                success = true;
                break;
            } else {
                success = false;
                break;
            }
        }
        auto child = d->iteratorForTreeIndex(idx, it);
        if (child != d->textData.childEnd()) {
            child->properties = props;
            success = true;
            break;
        }
    }
    if (success) {
        notifyChanged();
        shapeChangedPriv(ContentChanged);
    }
    return success;
}

QPair<int, int> KoSvgTextShape::findRangeForTreeIndex(const QVector<int> treeIndex) const
{
    QVector<int> idx = treeIndex;
    if (idx.isEmpty()) {
        return qMakePair(-1, -1);
    }
    if (idx.size() == 1) {
        return qMakePair(0, d->cursorPos.size());
    }
    idx.removeFirst();

    int startIndex = 0;
    int endIndex = 0;
    for (auto it = d->textData.childBegin(); it != d->textData.childEnd(); it++) {
        auto child = d->iteratorForTreeIndex(idx, it);
        if (child != d->textData.childEnd()) {
            // count children
            d->startIndexOfIterator(d->textData.childBegin(), child, startIndex);
            endIndex = d->numChars(child) + startIndex;
        }
    }
    return qMakePair(posForIndex(startIndex), posForIndex(endIndex));
}

KoSvgTextProperties KoSvgTextShape::textProperties() const
{
    return KisForestDetail::size(d->textData)? d->textData.childBegin()->properties: KoSvgTextProperties();
}

QSharedPointer<KoShapeBackground> KoSvgTextShape::background() const
{
    KoSvgTextProperties props = KisForestDetail::size(d->textData)? d->textData.childBegin()->properties: KoSvgTextProperties();
    if (props.hasProperty(KoSvgTextProperties::FillId)) {
        return props.property(KoSvgTextProperties::FillId).value<KoSvgText::BackgroundProperty>().property;
    }
    return QSharedPointer<KoShapeBackground>(new KoColorBackground(Qt::black));
}

void KoSvgTextShape::setBackground(QSharedPointer<KoShapeBackground> background)
{
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
    QVector<PaintOrder> order = KoShape::defaultPaintOrder();

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
    if (d->isLoading) {
        return;
    }
    Q_FOREACH (KoShape::ShapeChangeListener *listener, listeners()) {
        TextCursorChangeListener *cursorListener = dynamic_cast<TextCursorChangeListener*>(listener);
        if (cursorListener) {
            cursorListener->notifyCursorPosChanged(pos, anchor);
        }
    }
}

void KoSvgTextShape::notifyMarkupChanged()
{
    if (d->isLoading) {
        return;
    }
    Q_FOREACH (KoShape::ShapeChangeListener *listener, listeners()) {
        TextCursorChangeListener *cursorListener = dynamic_cast<TextCursorChangeListener*>(listener);
        if (cursorListener) {
            cursorListener->notifyMarkupChanged();
        }
    }
}

void KoSvgTextShape::convertCharTransformsToPreformatted(bool makeInlineSize)
{
    const int inlineSize = writingMode() == KoSvgText::HorizontalTB? outlineRect().width(): outlineRect().height();
    d->applyWhiteSpace(d->textData, true);
    d->insertNewLinesAtAnchors(d->textData, !d->shapesInside.isEmpty());
    d->cleanUp(d->textData);

    KoSvgTextProperties props = this->propertiesForPos(-1);
    if (makeInlineSize) {
        KoSvgText::AutoValue val;
        // Using QCeil here because otherwise the text will layout too tight.
        val.customValue = qCeil(inlineSize);
        val.isAuto = false;
        if (!props.hasProperty(KoSvgTextProperties::InlineSizeId)) {
            props.setProperty(KoSvgTextProperties::InlineSizeId, QVariant::fromValue(val));
        }
    } else {
        props.removeProperty(KoSvgTextProperties::InlineSizeId);
    }
    // NOTE: applyWhiteSpace and insertNewLines don't notify changes,
    // so setProperties is the only thing triggering relayout();
    setPropertiesAtPos(-1, props);
}

void KoSvgTextShape::setCharacterTransformsFromLayout()
{
    if (d->result.isEmpty()) return;
    d->setTransformsFromLayout(d->textData, d->result);
    d->cleanUp(d->textData);
    //d->applyWhiteSpace(d->textData, true);
    KoSvgTextProperties props = this->propertiesForPos(-1);
    props.removeProperty(KoSvgTextProperties::InlineSizeId);
    props.removeProperty(KoSvgTextProperties::TextCollapseId);
    props.removeProperty(KoSvgTextProperties::TextWrapId);

    setPropertiesAtPos(-1, props);
}

#include "KoXmlWriter.h"
bool KoSvgTextShape::saveSvg(SvgSavingContext &context)
{
    bool success = false;
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

                    // save the version to distinguish from the buggy Krita version
                    // 2: Wrong font-size.
                    // 3: Wrong font-size-adjust.
                    context.shapeWriter().addAttribute("krita:textVersion", 3);

                    SvgUtil::writeTransformAttributeLazy("transform", transformation(), context.shapeWriter());
                    SvgStyleWriter::saveSvgStyle(this, context);
                } else {
                    SvgStyleWriter::saveSvgFill(this->background(), false, this->outlineRect(), this->size(), this->absoluteTransformation(), context);
                    SvgStyleWriter::saveSvgStroke(this->stroke(), context);
                    SvgStyleWriter::saveSvgBasicStyle(true, 0, paintOrder(),
                                                      inheritPaintOrder(), context, true);
                }
                shapeSpecificStyles = this->shapeTypeSpecificStyles(context);
            } else {
                if (isTextPath) {
                    context.shapeWriter().startElement("textPath", false);
                } else {
                    context.shapeWriter().startElement("tspan", false);
                }
                SvgStyleWriter::saveSvgBasicStyle(it->properties.property(KoSvgTextProperties::Visiblity, true).toBool(),
                                                  it->properties.property(KoSvgTextProperties::Opacity, 0).toReal(),
                                                  it->properties.property(KoSvgTextProperties::PaintOrder,
                                                                          QVariant::fromValue(paintOrder())
                                                                          ).value<QVector<KoShape::PaintOrder>>(),
                                                  !it->properties.hasProperty(KoSvgTextProperties::PaintOrder), context, true);

            }

            success = it->saveSvg(context,
                                  it == d->textData.compositionBegin(),
                                  d->childCount(siblingCurrent(it)) == 0,
                                  shapeSpecificStyles);
        } else {
            if (it == d->textData.compositionBegin()) {
                SvgStyleWriter::saveMetadata(this, context);
            }
            context.shapeWriter().endElement();
        }
    }
    return success;
}

bool KoSvgTextShape::saveHtml(HtmlSavingContext &context)
{
    bool success = true;
    QList<KoSvgTextProperties> parentProps = {KoSvgTextProperties::defaultProperties()};
    for (auto it = d->textData.compositionBegin(); it != d->textData.compositionEnd(); it++) {
        if (it.state() == KisForestDetail::Enter) {
            QMap<QString, QString> shapeSpecificStyles;

            if (it == d->textData.compositionBegin()) {
                context.shapeWriter().startElement("p", false);
            } else {
                context.shapeWriter().startElement("span", false);
            }
            KoSvgTextProperties ownProperties = it->properties.ownProperties(parentProps.last(),
                                                                             it == d->textData.compositionBegin());
            parentProps.append(ownProperties);
            QMap<QString, QString> attributes = ownProperties.convertToSvgTextAttributes();
            if (it == d->textData.compositionBegin())
                attributes.insert(ownProperties.convertParagraphProperties());
            bool addedFill = false;
            if (attributes.size() > 0) {
                QString styleString;
                for (auto it = attributes.constBegin(); it != attributes.constEnd(); ++it) {
                    if (QString(it.key().toLatin1().data()).contains("text-anchor")) {
                        QString val = it.value();
                        if (it.value()=="middle") {
                            val = "center";
                        } else if (it.value()=="end") {
                            val = "right";
                        } else {
                            val = "left";
                        }
                        styleString.append("text-align")
                                .append(": ")
                                .append(val)
                                .append(";" );
                    } else if (QString(it.key().toLatin1().data()).contains("fill")) {
                        styleString.append("color")
                                .append(": ")
                                .append(it.value())
                                .append(";" );
                        addedFill = true;
                    } else if (QString(it.key().toLatin1().data()).contains("font-size")) {
                        QString val = it.value();
                        styleString.append(it.key().toLatin1().data())
                                .append(": ")
                                .append(val)
                                .append(";" );
                    } else {
                        styleString.append(it.key().toLatin1().data())
                                .append(": ")
                                .append(it.value())
                                .append(";" );
                    }
                }
                if (ownProperties.hasProperty(KoSvgTextProperties::FillId) && !addedFill) {
                    KoColorBackground *b = dynamic_cast<KoColorBackground *>(it->properties.background().data());
                    if (b) {
                        styleString.append("color")
                                .append(": ")
                                .append(b->color().name())
                                .append(";" );
                    }
                }
                context.shapeWriter().addAttribute("style", styleString);

                if (d->childCount(siblingCurrent(it)) == 0) {
                    debugFlake << "saveHTML" << this << it->text;
                    // After adding all the styling to the <p> element, add the text
                    context.shapeWriter().addTextNode(it->text);
                }
            }
        } else {
            parentProps.pop_back();
            context.shapeWriter().endElement();
        }
    }
    return success;
}

void KoSvgTextShape::enterNodeSubtree()
{

}

void KoSvgTextShape::leaveNodeSubtree()
{

}

KoSvgTextShapeMementoSP KoSvgTextShape::getMemento()
{
    return KoSvgTextShapeMementoSP(new KoSvgTextShapeMementoImpl(d->textData,
                                                                 d->result,
                                                                 d->lineBoxes,
                                                                 d->cursorPos,
                                                                 d->logicalToVisualCursorPos,
                                                                 d->plainText,
                                                                 d->isBidi,
                                                                 d->initialTextPosition));
}

void KoSvgTextShape::setMementoImpl(const KoSvgTextShapeMementoSP memento)
{
    KoSvgTextShapeMementoImpl *impl = dynamic_cast<KoSvgTextShapeMementoImpl*>(memento.data());
    if (impl) {
        d->textData = impl->textData;
        d->result = impl->result;
        d->lineBoxes = impl->lineBoxes;
        d->cursorPos = impl->cursorPos;
        d->logicalToVisualCursorPos = impl->logicalToVisualCursorPos;
        d->plainText = impl->plainText;
        d->isBidi = impl->isBidi;
        d->initialTextPosition = impl->initialTextPosition;
    }
}

void KoSvgTextShape::setMemento(const KoSvgTextShapeMementoSP memento)
{
    setMementoImpl(memento);
    notifyCursorPosChanged(0, 0);
    notifyMarkupChanged();
}

void KoSvgTextShape::setMemento(const KoSvgTextShapeMementoSP memento, int pos, int anchor)
{
    setMementoImpl(memento);
    notifyCursorPosChanged(pos, anchor);
    notifyMarkupChanged();
}

void KoSvgTextShape::debugParsing()
{
    qDebug() << "Tree size:" << KisForestDetail::size(d->textData);
    QString spaces;
    for (auto it = compositionBegin(d->textData); it != compositionEnd(d->textData); it++) {
        if (it.state() == KisForestDetail::Enter) {

            qDebug() << QString(spaces + "+") << it->text;
            qDebug() << QString(spaces + "|") << it->properties.convertToSvgTextAttributes();
            qDebug() << QString(spaces + "| PropertyType:") << it->properties.property(KoSvgTextProperties::KraTextStyleType).toString();
            qDebug() << QString(spaces + "| Fill set: ") << it->properties.hasProperty(KoSvgTextProperties::FillId);
            qDebug() << QString(spaces + "| Stroke set: ") << it->properties.hasProperty(KoSvgTextProperties::StrokeId);
            qDebug() << QString(spaces + "| Opacity: ") << it->properties.property(KoSvgTextProperties::Opacity);
            qDebug() << QString(spaces + "| PaintOrder: ") << it->properties.hasProperty(KoSvgTextProperties::PaintOrder);
            qDebug() << QString(spaces + "| Visibility set: ") << it->properties.hasProperty(KoSvgTextProperties::Visiblity);
            qDebug() << QString(spaces + "| TextPath set: ") << (!it->textPath.isNull());
            qDebug() << QString(spaces + "| Transforms set: ") << it->localTransformations;
            spaces.append(" ");
        }

        if (it.state() == KisForestDetail::Leave) {
            spaces.chop(1);
        }
    }
}

void KoSvgTextShape::setRelayoutBlocked(const bool disable)
{
    d->isLoading = disable;
}

bool KoSvgTextShape::relayoutIsBlocked() const
{
    return d->isLoading;
}

void KoSvgTextShape::setFontMatchingDisabled(const bool disable)
{
    d->disableFontMatching = disable;
}

bool KoSvgTextShape::fontMatchingDisabled() const
{
    return d->disableFontMatching;
}

void KoSvgTextShape::paint(QPainter &painter) const
{
    painter.save();
    KoSvgText::TextRendering textRendering = KoSvgText::TextRendering(textProperties().propertyOrDefault(KoSvgTextProperties::TextRenderingId).toInt());
    if (textRendering == KoSvgText::RenderingOptimizeSpeed || !painter.testRenderHint(QPainter::Antialiasing)) {
        // also apply antialiasing only if antialiasing is active on provided target QPainter
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
        d->paintTextDecoration(painter, rootBounds, this, KoSvgText::DecorationUnderline, textRendering);
        d->paintTextDecoration(painter, rootBounds, this, KoSvgText::DecorationOverline, textRendering);
        d->paintPaths(painter, rootBounds, this, d->result, textRendering, chunk, currentIndex);
        d->paintTextDecoration(painter, rootBounds, this, KoSvgText::DecorationLineThrough, textRendering);
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
    // do nothing! everything is painted in paint()
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
    QList<KoShapeStrokeModelSP> parentStrokes;
    for (auto it = d->textData.compositionBegin(); it != d->textData.compositionEnd(); it++) {
        if (it.state() == KisForestDetail::Enter) {
            if (it->properties.hasProperty(KoSvgTextProperties::StrokeId)) {
                parentStrokes.append(it->properties.property(KoSvgTextProperties::StrokeId).value<KoSvgText::StrokeProperty>().property);
            }
        } else {
            KoShapeStrokeModelSP stroke = parentStrokes.size() > 0? parentStrokes.last(): nullptr;
            QRectF bb = it->associatedOutline.boundingRect();
            QMap<KoSvgText::TextDecoration, QPainterPath> decorations = it->textDecorations;
            for (int i = 0; i < decorations.values().size(); ++i) {
                bb |= decorations.values().at(i).boundingRect();
            }
            if (!bb.isEmpty()) {
                if (stroke) {
                    KoInsets insets;
                    stroke->strokeInsets(this, insets);
                    result |= bb.adjusted(-insets.left, -insets.top, insets.right, insets.bottom);
                } else {
                    result |= bb;
                }
            }
            if (it->properties.hasProperty(KoSvgTextProperties::StrokeId)) {
                // reset stroke to use parent stroke.
                parentStrokes.pop_back();
            }
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

                pen.setColor(QColor(255, 0, 0, 128));
                pen.setStyle(Qt::DashDotDotLine);
                painter.setPen(pen);
                painter.drawLine(chunk.length.translated(lineBox.baselineTop));
                pen.setColor(QColor(0, 128, 0, 128));
                pen.setStyle(Qt::DashDotLine);
                painter.setPen(pen);
                painter.drawLine(chunk.length.translated(lineBox.baselineBottom));
            }
        }
    }
}

KoShape * KoSvgTextShape::textOutline() const
{
    KoShape *shape;
    int currentIndex = 0;
    if (!d->result.empty()) {
        shape = d->collectPaths(this, d->result, currentIndex);
    }

    return shape;
}

KoSvgTextShape::TextType KoSvgTextShape::textType() const
{
    KoSvgText::AutoValue inlineSize = d->textData.childBegin()->properties.propertyOrDefault(KoSvgTextProperties::InlineSizeId).value<KoSvgText::AutoValue>();
    if (!d->shapesInside.isEmpty()) {
        return TextType::TextInShape;
    } else if (!inlineSize.isAuto) {
        return TextType::InlineWrap;
    } else {
        bool textSpaceCollapse = false;
        for (auto it = d->textData.depthFirstTailBegin(); it != d->textData.depthFirstTailEnd(); it++) {
            KoSvgText::TextSpaceCollapse collapse = KoSvgText::TextSpaceCollapse(it->properties.propertyOrDefault(KoSvgTextProperties::TextCollapseId).toInt());
            if (collapse == KoSvgText::Collapse || collapse == KoSvgText::PreserveSpaces) {
                textSpaceCollapse = true;
                break;
            }
        }
        return textSpaceCollapse? TextType::PrePositionedText: TextType::PreformattedText;
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

void KoSvgTextShape::relayout() const
{
    d->relayout();
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
    Q_UNUSED(documentResources);
    debugFlake << "Create default svg text shape";

    KoSvgTextShape *shape = new KoSvgTextShape();
    shape->setShapeId(KoSvgTextShape_SHAPEID);
    shape->insertText(0, i18nc("Default text for the text shape", "Placeholder Text"));

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


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
#include <functional>

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

    ~KoSvgTextShapeMementoImpl() {
    }

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

struct KoSvgTextNodeIndex::Private {
    Private(KisForest<KoSvgTextContentElement>::child_iterator _textElement, KoShape *_textPath)
        :textElement(_textElement)
        , textPath(_textPath){}
    KisForest<KoSvgTextContentElement>::child_iterator textElement;
    // textPath is owned by its textshape.
    KoShape *textPath = nullptr;
};

// for the use in KoSvgTextShape::Private::createTextNodeIndex() only
KoSvgTextNodeIndex::KoSvgTextNodeIndex()
    : d() // Private is **not** initialized, to be initialized by the factory method
{
}

KoSvgTextNodeIndex KoSvgTextShape::Private::createTextNodeIndex(KisForest<KoSvgTextContentElement>::child_iterator textElement) const
{
    KoSvgTextNodeIndex index;
    index.d.reset(new KoSvgTextNodeIndex::Private(textElement, Private::textPathByName(textElement->textPathId, textPaths)));
    return index;
}

KoSvgTextNodeIndex::KoSvgTextNodeIndex(const KoSvgTextNodeIndex &rhs)
{
    d.reset(new KoSvgTextNodeIndex::Private(rhs.d->textElement, rhs.d->textPath));
}

KoSvgTextNodeIndex::~KoSvgTextNodeIndex() {

}

KoSvgTextProperties *KoSvgTextNodeIndex::properties(){
    if (!d) { return nullptr;}
    return &d->textElement->properties;
}

KoSvgText::TextOnPathInfo *KoSvgTextNodeIndex::textPathInfo() {
    if (!d) {
        qDebug() << "d not initialized...";
        return nullptr;
    }
    return &d->textElement->textPathInfo;
}

KoShape *KoSvgTextNodeIndex::textPath() {
    if (!d) { qDebug() << "d not initialized...";return nullptr;}
    return d->textPath;
}

KoSvgTextShape::KoSvgTextShape()
    : KoShape()
    , d(new Private)
{
    setShapeId(KoSvgTextShape_SHAPEID);
    d->shapeGroup->setTransformation(this->transformation());
    d->textData.insert(d->textData.childBegin(), KoSvgTextContentElement());
    d->internalShapesPainter->setUpdateFunction(std::bind(&KoSvgTextShape::updateAbsolute, this, std::placeholders::_1));
}

KoSvgTextShape::KoSvgTextShape(const KoSvgTextShape &rhs)
    : KoShape(rhs)
    , d(new Private(*rhs.d))
{
    setShapeId(KoSvgTextShape_SHAPEID);
    d->shapeGroup->setTransformation(this->transformation());
    d->internalShapesPainter->setUpdateFunction(std::bind(&KoSvgTextShape::updateAbsolute, this, std::placeholders::_1));
    Q_FOREACH(KoShape *shape, d->shapeGroup->shapes()) {
        shape->addDependee(this);
    }
}

KoSvgTextShape::~KoSvgTextShape()
{
    Q_FOREACH(KoShape *shape, internalShapeManager()->shapes()) {
        shape->removeDependee(this);
    }
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

    const QVector<ChangeType> transformationTypes = {PositionChanged, RotationChanged, ScaleChanged, ShearChanged, SizeChanged, GenericMatrixChange};

    qDebug() << shape << type;
    if ((d->shapesInside.contains(shape) || d->shapesSubtract.contains(shape) || d->textPaths.contains(shape))
            && (transformationTypes.contains(type)
                || type == ParameterChanged
                || type == ParentChanged
                || type == Deleted)) {
        if (type == Deleted) {
            if (d->shapesInside.contains(shape)) {
                d->shapesInside.removeAll(shape);
            }
            if (d->shapesSubtract.contains(shape)) {
                d->shapesSubtract.removeAll(shape);
            }
            if (d->textPaths.contains(shape)) {
                d->textPaths.removeAll(shape);
                // TODO: remove ID from relevant text content element.
            }
            if (d->shapeGroup && d->shapeGroup->shapes().contains(shape)) {
                d->shapeGroup->removeShape(shape);
            }
            d->updateInternalShapesList();
        }

        // Updates the contours and calls relayout.
        // Would be great if we could compress the updates here somehow...
        d->updateTextWrappingAreas();
        // NotifyChanged ensures that boundingRect() is called on this shape.
        this->notifyChanged();
        if (d->shapesSubtract.contains(shape)) {
            // Shape subtract will otherwise only
            // update it's own bounding rect.
            this->update();
        }
        qDebug() << "child updated";
    }
    if ((!shape || shape == this)) {
        qDebug() << "text updated";
        if ((type == ContentChanged)) {
            relayout();
        } else if (transformationTypes.contains(type) || type == ParentChanged) {
            qDebug() << "update transform" << this->absoluteTransformation();
            d->shapeGroup->setTransformation(this->absoluteTransformation());
        } else if (type == TextRunAroundChanged) {
            // Hack: we don't use runaround else where, so we're using it for padding and margin.
            qDebug() << "update wrapping areas";
            d->updateTextWrappingAreas();
        }
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
    QList<KisForest<KoSvgTextContentElement>::hierarchy_iterator> hierarchy;
    for (auto parentIt = KisForestDetail::hierarchyBegin(siblingCurrent(it));
         parentIt != KisForestDetail::hierarchyEnd(siblingCurrent(it)); parentIt++) {
        hierarchy.append(parentIt);
    }
    KoSvgTextProperties props = KoSvgTextProperties::defaultProperties();
    while (!hierarchy.isEmpty()) {
        auto it = hierarchy.takeLast();
        KoSvgTextProperties p = it->properties;
        p.inheritFrom(props, true);
        props = p;
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
                // The root text properties should be retrieved explicitly (either by using -1 as pos, or by calling textProperties()).
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
        if (properties.hasProperty(KoSvgTextProperties::ShapePaddingId)
                || properties.hasProperty(KoSvgTextProperties::ShapeMarginId)
                || removeProperties.contains(KoSvgTextProperties::ShapePaddingId)
                || removeProperties.contains(KoSvgTextProperties::ShapeMarginId)) {
            shapeChangedPriv(TextRunAroundChanged);
        } else {
            shapeChangedPriv(ContentChanged);
        }
        if (properties.hasProperty(KoSvgTextProperties::FillId)) {
            shapeChangedPriv(BackgroundChanged);
        }
        if (properties.hasProperty(KoSvgTextProperties::StrokeId)) {
            shapeChangedPriv(StrokeChanged);
        }
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

bool KoSvgTextShape::setCharacterTransformsOnRange(const int startPos, const int endPos, const QVector<QPointF> positions, const QVector<qreal> rotateDegrees, const bool deltaPosition)
{
    if ((startPos < 0 && startPos == endPos) || d->cursorPos.isEmpty()) {
        return false;
    }
    const int finalPos = d->cursorPos.size()-1;
    const int startIndex = d->cursorPos.at(qBound(0, qMin(startPos, endPos), finalPos)).index;
    int endIndex = d->cursorPos.at(qBound(0, qMax(startPos, endPos), finalPos)).index;
    if (startIndex == endIndex) {
        endIndex += 1;
        while(d->result.at(endIndex).middle) {
            endIndex += 1;
            if (endIndex > d->result.size()) break;
        }
    }

    bool changed = false;

    QVector<KoSvgText::CharTransformation> resolvedTransforms = Private::resolvedTransformsForTree(d->textData, !shapesInside().isEmpty(), true);
    Private::removeTransforms(d->textData, startIndex, endIndex-startIndex);
    QPointF totalStartDelta;
    QPointF anchorAbsolute;
    QPointF anchorCssPos;

    const KoSvgText::Direction dir = KoSvgText::Direction(this->textProperties().propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());
    const KoSvgText::TextAnchor anchor = KoSvgText::TextAnchor(this->textProperties().propertyOrDefault(KoSvgTextProperties::TextAnchorId).toInt());

    const CharacterResult startRes = d->result.value(startIndex);
    const QPointF startAdvance = startRes.cursorInfo.rtl? startRes.advance: QPointF();

    if (deltaPosition) {
        for (int i = 0; i< startIndex; i++) {
            KoSvgText::CharTransformation tf = resolvedTransforms.value(i);
            if (tf.xPos) {
                totalStartDelta.setX(0);
                anchorAbsolute.setX(*tf.xPos);
            }
            if (tf.yPos) {
                totalStartDelta.setY(0);
                anchorAbsolute.setY(*tf.yPos);
            }
            if (tf.startsNewChunk()) {
                const CharacterResult res = d->result.value(i);
                anchorCssPos = res.cssPosition;
            }
            totalStartDelta += tf.relativeOffset();
        }
    } else {
        bool rtl = (dir == KoSvgText::DirectionRightToLeft);
        QPointF positionAtVisualEnd = (rtl? d->result.first(): d->result.last()).finalPosition;
        if (anchor == KoSvgText::AnchorEnd) {
            anchorAbsolute = positionAtVisualEnd;
        } else if (anchor == KoSvgText::AnchorMiddle) {
            anchorAbsolute = positionAtVisualEnd/2;
        }
    }

    int currentIndex = 0;
    QPointF accumulatedOffset;
    for (auto it = d->textData.depthFirstTailBegin(); it != d->textData.depthFirstTailEnd(); it++) {
        if (KoSvgTextShape::Private::childCount(siblingCurrent(it)) > 0) {
            continue;
            // TODO: also skip textpaths, their transforms aren't read.
        }

        int endContentElement = it->finalResultIndex;

        if (endContentElement >= startIndex && currentIndex <= endIndex) {

            QVector<KoSvgText::CharTransformation> transforms;
            int addressableOffset = 0;
            for (int i = currentIndex; (i < endContentElement); i++) {
                const CharacterResult res = d->result.value(i);
                if (!res.addressable) {
                    addressableOffset += 1;
                    continue;
                }

                const int transformIndex = (i - startIndex) - addressableOffset;
                KoSvgText::CharTransformation tf = resolvedTransforms.value(i, KoSvgText::CharTransformation());

                // Function to get the delta position.
                auto getDelta = [res, totalStartDelta, accumulatedOffset, anchorAbsolute, anchorCssPos, startAdvance] (QPointF pos) -> QPointF {
                    QPointF delta = pos - (res.textPathAndAnchoringOffset + anchorAbsolute + res.textLengthOffset);
                    delta -= (totalStartDelta + accumulatedOffset + (res.cssPosition-anchorCssPos) + startAdvance);
                    return delta;
                };
                // Function to get absolute position.
                auto getAbsolute = [res, tf, anchorAbsolute] (QPointF pos) -> QPointF {
                    QPointF p = pos - (res.textPathAndAnchoringOffset - anchorAbsolute) - tf.relativeOffset();
                    return p;
                };

                if (i < startIndex) {
                    if (!deltaPosition) {
                        // Because we don't split the text content element, we need to set the absolute pos for every preceding transform.
                        const QPointF p = getAbsolute(res.finalPosition);
                        if (!tf.xPos) {
                            tf.xPos = p.x();
                        }
                        if (!tf.yPos) {
                            tf.yPos = p.y();
                        }
                    }
                    transforms << tf;
                    continue;
                }

                if (i >= endIndex) {
                    if (i == endIndex) {
                        // Counter transform to keep unselected characters at the same pos.
                        if (deltaPosition && !tf.startsNewChunk()) {
                            QPointF delta = getDelta(res.finalPosition);
                            tf.dxPos = delta.x();
                            tf.dyPos = delta.y();
                        } else {
                            const QPointF p = getAbsolute(res.finalPosition) - anchorAbsolute;
                            tf.xPos = p.x();
                            tf.yPos = p.y();
                        }
                    }
                    transforms << tf;
                    continue;
                }

                if (rotateDegrees.size()+startIndex > i) {
                    tf.rotate = kisDegreesToRadians(rotateDegrees.value(transformIndex, tf.rotate? *tf.rotate: rotateDegrees.last()));
                }
                if (positions.size()+startIndex > i) {
                    const QPointF pos = positions.value(transformIndex, QPointF());

                    if (deltaPosition) {
                        if (tf.startsNewChunk()) {
                            anchorAbsolute = tf.absolutePos();
                            totalStartDelta = tf.relativeOffset();
                            anchorCssPos = res.cssPosition;
                        }

                        QPointF delta = getDelta(pos);
                        tf.dxPos = delta.x();
                        tf.dyPos = delta.y();

                        if (tf.startsNewChunk()) {
                            accumulatedOffset = QPointF();
                            totalStartDelta = tf.relativeOffset();
                        } else {
                            accumulatedOffset += tf.relativeOffset();
                        }
                    } else {
                        const QPointF delta = getAbsolute(pos);
                        tf.xPos = delta.x();
                        tf.yPos = delta.y();
                        accumulatedOffset = pos - res.finalPosition;
                    }

                }

                transforms << tf;
            }
            it->localTransformations = transforms;
            changed = true;
        }
        currentIndex = it->finalResultIndex;
        if (currentIndex > endIndex) {
            break;
        }
    }
    const CharacterResult res = d->result.last();

    if (changed) {
        KoSvgTextShape::Private::cleanUp(d->textData);
        notifyChanged();
        shapeChangedPriv(ContentChanged);
    }

    return changed;
}

KoSvgTextCharacterInfo infoFromCharacterResult(const CharacterResult &res, const int index) {
    KoSvgTextCharacterInfo info;
    info.finalPos = res.finalPosition;
    info.rotateDeg = kisRadiansToDegrees(res.rotate);
    info.visualIndex = res.visualIndex;
    info.middle = res.middle;
    info.advance = res.advance;
    info.logicalIndex = index;
    info.rtl = res.cursorInfo.rtl;
    info.metrics = res.metrics;
    return info;
}

QList<KoSvgTextCharacterInfo> KoSvgTextShape::getPositionsAndRotationsForRange(const int startPos, const int endPos) const
{
    QList<KoSvgTextCharacterInfo> infos;
    if ((startPos < 0 && startPos == endPos) || d->cursorPos.isEmpty()) {
        return infos;
    }
    const int finalPos = d->cursorPos.size()-1;
    const int startIndex = d->cursorPos.at(qBound(0, qMin(startPos, endPos), finalPos)).index;
    const int endIndex = d->cursorPos.at(qBound(0, qMax(startPos, endPos), finalPos)).index;

    for (int i = startIndex; i < endIndex; i++) {
        CharacterResult res = d->result.value(i);
        if (!res.addressable) continue;
        infos << infoFromCharacterResult(res, i);
    }

    if (endIndex == startIndex) {
        CharacterResult resFinal = d->result.value(startIndex);
        if (resFinal.addressable) {
            infos << infoFromCharacterResult(resFinal, startIndex);
        }
    }

    return infos;
}

void KoSvgTextShape::removeTransformsFromRange(const int startPos, const int endPos)
{
    if ((startPos < 0 && startPos == endPos) || d->cursorPos.isEmpty()) {
        return;
    }
    const int finalPos = d->cursorPos.size()-1;
    const int startIndex = d->cursorPos.at(qBound(0, qMin(startPos, endPos), finalPos)).index;
    const int endIndex = d->cursorPos.at(qBound(0, qMax(startPos, endPos), finalPos)).index;

    d->removeTransforms(d->textData, startIndex, endIndex-startIndex);

    KoSvgTextShape::Private::cleanUp(d->textData);
    notifyChanged();
    shapeChangedPriv(ContentChanged);
}

KisForest<KoSvgTextContentElement>::child_iterator findNodeIndexForPropertyIdImpl(KisForest<KoSvgTextContentElement>::child_iterator parent, KoSvgTextProperties::PropertyId propertyId) {
    for (auto child = KisForestDetail::childBegin(parent); child != KisForestDetail::childEnd(parent); child++) {
        if (child->properties.hasProperty(propertyId)) {
            return child;
        } else if (KisForestDetail::childBegin(child) != KisForestDetail::childEnd(child)) {
            auto found = findNodeIndexForPropertyIdImpl(child, propertyId);
            if (found != child) {
                return found;
            }
        }
    }
    return parent;
}

KoSvgTextNodeIndex KoSvgTextShape::findNodeIndexForPropertyId(KoSvgTextProperties::PropertyId propertyId)
{
    for (auto it = d->textData.childBegin(); it != d->textData.childEnd(); it++) {
        if (it->properties.hasProperty(propertyId)) {
            return d->createTextNodeIndex(it);
        } else if (KisForestDetail::childBegin(it) != KisForestDetail::childEnd(it)) {
            auto found = findNodeIndexForPropertyIdImpl(it, propertyId);
            if (found != it) {
                return d->createTextNodeIndex(found);
            }
        }
    }
    return d->createTextNodeIndex(d->textData.childBegin());
}

QPair<int, int> KoSvgTextShape::findRangeForNodeIndex(const KoSvgTextNodeIndex &node) const
{
    int startIndex = 0;
    int endIndex = 0;
    for (auto child = d->textData.childBegin(); child != d->textData.childEnd(); child++) {
        // count children
        d->startIndexOfIterator(child, node.d->textElement, startIndex);
        endIndex = d->numChars(node.d->textElement) + startIndex;
    }
    return qMakePair(posForIndex(startIndex), posForIndex(endIndex));
}

KoSvgTextNodeIndex KoSvgTextShape::topLevelNodeForPos(int pos) const
{
    auto candidate = d->textData.childBegin();
    if (d->isLoading || d->cursorPos.isEmpty()) return d->createTextNodeIndex(candidate);
    if (childBegin(d->textData.childBegin()) != childEnd(d->textData.childBegin())) {
        candidate = childBegin(d->textData.childBegin());
    }
    const int finalPos = d->cursorPos.size() - 1;
    const int index = d->cursorPos.at(qBound(0, pos, finalPos)).index;
    int currentIndex = 0;

    auto e = Private::findTextContentElementForIndex(d->textData, currentIndex, index, true);
    if (e == d->textData.depthFirstTailEnd()) {
        return d->createTextNodeIndex(candidate);
    }

    auto element = KisForestDetail::siblingCurrent(e);
    auto parent = Private::findTopLevelParent(d->textData.childBegin(), element);
    if (parent == childEnd(d->textData.childBegin())) return d->createTextNodeIndex(candidate);

    return d->createTextNodeIndex(parent);
}

KoSvgTextNodeIndex KoSvgTextShape::nodeForTextPath(KoShape *textPath) const
{
    auto root = d->textData.childBegin();
    if (d->isLoading || d->cursorPos.isEmpty()) return d->createTextNodeIndex(root);

    for (auto child = childBegin(root); child != childEnd(root); child++) {
        if (child->textPathId == textPath->name()) {
            return d->createTextNodeIndex(child);
        }
    }
    return d->createTextNodeIndex(root);
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
#include "SvgWriter.h"
bool KoSvgTextShape::saveSvg(SvgSavingContext &context)
{
    bool success = false;

    QList<KoShape*> visibleShapes;
    Q_FOREACH(KoShape *shape, d->internalShapes()) {
        if (shape->isVisible(false)) {
            visibleShapes.append(shape);
        }
    }
    const bool writeGroup = !(visibleShapes.isEmpty() || context.strippedTextMode());
    if (writeGroup) {
        context.shapeWriter().startElement("g", false);
        context.shapeWriter().addAttribute("id", context.createUID("group"));
        context.shapeWriter().addAttribute(KoSvgTextShape_TEXTCONTOURGROUP, "true");

        SvgUtil::writeTransformAttributeLazy("transform", transformation(), context.shapeWriter());
        SvgWriter writer(visibleShapes);
        writer.saveDetached(context);
    }
    for (auto it = d->textData.compositionBegin(); it != d->textData.compositionEnd(); it++) {
        if (it.state() == KisForestDetail::Enter) {
            bool isTextPath = false;
            QMap<QString, QString> shapeSpecificStyles;
            if (!it->textPathId.isEmpty()) {
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

                    if (visibleShapes.isEmpty()) {
                        SvgUtil::writeTransformAttributeLazy("transform", transformation(), context.shapeWriter());
                    }
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
                SvgStyleWriter::saveSvgBasicStyle(it->properties.property(KoSvgTextProperties::Visibility, true).toBool(),
                                                  it->properties.property(KoSvgTextProperties::Opacity, 0).toReal(),
                                                  it->properties.property(KoSvgTextProperties::PaintOrder,
                                                                          QVariant::fromValue(paintOrder())
                                                                          ).value<QVector<KoShape::PaintOrder>>(),
                                                  !it->properties.hasProperty(KoSvgTextProperties::PaintOrder), context, true);

            }

            KoShape *textPath = KoSvgTextShape::Private::textPathByName(it->textPathId, d->textPaths);
            success = it->saveSvg(context,
                                  it == d->textData.compositionBegin(),
                                  d->childCount(siblingCurrent(it)) == 0,
                                  shapeSpecificStyles,
                                  textPath);
        } else {
            if (it == d->textData.compositionBegin()) {
                SvgStyleWriter::saveMetadata(this, context);
            }
            context.shapeWriter().endElement();
        }
    }
    if (writeGroup) {
        context.shapeWriter().endElement();
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
    // TODO: add an assert that all linked shpaes in memento are present in
    // the current state of d->textPaths. That is the responsibility of
    // KoSvgTextAddRemoveShapeCommandImpl to prepare the shapes for us

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

        // Ensure that any text paths exist.
        auto root = d->textData.childBegin();
        for (auto child = childBegin(root); child != childEnd(root); child++) {
            if (!child->textPathId.isEmpty()) {
                KIS_SAFE_ASSERT_RECOVER(Private::textPathByName(child->textPathId, d->textPaths)) {
                    qDebug() << "missing path is" << child->textPathId;
                    child->textPathId = QString();
                }
            }
        }
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
    const bool shapeOffsetBefore = (textProperties().hasProperty(KoSvgTextProperties::ShapeMarginId)
                                    || textProperties().hasProperty(KoSvgTextProperties::ShapePaddingId));
    setMementoImpl(memento);
    const bool shapeOffsetAfter = (textProperties().hasProperty(KoSvgTextProperties::ShapeMarginId)
                                   || textProperties().hasProperty(KoSvgTextProperties::ShapePaddingId));
    if (shapeOffsetBefore || shapeOffsetAfter) {
        d->updateTextWrappingAreas();
    }
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
            qDebug() << QString(spaces + "| Visibility set: ") << it->properties.hasProperty(KoSvgTextProperties::Visibility);
            qDebug() << QString(spaces + "| TextPath set: ") << it->textPathId;
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

QList<QPainterPath> KoSvgTextShape::generateTextAreas(const QList<KoShape *> shapesInside, const QList<KoShape *> shapesSubtract, const KoSvgTextProperties &props)
{
    return Private::generateShapes(shapesInside, shapesSubtract, props);
}

void KoSvgTextShape::paint(QPainter &painter) const
{
    painter.save();

    painter.setTransform(d->shapeGroup->absoluteTransformation().inverted()*painter.transform());
    d->internalShapesPainter->paint(painter);
    painter.restore();

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

    painter.restore();
}

void KoSvgTextShape::paintStroke(QPainter &painter) const
{
    Q_UNUSED(painter);
    // do nothing! everything is painted in paint()
}

QPainterPath KoSvgTextShape::outline() const {
    QPainterPath result;
    if (!d->internalShapes().isEmpty()) {
        Q_FOREACH(KoShape *shape, d->internalShapes()) {
            result.addPath(shape->transformation().map(shape->outline()));
        }
    }
    if ((d->shapesInside.isEmpty() && d->shapesSubtract.isEmpty())) {
        for (auto it = d->textData.depthFirstTailBegin(); it != d->textData.depthFirstTailEnd(); it++) {
            result.addPath(it->associatedOutline);
            for (int i = 0; i < it->textDecorations.values().size(); ++i) {
                result.addPath(it->textDecorations.values().at(i));
            }
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
    QRectF shapesRect;
    if (d->internalShapesPainter->contentRect().isValid()) {
        shapesRect = d->internalShapesPainter->contentRect();
        if (!(d->shapesInside.isEmpty() && d->shapesSubtract.isEmpty())) {
            return shapesRect;
        }
    }
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

    return (this->absoluteTransformation().mapRect(result) | shapesRect);
}

QSizeF KoSvgTextShape::size() const
{
    // TODO: check if KoShape::m_d->size is consistent, check cache in KoShapeGroup::size()
    return outlineRect().size();
}

void KoSvgTextShape::setSize(const QSizeF &size)
{
    const QRectF oRect = this->outlineRect();
    const QSizeF oldSize = oRect.size();

    if (size == oldSize) return;

    // don't try to divide by zero
    if (oldSize.isEmpty()) return;

    const qreal scaleX = size.width() / oldSize.width();
    const qreal scaleY = size.height() / oldSize.height();

    if (d->internalShapes().isEmpty()) {
        /**
         * We don't have any contours, just scale (and distort) the text,
         * as if "Scale Styles" were activated
         */

        const qreal scaleX = size.width() / oldSize.width();
        const qreal scaleY = size.height() / oldSize.height();

        this->scale(scaleX, scaleY);
        // TODO: use scaling function for kosvgtextproperties when styles presets are merged.
        notifyChanged();
        shapeChangedPriv(ScaleChanged);
    } else {
        const bool allInternalShapeAreTranslatedOnly = [this] () {
            Q_FOREACH(KoShape *shape, d->internalShapes()) {
                if (shape->transformation().type() > QTransform::TxTranslate) {
                    return false;
                }
            }
            return true;
        }();

        if (allInternalShapeAreTranslatedOnly) {
            /**
             * We have only one contour that has no transformations, so we can just pass
             * the resize to it and preserve all the contours intact
             */
            Q_FOREACH (KoShape *internalShape, d->internalShapes()) {
                /**
                 * When resizing the shapes via setSize() we expect the method to
                 * scale the shapes bluntly in the parent's coordinate system. So,
                 * when resizing embedded shapes we should just anchor them to the
                 * origin of the text-shape's coordinate system.
                 */
                const QPointF stillPoint = this->absoluteTransformation().map(QPointF());
                const bool useGlobalMode = false;
                const bool usePostScaling = false;
                KoFlake::resizeShapeCommon(internalShape,
                                           scaleX,
                                           scaleY,
                                           stillPoint,
                                           useGlobalMode,
                                           usePostScaling,
                                           QTransform());
            }

            const QSizeF realNewSize = outlineRect().size();
            KoShape::setSize(realNewSize);
        } else {
            /**
             * When we have transformed (scaled, rotated and etc.) shapes
             * internally, we cannot pass resize action to them, because
             * it is impossible to mimic the parent resize by mere resizing
             * the child shapes. We would have to add shear to them [1].
             *
             * That is why we just add a scaling transform to every contour
             * shape while **keeping the text unscaled**. Keeping the text
             * unscaled covers the case when the user want to simply
             * transform a text balloon without contours.
             *
             * [1] Proof of the "impossible to pass the resize" claim
             *
             * That comes from the matrix equation to the child-parent
             * transformation. Here is an example for a rotated child:
             *
             * Original transformation is:
             *
             * parentPoint = childPoint * ChildRotate,
             *
             * now apply Scale to both sides, to simulate scale in the parent's
             * coordinate system:
             *
             * parentPoint * Scale = childPoint * ChildRotate * Scale,
             *
             * now we need to preserve the original child's transformation
             * to be intact, which was `ChildRotate`. To achieve that,
             * add (ChildRotate.inverted() * ChildRotate) at the end of the
             * equation:
             *
             * parentPoint * Scale = childPoint * ChildRotate * Scale * (ChildRotate.inverted() * ChildRotate),
             *
             * now regroup the matrices:
             *
             * parentPoint * Scale = childPoint * (ChildRotate * Scale * ChildRotate.inverted()) * ChildRotate,
             *
             * notice matrix (ChildRotate * Scale * ChildRotate.inverted()) is the one
             * that we should apply to the child shape in setSize() to preserve
             * its transform. This matrix can not be a scale matrix, it will
             * always recieve shear components for any non-trivial ChildRotate
             * values.
             */

            const QTransform scale = QTransform::fromScale(scaleX, scaleY);

            Q_FOREACH (KoShape *shape, d->internalShapes()) {
                shape->setTransformation(shape->transformation() * scale);
            }

            const QSizeF realNewSize = outlineRect().size();
            KoShape::setSize(realNewSize);
        }
    }
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

        //Debug shape outlines.
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
    KoShape *shape = nullptr;
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
            if (!it->localTransformations.isEmpty()) {
                textSpaceCollapse = true;
                break;
            }
        }
        return textSpaceCollapse? TextType::PrePositionedText: TextType::PreformattedText;
    }
    return TextType::PrePositionedText;
}

void KoSvgTextShape::setShapesInside(QList<KoShape *> shapesInside)
{
    removeShapesFromContours(d->shapesInside, false);
    addShapeContours(shapesInside, true);
}

void KoSvgTextShape::addShapeContours(QList<KoShape *> shapes, const bool inside)
{
    Q_FOREACH(KoShape *shape, shapes) {
        if (d->textPaths.contains(shape)) {
            d->removeTextPathId(d->textData.childBegin(), shape->name());
            d->cleanUp(d->textData);
            d->textPaths.removeAll(shape);
        }

        if (inside) {
            if (d->shapesSubtract.contains(shape)) {
                d->shapesSubtract.removeAll(shape);
            }
            d->shapesInside.append(shape);
        } else {
            if (d->shapesInside.contains(shape)) {
                d->shapesInside.removeAll(shape);
            }
            d->shapesSubtract.append(shape);
        }
        if (!d->shapeGroup->shapes().contains(shape)) {
            d->shapeGroup->addShape(shape);
            shape->addDependee(this);
        }
    }
    notifyChanged(); // notify shape manager that our geometry has changed
    d->updateTextWrappingAreas();
    d->updateInternalShapesList();
    shapeChangedPriv(ContentChanged);
    update();
}

bool KoSvgTextShape::shapeInContours(KoShape *shape)
{
    return (shape->parent() == d->shapeGroup.data());
}

void KoSvgTextShape::removeShapesFromContours(QList<KoShape *> shapes, bool callUpdate, bool cleanup)
{
    Q_FOREACH(KoShape *shape, shapes) {
        if (shape) {
            d->removeTextPathId(d->textData.childBegin(), shape->name());
            shape->removeDependee(this);
            d->shapesInside.removeAll(shape);
            d->shapesSubtract.removeAll(shape);
            d->textPaths.removeAll(shape);

        }
        d->shapeGroup->removeShape(shape);
    }
    if (cleanup) {
        d->cleanUp(d->textData);
    }
    if (callUpdate) {
        notifyChanged(); // notify shape manager that our geometry has changed
        d->updateTextWrappingAreas();
        d->updateInternalShapesList();
        shapeChangedPriv(ContentChanged);
        update();
    }
}

void KoSvgTextShape::moveShapeInsideToIndex(KoShape *shapeInside, const int index)
{
    const int oldIndex = d->shapesInside.indexOf(shapeInside);
    if (oldIndex < 0) return;

    // Update.
    d->shapesInside.move(oldIndex, index);
    d->updateTextWrappingAreas();
    d->updateInternalShapesList();
    shapeChangedPriv(ContentChanged);
    update();
}

bool KoSvgTextShape::setTextPathOnRange(KoShape *textPath, const int startPos, const int endPos)
{
    const int finalPos = d->cursorPos.size() - 1;
    const int startIndex = (startPos == endPos && startPos < 0)? 0: d->cursorPos.at(qBound(0, startPos, finalPos)).index;
    const int endIndex = (startPos == endPos && startPos < 0)? finalPos: d->cursorPos.at(qBound(0, endPos, finalPos)).index;

    Private::splitTree(d->textData, startIndex, false);
    Private::splitTree(d->textData, endIndex, true);
    int currentIndex = 0;

    Private::makeTextPathNameUnique(d->textPaths, textPath);
    KoSvgTextContentElement textPathElement;
    textPathElement.textPathId = textPath->name();
    d->shapeGroup->addShape(textPath);
    d->textPaths.append(textPath);
    textPath->addDependee(this);


    if (KisForestDetail::depth(d->textData) == 1) {
        textPathElement.text = d->textData.childBegin()->text;
        QList<KoSvgTextProperties::PropertyId> copyIds = {KoSvgTextProperties::TextDecorationLineId,
                                                          KoSvgTextProperties::TextDecorationColorId,
                                                          KoSvgTextProperties::TextDecorationStyleId};
        Q_FOREACH(KoSvgTextProperties::PropertyId p, copyIds) {
            if (d->textData.childBegin()->properties.hasProperty(KoSvgTextProperties::TextDecorationLineId)) {
                textPathElement.properties.setProperty(p, d->textData.childBegin()->properties.property(p));
                d->textData.childBegin()->properties.removeProperty(p);
            }
        }

        d->textData.childBegin()->text = QString();
        d->textData.insert(childEnd(d->textData.childBegin()), textPathElement);
    } else {
        // find nodes
        auto startElement = Private::findTextContentElementForIndex(d->textData, currentIndex, startIndex, true);
        currentIndex = 0;
        auto endElement = Private::findTextContentElementForIndex(d->textData, currentIndex, endIndex, true);

        auto first = startElement.node()?Private::findTopLevelParent(d->textData.childBegin(), KisForestDetail::siblingCurrent(startElement))
                                         : childEnd(d->textData.childBegin());
        auto last = endElement.node()? Private::findTopLevelParent(d->textData.childBegin(), KisForestDetail::siblingCurrent(endElement))
                                     : childEnd(d->textData.childBegin());

        // move children. we collect them before inserting the text path,
        // so we don't get iterator issues.
        KisForest<KoSvgTextContentElement> textPathTree;
        auto textPathIt = textPathTree.insert(
                    textPathTree.childEnd(),
                    textPathElement);
        QVector<KisForest<KoSvgTextContentElement>::child_iterator> movableChildren;
        for (auto child = first;
             (child != last && child != childEnd(d->textData.childBegin()));
             child++) {
            if (!child->textPathId.isEmpty()) {
                Q_FOREACH(KoShape *shape, d->textPaths) {
                    if (shape->name() == child->textPathId) {
                        removeShapesFromContours({shape}, false, false);
                        break;
                    }
                }
                child->textPathId = QString();
            }
            movableChildren.append(child);
        }
        while (!movableChildren.isEmpty()) {
            auto child = movableChildren.takeLast();
            textPathTree.move(child, KisForestDetail::childBegin(textPathIt));
        }
        d->textData.move(textPathIt, last);
    }
    Private::cleanUp(d->textData);

    d->updateInternalShapesList();
    notifyChanged();
    shapeChangedPriv(ContentChanged);
    update();
    return true;
}

QList<KoShape *> KoSvgTextShape::textPathsAtRange(const int startPos, const int endPos)
{
    const int finalPos = d->cursorPos.size() - 1;
    const int startIndex = (startPos == endPos && startPos < 0)? 0: d->cursorPos.at(qBound(0, startPos, finalPos)).index;
    const int endIndex = (startPos == endPos && startPos < 0)? finalPos: d->cursorPos.at(qBound(0, endPos, finalPos)).index;
    QList<KoShape*> textPaths;
    int currentIndex = 0;
    auto startElement = Private::findTextContentElementForIndex(d->textData, currentIndex, startIndex, true);
    currentIndex = 0;
    auto endElement = Private::findTextContentElementForIndex(d->textData, currentIndex, endIndex, true);

    auto first = startElement.node()? Private::findTopLevelParent(d->textData.childBegin(), KisForestDetail::siblingCurrent(startElement))
                                     : childEnd(d->textData.childBegin());
    auto last = endElement.node()? Private::findTopLevelParent(d->textData.childBegin(), KisForestDetail::siblingCurrent(endElement))
                                 : childEnd(d->textData.childBegin());
    if (last != childEnd(d->textData.childBegin())) {
        last++;
    }
    for (auto child = first; (child != last && child != KisForestDetail::siblingEnd(first)); child++) {
        if (KoShape *path = Private::textPathByName(child->textPathId, d->textPaths)) {
            textPaths.append(path);
        }
    }
    return textPaths;
}

void KoSvgTextShape::addTextPathAtEnd(KoShape *textPath)
{
    auto root = d->textData.childBegin();
    if (root == d->textData.childEnd()) return;
    if (d->textPaths.contains(textPath)) return;

    Private::makeTextPathNameUnique(d->textPaths, textPath);
    KoSvgTextContentElement textPathElement;
    textPathElement.textPathId = textPath->name();
    d->shapeGroup->addShape(textPath);
    d->textPaths.append(textPath);
    textPath->addDependee(this);

    d->textData.insert(childEnd(root), textPathElement);
}

QList<KoShape *> KoSvgTextShape::shapesInside() const
{
    return d->shapesInside;
}

void KoSvgTextShape::setShapesSubtract(QList<KoShape *> shapesSubtract)
{
    removeShapesFromContours(d->shapesSubtract, false);
    addShapeContours(shapesSubtract, false);
}

QList<QPainterPath> KoSvgTextShape::textWrappingAreas() const
{
    return d->currentTextWrappingAreas;
}

QList<KoShape *> KoSvgTextShape::shapesSubtract() const
{
    return d->shapesSubtract;
}

KoShapeManager *KoSvgTextShape::internalShapeManager() const
{
    return d->internalShapesPainter->internalShapeManager();
}

QMap<QString, QString> KoSvgTextShape::shapeTypeSpecificStyles(SvgSavingContext &context) const
{
    QMap<QString, QString> map = this->textProperties().convertParagraphProperties();
    if (!d->shapesInside.isEmpty()) {
        QStringList shapesInsideList;
        Q_FOREACH(KoShape* shape, d->shapesInside) {
            QString id = (shape->isVisible(false) && !context.strippedTextMode())? context.getID(shape): SvgStyleWriter::embedShape(shape, context);
            shapesInsideList.append(QString("url(#%1)").arg(id));
        }
        map.insert("shape-inside", shapesInsideList.join(" "));
        /// Right now we don't support showing glyphs outside the shape. Nor does Inkscape.
        /// Therefore being excplit about clipping these glyphs is preferable.
        map.insert("overflow", "clip");
    }
    if (!d->shapesSubtract.isEmpty()) {
        QStringList shapesInsideList;
        Q_FOREACH(KoShape* shape, d->shapesSubtract) {
            QString id = shape->isVisible(false)? context.getID(shape): SvgStyleWriter::embedShape(shape, context);
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


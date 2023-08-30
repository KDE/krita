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
    KoSvgText::WritingMode mode = KoSvgText::WritingMode(this->textProperties().propertyOrDefault(KoSvgTextProperties::WritingModeId).toInt());
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
    KoSvgText::WritingMode mode = KoSvgText::WritingMode(this->textProperties().propertyOrDefault(KoSvgTextProperties::WritingModeId).toInt());
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
    KoSvgText::WritingMode mode = KoSvgText::WritingMode(this->textProperties().propertyOrDefault(KoSvgTextProperties::WritingModeId).toInt());
    if (mode == KoSvgText::VerticalRL || mode == KoSvgText::VerticalLR) {
        return previousPos(pos, visual);
    } else {
        return previousLine(pos);
    }
}

int KoSvgTextShape::posDown(int pos, bool visual)
{
    KoSvgText::WritingMode mode = KoSvgText::WritingMode(this->textProperties().propertyOrDefault(KoSvgTextProperties::WritingModeId).toInt());
    if (mode == KoSvgText::VerticalRL || mode == KoSvgText::VerticalLR) {
        return nextPos(pos, visual);
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
    if (d->result.at(p.cluster).anchored_chunk) {
        return pos;
    }
    int candidate = 0;
    for (int i = 0; i < pos; i++) {
        CursorPos p2 = d->cursorPos.at(i);
        if (d->result.at(p2.cluster).anchored_chunk) {
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
    if (pos > d->cursorPos.size()-1) {
        return d->cursorPos.size() - 1;
    }
    int candidate = 0;
    for (int i = pos; i < d->cursorPos.size(); i++) {
        CursorPos p = d->cursorPos.at(i);
        if (d->result.at(p.cluster).anchored_chunk && i != pos) {
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

    if (nextLineStart > d->cursorPos.size()-1) {
        return d->cursorPos.size()-1;
    }

    QPointF currentPoint = d->result.at(d->cursorPos.at(pos).cluster).finalPosition;
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

    QPointF currentPoint = d->result.at(d->cursorPos.at(pos).cluster).finalPosition;
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
    if (pos == currentLineEnd) {
        currentLineEnd = lineEnd(pos+1);
    }

    int wordEnd = pos;
    bool breakNext = false;
    for (int i = pos; i<= currentLineEnd; i++) {
        wordEnd = i;
        if (breakNext) break;
        if (d->result.at(d->cursorPos.at(i).cluster).cursorInfo.isWordBoundary && i != pos) {
            breakNext = true;
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
    if (pos == currentLineStart) {
        currentLineStart = lineStart(pos-1);
    }

    int wordStart = pos;
    for (int i = pos; i >= currentLineStart; i--) {
        if (i == currentLineStart) {
            wordStart = i;
            break;
        }
        if (d->result.at(d->cursorPos.at(i).cluster).cursorInfo.isWordBoundary && i != pos && wordStart != pos) {
            break;
        }
        wordStart = i;
    }

    return wordStart;
}

QPainterPath KoSvgTextShape::defaultCursorShape()
{
    KoSvgText::WritingMode mode = KoSvgText::WritingMode(this->textProperties().propertyOrDefault(KoSvgTextProperties::WritingModeId).toInt());
    double fontSize = this->textProperties().propertyOrDefault(KoSvgTextProperties::FontSizeId).toReal();
    QPainterPath p;
    if (mode == KoSvgText::HorizontalTB) {
        p.moveTo(0, fontSize*0.2);
        p.lineTo(0, -fontSize);
    } else {
        p.moveTo(-fontSize * 0.5, 0);
        p.lineTo(fontSize, 0);
    }
    QVector<KoSvgText::CharTransformation> transforms = this->layoutInterface()->localCharTransformations();
    if (transforms.size()>0) {
        p.translate(transforms.first().absolutePos());
    }
    //TODO: We should handle the case for positioning on path or in shape.

    return p;
}

QPainterPath KoSvgTextShape::cursorForPos(int pos, double bidiFlagSize)
{
    if (d->result.isEmpty() || d->cursorPos.isEmpty() || pos < 0 || pos >= d->cursorPos.size()) {
        return defaultCursorShape();
    }
    QPainterPath p;

    CursorPos cursorPos = d->cursorPos.at(pos);

    CharacterResult res = d->result.at(cursorPos.cluster);

    const QTransform tf = res.finalTransform();
    QLineF caret = res.cursorInfo.caret;
    if (cursorPos.offset > 0 && cursorPos.offset-1 < res.cursorInfo.offsets.size()) {
        caret.translate(res.cursorInfo.offsets.at(cursorPos.offset-1));
    } else if (res.cursorInfo.rtl) {
        caret.translate(res.advance);
    }

    p.moveTo(tf.map(caret.p1()));
    p.lineTo(tf.map(caret.p2()));
    if (d->isBidi) {
        int sign = res.cursorInfo.rtl ? -1 : 1;
        double bidiFlagHalf = bidiFlagSize * 0.5;
        // TODO: handle top-to-bottom once text-orientation is tackled.
        QPointF point3 = QPointF(caret.p2().x() + (sign * bidiFlagHalf), caret.p2().y() + bidiFlagHalf);
        QPointF point4 = QPointF(point3.x() - (sign * bidiFlagHalf),point3.y() + bidiFlagHalf);
        p.lineTo(tf.map(point3));
        p.lineTo(tf.map(point4));
    }

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
    for (int i = start; i < end; i++) {
        CursorPos pos = d->cursorPos.at(i);
        CharacterResult res = d->result.at(pos.cluster);
        const QTransform tf = res.finalTransform();
        QLineF first = res.cursorInfo.caret;
        QLineF last = first;
        if (pos.offset > 0) {
            first.translate(res.cursorInfo.offsets.at(pos.offset-1));
        }

        if (res.cursorInfo.offsets.size() > 0 && pos.offset == 0) {
            last.translate(res.cursorInfo.offsets.at(0));
        } else if (pos.offset > 0 && pos.offset < res.cursorInfo.offsets.size()) {
            last.translate(res.cursorInfo.offsets.at(pos.offset));
        } else {
            last.translate(res.advance);
        }
        QPolygonF poly;
        poly << first.p1() << first.p2() << last.p2() << last.p1() << first.p1();
        p.addPolygon(tf.map(poly));
    }

    return p;
}

int KoSvgTextShape::posForPoint(QPointF point, int start, int end)
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
        if (pos.offset>0 && pos.offset-1 < res.cursorInfo.offsets.size()) {
            cursorStart += res.cursorInfo.offsets.at(pos.offset-1);
        } else if (res.cursorInfo.rtl) {
            cursorStart += res.advance;
        }
        double distance = kisDistance(cursorStart, point);
        if (distance < closest) {
            candidate = i;
            closest = distance;
        }

    }
    return candidate;
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
    int index = -1;
    if (d->cursorPos.isEmpty() || pos < 0) {
        return index;
    }

    return d->cursorPos.at(qMin(d->cursorPos.size()-1, pos)).index;
}

KoSvgTextChunkShape *findTextChunkForIndex(KoShape *rootShape, int &currentIndex, int sought, bool skipZeroWidth = false)
{
    KoSvgTextChunkShape *chunkShape = dynamic_cast<KoSvgTextChunkShape *>(rootShape);

    if (!chunkShape) {
        return nullptr;
    }

    if (!chunkShape->shapeCount()) {
        int length = chunkShape->layoutInterface()->numChars(false);
        if (length == 0 && skipZeroWidth) {
            return nullptr;
        }
        if (sought == currentIndex || (sought > currentIndex && sought < currentIndex + length)) {
            return chunkShape;
        } else {
            currentIndex += length;
        }
    } else {
        Q_FOREACH (KoShape *child, chunkShape->shapes()) {
            KoSvgTextChunkShape *shape = findTextChunkForIndex(child, currentIndex, sought, skipZeroWidth);
            if (shape) {
                return shape;
            }
        }
    }

    return nullptr;
}

bool KoSvgTextShape::insertText(int pos, QString text)
{
    bool succes = false;
    int currentIndex = 0;
    int index = 0;
    int oldIndex = 0;
    if (pos > -1 && !d->cursorPos.isEmpty()) {
        index = indexForPos(pos);
        oldIndex = index;
        index = qMin(index, d->result.size()-1);
    }
    KoSvgTextChunkShape *chunkShape = findTextChunkForIndex(this, currentIndex, index);
    if (chunkShape) {
        int offset = oldIndex - currentIndex;
        chunkShape->layoutInterface()->insertText(offset, text);
        notifyChanged();
        shapeChangedPriv(ContentChanged);
        succes = true;
    }
    return succes;
}

bool KoSvgTextShape::removeText(int pos, int length)
{
    bool succes = false;
    if (pos < -1 || d->cursorPos.isEmpty()) {
        return succes;
    }
    int index = indexForPos(pos);
    int currentLength = length;
    while (currentLength > 0) {
        int currentIndex = 0;
        KoSvgTextChunkShape *chunkShape = findTextChunkForIndex(this, currentIndex, index, true);
        if (chunkShape) {
            int offset =  index - currentIndex;
            int size = chunkShape->layoutInterface()->numChars(false);
            chunkShape->layoutInterface()->removeText(offset, currentLength);
            currentLength -= (size - chunkShape->layoutInterface()->numChars(false));
            succes = true;
        } else {
            currentLength = -1;
        }
    }
    if (succes) {
        notifyChanged();
        shapeChangedPriv(ContentChanged);
    }
    return succes;
}

QString KoSvgTextShape::plainText()
{
    return d->plainText;
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

void KoSvgTextShape::paintDebug(QPainter &painter, const DebugElements elements) const
{
    if (elements & DebugElement::CharBbox) {
        QPainterPath chunk;
        int currentIndex = 0;
        if (!d->result.isEmpty()) {
            QPainterPath rootBounds;
            rootBounds.addRect(this->outline().boundingRect());
            d->paintDebug(painter, rootBounds, this, d->result, chunk, currentIndex);
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
    d->relayout(this);
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

    if (rect.type()==QVariant::RectF) {
        shapeRect = rect.toRectF();
    }

    KoSvgTextShapeMarkupConverter converter(shape);
    converter.convertFromSvg(svgText,
                             defs,
                             shapeRect,
                             documentResources->documentResolution());

    shape->setPosition(shapeRect.topLeft());

    return shape;
}

bool KoSvgTextShapeFactory::supports(const QDomElement &/*e*/, KoShapeLoadingContext &/*context*/) const
{
    return false;
}

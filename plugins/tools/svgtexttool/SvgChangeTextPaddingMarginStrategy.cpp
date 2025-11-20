/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "SvgChangeTextPaddingMarginStrategy.h"

#include "SvgTextTool.h"
#include <KoPathShape.h>
#include <KoPathSegment.h>

#include <KoSvgTextProperties.h>
#include <KisHandlePainterHelper.h>

#include "SvgTextMergePropertiesRangeCommand.h"

#include <kis_global.h>

SvgChangeTextPaddingMarginStrategy::SvgChangeTextPaddingMarginStrategy(SvgTextTool *tool, KoSvgTextShape *shape, const QPointF &clicked)
    : KoInteractionStrategy(tool)
    , m_shape(shape)
    , m_lastMousePos(clicked)
{
    KoSvgTextProperties props = m_shape->textProperties();
    props.inheritFrom(KoSvgTextProperties::defaultProperties(), true);

    const qreal shapePadding = props.propertyOrDefault(KoSvgTextProperties::ShapePaddingId).value<KoSvgText::CssLengthPercentage>().value;
    const qreal shapeMargin = props.propertyOrDefault(KoSvgTextProperties::ShapeMarginId).value<KoSvgText::CssLengthPercentage>().value;

    const QPointF padding(shapePadding+2, shapePadding+2);
    const QPointF margin(shapeMargin+2, shapeMargin+2);

    qreal minDistance = std::numeric_limits<qreal>::max();
    KoPathShape *candidate = nullptr;
    bool isPadding = false;
    Q_FOREACH(KoShape *shape, m_shape->shapesSubtract()) {
        KoPathShape *path = dynamic_cast<KoPathShape*>(shape);
        if (!path) continue;
        const QPointF mp = shape->documentToShape(clicked);
        const QRectF marginRect(mp - margin, mp + margin);

        Q_FOREACH(KoPathSegment segment, path->segmentsAt(marginRect)) {
            const qreal nearestT = segment.nearestPoint(mp);
            const QPointF nearestP = segment.pointAt(nearestT);
            const qreal distance = kisDistance(mp, nearestP) - shapeMargin;
            if (distance < minDistance) {
                candidate = path;
                minDistance = distance;
            }
        }
    }

    Q_FOREACH(KoShape *shape, m_shape->shapesInside()) {
        KoPathShape *path = dynamic_cast<KoPathShape*>(shape);
        if (!path) continue;
        const QPointF mp = shape->documentToShape(clicked);
        const QRectF paddingRect(mp - padding, mp + padding);

        qDebug() << "segments" << path->segmentsAt(paddingRect).size() << paddingRect;
        Q_FOREACH(KoPathSegment segment, path->segmentsAt(paddingRect)) {
            const qreal nearestT = segment.nearestPoint(mp);
            const QPointF nearestP = segment.pointAt(nearestT);
            const qreal distance = kisDistance(mp, nearestP) - shapePadding;
            if (distance < minDistance) {
                candidate = path;
                minDistance = distance;
                isPadding = true;
            }
        }
    }

    m_referenceShape = candidate;
    m_isPadding = isPadding;
}

SvgChangeTextPaddingMarginStrategy::~SvgChangeTextPaddingMarginStrategy()
{

}

std::optional<QPointF> SvgChangeTextPaddingMarginStrategy::hitTest(KoSvgTextShape *shape, const QPointF &mousePos, const qreal grabSensitivityInPts)
{
    if (!shape) return std::nullopt;
    const QList<QPainterPath> textAreas = shape->textWrappingAreas();
    if (textAreas.isEmpty()) return std::nullopt;

    const QPointF grab(grabSensitivityInPts, grabSensitivityInPts);
    const QRectF grabRect(mousePos-grab, mousePos+grab);

    Q_FOREACH(const QPainterPath area, shape->textWrappingAreas()) {
        KoPathShape *s = KoPathShape::createShapeFromPainterPath(area);
        if (!s) continue;
        s->setTransformation(shape->absoluteTransformation());
        KoPathSegment segment = s->segmentAtPoint(mousePos, grabRect);
        if (segment.isValid()) {
            qreal nearest = segment.nearestPoint(s->documentToShape(mousePos));
            return std::make_optional(segment.angleVectorAtParam(nearest));
        }
    }

    return std::nullopt;
}

QLineF getLine(QPointF mousePos, KoPathShape *referenceShape, bool isPadding) {
    if (!referenceShape) return QLineF();
    const bool hit = referenceShape->hitTest(mousePos);
    if ((!hit && isPadding) || (hit && !isPadding)) return QLineF();

    QPointF pos = referenceShape->documentToShape(mousePos);
    QLineF l(pos, pos);

    qreal minDistance = std::numeric_limits<qreal>::max();

    Q_FOREACH(KoPathSegment segment, referenceShape->segmentsAt(referenceShape->outlineRect().adjusted(-2, -2, 2, 2))) {
        const qreal nearestT = segment.nearestPoint(pos);
        const QPointF nearestP = segment.pointAt(nearestT);
        const qreal distance = kisDistance(pos, nearestP);
        if (distance < minDistance) {
            l.setP1(nearestP);
            minDistance = distance;
        }
    }
    if (l.length() < 0) {
        l.setLength(0);
    }
    return l;
}

KoSvgTextProperties getProperties(bool isPadding, QLineF line, KoSvgTextProperties previous = KoSvgTextProperties()) {
    KoSvgTextProperties::PropertyId propId = isPadding? KoSvgTextProperties::ShapePaddingId: KoSvgTextProperties::ShapeMarginId;
    KoSvgText::CssLengthPercentage length;
    length.value = line.length();
    previous.setProperty(propId, QVariant::fromValue(length));
    return previous;
}

void SvgChangeTextPaddingMarginStrategy::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (!(m_referenceShape && m_shape)) return;
    painter.save();
    KisHandlePainterHelper handlePainter =
            KoShape::createHandlePainterHelperView(&painter, m_shape, converter, handleRadius(), decorationThickness());
    handlePainter.setHandleStyle(KisHandleStyle::selectedPrimaryHandles());

    const QLineF line = getLine(m_lastMousePos, m_referenceShape, m_isPadding);
    const QTransform lineTf = m_referenceShape->absoluteTransformation() * m_shape->absoluteTransformation().inverted();
    handlePainter.drawConnectionLine(lineTf.map(line));

    KoSvgTextProperties props = getProperties(m_isPadding, line, m_shape->textProperties());
    QList<QPainterPath> areas = m_shape->generateTextAreas(m_shape->shapesInside(), m_shape->shapesSubtract(), props);
    Q_FOREACH(QPainterPath area, areas) {
        handlePainter.drawPath(area);
    }

    painter.restore();
}

void SvgChangeTextPaddingMarginStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    m_lastMousePos = mouseLocation;
}

KUndo2Command *SvgChangeTextPaddingMarginStrategy::createCommand()
{
    if (!(m_referenceShape && m_shape)) return nullptr;
    const QLineF l = getLine(m_lastMousePos, m_referenceShape, m_isPadding);
    KoSvgTextProperties props = getProperties(m_isPadding, l);
    return new SvgTextMergePropertiesRangeCommand(m_shape, props, -1, -1);
}

void SvgChangeTextPaddingMarginStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
}

/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "SvgTextOnPathDecorationHelper.h"

#include <KoSvgTextShape.h>
#include <KisHandlePainterHelper.h>
#include <KoViewConverter.h>
#include <QDebug>
#include <KoPathShape.h>
struct SvgTextOnPathDecorationHelper::Private {
    KoSvgTextShape *shape = nullptr;
    int pos = 0;

    qreal handleRadius = 7;
    qreal decorationThickness = 1;

    bool isHovered = false;
    bool isActive = false;

    QLineF getLineAnchorTextNode(KoSvgTextNodeIndex &index, qreal lineLength) {
        KoPathShape *s = dynamic_cast<KoPathShape*>(index.textPath());
        QPainterPath outline = s->transformation().map(s->outline());
        QLineF line;
        if (index.textPathInfo()->side == KoSvgText::TextPathSideRight) {
            outline = outline.toReversed();
        }
        qreal percent = index.textPathInfo()->startOffsetIsPercentage?
                    outline.percentAtLength(index.textPathInfo()->startOffset * 0.01 * outline.length()):
                    outline.percentAtLength(index.textPathInfo()->startOffset);
        if (s->isClosedSubpath(s->subpathCount()-1)) {
            percent = fmod(percent, 1.0);
        } else {
            percent = qBound(0.0, percent, 1.0);
        }
        line.setP1(outline.pointAtPercent(percent));
        line.setAngle(outline.angleAtPercent(percent) - 90);
        line.setLength(lineLength);
        return line;
    }
};

SvgTextOnPathDecorationHelper::SvgTextOnPathDecorationHelper(): d(new Private)
{

}

SvgTextOnPathDecorationHelper::~SvgTextOnPathDecorationHelper()
{

}

void SvgTextOnPathDecorationHelper::setPos(int pos)
{
    d->pos = pos;
}

void SvgTextOnPathDecorationHelper::setShape(KoSvgTextShape *shape)
{
    d->shape = shape;
}

void SvgTextOnPathDecorationHelper::setHandleRadius(qreal radius)
{
    d->handleRadius = radius;
}

void SvgTextOnPathDecorationHelper::setDecorationThickness(qreal thickness)
{
    d->decorationThickness = thickness;
}

bool SvgTextOnPathDecorationHelper::hitTest(QPointF mouseInPts, const QTransform viewToDocument)
{
    if (!d->shape) return false;
    KoSvgTextNodeIndex index = d->shape->topLevelNodeForPos(d->pos);
    if (!(index.textPath() && index.textPathInfo())) return false;
    QPointF handleInPts = viewToDocument.map(QPointF(d->handleRadius, d->handleRadius));

    QLineF line = d->getLineAnchorTextNode(index, handleInPts.x()*2);
    line = d->shape->absoluteTransformation().map(line);
    bool hit = (QLineF(line.p2(), mouseInPts).length() <= handleInPts.x()*2);
    d->isHovered = hit;
    return hit;
}

void SvgTextOnPathDecorationHelper::paint(QPainter *p, const KoViewConverter &converter)
{
    if (!d->shape) return;
    KoSvgTextNodeIndex index = d->shape->topLevelNodeForPos(d->pos);
    if (!(index.textPath() && index.textPathInfo())) return;

    QPointF handleInPts = converter.viewToDocument().map(QPointF(d->handleRadius, d->handleRadius));
    QLineF line = d->getLineAnchorTextNode(index, handleInPts.x()*3);
    p->save();
    KisHandlePainterHelper helper =
            KoShape::createHandlePainterHelperView(p, d->shape, converter, d->handleRadius, d->decorationThickness);

    if (d->isActive) {
        helper.setHandleStyle(KisHandleStyle::selectedPrimaryHandles());
    } else if (d->isHovered) {
        helper.setHandleStyle(KisHandleStyle::highlightedPrimaryHandles());
    } else {
        helper.setHandleStyle(KisHandleStyle::primarySelection());
    }

    helper.drawConnectionLine(line);
    helper.drawHandleCircle(line.p2());
    //qDebug() << line.p2();
    p->restore();
}

QRectF SvgTextOnPathDecorationHelper::decorationRect(const QTransform documentToView) const
{
    QRectF r;
    if (!d->shape) return r;
    KoSvgTextNodeIndex index = d->shape->topLevelNodeForPos(d->pos);
    if (!(index.textPath() && index.textPathInfo())) return r;
    QPointF handleInPts = documentToView.inverted().map(QPointF(d->handleRadius, d->handleRadius));

    QLineF line = d->getLineAnchorTextNode(index, handleInPts.x()*3);

    line = QTransform(d->shape->absoluteTransformation()*documentToView).map(line);
    r |= QRectF(line.p1()-QPointF(d->decorationThickness, d->decorationThickness)
                , line.p2()+QPointF(d->decorationThickness, d->decorationThickness));

    QPointF handle(d->handleRadius, d->handleRadius);
    r |= QRectF(line.p2() - handle, line.p2() + handle);

    //qDebug() << "decor rect" << r << line.p2();
    return r;
}

void SvgTextOnPathDecorationHelper::setStrategyActive(bool isActive)
{
    d->isActive = isActive;
}

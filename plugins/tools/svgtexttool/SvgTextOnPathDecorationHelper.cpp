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
    QPointF lastMousePos;

    QLineF getLineAnchorTextNode(KoSvgTextNodeIndex &index, qreal lineLength) {
        KoPathShape *s = dynamic_cast<KoPathShape*>(index.textPath());
        QPainterPath outline = s->transformation().map(s->outline());
        QLineF line;
        qreal percent = index.textPathInfo()->startOffsetIsPercentage?
                    index.textPathInfo()->startOffset * 0.01:
                    index.textPathInfo()->startOffset / outline.length();
        if (s->isClosedSubpath(s->subpathCount()-1)) {
            percent = fmod(percent, 1.0);
        } else {
            percent = qBound(0.0, percent, 1.0);
        }
        if (index.textPathInfo()->side == KoSvgText::TextPathSideRight) {
            percent = 1.0 - percent;
        }
        line.setP1(outline.pointAtPercent(percent));
        line.setAngle(outline.angleAtPercent(percent)-90.0);
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

bool SvgTextOnPathDecorationHelper::hitTest(QPointF mouseInPts)
{
    if (d->lastMousePos != mouseInPts) {
        d->lastMousePos = mouseInPts;
    }
    if (!d->shape) return false;
    KoSvgTextNodeIndex index = d->shape->topLevelNodeForPos(d->pos);
    if (!(index.textPath() && index.textPathInfo())) return false;

    QLineF line = d->getLineAnchorTextNode(index, 0);
    line = d->shape->absoluteTransformation().map(line);
    return (QLineF(line.p2(), mouseInPts).length() <= d->handleRadius);
}

void SvgTextOnPathDecorationHelper::paint(QPainter *p, const KoViewConverter &converter)
{
    if (!d->shape) return;
    KoSvgTextNodeIndex index = d->shape->topLevelNodeForPos(d->pos);
    if (!(index.textPath() && index.textPathInfo())) return;

    QLineF line = d->getLineAnchorTextNode(index, 0);
    p->save();
    KisHandlePainterHelper helper =
            KoShape::createHandlePainterHelperView(p, d->shape, converter, d->handleRadius, d->decorationThickness);
    // TODO: fix handle.
    if (QLineF(converter.documentToView().map(line.p2()), d->lastMousePos).length() <= converter.viewToDocumentX(d->handleRadius)) {
        helper.setHandleStyle(KisHandleStyle::highlightedPrimaryHandles());
    } else {
        helper.setHandleStyle(KisHandleStyle::secondarySelection());
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

    QLineF line = d->getLineAnchorTextNode(index, 0);
    line = documentToView.map(line);
    r |= QRectF(line.p1()-QPointF(d->decorationThickness, d->decorationThickness), line.p2()+QPointF(d->decorationThickness, d->decorationThickness));

    QPointF handle(d->handleRadius, d->handleRadius);
    r |= QRectF(line.p2() - handle, line.p2() + handle);

    //qDebug() << "decor rect" << r << line.p2();
    return r;
}

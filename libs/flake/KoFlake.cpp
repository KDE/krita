/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Jos van den Oever <jos@vandenoever.info>
 * SPDX-FileCopyrightText: 2009 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2010 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoFlake.h"
#include "KoShape.h"

#include <QGradient>
#include <math.h>
#include "kis_global.h"

QGradient *KoFlake::cloneGradient(const QGradient *gradient)
{
    if (! gradient)
        return 0;

    QGradient *clone = 0;

    switch (gradient->type()) {
    case QGradient::LinearGradient:
    {
        const QLinearGradient *lg = static_cast<const QLinearGradient*>(gradient);
        clone = new QLinearGradient(lg->start(), lg->finalStop());
        break;
    }
    case QGradient::RadialGradient:
    {
        const QRadialGradient *rg = static_cast<const QRadialGradient*>(gradient);
        clone = new QRadialGradient(rg->center(), rg->radius(), rg->focalPoint());
        break;
    }
    case QGradient::ConicalGradient:
    {
        const QConicalGradient *cg = static_cast<const QConicalGradient*>(gradient);
        clone = new QConicalGradient(cg->center(), cg->angle());
        break;
    }
    default:
        return 0;
    }

    clone->setCoordinateMode(gradient->coordinateMode());
    clone->setSpread(gradient->spread());
    clone->setStops(gradient->stops());

    return clone;
}

QGradient *KoFlake::mergeGradient(const QGradient *coordsSource, const QGradient *fillSource)
{
    QPointF start;
    QPointF end;
    QPointF focalPoint;

    switch (coordsSource->type()) {
    case QGradient::LinearGradient: {
        const QLinearGradient *lg = static_cast<const QLinearGradient*>(coordsSource);
        start = lg->start();
        focalPoint = start;
        end = lg->finalStop();
        break;
    }
    case QGradient::RadialGradient: {
        const QRadialGradient *rg = static_cast<const QRadialGradient*>(coordsSource);
        start = rg->center();
        end = start + QPointF(rg->radius(), 0);
        focalPoint = rg->focalPoint();
        break;
    }
    case QGradient::ConicalGradient: {
        const QConicalGradient *cg = static_cast<const QConicalGradient*>(coordsSource);

        start = cg->center();
        focalPoint = start;

        QLineF l (start, start + QPointF(1.0, 0));
        l.setAngle(cg->angle());
        end = l.p2();
        break;
    }
    default:
        return 0;
    }

    QGradient *clone = 0;

    switch (fillSource->type()) {
    case QGradient::LinearGradient:
        clone = new QLinearGradient(start, end);
        break;
    case QGradient::RadialGradient:
        clone = new QRadialGradient(start, kisDistance(start, end), focalPoint);
        break;
    case QGradient::ConicalGradient: {
        QLineF l(start, end);
        clone = new QConicalGradient(l.p1(), l.angle());
        break;
    }
    default:
        return 0;
    }

    clone->setCoordinateMode(fillSource->coordinateMode());
    clone->setSpread(fillSource->spread());
    clone->setStops(fillSource->stops());

    return clone;
}

QPointF KoFlake::toRelative(const QPointF &absolute, const QSizeF &size)
{
    return QPointF(size.width() == 0 ? 0: absolute.x() / size.width(),
                   size.height() == 0 ? 0: absolute.y() / size.height());
}

QPointF KoFlake::toAbsolute(const QPointF &relative, const QSizeF &size)
{
    return QPointF(relative.x() * size.width(), relative.y() * size.height());
}

#include <QTransform>
#include "kis_debug.h"
#include "kis_algebra_2d.h"

namespace {

qreal getScaleByPointsPair(qreal x1, qreal x2, qreal expX1, qreal expX2)
{
    static const qreal eps = 1e-10;

    const qreal diff = x2 - x1;
    const qreal expDiff = expX2 - expX1;

    return qAbs(diff) > eps ? expDiff / diff : 1.0;
}

void findMinMaxPoints(const QPolygonF &poly, int *minPoint, int *maxPoint, std::function<qreal(const QPointF&)> dimension)
{
    KIS_ASSERT_RECOVER_RETURN(minPoint);
    KIS_ASSERT_RECOVER_RETURN(maxPoint);

    qreal minValue = dimension(poly[*minPoint]);
    qreal maxValue = dimension(poly[*maxPoint]);

    for (int i = 0; i < poly.size(); i++) {
        const qreal value = dimension(poly[i]);

        if (value < minValue) {
            *minPoint = i;
            minValue = value;
        }

        if (value > maxValue) {
            *maxPoint = i;
            maxValue = value;
        }
    }
}

}


Qt::Orientation KoFlake::significantScaleOrientation(qreal scaleX, qreal scaleY)
{
    const qreal scaleXDeviation = qAbs(1.0 - scaleX);
    const qreal scaleYDeviation = qAbs(1.0 - scaleY);

    return scaleXDeviation > scaleYDeviation ? Qt::Horizontal : Qt::Vertical;
}

void KoFlake::scaleShape(KoShape *shape, qreal scaleX, qreal scaleY,
                          const QPointF &absoluteStillPoint,
                          const QTransform &postScalingCoveringTransform)
{
    const QTransform scale = QTransform::fromScale(scaleX, scaleY);
    QPointF localStillPoint = postScalingCoveringTransform.inverted().map(absoluteStillPoint);
    const QTransform localStillPointOffset = QTransform::fromTranslate(-localStillPoint.x(), -localStillPoint.y());

    shape->setTransformation( shape->transformation() *
                postScalingCoveringTransform.inverted() *
                localStillPointOffset *
                scale *
                localStillPointOffset.inverted() *
                postScalingCoveringTransform);
}

void KoFlake::scaleShapeGlobal(KoShape *shape, qreal scaleX, qreal scaleY,
                               const QPointF &absoluteStillPoint)
{
    const QTransform scale = QTransform::fromScale(scaleX, scaleY);
    const QTransform absoluteStillPointOffset = QTransform::fromTranslate(-absoluteStillPoint.x(), -absoluteStillPoint.y());

    const QTransform uniformGlobalTransform =
            shape->absoluteTransformation() *
            absoluteStillPointOffset *
            scale *
            absoluteStillPointOffset.inverted() *
            shape->absoluteTransformation().inverted() *
            shape->transformation();

    shape->setTransformation(uniformGlobalTransform);
}

void KoFlake::resizeShape(KoShape *shape, qreal scaleX, qreal scaleY,
                          const QPointF &absoluteStillPoint,
                          bool useGlobalMode)
{
    using namespace KisAlgebra2D;

    if (useGlobalMode) {
        const QTransform scale = QTransform::fromScale(scaleX, scaleY);
        const QTransform uniformGlobalTransform =
                shape->absoluteTransformation() *
                scale *
                shape->absoluteTransformation().inverted();

        const QRectF rect = shape->outlineRect();

        /**
         * The basic idea of such global scaling:
         *
         * 1) We choose two the most distant points of the original outline rect
         * 2) Calculate their expected position if transformed using `uniformGlobalTransform`
         * 3) NOTE1: we do not transform the entire shape using `uniformGlobalTransform`,
         *           because it will cause massive shearing. We transform only two points
         *           and adjust other points using dumb scaling.
         * 4) NOTE2: given that `scale` transform is much more simpler than
         *           `uniformGlobalTransform`, we cannot guarantee equivalent changes on
         *           both globalScaleX and globalScaleY at the same time. We can guarantee
         *           only one of them. Therefore we select the most "important" axis and
         *           guarantee scael along it. The scale along the other direction is not
         *           controlled.
         * 5) After we have the two most distant points, we can just calculate the scale
         *    by dividing difference between their expected and original positions. This
         *    formula can be derived from equation:
         *
         *    localPoint_i * ScaleMatrix = localPoint_i * UniformGlobalTransform = expectedPoint_i
         */

        // choose the most significant scale direction
        Qt::Orientation significantOrientation = significantScaleOrientation(scaleX, scaleY);

        std::function<qreal(const QPointF&)> dimension;

        if (significantOrientation == Qt::Horizontal) {
            dimension = [] (const QPointF &pt) {
                return pt.x();
            };

        } else {
            dimension = [] (const QPointF &pt) {
                return pt.y();
            };
        }

        // find min and max points (in absolute coordinates),
        // by default use top-left and bottom-right
        QPolygonF localPoints(rect);
        QPolygonF globalPoints = shape->absoluteTransformation().map(localPoints);

        int minPointIndex = 0;
        int maxPointIndex = 2;

        findMinMaxPoints(globalPoints, &minPointIndex, &maxPointIndex, dimension);

        // calculate the scale using the extremum points
        const QPointF minPoint = localPoints[minPointIndex];
        const QPointF maxPoint = localPoints[maxPointIndex];

        const QPointF minPointExpected = uniformGlobalTransform.map(minPoint);
        const QPointF maxPointExpected = uniformGlobalTransform.map(maxPoint);

        scaleX = getScaleByPointsPair(minPoint.x(), maxPoint.x(),
                                      minPointExpected.x(), maxPointExpected.x());
        scaleY = getScaleByPointsPair(minPoint.y(), maxPoint.y(),
                                      minPointExpected.y(), maxPointExpected.y());
    }

    const QSizeF oldSize(shape->size());
    const QSizeF newSize(oldSize.width() * qAbs(scaleX), oldSize.height() * qAbs(scaleY));

    const QTransform mirrorTransform = QTransform::fromScale(signPZ(scaleX), signPZ(scaleY));

    /**
     * NOTE: when resizing a shape we expect top-left corner in parent's
     *       coordinates to keep it's position.
     */

    shape->setSize(newSize);

    QPointF localStillPoint = shape->absoluteTransformation().inverted().map(absoluteStillPoint);
    const QTransform localStillPointOffset = QTransform::fromTranslate(-localStillPoint.x(), -localStillPoint.y());
    const QSizeF realNewSize = shape->size();

    const QTransform realResizeTransform =
        QTransform::fromScale(oldSize.width() > 0 ? realNewSize.width() / oldSize.width() : 1.0,
                              oldSize.height() > 0 ? realNewSize.height() / oldSize.height() : 1.0);

    shape->setTransformation(realResizeTransform.inverted() *
                             localStillPointOffset *
                             realResizeTransform *
                             mirrorTransform *
                             localStillPointOffset.inverted() *
                             shape->transformation()
                             );
}

void KoFlake::resizeShapeCommon(KoShape *shape, qreal scaleX, qreal scaleY,
                          const QPointF &absoluteStillPoint,
                          bool useGlobalMode,
                          bool usePostScaling, const QTransform &postScalingCoveringTransform)
{
    if (usePostScaling) {
        if (!useGlobalMode) {
            scaleShape(shape, scaleX, scaleY, absoluteStillPoint, postScalingCoveringTransform);
        } else {
            scaleShapeGlobal(shape, scaleX, scaleY, absoluteStillPoint);
        }
    } else {
        resizeShape(shape, scaleX, scaleY, absoluteStillPoint, useGlobalMode);
    }
}

QPointF KoFlake::anchorToPoint(AnchorPosition anchor, const QRectF rect, bool *valid)
{
    static QVector<QPointF> anchorTable;

    if (anchorTable.isEmpty()) {
        anchorTable << QPointF(0.0,0.0);
        anchorTable << QPointF(0.5,0.0);
        anchorTable << QPointF(1.0,0.0);

        anchorTable << QPointF(0.0,0.5);
        anchorTable << QPointF(0.5,0.5);
        anchorTable << QPointF(1.0,0.5);

        anchorTable << QPointF(0.0,1.0);
        anchorTable << QPointF(0.5,1.0);
        anchorTable << QPointF(1.0,1.0);
    }

    if (valid)
        *valid = false;

    switch(anchor)
    {
        case AnchorPosition::TopLeft:
        case AnchorPosition::Top:
        case AnchorPosition::TopRight:
        case AnchorPosition::Left:
        case AnchorPosition::Center:
        case AnchorPosition::Right:
        case AnchorPosition::BottomLeft:
        case AnchorPosition::Bottom:
        case AnchorPosition::BottomRight:
            if (valid)
                *valid = true;
            return KisAlgebra2D::relativeToAbsolute(anchorTable[int(anchor)], rect);
        default:
            KIS_SAFE_ASSERT_RECOVER_NOOP(anchor >= AnchorPosition::TopLeft && anchor < AnchorPosition::NumAnchorPositions);
            return rect.topLeft();
    }
}

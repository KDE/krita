/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006-2008, 2010-2011 Thorsten Zachmann <zachmann@kde.org>
   SPDX-FileCopyrightText: 2006-2011 Jan Hambrecht <jaham@gmx.net>
   SPDX-FileCopyrightText: 2007-2009 Thomas Zander <zander@kde.org>
   SPDX-FileCopyrightText: 2011 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "KoPathShape.h"
#include "KoPathShape_p.h"

#include "KoPathSegment.h"
#include "KoPathPoint.h"
#include "KoShapeStrokeModel.h"
#include "KoPathShapeLoader.h"
#include "KoShapeSavingContext.h"
#include "KoShapeLoadingContext.h"
#include "KoShapeShadow.h"
#include "KoShapeBackground.h"
#include "KoShapeContainer.h"
#include "KoFilterEffectStack.h"
#include "KoMarker.h"
#include "KoShapeStroke.h"
#include "KoInsets.h"

#include <KoUnit.h>
#include "KisQPainterStateSaver.h"

#include <FlakeDebug.h>
#include <QPainter>
#include <QPainterPath>

#include "kis_global.h"

#include <qnumeric.h> // for qIsNaN
static bool qIsNaNPoint(const QPointF &p) {
    return qIsNaN(p.x()) || qIsNaN(p.y());
}

KoPathShape::Private::Private()
    : fillRule(Qt::OddEvenFill)
    , autoFillMarkers(false)
{
}

KoPathShape::Private::Private(const Private &rhs)
    : fillRule(rhs.fillRule)
    , markersNew(rhs.markersNew)
    , autoFillMarkers(rhs.autoFillMarkers)
{
}

QRectF KoPathShape::Private::handleRect(const QPointF &p, qreal radius) const
{
    return QRectF(p.x() - radius, p.y() - radius, 2*radius, 2*radius);
}


KoPathShape::KoPathShape()
    : KoTosContainer()
    , d(new Private)
{
}

KoPathShape::KoPathShape(const KoPathShape &rhs)
    : KoTosContainer(rhs)
    , d(new Private(*rhs.d))
{
    // local data cannot be shared via QSharedData because
    // every path point holds a pointer to the parent shape
    KoSubpathList subpaths;
    Q_FOREACH (KoSubpath *subPath, rhs.d->subpaths) {
        KoSubpath *clonedSubPath = new KoSubpath();

        Q_FOREACH (KoPathPoint *point, *subPath) {
            *clonedSubPath << new KoPathPoint(*point, this);
        }

        subpaths << clonedSubPath;
    }
    d->subpaths = subpaths;
}

KoPathShape::~KoPathShape()
{
    clear();
}

KoShape *KoPathShape::cloneShape() const
{
    return new KoPathShape(*this);
}

void KoPathShape::clear()
{
    Q_FOREACH (KoSubpath *subpath, d->subpaths) {
        Q_FOREACH (KoPathPoint *point, *subpath)
            delete point;
        delete subpath;
    }
    d->subpaths.clear();

    notifyPointsChanged();
}

void KoPathShape::paint(QPainter &painter, KoShapePaintingContext &paintContext) const
{
    KisQPainterStateSaver saver(&painter);

    QPainterPath path(outline());
    path.setFillRule(d->fillRule);

    if (background()) {
        background()->paint(painter, paintContext, path);
    }
    //d->paintDebug(painter);
}


#ifndef NDEBUG
void KoPathShape::Private::paintDebug(QPainter &painter)
{
    KoSubpathList::const_iterator pathIt(subpaths.constBegin());
    int i = 0;

    QPen pen(Qt::black, 0);
    painter.save();
    painter.setPen(pen);
    for (; pathIt != subpaths.constEnd(); ++pathIt) {
        KoSubpath::const_iterator it((*pathIt)->constBegin());
        for (; it != (*pathIt)->constEnd(); ++it) {
            ++i;
            KoPathPoint *point = (*it);
            QRectF r(point->point(), QSizeF(5, 5));
            r.translate(-2.5, -2.5);
            QPen pen(Qt::black, 0);
            painter.setPen(pen);
            if (point->activeControlPoint1() && point->activeControlPoint2()) {
                QBrush b(Qt::red);
                painter.setBrush(b);
            } else if (point->activeControlPoint1()) {
                QBrush b(Qt::yellow);
                painter.setBrush(b);
            } else if (point->activeControlPoint2()) {
                QBrush b(Qt::darkYellow);
                painter.setBrush(b);
            }
            painter.drawEllipse(r);
        }
    }
    painter.restore();
    debugFlake << "nop =" << i;
}

void KoPathShape::Private::debugPath() const
{
    KoSubpathList::const_iterator pathIt(subpaths.constBegin());
    for (; pathIt != subpaths.constEnd(); ++pathIt) {
        KoSubpath::const_iterator it((*pathIt)->constBegin());
        for (; it != (*pathIt)->constEnd(); ++it) {
            debugFlake << "p:" << (*pathIt) << "," << *it << "," << (*it)->point() << "," << (*it)->properties();
        }
    }
}
#endif

void KoPathShape::paintPoints(KisHandlePainterHelper &handlesHelper)
{
    KoSubpathList::const_iterator pathIt(d->subpaths.constBegin());

    for (; pathIt != d->subpaths.constEnd(); ++pathIt) {
        KoSubpath::const_iterator it((*pathIt)->constBegin());
        for (; it != (*pathIt)->constEnd(); ++it)
            (*it)->paint(handlesHelper, KoPathPoint::Node);
    }
}

QRectF KoPathShape::outlineRect() const
{
    return outline().boundingRect();
}

QPainterPath KoPathShape::outline() const
{
    QPainterPath path;
    for (auto subpathIt = d->subpaths.constBegin(); subpathIt != d->subpaths.constEnd(); ++subpathIt) {
        const KoSubpath * subpath = *subpathIt;
        const KoPathPoint * lastPoint = subpath->constFirst();
        bool activeCP = false;
        for (auto pointIt = subpath->constBegin(); pointIt != subpath->constEnd(); ++pointIt) {
            const KoPathPoint * currPoint = *pointIt;
            KoPathPoint::PointProperties currProperties = currPoint->properties();
            if (currPoint == subpath->constFirst()) {
                if (currProperties & KoPathPoint::StartSubpath) {
                    Q_ASSERT(!qIsNaNPoint(currPoint->point()));
                    path.moveTo(currPoint->point());
                }
            } else if (activeCP && currPoint->activeControlPoint1()) {
                Q_ASSERT(!qIsNaNPoint(lastPoint->controlPoint2()));
                Q_ASSERT(!qIsNaNPoint(currPoint->controlPoint1()));
                Q_ASSERT(!qIsNaNPoint(currPoint->point()));
                path.cubicTo(
                            lastPoint->controlPoint2(),
                            currPoint->controlPoint1(),
                            currPoint->point());
            } else if (activeCP || currPoint->activeControlPoint1()) {
                Q_ASSERT(!qIsNaNPoint(lastPoint->controlPoint2()));
                Q_ASSERT(!qIsNaNPoint(currPoint->controlPoint1()));
                path.quadTo(
                            activeCP ? lastPoint->controlPoint2() : currPoint->controlPoint1(),
                            currPoint->point());
            } else {
                Q_ASSERT(!qIsNaNPoint(currPoint->point()));
                path.lineTo(currPoint->point());
            }
            if (currProperties & KoPathPoint::CloseSubpath && currProperties & KoPathPoint::StopSubpath) {
                // add curve when there is a curve on the way to the first point
                KoPathPoint * firstPoint = subpath->first();
                Q_ASSERT(!qIsNaNPoint(firstPoint->point()));
                if (currPoint->activeControlPoint2() && firstPoint->activeControlPoint1()) {
                    path.cubicTo(
                                currPoint->controlPoint2(),
                                firstPoint->controlPoint1(),
                                firstPoint->point());
                }
                else if (currPoint->activeControlPoint2() || firstPoint->activeControlPoint1()) {
                    Q_ASSERT(!qIsNaNPoint(currPoint->point()));
                    Q_ASSERT(!qIsNaNPoint(currPoint->controlPoint1()));
                    path.quadTo(
                                currPoint->activeControlPoint2() ? currPoint->controlPoint2() : firstPoint->controlPoint1(),
                                firstPoint->point());
                }
                path.closeSubpath();
            }

            if (currPoint->activeControlPoint2()) {
                activeCP = true;
            } else {
                activeCP = false;
            }
            lastPoint = currPoint;
        }
    }

    return path;
}

QRectF KoPathShape::boundingRect() const
{
    const QTransform transform = absoluteTransformation();

    /**
     * First we approximate the insets of the stroke by rendering a fat bezier curve
     * with width set to the maximum inset of miters and markers. The are swept by this
     * curve will be a good approximation of the real curve bounding rect.
     */
    qreal outlineSweepWidth = 0;

    const QSharedPointer<KoShapeStroke> lineBorder = qSharedPointerDynamicCast<KoShapeStroke>(stroke());
    if (lineBorder) {
        outlineSweepWidth = lineBorder->lineWidth();
    }

    if (stroke()) {
        KoInsets inset;
        stroke()->strokeInsets(this, inset);
        const qreal maxInset = std::max({inset.left, inset.top, inset.right, inset.bottom});

        // insets extend outside the shape, but width extends both inside and outside,
        // so we should multiply insets by 2.0
        outlineSweepWidth = std::max({outlineSweepWidth,
                                      2.0 * maxInset,
                                      2.0 * stroke()->strokeMaxMarkersInset(this)});
    }



    /// NOTE: stroking the entire shape might be too expensive, so try to
    ///       estimate the bounds using insets only...

#if 0
    QPen pen(Qt::black, outlineSweepWidth);

    // select round joins and caps to ensure it sweeps exactly
    // 'outlineSweepWidth' pixels in every possible
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    QRectF bb = transform.map(pathStroke(pen)).boundingRect();
#endif

    // add 10% extra update area around the doubled insets
    QRectF bb = transform.mapRect(kisGrowRect(outline().boundingRect(), 1.1 * 0.5 * outlineSweepWidth));

    if (shadow()) {
        KoInsets insets;
        shadow()->insets(insets);
        bb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }
    if (filterEffectStack()) {
        QRectF clipRect = filterEffectStack()->clipRectForBoundingRect(QRectF(QPointF(), size()));
        bb |= transform.mapRect(clipRect);
    }
    return bb;
}

QSizeF KoPathShape::size() const
{
    // don't call boundingRect here as it uses absoluteTransformation
    // which itself uses size() -> leads to infinite recursion
    return outlineRect().size();
}

void KoPathShape::setSize(const QSizeF &newSize)
{
    QTransform matrix(resizeMatrix(newSize));

    KoShape::setSize(newSize);
    d->map(matrix);
}

QTransform KoPathShape::resizeMatrix(const QSizeF & newSize) const
{
    QSizeF oldSize = size();
    if (oldSize.width() == 0.0) {
        oldSize.setWidth(0.000001);
    }
    if (oldSize.height() == 0.0) {
        oldSize.setHeight(0.000001);
    }

    QSizeF sizeNew(newSize);
    if (sizeNew.width() == 0.0) {
        sizeNew.setWidth(0.000001);
    }
    if (sizeNew.height() == 0.0) {
        sizeNew.setHeight(0.000001);
    }

    return QTransform(sizeNew.width() / oldSize.width(), 0, 0, sizeNew.height() / oldSize.height(), 0, 0);
}

KoPathPoint * KoPathShape::moveTo(const QPointF &p)
{
    KoPathPoint * point = new KoPathPoint(this, p, KoPathPoint::StartSubpath | KoPathPoint::StopSubpath);
    KoSubpath * path = new KoSubpath;
    path->push_back(point);
    d->subpaths.push_back(path);
    notifyPointsChanged();
    return point;
}

KoPathPoint * KoPathShape::lineTo(const QPointF &p)
{
    if (d->subpaths.empty()) {
        moveTo(QPointF(0, 0));
    }
    KoPathPoint * point = new KoPathPoint(this, p, KoPathPoint::StopSubpath);
    KoPathPoint * lastPoint = d->subpaths.last()->last();
    updateLastPriv(&lastPoint);
    d->subpaths.last()->push_back(point);
    notifyPointsChanged();
    return point;
}

KoPathPoint * KoPathShape::curveTo(const QPointF &c1, const QPointF &c2, const QPointF &p)
{
    if (d->subpaths.empty()) {
        moveTo(QPointF(0, 0));
    }
    KoPathPoint * lastPoint = d->subpaths.last()->last();
    updateLastPriv(&lastPoint);
    lastPoint->setControlPoint2(c1);
    KoPathPoint * point = new KoPathPoint(this, p, KoPathPoint::StopSubpath);
    point->setControlPoint1(c2);
    d->subpaths.last()->push_back(point);
    notifyPointsChanged();
    return point;
}

KoPathPoint * KoPathShape::curveTo(const QPointF &c, const QPointF &p)
{
    if (d->subpaths.empty())
        moveTo(QPointF(0, 0));

    KoPathPoint * lastPoint = d->subpaths.last()->last();
    updateLastPriv(&lastPoint);
    lastPoint->setControlPoint2(c);
    KoPathPoint * point = new KoPathPoint(this, p, KoPathPoint::StopSubpath);
    d->subpaths.last()->push_back(point);
    notifyPointsChanged();
    return point;
}

KoPathPoint * KoPathShape::arcTo(qreal rx, qreal ry, qreal startAngle, qreal sweepAngle)
{
    if (d->subpaths.empty()) {
        moveTo(QPointF(0, 0));
    }

    KoPathPoint * lastPoint = d->subpaths.last()->last();
    if (lastPoint->properties() & KoPathPoint::CloseSubpath) {
        lastPoint = d->subpaths.last()->first();
    }
    QPointF startpoint(lastPoint->point());

    KoPathPoint * newEndPoint = lastPoint;

    QPointF curvePoints[12];
    int pointCnt = arcToCurve(rx, ry, startAngle, sweepAngle, startpoint, curvePoints);
    for (int i = 0; i < pointCnt; i += 3) {
        newEndPoint = curveTo(curvePoints[i], curvePoints[i+1], curvePoints[i+2]);
    }
    return newEndPoint;
}

int KoPathShape::arcToCurve(qreal rx, qreal ry, qreal startAngle, qreal sweepAngle, const QPointF & offset, QPointF * curvePoints) const
{
    int pointCnt = 0;

    // check Parameters
    if (sweepAngle == 0.0)
        return pointCnt;

    sweepAngle = qBound(-360.0, sweepAngle, 360.0);

    if (rx == 0 || ry == 0) {
        //TODO
    }

    // split angles bigger than 90° so that it gives a good approximation to the circle
    qreal parts = ceil(qAbs(sweepAngle / 90.0));

    qreal sa_rad = startAngle * M_PI / 180.0;
    qreal partangle = sweepAngle / parts;
    qreal endangle = startAngle + partangle;
    qreal se_rad = endangle * M_PI / 180.0;
    qreal sinsa = sin(sa_rad);
    qreal cossa = cos(sa_rad);
    qreal kappa = 4.0 / 3.0 * tan((se_rad - sa_rad) / 4);

    // startpoint is at the last point is the path but when it is closed
    // it is at the first point
    QPointF startpoint(offset);

    //center berechnen
    QPointF center(startpoint - QPointF(cossa * rx, -sinsa * ry));

    //debugFlake <<"kappa" << kappa <<"parts" << parts;

    for (int part = 0; part < parts; ++part) {
        // start tangent
        curvePoints[pointCnt++] = QPointF(startpoint - QPointF(sinsa * rx * kappa, cossa * ry * kappa));

        qreal sinse = sin(se_rad);
        qreal cosse = cos(se_rad);

        // end point
        QPointF endpoint(center + QPointF(cosse * rx, -sinse * ry));
        // end tangent
        curvePoints[pointCnt++] = QPointF(endpoint - QPointF(-sinse * rx * kappa, -cosse * ry * kappa));
        curvePoints[pointCnt++] = endpoint;

        // set the endpoint as next start point
        startpoint = endpoint;
        sinsa = sinse;
        cossa = cosse;
        endangle += partangle;
        se_rad = endangle * M_PI / 180.0;
    }

    return pointCnt;
}

void KoPathShape::close()
{
    if (d->subpaths.empty()) {
        return;
    }
    closeSubpathPriv(d->subpaths.last());
}

void KoPathShape::closeMerge()
{
    if (d->subpaths.empty()) {
        return;
    }
    closeMergeSubpathPriv(d->subpaths.last());
}

QPointF KoPathShape::normalize()
{
    QPointF tl(outline().boundingRect().topLeft());
    QTransform matrix;
    matrix.translate(-tl.x(), -tl.y());
    d->map(matrix);

    // keep the top left point of the object
    applyTransformation(matrix.inverted());
    shapeChangedPriv(ContentChanged);
    return tl;
}

void KoPathShape::Private::map(const QTransform &matrix)
{
    KoSubpathList::const_iterator pathIt(subpaths.constBegin());
    for (; pathIt != subpaths.constEnd(); ++pathIt) {
        KoSubpath::const_iterator it((*pathIt)->constBegin());
        for (; it != (*pathIt)->constEnd(); ++it) {
            // It's possible there are null points in the map...
            if (*it) {
                (*it)->map(matrix);
            }
        }
    }
}

void KoPathShape::updateLastPriv(KoPathPoint **lastPoint)
{
    // check if we are about to add a new point to a closed subpath
    if ((*lastPoint)->properties() & KoPathPoint::StopSubpath
            && (*lastPoint)->properties() & KoPathPoint::CloseSubpath) {
        // get the first point of the subpath
        KoPathPoint *subpathStart = d->subpaths.last()->first();
        // clone the first point of the subpath...
        KoPathPoint * newLastPoint = new KoPathPoint(*subpathStart, this);
        // ... and make it a normal point
        newLastPoint->setProperties(KoPathPoint::Normal);
        // now start a new subpath with the cloned start point
        KoSubpath *path = new KoSubpath;
        path->push_back(newLastPoint);
        d->subpaths.push_back(path);
        *lastPoint = newLastPoint;
    } else {
        // the subpath was not closed so the formerly last point
        // of the subpath is no end point anymore
        (*lastPoint)->unsetProperty(KoPathPoint::StopSubpath);
    }
    (*lastPoint)->unsetProperty(KoPathPoint::CloseSubpath);
}

QList<KoPathPoint*> KoPathShape::pointsAt(const QRectF &r) const
{
    QList<KoPathPoint*> result;

    KoSubpathList::const_iterator pathIt(d->subpaths.constBegin());
    for (; pathIt != d->subpaths.constEnd(); ++pathIt) {
        KoSubpath::const_iterator it((*pathIt)->constBegin());
        for (; it != (*pathIt)->constEnd(); ++it) {
            if (r.contains((*it)->point()))
                result.append(*it);
            else if ((*it)->activeControlPoint1() && r.contains((*it)->controlPoint1()))
                result.append(*it);
            else if ((*it)->activeControlPoint2() && r.contains((*it)->controlPoint2()))
                result.append(*it);
        }
    }
    return result;
}

QList<KoPathSegment> KoPathShape::segmentsAt(const QRectF &r) const
{
    QList<KoPathSegment> segments;
    int subpathCount = d->subpaths.count();
    for (int subpathIndex = 0; subpathIndex < subpathCount; ++subpathIndex) {
        KoSubpath * subpath = d->subpaths[subpathIndex];
        int pointCount = subpath->count();
        bool subpathClosed = isClosedSubpath(subpathIndex);
        for (int pointIndex = 0; pointIndex < pointCount; ++pointIndex) {
            if (pointIndex == (pointCount - 1) && ! subpathClosed)
                break;
            KoPathSegment s(subpath->at(pointIndex), subpath->at((pointIndex + 1) % pointCount));
            QRectF controlRect = s.controlPointRect();
            if (! r.intersects(controlRect) && ! controlRect.contains(r))
                continue;
            QRectF bound = s.boundingRect();
            if (! r.intersects(bound) && ! bound.contains(r))
                continue;

            segments.append(s);
        }
    }
    return segments;
}

KoPathPointIndex KoPathShape::pathPointIndex(const KoPathPoint *point) const
{
    for (int subpathIndex = 0; subpathIndex < d->subpaths.size(); ++subpathIndex) {
        KoSubpath * subpath = d->subpaths.at(subpathIndex);
        for (int pointPos = 0; pointPos < subpath->size(); ++pointPos) {
            if (subpath->at(pointPos) == point) {
                return KoPathPointIndex(subpathIndex, pointPos);
            }
        }
    }
    return KoPathPointIndex(-1, -1);
}

KoPathPoint * KoPathShape::pointByIndex(const KoPathPointIndex &pointIndex) const
{
    KoSubpath *subpath = d->subPath(pointIndex.first);

    if (subpath == 0 || pointIndex.second < 0 || pointIndex.second >= subpath->size())
        return 0;

    return subpath->at(pointIndex.second);
}

KoPathSegment KoPathShape::segmentByIndex(const KoPathPointIndex &pointIndex) const
{
    KoPathSegment segment(0, 0);

    KoSubpath *subpath = d->subPath(pointIndex.first);

    if (subpath != 0 && pointIndex.second >= 0 && pointIndex.second < subpath->size()) {
        KoPathPoint * point = subpath->at(pointIndex.second);
        int index = pointIndex.second;
        // check if we have a (closing) segment starting from the last point
        if ((index == subpath->size() - 1) && point->properties() & KoPathPoint::CloseSubpath)
            index = 0;
        else
            ++index;

        if (index < subpath->size()) {
            segment = KoPathSegment(point, subpath->at(index));
        }
    }
    return segment;
}

int KoPathShape::pointCount() const
{
    int i = 0;
    KoSubpathList::const_iterator pathIt(d->subpaths.constBegin());
    for (; pathIt != d->subpaths.constEnd(); ++pathIt) {
        i += (*pathIt)->size();
    }

    return i;
}

int KoPathShape::subpathCount() const
{
    return d->subpaths.count();
}

int KoPathShape::subpathPointCount(int subpathIndex) const
{
    KoSubpath *subpath = d->subPath(subpathIndex);

    if (subpath == 0)
        return -1;

    return subpath->size();
}

bool KoPathShape::isClosedSubpath(int subpathIndex) const
{
    KoSubpath *subpath = d->subPath(subpathIndex);

    if (subpath == 0)
        return false;

    const bool firstClosed = subpath->first()->properties() & KoPathPoint::CloseSubpath;
    const bool lastClosed = subpath->last()->properties() & KoPathPoint::CloseSubpath;

    return firstClosed && lastClosed;
}

bool KoPathShape::insertPoint(KoPathPoint* point, const KoPathPointIndex &pointIndex)
{
    KoSubpath *subpath = d->subPath(pointIndex.first);

    if (subpath == 0 || pointIndex.second < 0 || pointIndex.second > subpath->size())
        return false;

    KoPathPoint::PointProperties properties = point->properties();
    properties &= ~KoPathPoint::StartSubpath;
    properties &= ~KoPathPoint::StopSubpath;
    properties &= ~KoPathPoint::CloseSubpath;
    // check if new point starts subpath
    if (pointIndex.second == 0) {
        properties |= KoPathPoint::StartSubpath;
        // subpath was closed
        if (subpath->last()->properties() & KoPathPoint::CloseSubpath) {
            // keep the path closed
            properties |= KoPathPoint::CloseSubpath;
        }
        // old first point does not start the subpath anymore
        subpath->first()->unsetProperty(KoPathPoint::StartSubpath);
    }
    // check if new point stops subpath
    else if (pointIndex.second == subpath->size()) {
        properties |= KoPathPoint::StopSubpath;
        // subpath was closed
        if (subpath->last()->properties() & KoPathPoint::CloseSubpath) {
            // keep the path closed
            properties = properties | KoPathPoint::CloseSubpath;
        }
        // old last point does not end subpath anymore
        subpath->last()->unsetProperty(KoPathPoint::StopSubpath);
    }

    point->setProperties(properties);
    point->setParent(this);
    subpath->insert(pointIndex.second , point);
    notifyPointsChanged();

    return true;
}

KoPathPoint * KoPathShape::removePoint(const KoPathPointIndex &pointIndex)
{
    KoSubpath *subpath = d->subPath(pointIndex.first);

    if (subpath == 0 || pointIndex.second < 0 || pointIndex.second >= subpath->size())
        return 0;

    KoPathPoint * point = subpath->takeAt(pointIndex.second);
    point->setParent(0);

    //don't do anything (not even crash), if there was only one point
    if (pointCount()==0) {
        return point;
    }
    // check if we removed the first point
    else if (pointIndex.second == 0) {
        // first point removed, set new StartSubpath
        subpath->first()->setProperty(KoPathPoint::StartSubpath);
        // check if path was closed
        if (subpath->last()->properties() & KoPathPoint::CloseSubpath) {
            // keep path closed
            subpath->first()->setProperty(KoPathPoint::CloseSubpath);
        }
    }
    // check if we removed the last point
    else if (pointIndex.second == subpath->size()) { // use size as point is already removed
        // last point removed, set new StopSubpath
        subpath->last()->setProperty(KoPathPoint::StopSubpath);
        // check if path was closed
        if (point->properties() & KoPathPoint::CloseSubpath) {
            // keep path closed
            subpath->last()->setProperty(KoPathPoint::CloseSubpath);
        }
    }

    notifyPointsChanged();

    return point;
}

bool KoPathShape::breakAfter(const KoPathPointIndex &pointIndex)
{
    KoSubpath *subpath = d->subPath(pointIndex.first);

    if (!subpath || pointIndex.second < 0 || pointIndex.second > subpath->size() - 2
            || isClosedSubpath(pointIndex.first))
        return false;

    KoSubpath * newSubpath = new KoSubpath;

    int size = subpath->size();
    for (int i = pointIndex.second + 1; i < size; ++i) {
        newSubpath->append(subpath->takeAt(pointIndex.second + 1));
    }
    // now make the first point of the new subpath a starting node
    newSubpath->first()->setProperty(KoPathPoint::StartSubpath);
    // the last point of the old subpath is now an ending node
    subpath->last()->setProperty(KoPathPoint::StopSubpath);

    // insert the new subpath after the broken one
    d->subpaths.insert(pointIndex.first + 1, newSubpath);
    notifyPointsChanged();

    return true;
}

bool KoPathShape::join(int subpathIndex)
{
    KoSubpath *subpath = d->subPath(subpathIndex);
    KoSubpath *nextSubpath = d->subPath(subpathIndex + 1);

    if (!subpath || !nextSubpath || isClosedSubpath(subpathIndex)
            || isClosedSubpath(subpathIndex+1))
        return false;

    // the last point of the subpath does not end the subpath anymore
    subpath->last()->unsetProperty(KoPathPoint::StopSubpath);
    // the first point of the next subpath does not start a subpath anymore
    nextSubpath->first()->unsetProperty(KoPathPoint::StartSubpath);

    // append the second subpath to the first
    Q_FOREACH (KoPathPoint * p, *nextSubpath)
        subpath->append(p);

    // remove the nextSubpath from path
    d->subpaths.removeAt(subpathIndex + 1);

    // delete it as it is no longer possible to use it
    delete nextSubpath;

    notifyPointsChanged();

    return true;
}

bool KoPathShape::moveSubpath(int oldSubpathIndex, int newSubpathIndex)
{
    KoSubpath *subpath = d->subPath(oldSubpathIndex);

    if (subpath == 0 || newSubpathIndex >= d->subpaths.size())
        return false;

    if (oldSubpathIndex == newSubpathIndex)
        return true;

    d->subpaths.removeAt(oldSubpathIndex);
    d->subpaths.insert(newSubpathIndex, subpath);

    notifyPointsChanged();

    return true;
}

KoPathPointIndex KoPathShape::openSubpath(const KoPathPointIndex &pointIndex)
{
    KoSubpath *subpath = d->subPath(pointIndex.first);

    if (!subpath || pointIndex.second < 0 || pointIndex.second >= subpath->size()
            || !isClosedSubpath(pointIndex.first))
        return KoPathPointIndex(-1, -1);

    KoPathPoint * oldStartPoint = subpath->first();
    // the old starting node no longer starts the subpath
    oldStartPoint->unsetProperty(KoPathPoint::StartSubpath);
    // the old end node no longer closes the subpath
    subpath->last()->unsetProperty(KoPathPoint::StopSubpath);

    // reorder the subpath
    for (int i = 0; i < pointIndex.second; ++i) {
        subpath->append(subpath->takeFirst());
    }
    // make the first point a start node
    subpath->first()->setProperty(KoPathPoint::StartSubpath);
    // make the last point an end node
    subpath->last()->setProperty(KoPathPoint::StopSubpath);

    notifyPointsChanged();

    return pathPointIndex(oldStartPoint);
}

KoPathPointIndex KoPathShape::closeSubpath(const KoPathPointIndex &pointIndex)
{
    KoSubpath *subpath = d->subPath(pointIndex.first);

    if (!subpath || pointIndex.second < 0 || pointIndex.second >= subpath->size()
            || isClosedSubpath(pointIndex.first))
        return KoPathPointIndex(-1, -1);

    KoPathPoint * oldStartPoint = subpath->first();
    // the old starting node no longer starts the subpath
    oldStartPoint->unsetProperty(KoPathPoint::StartSubpath);
    // the old end node no longer ends the subpath
    subpath->last()->unsetProperty(KoPathPoint::StopSubpath);

    // reorder the subpath
    for (int i = 0; i < pointIndex.second; ++i) {
        subpath->append(subpath->takeFirst());
    }
    subpath->first()->setProperty(KoPathPoint::StartSubpath);
    subpath->last()->setProperty(KoPathPoint::StopSubpath);

    closeSubpathPriv(subpath);

    notifyPointsChanged();

    return pathPointIndex(oldStartPoint);
}

bool KoPathShape::reverseSubpath(int subpathIndex)
{
    KoSubpath *subpath = d->subPath(subpathIndex);

    if (subpath == 0)
        return false;

    int size = subpath->size();
    for (int i = 0; i < size; ++i) {
        KoPathPoint *p = subpath->takeAt(i);
        p->reverse();
        subpath->prepend(p);
    }

    // adjust the position dependent properties
    KoPathPoint *first = subpath->first();
    KoPathPoint *last = subpath->last();

    KoPathPoint::PointProperties firstProps = first->properties();
    KoPathPoint::PointProperties lastProps = last->properties();

    firstProps |= KoPathPoint::StartSubpath;
    firstProps &= ~KoPathPoint::StopSubpath;
    lastProps |= KoPathPoint::StopSubpath;
    lastProps &= ~KoPathPoint::StartSubpath;
    if (firstProps & KoPathPoint::CloseSubpath) {
        firstProps |= KoPathPoint::CloseSubpath;
        lastProps |= KoPathPoint::CloseSubpath;
    }
    first->setProperties(firstProps);
    last->setProperties(lastProps);

    notifyPointsChanged();

    return true;
}

KoSubpath * KoPathShape::removeSubpath(int subpathIndex)
{
    KoSubpath *subpath = d->subPath(subpathIndex);

    if (subpath != 0) {
        Q_FOREACH (KoPathPoint* point, *subpath) {
            point->setParent(this);
        }
        d->subpaths.removeAt(subpathIndex);
    }

    notifyPointsChanged();

    return subpath;
}

bool KoPathShape::addSubpath(KoSubpath * subpath, int subpathIndex)
{
    if (subpathIndex < 0 || subpathIndex > d->subpaths.size())
        return false;

    Q_FOREACH (KoPathPoint* point, *subpath) {
        point->setParent(this);
    }

    d->subpaths.insert(subpathIndex, subpath);
    notifyPointsChanged();


    return true;
}
int KoPathShape::combine(KoPathShape *path)
{
    int insertSegmentPosition = -1;
    if (!path) return insertSegmentPosition;

    QTransform pathMatrix = path->absoluteTransformation();
    QTransform myMatrix = absoluteTransformation().inverted();

    Q_FOREACH (KoSubpath* subpath, path->d->subpaths) {
        KoSubpath *newSubpath = new KoSubpath();

        Q_FOREACH (KoPathPoint* point, *subpath) {
            KoPathPoint *newPoint = new KoPathPoint(*point, this);
            newPoint->map(pathMatrix);
            newPoint->map(myMatrix);
            newSubpath->append(newPoint);
        }
        d->subpaths.append(newSubpath);

        if (insertSegmentPosition < 0) {
            insertSegmentPosition = d->subpaths.size() - 1;
        }
    }
    normalize();

    notifyPointsChanged();
    return insertSegmentPosition;
}

bool KoPathShape::separate(QList<KoPathShape*> & separatedPaths)
{
    if (! d->subpaths.size())
        return false;

    QTransform myMatrix = absoluteTransformation();

    Q_FOREACH (KoSubpath* subpath, d->subpaths) {
        KoPathShape *shape = new KoPathShape();

        shape->setStroke(stroke());
        shape->setBackground(background());
        shape->setShapeId(shapeId());
        shape->setZIndex(zIndex());

        KoSubpath *newSubpath = new KoSubpath();

        Q_FOREACH (KoPathPoint* point, *subpath) {
            KoPathPoint *newPoint = new KoPathPoint(*point, shape);
            newPoint->map(myMatrix);
            newSubpath->append(newPoint);
        }
        shape->d->subpaths.append(newSubpath);
        shape->normalize();

        // NOTE: shape cannot have any listeners yet, so no notification about
        //       points modification is needed

        separatedPaths.append(shape);
    }
    return true;
}

void KoPathShape::closeSubpathPriv(KoSubpath *subpath)
{
    if (! subpath)
        return;

    subpath->last()->setProperty(KoPathPoint::CloseSubpath);
    subpath->first()->setProperty(KoPathPoint::CloseSubpath);

    notifyPointsChanged();
}

void KoPathShape::closeMergeSubpathPriv(KoSubpath *subpath)
{
    if (! subpath || subpath->size() < 2)
        return;

    KoPathPoint * lastPoint = subpath->last();
    KoPathPoint * firstPoint = subpath->first();

    // check if first and last points are coincident
    if (lastPoint->point() == firstPoint->point()) {
        // we are removing the current last point and
        // reuse its first control point if active
        firstPoint->setProperty(KoPathPoint::StartSubpath);
        firstPoint->setProperty(KoPathPoint::CloseSubpath);
        if (lastPoint->activeControlPoint1())
            firstPoint->setControlPoint1(lastPoint->controlPoint1());
        // remove last point
        delete subpath->takeLast();
        // the new last point closes the subpath now
        lastPoint = subpath->last();
        lastPoint->setProperty(KoPathPoint::StopSubpath);
        lastPoint->setProperty(KoPathPoint::CloseSubpath);

        notifyPointsChanged();
    } else {
        closeSubpathPriv(subpath);
    }
}

const KoSubpathList &KoPathShape::subpaths() const
{
    return d->subpaths;
}

KoSubpathList &KoPathShape::subpaths()
{
    return d->subpaths;
}

void KoPathShape::map(const QTransform &matrix)
{
    return d->map(matrix);
}

KoSubpath *KoPathShape::Private::subPath(int subpathIndex) const
{
    if (subpathIndex < 0 || subpathIndex >= subpaths.size())
        return 0;

    return subpaths.at(subpathIndex);
}

QString KoPathShape::pathShapeId() const
{
    return KoPathShapeId;
}

QString KoPathShape::toString(const QTransform &matrix) const
{
    QString pathString;

    // iterate over all subpaths
    KoSubpathList::const_iterator pathIt(d->subpaths.constBegin());
    for (; pathIt != d->subpaths.constEnd(); ++pathIt) {
        KoSubpath::const_iterator pointIt((*pathIt)->constBegin());
        // keep a pointer to the first point of the subpath
        KoPathPoint *firstPoint(*pointIt);
        // keep a pointer to the previous point of the subpath
        KoPathPoint *lastPoint = firstPoint;
        // keep track if the previous point has an active control point 2
        bool activeControlPoint2 = false;

        // iterate over all points of the current subpath
        for (; pointIt != (*pathIt)->constEnd(); ++pointIt) {
            KoPathPoint *currPoint(*pointIt);
            if (!currPoint) {
                qWarning() << "Found a zero point in the shape's path!";
                continue;
            }
            // first point of subpath ?
            if (currPoint == firstPoint) {
                // are we starting a subpath ?
                if (currPoint->properties() & KoPathPoint::StartSubpath) {
                    const QPointF p = matrix.map(currPoint->point());
                    pathString += QString("M%1 %2").arg(p.x()).arg(p.y());
                }
            }
            // end point of curve segment ?
            else if (activeControlPoint2 || currPoint->activeControlPoint1()) {
                // check if we have a cubic or quadratic curve
                const bool isCubic = activeControlPoint2 && currPoint->activeControlPoint1();
                KoPathSegment cubicSeg = isCubic ? KoPathSegment(lastPoint, currPoint)
                                                 : KoPathSegment(lastPoint, currPoint).toCubic();
                if (cubicSeg.first()  && cubicSeg.second()) {
                    const QPointF cp1 = matrix.map(cubicSeg.first()->controlPoint2());
                    const QPointF cp2 = matrix.map(cubicSeg.second()->controlPoint1());
                    const QPointF p = matrix.map(cubicSeg.second()->point());
                    pathString += QString("C%1 %2 %3 %4 %5 %6")
                            .arg(cp1.x()).arg(cp1.y())
                            .arg(cp2.x()).arg(cp2.y())
                            .arg(p.x()).arg(p.y());
                }
            }
            // end point of line segment!
            else {
                const QPointF p = matrix.map(currPoint->point());
                pathString += QString("L%1 %2").arg(p.x()).arg(p.y());
            }
            // last point closes subpath ?
            if (currPoint->properties() & KoPathPoint::StopSubpath
                    && currPoint->properties() & KoPathPoint::CloseSubpath) {
                // add curve when there is a curve on the way to the first point
                if (currPoint->activeControlPoint2() || firstPoint->activeControlPoint1()) {
                    // check if we have a cubic or quadratic curve
                    const bool isCubic = currPoint->activeControlPoint2() && firstPoint->activeControlPoint1();
                    KoPathSegment cubicSeg = isCubic ? KoPathSegment(currPoint, firstPoint)
                                                     : KoPathSegment(currPoint, firstPoint).toCubic();
                    if (cubicSeg.first()  && cubicSeg.second()) {
                        const QPointF cp1 = matrix.map(cubicSeg.first()->controlPoint2());
                        const QPointF cp2 = matrix.map(cubicSeg.second()->controlPoint1());

                        const QPointF p = matrix.map(cubicSeg.second()->point());
                        pathString += QString("C%1 %2 %3 %4 %5 %6")
                                .arg(cp1.x()).arg(cp1.y())
                                .arg(cp2.x()).arg(cp2.y())
                                .arg(p.x()).arg(p.y());
                    }
                }
                pathString += QString("Z");
            }

            activeControlPoint2 = currPoint->activeControlPoint2();
            lastPoint = currPoint;
        }
    }

    return pathString;
}

char nodeType(const KoPathPoint * point)
{
    if (point->properties() & KoPathPoint::IsSmooth) {
        return 's';
    }
    else if (point->properties() & KoPathPoint::IsSymmetric) {
        return 'z';
    }
    else {
        return 'c';
    }
}

QString KoPathShape::nodeTypes() const
{
    QString types;
    KoSubpathList::const_iterator pathIt(d->subpaths.constBegin());
    for (; pathIt != d->subpaths.constEnd(); ++pathIt) {
        KoSubpath::const_iterator it((*pathIt)->constBegin());
        for (; it != (*pathIt)->constEnd(); ++it) {
            if (it == (*pathIt)->constBegin()) {
                types.append('c');
            }
            else {
                types.append(nodeType(*it));
            }

            if ((*it)->properties() & KoPathPoint::StopSubpath
                    && (*it)->properties() & KoPathPoint::CloseSubpath) {
                KoPathPoint * firstPoint = (*pathIt)->first();
                types.append(nodeType(firstPoint));
            }
        }
    }
    return types;
}

void updateNodeType(KoPathPoint * point, const QChar & nodeType)
{
    if (nodeType == 's') {
        point->setProperty(KoPathPoint::IsSmooth);
    }
    else if (nodeType == 'z') {
        point->setProperty(KoPathPoint::IsSymmetric);
    }
}

void KoPathShape::loadNodeTypes(const QString &nodeTypes)
{
    QString::const_iterator nIt(nodeTypes.constBegin());
    KoSubpathList::const_iterator pathIt(d->subpaths.constBegin());
    for (; pathIt != d->subpaths.constEnd(); ++pathIt) {
        KoSubpath::const_iterator it((*pathIt)->constBegin());
        for (; it != (*pathIt)->constEnd(); ++it, nIt++) {
            // be sure not to crash if there are not enough nodes in nodeTypes
            if (nIt == nodeTypes.constEnd()) {
                warnFlake << "not enough nodes in sodipodi:nodetypes";
                return;
            }
            // the first node is always of type 'c'
            if (it != (*pathIt)->constBegin()) {
                updateNodeType(*it, *nIt);
            }

            if ((*it)->properties() & KoPathPoint::StopSubpath
                    && (*it)->properties() & KoPathPoint::CloseSubpath) {
                ++nIt;
                updateNodeType((*pathIt)->first(), *nIt);
            }
        }
    }
}

Qt::FillRule KoPathShape::fillRule() const
{
    return d->fillRule;
}

void KoPathShape::setFillRule(Qt::FillRule fillRule)
{
    d->fillRule = fillRule;
}

KoPathShape * KoPathShape::createShapeFromPainterPath(const QPainterPath &path)
{
    KoPathShape * shape = new KoPathShape();

    int elementCount = path.elementCount();
    for (int i = 0; i < elementCount; i++) {
        QPainterPath::Element element = path.elementAt(i);
        switch (element.type) {
        case QPainterPath::MoveToElement:
            shape->moveTo(QPointF(element.x, element.y));
            break;
        case QPainterPath::LineToElement:
            shape->lineTo(QPointF(element.x, element.y));
            break;
        case QPainterPath::CurveToElement:
            shape->curveTo(QPointF(element.x, element.y),
                           QPointF(path.elementAt(i + 1).x, path.elementAt(i + 1).y),
                           QPointF(path.elementAt(i + 2).x, path.elementAt(i + 2).y));
            break;
        default:
            continue;
        }
    }

    shape->setShapeId(KoPathShapeId);

    //shape->normalize();
    return shape;
}

bool KoPathShape::hitTest(const QPointF &position) const
{
    if (parent() && parent()->isClipped(this) && ! parent()->hitTest(position))
        return false;

    QPointF point = absoluteTransformation().inverted().map(position);
    const QPainterPath outlinePath = outline();
    if (stroke()) {
        KoInsets insets;
        stroke()->strokeInsets(this, insets);
        QRectF roi(QPointF(-insets.left, -insets.top), QPointF(insets.right, insets.bottom));

        roi.moveCenter(point);
        if (outlinePath.intersects(roi) || outlinePath.contains(roi))
            return true;
    } else {
        if (outlinePath.contains(point))
            return true;
    }

    // if there is no shadow we can as well just leave
    if (! shadow())
        return false;

    // the shadow has an offset to the shape, so we simply
    // check if the position minus the shadow offset hits the shape
    point = absoluteTransformation().inverted().map(position - shadow()->offset());

    return outlinePath.contains(point);
}

void KoPathShape::setMarker(KoMarker *marker, KoFlake::MarkerPosition pos)
{
    if (!marker && d->markersNew.contains(pos)) {
        d->markersNew.remove(pos);
    } else {
        d->markersNew[pos] = marker;
    }

    notifyChanged();
    shapeChangedPriv(StrokeChanged);
}

KoMarker *KoPathShape::marker(KoFlake::MarkerPosition pos) const
{
    return d->markersNew[pos].data();
}

bool KoPathShape::hasMarkers() const
{
    return !d->markersNew.isEmpty();
}

bool KoPathShape::autoFillMarkers() const
{
    return d->autoFillMarkers;
}

void KoPathShape::setAutoFillMarkers(bool value)
{
    d->autoFillMarkers = value;
}

void KoPathShape::recommendPointSelectionChange(const QList<KoPathPointIndex> &newSelection)
{
    Q_FOREACH (KoShape::ShapeChangeListener *listener, listeners()) {
        PointSelectionChangeListener *pointListener = dynamic_cast<PointSelectionChangeListener*>(listener);
        if (pointListener) {
            pointListener->recommendPointSelectionChange(this, newSelection);
        }
    }
}

void KoPathShape::notifyPointsChanged()
{
    Q_FOREACH (KoShape::ShapeChangeListener *listener, listeners()) {
        PointSelectionChangeListener *pointListener = dynamic_cast<PointSelectionChangeListener*>(listener);
        if (pointListener) {
            pointListener->notifyPathPointsChanged(this);
        }
    }
}

QPainterPath KoPathShape::pathStroke(const QPen &pen) const
{
    if (d->subpaths.isEmpty()) {
        return QPainterPath();
    }
    QPainterPath pathOutline;

    QPainterPathStroker stroker;
    stroker.setWidth(0);
    stroker.setJoinStyle(Qt::MiterJoin);
    stroker.setWidth(pen.widthF());
    stroker.setJoinStyle(pen.joinStyle());
    stroker.setMiterLimit(pen.miterLimit());
    stroker.setCapStyle(pen.capStyle());
    stroker.setDashOffset(pen.dashOffset());
    stroker.setDashPattern(pen.dashPattern());

    QPainterPath path = stroker.createStroke(outline());

    pathOutline.addPath(path);
    pathOutline.setFillRule(Qt::WindingFill);

    return pathOutline;
}

void KoPathShape::PointSelectionChangeListener::notifyShapeChanged(KoShape::ChangeType type, KoShape *shape)
{
    Q_UNUSED(type);
    Q_UNUSED(shape);
}

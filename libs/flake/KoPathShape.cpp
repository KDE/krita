/* This file is part of the KDE project
   Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2008 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2007-2009 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoPathShape.h"
#include "KoPathShape_p.h"
#include "KoPathPoint.h"
#include "KoPointGroup.h"
#include "KoShapeBorderModel.h"
#include "KoViewConverter.h"
#include "KoPathShapeLoader.h"
#include "KoShapeSavingContext.h"
#include "KoShapeLoadingContext.h"
#include "KoShapeShadow.h"
#include "KoShapeBackground.h"
#include "KoShapeContainer.h"
#include "KoFilterEffectStack.h"

#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoUnit.h>
#include <KoGenStyle.h>
#include <KoStyleStack.h>
#include <KoOdfLoadingContext.h>

#include <KDebug>
#include <QtGui/QPainter>

#ifndef QT_NO_DEBUG
#include <qnumeric.h> // for qIsNaN
static bool qIsNaNPoint(const QPointF &p) {
    return qIsNaN(p.x()) || qIsNaN(p.y());
}
#endif

KoPathShapePrivate::KoPathShapePrivate(KoPathShape *q)
    : KoShapePrivate(q),
    fillRule(Qt::OddEvenFill)
{
}

QRectF KoPathShapePrivate::handleRect(const QPointF &p, qreal radius) const
{
    return QRectF(p.x() - radius, p.y() - radius, 2*radius, 2*radius);
}

void KoPathShapePrivate::applyViewboxTransformation(const KoXmlElement &element)
{
    Q_Q(KoPathShape);
    // apply viewbox transformation
    QRectF viewBox = q->loadOdfViewbox(element);
    if (! viewBox.isEmpty()) {
        // load the desired size
        QSizeF size;
        size.setWidth(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "width", QString())));
        size.setHeight(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "height", QString())));

        // load the desired position
        QPointF pos;
        pos.setX(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "x", QString())));
        pos.setY(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "y", QString())));

        // create matrix to transform original path data into desired size and position
        QTransform viewMatrix;
        viewMatrix.translate(-viewBox.left(), -viewBox.top());
        viewMatrix.scale(size.width() / viewBox.width(), size.height() / viewBox.height());
        viewMatrix.translate(pos.x(), pos.y());

        // transform the path data
        map(viewMatrix);
    }
}


/////////////////////////
KoPathShape::KoPathShape()
    :KoShape(*(new KoPathShapePrivate(this)))
{
}

KoPathShape::KoPathShape(KoPathShapePrivate &dd)
    : KoShape(dd)
{
}

KoPathShape::~KoPathShape()
{
    clear();
}

void KoPathShape::saveOdf(KoShapeSavingContext & context) const
{
    Q_D(const KoPathShape);
    context.xmlWriter().startElement("draw:path");
    saveOdfAttributes(context, OdfAllAttributes | OdfViewbox);

    context.xmlWriter().addAttribute("svg:d", toString());
    context.xmlWriter().addAttribute("koffice:nodeTypes", d->nodeTypes());

    saveOdfCommonChildElements(context);
    context.xmlWriter().endElement();
}

bool KoPathShape::loadOdf(const KoXmlElement & element, KoShapeLoadingContext &context)
{
    Q_D(KoPathShape);
    loadOdfAttributes(element, context, OdfMandatories | OdfAdditionalAttributes | OdfCommonChildElements);

    // first clear the path data from the default path
    clear();

    if (element.localName() == "line") {
        QPointF start;
        start.setX(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "x1", "")));
        start.setY(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "y1", "")));
        QPointF end;
        end.setX(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "x2", "")));
        end.setY(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "y2", "")));
        moveTo(start);
        lineTo(end);
    } else if (element.localName() == "polyline" || element.localName() == "polygon") {
        QString points = element.attributeNS(KoXmlNS::draw, "points").simplified();
        points.replace(',', ' ');
        points.remove('\r');
        points.remove('\n');
        bool firstPoint = true;
        const QStringList coordinateList = points.split(' ');
        for (QStringList::ConstIterator it = coordinateList.constBegin(); it != coordinateList.constEnd(); ++it) {
            QPointF point;
            point.setX((*it).toDouble());
            ++it;
            point.setY((*it).toDouble());
            if (firstPoint) {
                moveTo(point);
                firstPoint = false;
            } else
                lineTo(point);
        }
        if (element.localName() == "polygon")
            close();
    } else { // path loading
        KoPathShapeLoader loader(this);
        loader.parseSvg(element.attributeNS(KoXmlNS::svg, "d"), true);
        d->loadNodeTypes(element);
    }

    d->applyViewboxTransformation(element);
    QPointF pos = normalize();
    setTransformation(QTransform());

    if (element.hasAttributeNS(KoXmlNS::svg, "x") || element.hasAttributeNS(KoXmlNS::svg, "y")) {
        pos.setX(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "x", QString())));
        pos.setY(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "y", QString())));
    }

    setPosition(pos);

    loadOdfAttributes(element, context, OdfTransformation);

    return true;
}

QString KoPathShape::saveStyle(KoGenStyle &style, KoShapeSavingContext &context) const
{
    Q_D(const KoPathShape);
    style.addProperty("svg:fill-rule", d->fillRule == Qt::OddEvenFill ? "evenodd" : "nonzero");

    return KoShape::saveStyle(style, context);
}

void KoPathShape::loadStyle(const KoXmlElement & element, KoShapeLoadingContext &context)
{
    Q_D(KoPathShape);
    KoShape::loadStyle(element, context);

    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    styleStack.setTypeProperties("graphic");

    if (styleStack.hasProperty(KoXmlNS::svg, "fill-rule")) {
        QString rule = styleStack.property(KoXmlNS::svg, "fill-rule");
        d->fillRule = rule == "nonzero" ?  Qt::WindingFill : Qt::OddEvenFill;
    }
}

QRectF KoPathShape::loadOdfViewbox(const KoXmlElement & element) const
{
    QRectF viewbox;

    QString data = element.attributeNS(KoXmlNS::svg, "viewBox");
    if (! data.isEmpty()) {
        data.replace(',', ' ');
        QStringList coordinates = data.simplified().split(' ', QString::SkipEmptyParts);
        if (coordinates.count() == 4) {
            viewbox.setRect(coordinates[0].toDouble(), coordinates[1].toDouble(),
                            coordinates[2].toDouble(), coordinates[3].toDouble());
        }
    }

    return viewbox;
}

void KoPathShape::clear()
{
    foreach(KoSubpath *subpath, m_subpaths) {
        foreach(KoPathPoint *point, *subpath)
            delete point;
        delete subpath;
    }
    m_subpaths.clear();
}

void KoPathShape::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_D(KoPathShape);
    applyConversion(painter, converter);
    QPainterPath path(outline());
    path.setFillRule(d->fillRule);

    if (background())
        background()->paint(painter, path);
    //paintDebug( painter );
}

#ifndef NDEBUG
void KoPathShapePrivate::paintDebug(QPainter &painter)
{
    Q_Q(KoPathShape);
    KoSubpathList::const_iterator pathIt(q->m_subpaths.constBegin());
    int i = 0;

    QPen pen(Qt::black);
    painter.save();
    painter.setPen(pen);
    for (; pathIt != q->m_subpaths.constEnd(); ++pathIt) {
        KoSubpath::const_iterator it((*pathIt)->constBegin());
        for (; it != (*pathIt)->constEnd(); ++it) {
            ++i;
            KoPathPoint *point = (*it);
            QRectF r(point->point(), QSizeF(5, 5));
            r.translate(-2.5, -2.5);
            QPen pen(Qt::black);
            painter.setPen(pen);
            if (point->group()) {
                QBrush b(Qt::blue);
                painter.setBrush(b);
            } else if (point->activeControlPoint1() && point->activeControlPoint2()) {
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
    kDebug(30006) << "nop =" << i;
}

void KoPathShapePrivate::debugPath() const
{
    Q_Q(const KoPathShape);
    KoSubpathList::const_iterator pathIt(q->m_subpaths.constBegin());
    for (; pathIt != q->m_subpaths.constEnd(); ++pathIt) {
        KoSubpath::const_iterator it((*pathIt)->constBegin());
        for (; it != (*pathIt)->constEnd(); ++it) {
            kDebug(30006) << "p:" << (*pathIt) << "," << *it << "," << (*it)->point() << "," << (*it)->properties() << "," << (*it)->group();
        }
    }
}
#endif

void KoPathShape::paintPoints(QPainter &painter, const KoViewConverter &converter, int handleRadius)
{
    Q_D(KoPathShape);
    applyConversion(painter, converter);

    KoSubpathList::const_iterator pathIt(m_subpaths.constBegin());

    QRectF handle = converter.viewToDocument(d->handleRect(QPoint(0, 0), handleRadius));

    for (; pathIt != m_subpaths.constEnd(); ++pathIt) {
        KoSubpath::const_iterator it((*pathIt)->constBegin());
        for (; it != (*pathIt)->constEnd(); ++it)
            (*it)->paint(painter, handleRadius, KoPathPoint::Node);
    }
}

QPainterPath KoPathShape::outline() const
{
    QPainterPath path;
    foreach(KoSubpath * subpath, m_subpaths) {
        KoPathPoint * lastPoint = subpath->first();
        bool activeCP = false;
        foreach(KoPathPoint * currPoint, *subpath) {
            KoPathPoint::PointProperties currProperties = currPoint->properties();
            if (currPoint == subpath->first()) {
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
            } else if( activeCP || currPoint->activeControlPoint1()) {
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
                else if(currPoint->activeControlPoint2() || firstPoint->activeControlPoint1()) {
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
    QTransform transform = absoluteTransformation(0);
    // calculate the bounding rect of the transformed outline
    QRectF bb(transform.map(outline()).boundingRect());
    if (border()) {
        KoInsets inset;
        border()->borderInsets(this, inset);

        // calculate transformed border insets
        QPointF center = transform.map(QPointF());
        QPointF tl = transform.map(QPointF(-inset.left,-inset.top)) - center;
        QPointF br = transform.map(QPointF(inset.right,inset.bottom)) -center;
        qreal left = qMin(tl.x(),br.x());
        qreal right = qMax(tl.x(),br.x());
        qreal top = qMin(tl.y(),br.y());
        qreal bottom = qMax(tl.y(),br.y());
        bb.adjust(left, top, right, bottom);
    }
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
    // which itself uses size() -> leads to infinite reccursion
    return outline().boundingRect().size();
}

void KoPathShape::setSize(const QSizeF &newSize)
{
    Q_D(KoPathShape);
    QTransform matrix(resizeMatrix(newSize));

    KoShape::setSize(newSize);
    d->map(matrix);
}

QTransform KoPathShape::resizeMatrix( const QSizeF & newSize ) const
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
    m_subpaths.push_back(path);
    return point;
}

KoPathPoint * KoPathShape::lineTo(const QPointF &p)
{
    Q_D(KoPathShape);
    if (m_subpaths.empty()) {
        moveTo(QPointF(0, 0));
    }
    KoPathPoint * point = new KoPathPoint(this, p, KoPathPoint::StopSubpath);
    KoPathPoint * lastPoint = m_subpaths.last()->last();
    d->updateLast(&lastPoint);
    m_subpaths.last()->push_back(point);
    return point;
}

KoPathPoint * KoPathShape::curveTo(const QPointF &c1, const QPointF &c2, const QPointF &p)
{
    Q_D(KoPathShape);
    if (m_subpaths.empty()) {
        moveTo(QPointF(0, 0));
    }
    KoPathPoint * lastPoint = m_subpaths.last()->last();
    d->updateLast(&lastPoint);
    lastPoint->setControlPoint2(c1);
    KoPathPoint * point = new KoPathPoint(this, p, KoPathPoint::StopSubpath);
    point->setControlPoint1(c2);
    m_subpaths.last()->push_back(point);
    return point;
}

KoPathPoint * KoPathShape::curveTo(const QPointF &c, const QPointF &p)
{
    Q_D(KoPathShape);
    if (m_subpaths.empty())
        moveTo(QPointF(0, 0));

    KoPathPoint * lastPoint = m_subpaths.last()->last();
    d->updateLast(&lastPoint);
    lastPoint->setControlPoint2(c);
    KoPathPoint * point = new KoPathPoint(this, p, KoPathPoint::StopSubpath);
    m_subpaths.last()->push_back(point);

    return point;
}

KoPathPoint * KoPathShape::arcTo(qreal rx, qreal ry, qreal startAngle, qreal sweepAngle)
{
    if (m_subpaths.empty()) {
        moveTo(QPointF(0, 0));
    }

    KoPathPoint * lastPoint = m_subpaths.last()->last();
    if (lastPoint->properties() & KoPathPoint::CloseSubpath) {
        lastPoint = m_subpaths.last()->first();
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
    if (sweepAngle == 0)
        return pointCnt;
    if (sweepAngle > 360)
        sweepAngle = 360;
    else if (sweepAngle < -360)
        sweepAngle = - 360;

    if (rx == 0 || ry == 0) {
        //TODO
    }

    // split angles bigger than 90° so that it gives a good aproximation to the circle
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

    //kDebug(30006) <<"kappa" << kappa <<"parts" << parts;;

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
    Q_D(KoPathShape);
    if (m_subpaths.empty()) {
        return;
    }
    d->closeSubpath(m_subpaths.last());
}

void KoPathShape::closeMerge()
{
    Q_D(KoPathShape);
    if (m_subpaths.empty()) {
        return;
    }
    d->closeMergeSubpath(m_subpaths.last());
}

QPointF KoPathShape::normalize()
{
    Q_D(KoPathShape);
    QPointF tl(outline().boundingRect().topLeft());
    QTransform matrix;
    matrix.translate(-tl.x(), -tl.y());
    d->map(matrix);

    // keep the top left point of the object
    applyTransformation(matrix.inverted());

    return tl;
}

void KoPathShapePrivate::map(const QTransform &matrix)
{
    Q_Q(KoPathShape);
    KoSubpathList::const_iterator pathIt(q->m_subpaths.constBegin());
    for (; pathIt != q->m_subpaths.constEnd(); ++pathIt) {
        KoSubpath::const_iterator it((*pathIt)->constBegin());
        for (; it != (*pathIt)->constEnd(); ++it) {
            (*it)->map(matrix);
        }
    }
}

void KoPathShapePrivate::updateLast(KoPathPoint **lastPoint)
{
    Q_Q(KoPathShape);
    // check if we are about to add a new point to a closed subpath
    if ((*lastPoint)->properties() & KoPathPoint::StopSubpath
            && (*lastPoint)->properties() & KoPathPoint::CloseSubpath) {
        // get the first point of the subpath
        KoPathPoint *subpathStart = q->m_subpaths.last()->first();
        // clone the first point of the subpath...
        KoPathPoint * newLastPoint = new KoPathPoint(*subpathStart);
        // ... and make it a normal point
        newLastPoint->setProperties(KoPathPoint::Normal);
        // make a point group of the first point and its clone
        KoPointGroup * group = subpathStart->group();
        if (group == 0) {
            group = new KoPointGroup();
            group->add(subpathStart);
        }
        group->add(newLastPoint);

        // now start a new subpath with the cloned start point
        KoSubpath *path = new KoSubpath;
        path->push_back(newLastPoint);
        q->m_subpaths.push_back(path);
        *lastPoint = newLastPoint;
    } else {
        // the subpath was not closed so the formerly last point
        // of the subpath is no end point anymore
        (*lastPoint)->unsetProperty(KoPathPoint::StopSubpath);
    }
    (*lastPoint)->unsetProperty(KoPathPoint::CloseSubpath);
}

QList<KoPathPoint*> KoPathShape::pointsAt(const QRectF &r)
{
    QList<KoPathPoint*> result;

    KoSubpathList::const_iterator pathIt(m_subpaths.constBegin());
    for (; pathIt != m_subpaths.constEnd(); ++pathIt) {
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

QList<KoPathSegment> KoPathShape::segmentsAt(const QRectF &r)
{
    QList<KoPathSegment> segments;
    int subpathCount = m_subpaths.count();
    for (int subpathIndex = 0; subpathIndex < subpathCount; ++subpathIndex) {
        KoSubpath * subpath = m_subpaths[subpathIndex];
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
    for (int subpathIndex = 0; subpathIndex < m_subpaths.size(); ++subpathIndex) {
        KoSubpath * subpath = m_subpaths.at(subpathIndex);
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
    Q_D(const KoPathShape);
    KoSubpath *subpath = d->subPath(pointIndex.first);

    if (subpath == 0 || pointIndex.second < 0 || pointIndex.second >= subpath->size())
        return 0;

    return subpath->at(pointIndex.second);
}

KoPathSegment KoPathShape::segmentByIndex(const KoPathPointIndex &pointIndex) const
{
    Q_D(const KoPathShape);
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
    KoSubpathList::const_iterator pathIt(m_subpaths.constBegin());
    for (; pathIt != m_subpaths.constEnd(); ++pathIt) {
        i += (*pathIt)->size();
    }

    return i;
}

int KoPathShape::subpathCount() const
{
    return m_subpaths.count();
}

int KoPathShape::subpathPointCount(int subpathIndex) const
{
    Q_D(const KoPathShape);
    KoSubpath *subpath = d->subPath(subpathIndex);

    if (subpath == 0)
        return -1;

    return subpath->size();
}

bool KoPathShape::isClosedSubpath(int subpathIndex)
{
    Q_D(KoPathShape);
    KoSubpath *subpath = d->subPath(subpathIndex);

    if (subpath == 0)
        return false;

    const bool firstClosed = subpath->first()->properties() & KoPathPoint::CloseSubpath;
    const bool lastClosed = subpath->last()->properties() & KoPathPoint::CloseSubpath;

    return firstClosed && lastClosed;
}

bool KoPathShape::insertPoint(KoPathPoint* point, const KoPathPointIndex &pointIndex)
{
    Q_D(KoPathShape);
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
    return true;
}

KoPathPoint * KoPathShape::removePoint(const KoPathPointIndex &pointIndex)
{
    Q_D(KoPathShape);
    KoSubpath *subpath = d->subPath(pointIndex.first);

    if (subpath == 0 || pointIndex.second < 0 || pointIndex.second >= subpath->size())
        return 0;

    KoPathPoint * point = subpath->takeAt(pointIndex.second);

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

    return point;
}

bool KoPathShape::breakAfter(const KoPathPointIndex &pointIndex)
{
    Q_D(KoPathShape);
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
    m_subpaths.insert(pointIndex.first + 1, newSubpath);

    return true;
}

bool KoPathShape::join(int subpathIndex)
{
    Q_D(KoPathShape);
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
    foreach(KoPathPoint * p, *nextSubpath)
        subpath->append(p);

    // remove the nextSubpath from path
    m_subpaths.removeAt(subpathIndex + 1);

    // delete it as it is no longer possible to use it
    delete nextSubpath;

    return true;
}

bool KoPathShape::moveSubpath(int oldSubpathIndex, int newSubpathIndex)
{
    Q_D(KoPathShape);
    KoSubpath *subpath = d->subPath(oldSubpathIndex);

    if (subpath == 0 || newSubpathIndex >= m_subpaths.size())
        return false;

    if (oldSubpathIndex == newSubpathIndex)
        return true;

    m_subpaths.removeAt(oldSubpathIndex);
    m_subpaths.insert(newSubpathIndex, subpath);

    return true;
}

KoPathPointIndex KoPathShape::openSubpath(const KoPathPointIndex &pointIndex)
{
    Q_D(KoPathShape);
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

    return pathPointIndex(oldStartPoint);
}

KoPathPointIndex KoPathShape::closeSubpath(const KoPathPointIndex &pointIndex)
{
    Q_D(KoPathShape);
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

    d->closeSubpath(subpath);
    return pathPointIndex(oldStartPoint);
}

bool KoPathShape::reverseSubpath(int subpathIndex)
{
    Q_D(KoPathShape);
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

    return true;
}

KoSubpath * KoPathShape::removeSubpath(int subpathIndex)
{
    Q_D(KoPathShape);
    KoSubpath *subpath = d->subPath(subpathIndex);

    if (subpath != 0)
        m_subpaths.removeAt(subpathIndex);

    return subpath;
}

bool KoPathShape::addSubpath(KoSubpath * subpath, int subpathIndex)
{
    if (subpathIndex < 0 || subpathIndex > m_subpaths.size())
        return false;

    m_subpaths.insert(subpathIndex, subpath);

    return true;
}

bool KoPathShape::combine(KoPathShape *path)
{
    if (! path)
        return false;

    QTransform pathMatrix = path->absoluteTransformation(0);
    QTransform myMatrix = absoluteTransformation(0).inverted();

    foreach(KoSubpath* subpath, path->m_subpaths) {
        KoSubpath *newSubpath = new KoSubpath();

        foreach(KoPathPoint* point, *subpath) {
            KoPathPoint *newPoint = new KoPathPoint(*point);
            newPoint->map(pathMatrix);
            newPoint->map(myMatrix);
            newPoint->setParent(this);
            newSubpath->append(newPoint);
        }
        m_subpaths.append(newSubpath);
    }
    normalize();
    return true;
}

bool KoPathShape::separate(QList<KoPathShape*> & separatedPaths)
{
    if (! m_subpaths.size())
        return false;

    QTransform myMatrix = absoluteTransformation(0);

    foreach(KoSubpath* subpath, m_subpaths) {
        KoPathShape *shape = new KoPathShape();
        if (! shape) continue;

        shape->setBorder(border());
        shape->setShapeId(shapeId());

        KoSubpath *newSubpath = new KoSubpath();

        foreach(KoPathPoint* point, *subpath) {
            KoPathPoint *newPoint = new KoPathPoint(*point);
            newPoint->map(myMatrix);
            newSubpath->append(newPoint);
        }
        shape->m_subpaths.append(newSubpath);
        shape->normalize();
        separatedPaths.append(shape);
    }
    return true;
}

void KoPathShapePrivate::closeSubpath(KoSubpath *subpath)
{
    if (! subpath)
        return;

    subpath->last()->setProperty(KoPathPoint::CloseSubpath);
    subpath->first()->setProperty(KoPathPoint::CloseSubpath);
}

void KoPathShapePrivate::closeMergeSubpath(KoSubpath *subpath)
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
    } else {
        closeSubpath(subpath);
    }
}

KoSubpath *KoPathShapePrivate::subPath(int subpathIndex) const
{
    Q_Q(const KoPathShape);
    if (subpathIndex < 0 || subpathIndex >= q->m_subpaths.size())
        return 0;

    return q->m_subpaths.at(subpathIndex);
}

QString KoPathShape::pathShapeId() const
{
    return KoPathShapeId;
}

QString KoPathShape::toString(const QTransform &matrix) const
{
    QString d;

    KoSubpathList::const_iterator pathIt(m_subpaths.constBegin());
    for (; pathIt != m_subpaths.constEnd(); ++pathIt) {
        KoSubpath::const_iterator it((*pathIt)->constBegin());
        KoPathPoint * lastPoint(*it);
        bool activeCP = false;
        for (; it != (*pathIt)->constEnd(); ++it) {
            // first point of subpath ?
            if (it == (*pathIt)->constBegin()) {
                if ((*it)->properties() & KoPathPoint::StartSubpath) {
                    QPointF p = matrix.map((*it)->point());
                    d += QString("M%1 %2").arg(p.x()).arg(p.y());
                }
            }
            // end point of curve ?
            else if (activeCP || (*it)->activeControlPoint1()) {
                QPointF cp1 = matrix.map(activeCP ? lastPoint->controlPoint2() : lastPoint->point());
                QPointF cp2 = matrix.map((*it)->activeControlPoint1() ? (*it)->controlPoint1() : (*it)->point());
                QPointF p = matrix.map((*it)->point());
                d += QString("C%1 %2 %3 %4 %5 %6")
                     .arg(cp1.x()).arg(cp1.y())
                     .arg(cp2.x()).arg(cp2.y())
                     .arg(p.x()).arg(p.y());
            }
            // end point of line
            else {
                QPointF p = matrix.map((*it)->point());
                d += QString("L%1 %2").arg(p.x()).arg(p.y());
            }
            // last point closes subpath ?
            if ((*it)->properties() & KoPathPoint::StopSubpath
                    && (*it)->properties() & KoPathPoint::CloseSubpath) {
                // add curve when there is a curve on the way to the first point
                KoPathPoint * firstPoint = (*pathIt)->first();
                if ((*it)->activeControlPoint2() || firstPoint->activeControlPoint1()) {
                    QPointF cp1 = matrix.map((*it)->activeControlPoint2() ? (*it)->controlPoint2() : (*it)->point());
                    QPointF cp2 = matrix.map(firstPoint->activeControlPoint1() ? firstPoint->controlPoint1() : (firstPoint)->point());
                    QPointF p = matrix.map(firstPoint->point());

                    d += QString("C%1 %2 %3 %4 %5 %6")
                         .arg(cp1.x()).arg(cp1.y())
                         .arg(cp2.x()).arg(cp2.y())
                         .arg(p.x()).arg(p.y());
                }
                d += QString("Z");
            }

            activeCP = (*it)->activeControlPoint2();
            lastPoint = *it;
        }
    }

    return d;
}

char nodeType( const KoPathPoint * point )
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

QString KoPathShapePrivate::nodeTypes() const
{
    Q_Q(const KoPathShape);
    QString types;
    KoSubpathList::const_iterator pathIt(q->m_subpaths.constBegin());
    for (; pathIt != q->m_subpaths.constEnd(); ++pathIt) {
        KoSubpath::const_iterator it((*pathIt)->constBegin());
        for (; it != (*pathIt)->constEnd(); ++it) {
            if (it == (*pathIt)->constBegin()) {
                types.append( 'c' );
            }
            else {
                types.append( nodeType( *it ) );
            }

            if ((*it)->properties() & KoPathPoint::StopSubpath
                && (*it)->properties() & KoPathPoint::CloseSubpath) {
                KoPathPoint * firstPoint = (*pathIt)->first();
                types.append( nodeType( firstPoint ) );
            }
        }
    }
    return types;
}

void updateNodeType( KoPathPoint * point, const QChar & nodeType )
{
    if (nodeType == 's') {
        point->setProperty(KoPathPoint::IsSmooth);
    }
    else if (nodeType == 'z') {
        point->setProperty(KoPathPoint::IsSymmetric);
    }
}

void KoPathShapePrivate::loadNodeTypes(const KoXmlElement &element)
{
    Q_Q(KoPathShape);
    if (element.hasAttributeNS(KoXmlNS::koffice, "nodeTypes")) {
        QString nodeTypes = element.attributeNS(KoXmlNS::koffice, "nodeTypes");
        QString::const_iterator nIt(nodeTypes.constBegin());
        KoSubpathList::const_iterator pathIt(q->m_subpaths.constBegin());
        for (; pathIt != q->m_subpaths.constEnd(); ++pathIt) {
            KoSubpath::const_iterator it((*pathIt)->constBegin());
            for (; it != (*pathIt)->constEnd(); ++it, nIt++) {
                // be sure not to crash if there are not enough nodes in nodeTypes
                if (nIt == nodeTypes.constEnd()) {
                    kWarning(30006) << "not enough nodes in koffice:nodeTypes";
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
}

Qt::FillRule KoPathShape::fillRule() const
{
    Q_D(const KoPathShape);
    return d->fillRule;
}

void KoPathShape::setFillRule(Qt::FillRule fillRule)
{
    Q_D(KoPathShape);
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

    shape->normalize();
    return shape;
}

bool KoPathShape::hitTest(const QPointF &position) const
{
    if (parent() && parent()->isClipped(this) && ! parent()->hitTest(position))
        return false;

    QPointF point = absoluteTransformation(0).inverted().map(position);
    const QPainterPath outlinePath = outline();
    if (border()) {
        KoInsets insets;
        border()->borderInsets(this, insets);
        QRectF roi( QPointF(-insets.left, -insets.top), QPointF(insets.right, insets.bottom) );
        roi.moveCenter( point );
        if( outlinePath.intersects( roi ) || outlinePath.contains( roi ) )
            return true;
    } else {
        if( outlinePath.contains( point ) )
            return true;
    }

    // if there is no shadow we can as well just leave
    if (! shadow())
        return false;

    // the shadow has an offset to the shape, so we simply
    // check if the position minus the shadow offset hits the shape
    point = absoluteTransformation(0).inverted().map(position - shadow()->offset());

    return outlinePath.contains(point);
}

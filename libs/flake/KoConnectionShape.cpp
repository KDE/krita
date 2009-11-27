/* This file is part of the KDE project
 * Copyright (C) 2007 Boudewijn Rempt <boud@kde.org>
 * Copyright (C) 2007,2009 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007,2009  Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoConnectionShape.h"

#include <KoViewConverter.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoStoreDevice.h>
#include <KoUnit.h>
#include "KoConnectionShapeLoadingUpdater.h"

#include <QPainter>

#include <KDebug>
// XXX: Add editable text in path shapes so we can get a label here

class KoConnectionShape::Private
{
public:
    Private()
            : shape1(0), shape2(0), connectionPointIndex1(-1), connectionPointIndex2(-1)
            , connectionType(Standard), forceUpdate(false) {}
    KoSubpath points;
    KoShape * shape1;
    KoShape * shape2;
    int connectionPointIndex1;
    int connectionPointIndex2;
    Type connectionType;
    bool forceUpdate;
};


KoConnectionShape::KoConnectionShape()
        : d(new Private)
{
    m_handles.push_back(QPointF(0, 0));
    m_handles.push_back(QPointF(140, 140));

    moveTo(m_handles[0]);
    lineTo(m_handles[1]);

    d->points = *m_subpaths[0];
    updatePath(QSizeF(140, 140));

    int connectionPointCount = connectionPoints().size();
    for (int i = 0; i < connectionPointCount; ++i)
        removeConnectionPoint(0);

    m_hasMoved = true;
}

KoConnectionShape::~KoConnectionShape()
{
    if (d->shape1)
        d->shape1->removeDependee(this);
    if (d->shape2)
        d->shape2->removeDependee(this);

    delete d;
}

void KoConnectionShape::paint(QPainter&, const KoViewConverter&)
{
}

void KoConnectionShape::saveOdf(KoShapeSavingContext & context) const
{
    context.xmlWriter().startElement("draw:connector");
    saveOdfAttributes( context, OdfMandatories | OdfAdditionalAttributes );

    switch( d->connectionType )
    {
        case Lines:
            context.xmlWriter().addAttribute( "draw:type", "lines" );
            break;
        case Straight:
            context.xmlWriter().addAttribute( "draw:type", "line" );
            break;
        case Curve:
            context.xmlWriter().addAttribute( "draw:type", "curve" );
            break;
        default:
            context.xmlWriter().addAttribute( "draw:type", "standard" );
            break;
    }

    if (d->shape1) {
        context.xmlWriter().addAttribute( "draw:start-shape", context.drawId(d->shape1) );
        context.xmlWriter().addAttribute( "draw:start-glue-point", d->connectionPointIndex1 );
    }
    else {
        QPointF p((m_handles[0] + position()) * context.shapeOffset(this));
        context.xmlWriter().addAttributePt( "svg:x1", p.x() );
        context.xmlWriter().addAttributePt( "svg:y1", p.y() );
    }
    if (d->shape2) {
        context.xmlWriter().addAttribute( "draw:end-shape", context.drawId(d->shape2) );
        context.xmlWriter().addAttribute( "draw:end-glue-point", d->connectionPointIndex2 );
    }
    else {
        QPointF p((m_handles[m_handles.count()-1] + position()) * context.shapeOffset(this));
        context.xmlWriter().addAttributePt( "svg:x2", p.x() );
        context.xmlWriter().addAttributePt( "svg:y2", p.y() );
    }

    saveOdfCommonChildElements(context);

    context.xmlWriter().endElement();
}

bool KoConnectionShape::loadOdf(const KoXmlElement & element, KoShapeLoadingContext &context)
{
    loadOdfAttributes(element, context, OdfMandatories | OdfCommonChildElements | OdfAdditionalAttributes);

    QString type = element.attributeNS(KoXmlNS::draw, "type", "standard");
    if (type == "lines")
        d->connectionType = Lines;
    else if (type == "line")
        d->connectionType = Straight;
    else if (type == "curve")
        d->connectionType = Curve;
    else
        d->connectionType = Standard;

    if (element.hasAttributeNS(KoXmlNS::draw, "start-shape")) {
        d->connectionPointIndex1 = element.attributeNS(KoXmlNS::draw, "start-glue-point", "").toInt();
        QString shapeId1 = element.attributeNS(KoXmlNS::draw, "start-shape", "");
        d->shape1 = context.shapeById(shapeId1);
        if (d->shape1) {
            d->shape1->addDependee(this);
        }
        else {
            context.updateShape(shapeId1, new KoConnectionShapeLoadingUpdater(this, KoConnectionShapeLoadingUpdater::First));
        }
    } else {
        m_handles[0].setX(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "x1", QString())));
        m_handles[0].setY(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "y1", QString())));
    }

    if (element.hasAttributeNS(KoXmlNS::draw, "end-shape")) {
        d->connectionPointIndex2 = element.attributeNS(KoXmlNS::draw, "end-glue-point", "").toInt();
        QString shapeId2 = element.attributeNS(KoXmlNS::draw, "end-shape", "");
        d->shape2 = context.shapeById(shapeId2);
        if (d->shape2) {
            d->shape2->addDependee(this);
        }
        else {
            context.updateShape(shapeId2, new KoConnectionShapeLoadingUpdater(this, KoConnectionShapeLoadingUpdater::Second));
        }
    } else {
        m_handles[m_handles.count() - 1].setX(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "x2", QString())));
        m_handles[m_handles.count() - 1].setY(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "y2", QString())));
    }

    QString skew = element.attributeNS(KoXmlNS::draw, "line-skew", "");
    QStringList skewValues = skew.simplified().split(' ', QString::SkipEmptyParts);
    // TODO apply skew values once we support them

    updateConnections();

    return true;
}

bool KoConnectionShape::hitTest(const QPointF &position) const
{
    return KoParameterShape::hitTest(position);
}

void KoConnectionShape::moveHandleAction(int handleId, const QPointF & point, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);

    if (handleId >= m_handles.size())
        return;

    m_handles[handleId] = point;
}

void KoConnectionShape::updatePath(const QSizeF &size)
{
    Q_UNUSED(size);

    QPointF dst = 0.3 * ( m_handles[0] - m_handles[m_handles.count() - 1]);
    const qreal MinimumEscapeLength = (qreal)20.;
    clear();
    switch (d->connectionType) {
    case Standard: {
        normalPath(MinimumEscapeLength);
        if( m_path.count() != 0 ){
            moveTo( m_path[0] );
            for(int index = 1; index < m_path.count(); ++index )
                lineTo( m_path[index] );
        }

        break;
    }
    case Lines: {
        QPointF direction1 = escapeDirection(0);
        QPointF direction2 = escapeDirection(m_handles.count() - 1);
        moveTo(m_handles[0]);
        if (! direction1.isNull())
            lineTo(m_handles[0] + MinimumEscapeLength * direction1);
        if (! direction2.isNull())
            lineTo(m_handles[m_handles.count() - 1] + MinimumEscapeLength * direction2);
        lineTo(m_handles[m_handles.count() - 1]);
        break;
    }
    case Straight:
        moveTo(m_handles[0]);
        lineTo(m_handles[m_handles.count() - 1]);
        break;
    case Curve:
        // TODO
        QPointF direction1 = escapeDirection(0);
        QPointF direction2 = escapeDirection(m_handles.count() - 1);
        moveTo(m_handles[0]);
        if (! direction1.isNull() && ! direction2.isNull()) {
            QPointF curvePoint1 = m_handles[0] + 5.0 * MinimumEscapeLength * direction1;
            QPointF curvePoint2 = m_handles[m_handles.count() - 1] + 5.0 * MinimumEscapeLength * direction2;
            curveTo(curvePoint1, curvePoint2, m_handles[m_handles.count() - 1]);
        } else {
            lineTo(m_handles[m_handles.count() - 1]);
        }
        break;
    }
    normalize();
}

void KoConnectionShape::normalPath( const qreal MinimumEscapeLength )
{
    if(m_hasMoved) {

        m_hasMoved = false;
        QPointF firstHandle;
        QPointF lastHandle;

        // Clear handles keeping the first and last one.
        firstHandle = m_handles[0];
        lastHandle = m_handles[m_handles.count() - 1];

        m_handles.clear();
        m_handles.append(firstHandle);
        m_handles.append(lastHandle);

        // Clear the path to build it again.
        m_path.clear();
        m_path.append( m_handles[0] );

        QList<QPointF> edges1;
        QList<QPointF> edges2;

        QPointF direction1 = escapeDirection(0);
        QPointF direction2 = escapeDirection(m_handles.count() - 1);
        
        QPointF edgePoint1 = m_handles[0] + MinimumEscapeLength * direction1;
        QPointF edgePoint2 = m_handles[m_handles.count() - 1] + MinimumEscapeLength * direction2;

        edges1.append(edgePoint1);
        edges2.prepend(edgePoint2);

        if (handleConnected(0) && handleConnected(1)) {
            QPointF intersection;
            bool connected = false;
            do {
                // first check if directions from current edge points intersect
                if (intersects(edgePoint1, direction1, edgePoint2, direction2, intersection)) {
                    // directions intersect, we have another edge point and be done
                    edges1.append(intersection);
                    break;
                }

                // check if we are going toward the other handle
                qreal sp = scalarProd(direction1, edgePoint2 - edgePoint1);
                if (sp >= 0.0) {
                    // if we are having the same direction, go all the way toward
                    // the other handle, else only go half the way
                    if (direction1 == direction2)
                        edgePoint1 += sp * direction1;
                    else
                        edgePoint1 += 0.5 * sp * direction1;
                    edges1.append(edgePoint1);
                    // switch direction
                    direction1 = perpendicularDirection(edgePoint1, direction1, edgePoint2);
                } else {
                    // we are not going into the same direction, so switch direction
                    direction1 = perpendicularDirection(edgePoint1, direction1, edgePoint2);
                }
            } while (! connected);
        }

        m_path.append(edges1);
        m_path.append(edges2);

        m_path.append( m_handles[m_handles.count() - 1] );
    }
}

bool KoConnectionShape::setConnection1(KoShape * shape1, int connectionPointIndex1)
{
    // refuse to connect to a shape that depends on us (e.g. a artistic text shape)
    if (hasDependee(shape1))
        return false;

    // check if the connection point does exist
    if (shape1 && connectionPointIndex1 >= shape1->connectionPoints().count())
        return false;

    if (d->shape1)
        d->shape1->removeDependee(this);
    d->shape1 = shape1;
    if (d->shape1)
        d->shape1->addDependee(this);

    d->connectionPointIndex1 = connectionPointIndex1;

    return true;
}

bool KoConnectionShape::setConnection2(KoShape * shape2, int connectionPointIndex2)
{
    // refuse to connect to a shape that depends on us (e.g. a artistic text shape)
    if (hasDependee(shape2))
        return false;

    // check if the connection point does exist
    if (shape2 && connectionPointIndex2 >= shape2->connectionPoints().count())
        return false;

    if (d->shape2)
        d->shape2->removeDependee(this);
    d->shape2 = shape2;
    if (d->shape2)
        d->shape2->addDependee(this);

    d->connectionPointIndex2 = connectionPointIndex2;

    return true;
}

KoConnection KoConnectionShape::connection1() const
{
    return KoConnection(d->shape1, d->connectionPointIndex1);
}

KoConnection KoConnectionShape::connection2() const
{
    return KoConnection(d->shape2, d->connectionPointIndex2);
}

void KoConnectionShape::updateConnections()
{
    bool updateHandles = false;

    if (handleConnected(0)) {
        QList<QPointF> connectionPoints = d->shape1->connectionPoints();
        if (d->connectionPointIndex1 < connectionPoints.count()) {
            // map connection point into our shape coordinates
            QPointF p = documentToShape(d->shape1->absoluteTransformation(0).map(connectionPoints[d->connectionPointIndex1]));
            if (m_handles[0] != p) {
                m_handles[0] = p;
                updateHandles = true;
            }
        }
    }
    if (handleConnected(1)) {
        QList<QPointF> connectionPoints = d->shape2->connectionPoints();
        if (d->connectionPointIndex2 < connectionPoints.count()) {
            // map connection point into our shape coordinates
            QPointF p = documentToShape(d->shape2->absoluteTransformation(0).map(connectionPoints[d->connectionPointIndex2]));
            if (m_handles[m_handles.count() - 1] != p) {
                m_handles[m_handles.count() - 1] = p;
                updateHandles = true;
            }
        }
    }
    if (updateHandles || d->forceUpdate) {
        update(); // ugly, for repainting the connection we just changed
        updatePath(QSizeF());
        update(); // ugly, for repainting the connection we just changed
        d->forceUpdate = false;
    }
}

KoConnectionShape::Type KoConnectionShape::connectionType() const
{
    return d->connectionType;
}

void KoConnectionShape::setConnectionType(Type connectionType)
{
    d->connectionType = connectionType;
    updatePath(size());
}

bool KoConnectionShape::handleConnected(int handleId) const
{
    if (handleId == 0 && d->shape1 && d->connectionPointIndex1 >= 0)
        return true;
    if (handleId == 1 && d->shape2 && d->connectionPointIndex2 >= 0)
        return true;

    return false;
}

QPointF KoConnectionShape::escapeDirection(int handleId) const
{
    QPointF direction;
    if (handleConnected(handleId)) {
        QMatrix absoluteMatrix = absoluteTransformation(0);
        QPointF handlePoint = absoluteMatrix.map(m_handles[handleId]);
        QPointF centerPoint;
        if (handleId == 0)
            centerPoint = d->shape1->absolutePosition(KoFlake::CenteredPosition);
        else
            centerPoint = d->shape2->absolutePosition(KoFlake::CenteredPosition);

        qreal angle = atan2(handlePoint.y() - centerPoint.y(), handlePoint.x() - centerPoint.x());
        if (angle < 0.0)
            angle += 2.0 * M_PI;
        angle *= 180.0 / M_PI;
        if (angle >= 45.0 && angle < 135.0)
            direction = QPointF(0.0, 1.0);
        else if (angle >= 135.0 && angle < 225.0)
            direction = QPointF(-1.0, 0.0);
        else if (angle >= 225.0 && angle < 315.0)
            direction = QPointF(0.0, -1.0);
        else
            direction = QPointF(1.0, 0.0);

        // transform escape direction by using our own transformation matrix
        QMatrix invMatrix = absoluteMatrix.inverted();
        direction = invMatrix.map(direction) - invMatrix.map(QPointF());
        direction /= sqrt(direction.x() * direction.x() + direction.y() * direction.y());
    }

    return direction;
}

bool KoConnectionShape::intersects(const QPointF &p1, const QPointF &d1, const QPointF &p2, const QPointF &d2, QPointF &isect)
{
    qreal sp1 = scalarProd(d1, p2 - p1);
    if (sp1 < 0.0)
        return false;

    qreal sp2 = scalarProd(d2, p1 - p2);
    if (sp2 < 0.0)
        return false;

    // use cross product to check if rays intersects at all
    qreal cp = crossProd(d1, d2);
    if (cp == 0.0) {
        // rays are parallel or coincidient
        if (p1.x() == p2.x() && d1.x() == 0.0 && d1.y() != d2.y()) {
            // vertical, coincident
            isect = 0.5 * (p1 + p2);
        } else if (p1.y() == p2.y() && d1.y() == 0.0 && d1.x() != d2.x()) {
            // horizontal, coincident
            isect = 0.5 * (p1 + p2);
        } else
            return false;
    } else {
        // they are intersecting normally
        isect = p1 + sp1 * d1;
    }

    return true;
}

qreal KoConnectionShape::scalarProd(const QPointF &v1, const QPointF &v2)
{
    return v1.x()*v2.x() + v1.y()*v2.y();
}

qreal KoConnectionShape::crossProd(const QPointF &v1, const QPointF &v2)
{
    return (v1.x()*v2.y() - v1.y()*v2.x());
}

QPointF KoConnectionShape::perpendicularDirection(const QPointF &p1, const QPointF &d1, const QPointF &p2)
{
    QPointF perpendicular(d1.y(), -d1.x());
    qreal sp = scalarProd(perpendicular, p2 - p1);
    if (sp < 0.0)
        perpendicular *= -1.0;

    return perpendicular;
}

void KoConnectionShape::shapeChanged(ChangeType type, KoShape * shape)
{
    // check if we are during a forced update
    const bool updateIsActive = d->forceUpdate;

    m_hasMoved = true;
    switch (type) {
        case PositionChanged:
        case RotationChanged:
        case ShearChanged:
        case ScaleChanged:
        case GenericMatrixChange:
        case ParameterChanged:
            if (isParametricShape() && shape == 0)
                d->forceUpdate = true;
            break;
        case Deleted:
            if (shape != d->shape1 && shape != d->shape2)
                return;
            if (shape == d->shape1)
                setConnection1(0, -1);
            if (shape == d->shape2)
                setConnection2(0, -1);
            break;
        default:
            return;
    }

    // the connection was moved while it is connected to some other shapes
    const bool connectionChanged = !shape && (d->shape1 || d->shape2);
    // one of the connected shape has moved
    const bool connectedShapeChanged = shape && (shape == d->shape1 || shape == d->shape2);
    
    if (!updateIsActive && (connectionChanged || connectedShapeChanged) && isParametricShape())
        updateConnections();
    
    // reset the forced update flag
    d->forceUpdate = false;
}


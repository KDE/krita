/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006 Casper Boemann Rasmussen <cbr@boemann.dk>
   Copyright (C) 2006 Thomas Zander <zander@kde.org>

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

#include "KoShape.h"
#include "KoShapeContainer.h"
#include "KoRepaintManager.h"
#include "KoSelection.h"
#include "KoPointerEvent.h"
#include "KoInsets.h"
#include "KoShapeBorderModel.h"

#include <QPainter>
#include <QtDebug>
#include <QPainterPath>

KoShape::KoShape()
: m_backgroundBrush(Qt::NoBrush)
, m_border(0)
, m_scaleX( 1 )
, m_scaleY( 1 )
, m_angle( 0 )
, m_shearX( 0 )
, m_shearY( 0 )
, m_size( 50, 50 )
, m_pos( 0, 0 )
, m_zIndex( 0 )
, m_parent( 0 )
, m_visible( true )
, m_locked( false )
, m_keepAspect( false )
, m_repaintManager(0)
{
    recalcMatrix();
}

KoShape::~KoShape()
{
}

void KoShape::paintDecorations(QPainter &painter, KoViewConverter &converter, bool selected) {
    if ( selected )
    {
        // draw connectors
        QPen pen( Qt::blue );
        pen.setWidth( 0 );
        painter.setPen( pen );
        painter.setBrush( Qt::NoBrush );
        for ( int i = 0; i < m_connectors.size(); ++i )
        {
            QPointF p = converter.normalToView(m_connectors[ i ]);
            painter.drawLine( QPointF( p.x() - 2, p.y() + 2 ), QPointF( p.x() + 2, p.y() - 2 ) );
            painter.drawLine( QPointF( p.x() + 2, p.y() + 2 ), QPointF( p.x() - 2, p.y() - 2 ) );
        }
    }
}

void KoShape::scale( double sx, double sy )
{
    if(m_scaleX == sx && m_scaleY == sy)
        return;
    m_scaleX = sx;
    m_scaleY = sy;
    recalcMatrix();
}

void KoShape::rotate( double angle )
{
    if(m_angle == angle)
        return;
    m_angle = angle;
    while(m_angle >= 360) m_angle -= 360;
    while(m_angle <= -360) m_angle += 360;
    recalcMatrix();
}

void KoShape::shear( double sx, double sy )
{
    if(m_shearX == sx && m_shearY == sy)
        return;
    m_shearX = sx;
    m_shearY = sy;
    recalcMatrix();
}

void KoShape::resize( const QSizeF &size )
{
    if(m_size == size)
        return;
    m_size = size;
}

void KoShape::setPosition( const QPointF &position )
{
    if(m_pos == position)
        return;
    m_pos = position;
    recalcMatrix();
}

bool KoShape::hitTest( const QPointF &position ) const
{
    if(m_parent && m_parent->childClipped(this) && !m_parent->hitTest(position))
        return false;

    QPointF point( position * m_invMatrix );
    KoInsets *insets = new KoInsets(0, 0, 0, 0);
    if(m_border)
        m_border->borderInsets(this, *insets);

    bool hit = point.x() >= -insets->left && point.x() <= m_size.width() + insets->right &&
             point.y() >= -insets->top && point.y() <= m_size.height() + insets->bottom;
    delete insets;
    return hit;
}

QRectF KoShape::boundingRect() const
{
    QRectF bb( QPointF(0, 0), m_size );
    return m_matrix.mapRect( bb );
}

void KoShape::recalcMatrix()
{
    m_matrix = transformationMatrix(0);
    m_invMatrix = m_matrix.inverted();
}

QMatrix KoShape::transformationMatrix(KoViewConverter *converter) const {
    QMatrix matrix;
    QRectF zoomedRect = QRectF(position(), size());
    if(converter)
        zoomedRect = converter->normalToView(zoomedRect);
    matrix.translate( zoomedRect.x(), zoomedRect.y() );

    // apply parents matrix to inherit any transformations done there.
    KoShapeContainer *container = m_parent;
    KoShape const *child = this;
    while(container) {
        if(container->childClipped(child))
            matrix *= container->transformationMatrix(0);
        else {
            QPointF containerPos =container->position();
            if(converter)
                containerPos = converter->normalToView(containerPos);
            matrix.translate(containerPos.x(), containerPos.y());
        }
        container = dynamic_cast<KoShapeContainer*>(container->parent());
        child = child->parent();
    }

    if ( m_angle != 0 )
    {
        matrix.translate( zoomedRect.width() / 2.0 * m_scaleX, zoomedRect.height() / 2.0 * m_scaleY );
        matrix.rotate( m_angle );
    }
    matrix.shear( m_shearX, m_shearY );
    matrix.scale( m_scaleX, m_scaleY );
    if ( m_angle != 0 )
    {
        matrix.translate( -zoomedRect.width() / 2.0, -zoomedRect.height() / 2.0 );
    }
    return matrix;
}


bool KoShape::compareShapeZIndex(KoShape *g1, KoShape *g2) {
    return g1->zIndex() < g2->zIndex();
}

void KoShape::setParent(KoShapeContainer *parent) {
    if(dynamic_cast<KoShape*>(parent) != this)
        m_parent = parent;
    else
        m_parent = 0;
    recalcMatrix();
}

int KoShape::zIndex() const {
    if(parent()) // we can't be under our parent...
        return qMax(m_zIndex, parent()->zIndex());
    return m_zIndex;
}

void KoShape::setRepaintManager(KoRepaintManager *manager) {
    Q_ASSERT(manager);
    if(m_repaintManager) {
        // swap repaint Manager (Since that will release old ones easier)
        m_repaintManager->removeUser();
        manager->addChainedManager(m_repaintManager);
    }
    m_repaintManager = manager;
    manager->addUser();
}

void KoShape::repaint() const {
    if(m_repaintManager == 0)
        return;
    QRectF rect(QPointF(0, 0), m_size);
    if(m_border) {
        KoInsets *insets = new KoInsets(0, 0, 0, 0);
        m_border->borderInsets(this, *insets);
        rect.adjust(-insets->left, -insets->top, insets->right, insets->bottom);
        delete insets;
    }
    rect = m_matrix.mapRect(rect);
    m_repaintManager->repaint(rect, this, true);
}

void KoShape::repaint(QRectF &shape) const {
    if(m_repaintManager == 0 || !isVisible())
        return;
    QRectF rect(m_matrix.mapRect(shape));
    m_repaintManager->repaint(rect);
}

void KoShape::repaint(double x, double y, double width, double height) const {
    QRectF rect(x, y, width, height);
    repaint(rect);
}

const QPainterPath KoShape::outline() const {
    QPainterPath path;
    path.addRect(QRectF( QPointF(0, 0), m_size ));
    return path;
}

QPointF KoShape::absolutePosition() const {
    return m_matrix.map(QPointF(size().width() / 2.0 , size().height() / 2.0));
}

void KoShape::setAbsolutePosition(QPointF newPosition) {
    QPointF zero(0, 0);
    QMatrix matrix;
    // apply parents matrix to inherit any transformations done there.
    KoShapeContainer *container = m_parent;
    KoShape const *child = this;
    while(container) {
        if(container->childClipped(child)) {
            matrix *= container->transformationMatrix(0);
            break;
        }
        else {
            QPointF containerPos =container->position();
            matrix.translate(containerPos.x(), containerPos.y());
        }
        container = dynamic_cast<KoShapeContainer*>(container->parent());
        child = child->parent();
    }
    QPointF vector1 = matrix.inverted().map(zero);

    matrix = QMatrix();
    matrix.translate( size().width() / 2.0 * m_scaleX, size().height() / 2.0 * m_scaleY );
    matrix.rotate( m_angle );
    matrix.shear( m_shearX, m_shearY );
    matrix.scale( m_scaleX, m_scaleY );
    matrix.translate( -size().width() / 2.0, -size().height() / 2.0 );

    QPointF vector2 = matrix.map( QPointF(size().width() / 2.0, size().height() / 2.0) );
    //qDebug() << "vector1: " << vector1 << ", vector2: " << vector2;

    setPosition(newPosition + vector1 - vector2);
}

void KoShape::copySettings(const KoShape *shape) {
    m_pos = shape->position();
    m_scaleX = shape->scaleX();
    m_scaleY = shape->scaleY();
    m_angle = shape->rotation();
    m_shearX = shape->shearX();
    m_shearY = shape->shearY();
    m_size = shape->size();
    m_connectors.clear();
    foreach(QPointF point, shape->connectors())
        addConnectionPoint(point);
    m_zIndex = shape->zIndex();
    m_visible = shape->isVisible();
    m_locked = shape->isLocked();
    m_keepAspect = shape->keepAspectRatio();
}

void KoShape::moveBy(double distanceX, double distanceY) {
    QPointF p = absolutePosition();
    setAbsolutePosition(QPointF(p.x() + distanceX, p.y() + distanceY));
}

// static
void KoShape::applyConversion(QPainter &painter, const KoViewConverter &converter) {
    double zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    painter.scale(zoomX, zoomY);
}

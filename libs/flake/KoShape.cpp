/* This file is part of the KDE project Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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
#include "KoSelection.h"
#include "KoPointerEvent.h"
#include "KoInsets.h"
#include "KoShapeBorderModel.h"
#include "KoShapeManager.h"
#include "KoShapeUserData.h"
#include "KoShapeApplicationData.h"
#include "KoViewConverter.h"

#include <QPainter>
#include <QVariant>
#include <QPainterPath>

class KoShapePrivate {
public:
    KoShapePrivate()
        //: backgroundBrush(Qt::NoBrush),
        //border(0),
        : scaleX( 1 ),
        scaleY( 1 ),
        angle( 0 ),
        shearX( 0 ),
        shearY( 0 ),
        size( 50, 50 ),
        pos( 0, 0 ),
        zIndex( 0 ),
        parent( 0 ),
        visible( true ),
        locked( false ),
        keepAspect( false ),
        selectable( true ),
        detectCollision( false ),
        userData(0),
        appData(0)
    {
    }

    ~KoShapePrivate() {
        delete userData;
        delete appData;
    }

    double scaleX;
    double scaleY;
    double angle; // degrees
    double shearX;
    double shearY;

    QSizeF size; // size in pt
    QPointF pos; // position (top left) in pt
    QString shapeId;

    QMatrix matrix;

    QVector<QPointF> connectors; // in pt

    int zIndex;
    KoShapeContainer *parent;

    bool visible;
    bool locked;
    bool keepAspect;
    bool selectable;
    bool detectCollision;

    QSet<KoShapeManager *> shapeManagers;
    KoShapeUserData *userData;
    KoShapeApplicationData *appData;
};

KoShape::KoShape()
    : d(new KoShapePrivate())
    , m_border(0)
{
    recalcMatrix();
}

KoShape::~KoShape()
{
    delete d;
    d = 0;
}

void KoShape::paintDecorations(QPainter &painter, const KoViewConverter &converter, bool selected) {
    if ( selected )
    {
        // draw connectors
        QPen pen( Qt::blue );
        pen.setWidth( 0 );
        painter.setPen( pen );
        painter.setBrush( Qt::NoBrush );
        for ( int i = 0; i < d->connectors.size(); ++i )
        {
            QPointF p = converter.documentToView(d->connectors[ i ]);
            painter.drawLine( QPointF( p.x() - 2, p.y() + 2 ), QPointF( p.x() + 2, p.y() - 2 ) );
            painter.drawLine( QPointF( p.x() + 2, p.y() + 2 ), QPointF( p.x() - 2, p.y() - 2 ) );
        }
    }
}

void KoShape::scale( double sx, double sy )
{
    if(d->scaleX == sx && d->scaleY == sy)
        return;
    d->scaleX = sx;
    d->scaleY = sy;
    recalcMatrix();
    shapeChanged(ScaleChanged);
}

void KoShape::rotate( double angle )
{
    if(d->angle == angle)
        return;
    d->angle = angle;
    while(d->angle >= 360) d->angle -= 360;
    while(d->angle <= -360) d->angle += 360;
    recalcMatrix();
    shapeChanged(RotationChanged);
}

void KoShape::shear( double sx, double sy )
{
    if(d->shearX == sx && d->shearY == sy)
        return;
    d->shearX = sx;
    d->shearY = sy;
    recalcMatrix();
    shapeChanged(ShearChanged);
}

void KoShape::resize( const QSizeF &newSize )
{
    QSizeF s( size() );
    if(s == newSize)
        return;

    double fx = newSize.width() / s.width();
    double fy = newSize.height() / s.height();

    d->size = newSize;

    for ( int i = 0; i < d->connectors.size(); ++i )
    {
        QPointF &point = d->connectors[i];
        point.setX(point.x() * fx);
        point.setY(point.y() * fy);
    }
    recalcMatrix();
    shapeChanged(SizeChanged);
}

void KoShape::setPosition( const QPointF &position )
{
    if(d->pos == position)
        return;
    d->pos = position;
    recalcMatrix();
    shapeChanged(PositionChanged);
}

bool KoShape::hitTest( const QPointF &position ) const
{
    if(d->parent && d->parent->childClipped(this) && !d->parent->hitTest(position))
        return false;

    QPointF point( position * m_invMatrix );
    KoInsets insets(0, 0, 0, 0);
    if(m_border)
        m_border->borderInsets(this, insets);

    QSizeF s( size() );
    return point.x() >= -insets.left && point.x() <= s.width() + insets.right &&
             point.y() >= -insets.top && point.y() <= s.height() + insets.bottom;
}

QRectF KoShape::boundingRect() const
{
    QRectF bb( QPointF(0, 0), size() );
    return d->matrix.mapRect( bb );
}

void KoShape::recalcMatrix()
{
    d->matrix = transformationMatrix(0);
    m_invMatrix = d->matrix.inverted();
    notifyChanged();
}

QMatrix KoShape::transformationMatrix(const KoViewConverter *converter) const {
    QMatrix matrix;
    QRectF zoomedRect = QRectF(position(), size());
    if(converter)
        zoomedRect = converter->documentToView(zoomedRect);
    matrix.translate( zoomedRect.x(), zoomedRect.y() );

    // apply parents matrix to inherit any transformations done there.
    KoShapeContainer *container = d->parent;
    KoShape const *child = this;
    while(container) {
        if(container->childClipped(child))
            matrix *= container->transformationMatrix(0);
        else {
            QPointF containerPos =container->position();
            if(converter)
                containerPos = converter->documentToView(containerPos);
            matrix.translate(containerPos.x(), containerPos.y());
        }
        container = dynamic_cast<KoShapeContainer*>(container->parent());
        child = child->parent();
    }

    if ( d->angle != 0 )
    {
        matrix.translate( zoomedRect.width() / 2.0 * d->scaleX, zoomedRect.height() / 2.0 * d->scaleY );
        matrix.translate( zoomedRect.height() / 2.0 * d->shearX, zoomedRect.width() / 2.0 * d->shearY );
        matrix.rotate( d->angle );
        matrix.translate( -zoomedRect.width() / 2.0 * d->scaleX, -zoomedRect.height() / 2.0 * d->scaleY );
        matrix.translate( -zoomedRect.height() / 2.0 * d->shearX, -zoomedRect.width() / 2.0 * d->shearY );
    }
    matrix.shear( d->shearX, d->shearY );
    matrix.scale( d->scaleX, d->scaleY );
    return matrix;
}


bool KoShape::compareShapeZIndex(KoShape *g1, KoShape *g2) {
    return g1->zIndex() < g2->zIndex();
}

void KoShape::setParent(KoShapeContainer *parent) {
    if(dynamic_cast<KoShape*>(parent) != this)
        d->parent = parent;
    else
        d->parent = 0;
    recalcMatrix();
    shapeChanged(ParentChanged);
}

int KoShape::zIndex() const {
    if(parent()) // we can't be under our parent...
        return qMax(d->zIndex, parent()->zIndex());
    return d->zIndex;
}

void KoShape::repaint() const {
    if ( !d->shapeManagers.empty() )
    {
        foreach( KoShapeManager * manager, d->shapeManagers )
        {
            QRectF rect(QPointF(0, 0), size() );
            if(m_border) {
                KoInsets insets(0, 0, 0, 0);
                m_border->borderInsets(this, insets);
                rect.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
            }
            rect = d->matrix.mapRect(rect);
            manager->repaint( rect, this, true );
        }
    }
}

void KoShape::repaint(const QRectF &shape) const {
    if ( !d->shapeManagers.empty() && isVisible() )
    {
        QRectF rect(d->matrix.mapRect(shape));
        foreach( KoShapeManager * manager, d->shapeManagers )
        {
            manager->repaint(rect);
        }
    }
}

void KoShape::repaint(double x, double y, double width, double height) const {
    QRectF rect(x, y, width, height);
    repaint(rect);
}

const QPainterPath KoShape::outline() const {
    QPainterPath path;
    path.addRect(QRectF( QPointF(0, 0), size() ));
    return path;
}

QPointF KoShape::absolutePosition() const {
    return d->matrix.map(QPointF(size().width() / 2.0 , size().height() / 2.0));
}

void KoShape::setAbsolutePosition(QPointF newPosition) {
    QPointF zero(0, 0);
    QMatrix matrix;
    // apply parents matrix to inherit any transformations done there.
    KoShapeContainer *container = d->parent;
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
    if ( d->angle != 0 )
    {
        matrix.translate( size().width() / 2.0 * d->scaleX, size().height() / 2.0 * d->scaleY );
        matrix.translate( size().height() / 2.0 * d->shearX, size().width() / 2.0 * d->shearY );
        matrix.rotate( d->angle );
        matrix.translate( -size().width() / 2.0 * d->scaleX, -size().height() / 2.0 * d->scaleY );
        matrix.translate( -size().height() / 2.0 * d->shearX, -size().width() / 2.0 * d->shearY );
    }
    matrix.shear( d->shearX, d->shearY );
    matrix.scale( d->scaleX, d->scaleY );

    QPointF vector2 = matrix.map( QPointF(size().width() / 2.0, size().height() / 2.0) );
    //kDebug() << "vector1: " << vector1 << ", vector2: " << vector2 << endl;

    setPosition(newPosition + vector1 - vector2);
}

void KoShape::copySettings(const KoShape *shape) {
    d->pos = shape->position();
    d->scaleX = shape->scaleX();
    d->scaleY = shape->scaleY();
    d->angle = shape->rotation();
    d->shearX = shape->shearX();
    d->shearY = shape->shearY();
    d->size = shape->size();
    d->connectors.clear();
    foreach(QPointF point, shape->connectors())
        addConnectionPoint(point);
    d->zIndex = shape->zIndex();
    d->visible = shape->isVisible();
    d->locked = shape->isLocked();
    d->keepAspect = shape->keepAspectRatio();
}

void KoShape::moveBy(double distanceX, double distanceY) {
    QPointF p = absolutePosition();
    setAbsolutePosition(QPointF(p.x() + distanceX, p.y() + distanceY));
}

void KoShape::notifyChanged()
{
    foreach( KoShapeManager * manager, d->shapeManagers )
    {
        manager->notifyShapeChanged( this );
    }
}

void KoShape::setUserData(KoShapeUserData *userData) {
    if(d->userData)
        delete d->userData;
    d->userData = userData;
}

KoShapeUserData *KoShape::userData() const {
    return d->userData;
}

void KoShape::setApplicationData(KoShapeApplicationData *appData) {
    delete d->appData;
    d->appData = appData;
}

KoShapeApplicationData *KoShape::applicationData() const {
    return d->appData;
}

bool KoShape::hasTransparency() {
    if(m_backgroundBrush.style() == Qt::NoBrush)
        return true;
    return !m_backgroundBrush.isOpaque();
}

KoInsets KoShape::borderInsets() const {
    KoInsets answer;
    if(m_border)
        m_border->borderInsets(this, answer);
    return answer;
}

double KoShape::scaleX() const {
    return d->scaleX;
}
double KoShape::scaleY() const {
    return d->scaleY;
}

double KoShape::rotation() const {
    return d->angle;
}

double KoShape::shearX() const {
    return d->shearX;
}

double KoShape::shearY() const {
    return d->shearY;
}

QSizeF KoShape::size () const {
    return d->size;
}

QPointF KoShape::position() const {
    return d->pos;
}

void KoShape::addConnectionPoint( const QPointF &point ) {
    d->connectors.append( point );
}

QList<QPointF> KoShape::connectors() const {
    return d->connectors.toList();
}

void KoShape::setBackground ( const QBrush & brush ) {
    m_backgroundBrush = brush;
}

const QBrush& KoShape::background () {
    return m_backgroundBrush;
}

void KoShape::setZIndex(int zIndex) {
    d->zIndex = zIndex;
}

void KoShape::setVisible(bool on) {
    d->visible = on;
}

bool KoShape::isVisible() const {
    return d->visible;
}

void KoShape::setSelectable(bool selectable) {
    d->selectable = selectable;
}

bool KoShape::isSelectable() const {
    return d->selectable;
}

void KoShape::setLocked(bool locked) {
    d->locked = locked;
}

bool KoShape::isLocked() const {
    return d->locked;
}

KoShapeContainer *KoShape::parent() const {
    return d->parent;
}

void KoShape::setKeepAspectRatio(bool keepAspect) {
    d->keepAspect = keepAspect;
}

bool KoShape::keepAspectRatio() const {
    return d->keepAspect;
}

const QString &KoShape::shapeId() const {
    return d->shapeId;
}

void KoShape::setShapeId(const QString &id) {
    d->shapeId = id;
}

void KoShape::setCollisionDetection(bool detect) {
    d->detectCollision = detect;
}

bool KoShape::collisionDetection() {
    return d->detectCollision;
}

void KoShape::addShapeManager( KoShapeManager * manager ) {
    d->shapeManagers.insert( manager );
}

void KoShape::removeShapeManager( KoShapeManager * manager ) {
    d->shapeManagers.remove( manager );
}

// static
void KoShape::applyConversion(QPainter &painter, const KoViewConverter &converter) {
    double zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    painter.scale(zoomX, zoomY);
}


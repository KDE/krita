/* This file is part of the KDE project Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006 Casper Boemann Rasmussen <cbr@boemann.dk>
   Copyright (C) 2006 Thomas Zander <zander@kde.org>
   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>

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
#include "KoShapeSavingContext.h"
#include "KoViewConverter.h"

#include <KoXmlWriter.h>
#include <KoGenStyles.h>
#include <KoGenStyle.h>

#include <QPainter>
#include <QVariant>
#include <QPainterPath>

#include <kdebug.h>

class KoShapePrivate {
public:
    KoShapePrivate()
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
        appData(0),
        backgroundBrush(Qt::NoBrush),
        border(0)
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
    QBrush backgroundBrush; ///< Stands for the background color / fill etc.
    KoShapeBorderModel *border; ///< points to a border, or 0 if there is no border
};

KoShape::KoShape()
    : d(new KoShapePrivate())
{
    recalcMatrix();
}

KoShape::~KoShape()
{
    delete d;
}

void KoShape::paintDecorations(QPainter &painter, const KoViewConverter &converter, const KoCanvasBase *canvas) {
    Q_UNUSED(painter);
    Q_UNUSED(converter);
    Q_UNUSED(canvas);
/* Since this code is not actually used (kivio is going to be the main user) lets disable instead of fix.
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
    }*/
}

bool KoShape::saveOdf( KoShapeSavingContext & context )
{
    const char * tag = odfTagName();
    if ( tag[0] == '\0' )
    {
        kWarning(30006) << "No support for shape yet. Not saving!" << endl;
    }
    else
    {
        context.xmlWriter().startElement( tag );
        context.xmlWriter().addAttribute( context.isSet( KoShapeSavingContext::PresentationShape ) ? 
                                          "presentation:style-name": "draw:style-name", 
                                          getStyle( context ) );

        if ( context.isSet( KoShapeSavingContext::DrawId ) )
        {
            context.xmlWriter().addAttribute( "draw:id", context.drawId( this ) );
        }

        saveOdfData( context );
        context.xmlWriter().endElement();
    }
    return true;
}

QString KoShape::getStyle( KoShapeSavingContext &context )
{
    KoGenStyle style;
    if ( context.isSet( KoShapeSavingContext::PresentationShape ) ) {
        style = KoGenStyle( KoGenStyle::STYLE_PRESENTATIONAUTO, "presentation" );
    }
    else {
        style = KoGenStyle( KoGenStyle::STYLE_GRAPHICAUTO, "graphic" );
    }

    fillStyle( style, context );
    
    if ( context.isSet( KoShapeSavingContext::AutoStyleInStyleXml ) ) {
        style.setAutoStyleInStylesDotXml( true );
    }

    return context.mainStyles().lookup( style, context.isSet( KoShapeSavingContext::PresentationShape ) ? "pr" : "gr" );
}

void KoShape::fillStyle( KoGenStyle &style, KoShapeSavingContext &context )
{
    KoShapeBorderModel * b = border();
    if ( b )
    {
        b->fillStyle( style, context );
    }
    QBrush bg( background() );
    switch ( bg.style() )
    {
        case Qt::NoBrush:
            style.addProperty( "draw:fill","none" );
            break;
        default:    
            //KoOasisStyles::saveOasisFillStyle( style, context.mainStyles(), bg );
            break;
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

    QPointF point( position * d->matrix.inverted() );
    KoInsets insets(0, 0, 0, 0);
    if(d->border)
        d->border->borderInsets(this, insets);

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
            if(d->border) {
                KoInsets insets(0, 0, 0, 0);
                d->border->borderInsets(this, insets);
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
    //kDebug(30006) << "vector1: " << vector1 << ", vector2: " << vector2 << endl;

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
    if(d->backgroundBrush.style() == Qt::NoBrush)
        return true;
    return !d->backgroundBrush.isOpaque();
}

KoInsets KoShape::borderInsets() const {
    KoInsets answer;
    if(d->border)
        d->border->borderInsets(this, answer);
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
    d->backgroundBrush = brush;
}

const QBrush& KoShape::background () {
    return d->backgroundBrush;
}

void KoShape::setZIndex(int zIndex) {
    notifyChanged();
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

KoShapeBorderModel *KoShape::border() const {
    return d->border;
}

void KoShape::setBorder(KoShapeBorderModel *border) {
    d->border = border;
}

const QMatrix& KoShape::matrix() const {
    return d->matrix;
}


// static
void KoShape::applyConversion(QPainter &painter, const KoViewConverter &converter) {
    double zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    painter.scale(zoomX, zoomY);
}


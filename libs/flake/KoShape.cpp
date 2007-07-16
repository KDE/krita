/* This file is part of the KDE project
   Copyright (C) 2006 Casper Boemann Rasmussen <cbr@boemann.dk>
   Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>

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
#include "KoShapeLayer.h"
#include "KoShapeContainerModel.h"
#include "KoSelection.h"
#include "KoPointerEvent.h"
#include "KoInsets.h"
#include "KoShapeBorderModel.h"
#include "KoShapeManager.h"
#include "KoShapeUserData.h"
#include "KoShapeApplicationData.h"
#include "KoShapeSavingContext.h"
#include "KoShapeLoadingContext.h"
#include "KoViewConverter.h"
#include "KoLineBorder.h"

#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoGenStyles.h>
#include <KoGenStyle.h>
#include <KoUnit.h>
#include <KoOasisStyles.h>
#include <KoOasisLoadingContext.h>

#include <QPainter>
#include <QVariant>
#include <QPainterPath>
#include <QList>

#include <kdebug.h>

class KoShape::Private {
public:
    Private(KoShape *shape)
        : size( 50, 50 ),
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
        border(0),
        me(shape)
    {
    }

    ~Private() {
        foreach(KoShapeManager *manager, shapeManagers)
            manager->remove(me);
        delete userData;
        delete appData;
    }

    void shapeChanged(ChangeType type) {
        if(parent)
            parent->model()->childChanged(me, type);
        me->shapeChanged(type);
    }

    QSizeF size; // size in pt
    QPointF pos; // position (top left) in pt
    QString shapeId;
    QString name; ///< the shapes names

    QMatrix localMatrix; ///< the shapes local transformation matrix

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
    QList<KoShapeConnection*> connections;
    KoShape *me;
};

KoShape::KoShape()
    : d(new Private(this))
{
    notifyChanged();
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

void KoShape::scale( double sx, double sy )
{
    QPointF pos = position();
    QMatrix scaleMatrix;
    scaleMatrix.translate( pos.x(), pos.y() );
    scaleMatrix.scale( sx, sy );
    scaleMatrix.translate( -pos.x(), -pos.y() );
    d->localMatrix = d->localMatrix * scaleMatrix;

    notifyChanged();
    d->shapeChanged(ScaleChanged);
}

void KoShape::rotate( double angle )
{
    QPointF center = d->localMatrix.map( QPointF( 0.5*size().width(), 0.5*size().height() ) );
    QMatrix rotateMatrix;
    rotateMatrix.translate( center.x(), center.y() );
    rotateMatrix.rotate( angle );
    rotateMatrix.translate( -center.x(), -center.y() );
    d->localMatrix = d->localMatrix * rotateMatrix;

    notifyChanged();
    d->shapeChanged(RotationChanged);
}

void KoShape::shear( double sx, double sy )
{
    QPointF pos = position();
    QMatrix shearMatrix;
    shearMatrix.translate( pos.x(), pos.y() );
    shearMatrix.shear( sx, sy );
    shearMatrix.translate( -pos.x(), -pos.y() );
    d->localMatrix = d->localMatrix * shearMatrix;

    notifyChanged();
    d->shapeChanged(ShearChanged);
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
    notifyChanged();
    d->shapeChanged(SizeChanged);
}

void KoShape::setPosition( const QPointF &newPosition )
{
    QPointF currentPos = position();
    if( newPosition == currentPos )
        return;
    QMatrix translateMatrix;
    translateMatrix.translate( newPosition.x()-currentPos.x(), newPosition.y()-currentPos.y() );
    d->localMatrix = d->localMatrix * translateMatrix;

    notifyChanged();
    d->shapeChanged(PositionChanged);
}

bool KoShape::hitTest( const QPointF &position ) const
{
    if(d->parent && d->parent->childClipped(this) && !d->parent->hitTest(position))
        return false;

    QPointF point = transformationMatrix(0).inverted().map( position );
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
    if(d->border) {
        KoInsets insets;
        d->border->borderInsets(this, insets);
        bb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }
    return transformationMatrix(0).mapRect( bb );
}

QMatrix KoShape::transformationMatrix(const KoViewConverter *converter) const {
    QMatrix matrix;
    // apply parents matrix to inherit any transformations done there.
    KoShapeContainer * container = d->parent;
    if( container ) {
        if( container->childClipped(this) )
            matrix = container->transformationMatrix(0);
        else {
            QSizeF containerSize = container->size();
            QPointF containerPos = container->absolutePosition() - QPointF( 0.5*containerSize.width(), 0.5*containerSize.height() );
            if(converter)
                containerPos = converter->documentToView(containerPos);
            matrix.translate( containerPos.x(), containerPos.y() );
        }
    }

    if(converter) {
        QPointF pos = d->localMatrix.map( QPoint() );
        QPointF trans = converter->documentToView( pos ) - pos;
        matrix.translate( trans.x(), trans.y() );
    }

    return d->localMatrix * matrix;
}

void KoShape::applyTransformation( const QMatrix &matrix )
{
    QMatrix globalMatrix = transformationMatrix(0);
    // the transformation is relative to the global coordinate system
    // but we want to change the local matrix, so convert the matrix
    // to relate to the local coordinate system
    QMatrix transformMatrix = globalMatrix * matrix * globalMatrix.inverted();
    d->localMatrix = transformMatrix * d->localMatrix;
    notifyChanged();
}

void KoShape::setTransformation( const QMatrix &matrix )
{
    d->localMatrix = matrix;
    notifyChanged();
}

bool KoShape::compareShapeZIndex(KoShape *s1, KoShape *s2) {
    int diff = s1->zIndex() - s2->zIndex();
    if(diff == 0) {
        KoShape *s = s1->parent();
        while(s) {
            if(s == s2) // s1 is a child of s2
                return false; // children are always on top of their parents.
            s = s->parent();
        }
        s = s2->parent();
        while(s) {
            if(s == s1) // s2 is a child of s1
                return true;
            s = s->parent();
        }
    }
    return diff < 0;
}

void KoShape::setParent(KoShapeContainer *parent) {
    if(d->parent == parent)
        return;
    if(parent && dynamic_cast<KoShape*>(parent) != this) {
        d->parent = parent;
        parent->addChild(this);
    }
    else
        d->parent = 0;
    notifyChanged();
    d->shapeChanged(ParentChanged);
}

int KoShape::zIndex() const {
    if(parent()) // we can't be under our parent...
        return qMax(d->zIndex, parent()->zIndex());
    return d->zIndex;
}

void KoShape::repaint() const {
    if ( !d->shapeManagers.empty() )
    {
        QRectF rect(QPointF(0, 0), size() );
        if(d->border) {
            KoInsets insets;
            d->border->borderInsets(this, insets);
            rect.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
        }
        rect = transformationMatrix(0).mapRect(rect);
        foreach( KoShapeManager * manager, d->shapeManagers )
            manager->repaint( rect, this, true );
    }
}

void KoShape::repaint(const QRectF &shape) const {
    if ( !d->shapeManagers.empty() && isVisible() )
    {
        QRectF rect(transformationMatrix(0).mapRect(shape));
        foreach( KoShapeManager * manager, d->shapeManagers )
        {
            manager->repaint(rect);
        }
    }
}

const QPainterPath KoShape::outline() const {
    QPainterPath path;
    path.addRect(QRectF( QPointF(0, 0), size() ));
    return path;
}

QPointF KoShape::absolutePosition(KoFlake::Position anchor) const {
    QPointF point;
    switch(anchor) {
        case KoFlake::TopLeftCorner: break;
        case KoFlake::TopRightCorner: point = QPointF(size().width(), 0.0); break;
        case KoFlake::BottomLeftCorner: point = QPointF(0.0, size().height()); break;
        case KoFlake::BottomRightCorner: point = QPointF(size().width(), size().height()); break;
        case KoFlake::CenteredPositon: point = QPointF(size().width() / 2.0, size().height() / 2.0); break;
    }
    return transformationMatrix(0).map(point);
}

void KoShape::setAbsolutePosition(QPointF newPosition, KoFlake::Position anchor) {
    QPointF currentAbsPosition = absolutePosition( anchor );
    QPointF translate = newPosition - currentAbsPosition;
    QMatrix translateMatrix;
    translateMatrix.translate( translate.x(), translate.y() );
    applyTransformation( translateMatrix );
    notifyChanged();
    d->shapeChanged(PositionChanged);
}

void KoShape::copySettings(const KoShape *shape) {
    d->pos = shape->position();
    d->size = shape->size();
    d->connectors.clear();
    foreach(QPointF point, shape->connectors())
        addConnectionPoint(point);
    d->zIndex = shape->zIndex();
    d->visible = shape->isVisible();
    d->locked = shape->isLocked();
    d->keepAspect = shape->keepAspectRatio();
    d->localMatrix = shape->d->localMatrix;
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
    return 0.0;
}
double KoShape::scaleY() const {
    return 0.0;
}

double KoShape::rotation() const {
    // try to extract the rotation angle out of the local matrix 
    // if it is a pure rotation matrix

    // check if the matrix has shearing mixed in
    if( fabs( fabs(d->localMatrix.m12()) - fabs(d->localMatrix.m21()) ) > 1e-10 )
        return NAN;
    // check if the matrix has scaling mixed in
    if( fabs( d->localMatrix.m11() - d->localMatrix.m22() ) > 1e-10 )
        return NAN;

    // calculate the angle from the matrix elements
    double angle = atan2( d->localMatrix.m21(), d->localMatrix.m11() ) * 180.0 / M_PI;
    if( angle < 0.0 )
        angle += 360.0;

    return angle;
}

double KoShape::shearX() const {
    return 0.0;
}

double KoShape::shearY() const {
    return 0.0;
}

QSizeF KoShape::size () const {
    return d->size;
}

QPointF KoShape::position() const {
    QPointF center( 0.5*size().width(), 0.5*size().height() );
    return d->localMatrix.map( center ) - center;
    //return d->localMatrix.map( QPointF(0,0) );
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

QBrush KoShape::background() const {
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
    return d->localMatrix;
}

void KoShape::addConnection(KoShapeConnection *connection) {
    d->connections.append(connection);
    foreach(KoShapeManager *sm, d->shapeManagers)
        sm->addShapeConnection( connection );
}

void KoShape::removeConnection(KoShapeConnection *connection) {
    d->connections.removeAll(connection);
}

QList<KoShapeConnection*> KoShape::connections() const {
    return d->connections;
}

QString KoShape::name() const {
    return d->name;
}

void KoShape::setName( const QString & name ) {
    d->name = name;
}

// loading & saving methods
void KoShape::saveOdfConnections(KoShapeSavingContext &context) const {
    // TODO  save "draw-glue-point" elements (9.2.19)
    Q_UNUSED( context );
}

QString KoShape::style( KoShapeSavingContext &context ) const
{
    KoGenStyle style;
    if ( context.isSet( KoShapeSavingContext::PresentationShape ) ) {
        style = KoGenStyle( KoGenStyle::STYLE_PRESENTATIONAUTO, "presentation" );
    }
    else {
        style = KoGenStyle( KoGenStyle::STYLE_GRAPHICAUTO, "graphic" );
    }

    // and fill the style
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
        default:    // TODO all the other ones.
            KoOasisStyles::saveOasisFillStyle( style, context.mainStyles(), bg );
            break;
    }

    if ( context.isSet( KoShapeSavingContext::AutoStyleInStyleXml ) ) {
        style.setAutoStyleInStylesDotXml( true );
    }

    return context.mainStyles().lookup( style, context.isSet( KoShapeSavingContext::PresentationShape ) ? "pr" : "gr" );
}

bool KoShape::loadOdfAttributes( const KoXmlElement & element, KoShapeLoadingContext &context, int attributes )
{
    if ( attributes & OdfMandatories ) {
        if ( element.hasAttributeNS( KoXmlNS::draw, "layer" ) ) {
            KoShapeLayer * layer = context.layer( element.attributeNS( KoXmlNS::draw, "layer" ) );
            if ( layer ) {
                setParent( layer );
            }
        }
        if ( element.hasAttributeNS( KoXmlNS::draw, "id" ) ) {
            QString id = element.attributeNS( KoXmlNS::draw, "id" );
            if ( !id.isNull() ) {
                context.addShapeId( this, id );
            }
        }
        if ( element.hasAttributeNS( KoXmlNS::draw, "z-index" ) ) {
            // TODO what do we do in case of copy/paste
            setZIndex( element.attributeNS( KoXmlNS::draw, "z-index" ).toInt() );
        }
        else {
            // TODO what do we do in the case the z-index is not there then the order in the doc
            // is the the order of the z-index
        }

        setBackground( loadOdfFill( element, context ) );
        setBorder( loadOdfStroke( element, context ) );
    }

    if ( attributes & OdfSize ) {
        QPointF pos;
        pos.setX( KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "x", QString() ) ) );
        pos.setY( KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "y", QString() ) ) );
        setPosition( pos );

        QSizeF size;
        size.setWidth( KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "width", QString() ) ) );
        size.setHeight( KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "height", QString() ) ) );
        resize( size );
    }

    if( attributes & OdfTransformation )
    {
        QString transform = element.attributeNS( KoXmlNS::draw, "transform", QString() );
        if( ! transform.isEmpty() )
            applyTransformation( parseOdfTransform( transform ) );
    }

    return true;
}

QBrush KoShape::loadOdfFill( const KoXmlElement & element, KoShapeLoadingContext & context )
{
    KoStyleStack &styleStack = context.koLoadingContext().styleStack();
    QString fill;
    if( element.hasAttributeNS( KoXmlNS::draw, "style-name" ) )
    {
        // fill the style stack with the shapes style
        context.koLoadingContext().fillStyleStack( element, KoXmlNS::draw, "style-name", "graphic" );
        styleStack.setTypeProperties( "graphic" );
        if( styleStack.hasProperty( KoXmlNS::draw, "fill" ) )
            fill = styleStack.property( KoXmlNS::draw, "fill" );
    }
    else if( element.hasAttributeNS( KoXmlNS::presentation, "style-name" ) )
    {
        // fill the style stack with the shapes style
        context.koLoadingContext().fillStyleStack( element, KoXmlNS::presentation, "style-name", "presentation" );
        styleStack.setTypeProperties( "presentation" );
        if ( styleStack.hasProperty( KoXmlNS::presentation, "fill" ) )
            fill = styleStack.property( KoXmlNS::presentation, "fill" );
    }

    if ( fill == "solid" || fill == "hatch" )
        return KoOasisStyles::loadOasisFillStyle( styleStack, fill, context.koLoadingContext().oasisStyles() );

    return QBrush();
}

KoShapeBorderModel * KoShape::loadOdfStroke( const KoXmlElement & element, KoShapeLoadingContext & context )
{
    KoStyleStack &styleStack = context.koLoadingContext().styleStack();
    QString stroke;
    if( element.hasAttributeNS( KoXmlNS::draw, "style-name" ) )
    {
        // fill the style stack with the shapes style
        context.koLoadingContext().fillStyleStack( element, KoXmlNS::draw, "style-name", "graphic" );
        styleStack.setTypeProperties( "graphic" );
        if( styleStack.hasProperty( KoXmlNS::draw, "stroke" ) )
            stroke = styleStack.property( KoXmlNS::draw, "stroke" );
    }
    else if( element.hasAttributeNS( KoXmlNS::draw, "style-name" ) )
    {
        // fill the style stack with the shapes style
        context.koLoadingContext().fillStyleStack( element, KoXmlNS::presentation, "style-name", "presentation" );
        styleStack.setTypeProperties( "presentation" );
        if ( styleStack.hasProperty( KoXmlNS::presentation, "stroke" ) )
            stroke = styleStack.property( KoXmlNS::presentation, "stroke" );
    }

    KoLineBorder * border = 0;

    if( stroke == "solid" )
    {
        border = new KoLineBorder();
    }
    else if( stroke == "dash" )
    {
        border = new KoLineBorder();
        if( styleStack.hasProperty( KoXmlNS::draw, "stroke-dash" ) )
        {
            QString dashStyleName = styleStack.property( KoXmlNS::draw, "stroke-dash" );
            // TODO load dashes
        }
    }
    else 
        return 0;

    if ( styleStack.hasProperty( KoXmlNS::svg, "stroke-color" ) )
        border->setColor( styleStack.property( KoXmlNS::svg, "stroke-color" ) );
    if ( styleStack.hasProperty( KoXmlNS::svg, "stroke-opacity" ) )
    {
        QColor color = border->color();
        QString opacity = styleStack.property( KoXmlNS::svg, "stroke-opacity" );
        color.setAlphaF( opacity.toDouble() );
        border->setColor( color );
    }
    if( styleStack.hasProperty( KoXmlNS::svg, "stroke-width" ) )
        border->setLineWidth( KoUnit::parseValue( styleStack.property( KoXmlNS::svg, "stroke-width" ) ) );
    if( styleStack.hasProperty( KoXmlNS::draw, "stroke-linejoin" ) )
    {
        QString join = styleStack.property( KoXmlNS::draw, "stroke-linejoin" );
        if( join == "bevel" )
            border->setJoinStyle( Qt::BevelJoin );
        else if( join == "round" )
            border->setJoinStyle( Qt::RoundJoin );
        else
            border->setJoinStyle( Qt::MiterJoin );
    }

    return border;
}

QMatrix KoShape::parseOdfTransform( const QString &transform )
{
    QMatrix matrix;

    // Split string for handling 1 transform statement at a time
    QStringList subtransforms = transform.split(')', QString::SkipEmptyParts);
    QStringList::ConstIterator it = subtransforms.begin();
    QStringList::ConstIterator end = subtransforms.end();
    for(; it != end; ++it)
    {
        QStringList subtransform = (*it).split('(', QString::SkipEmptyParts);

        subtransform[0] = subtransform[0].trimmed().toLower();
        subtransform[1] = subtransform[1].simplified();
        QRegExp reg("[,( ]");
        QStringList params = subtransform[1].split(reg, QString::SkipEmptyParts);

        if(subtransform[0].startsWith(';') || subtransform[0].startsWith(','))
            subtransform[0] = subtransform[0].right(subtransform[0].length() - 1);

        if(subtransform[0] == "rotate")
        {
            // TODO find out what oo2 really does when rotating, it seems severly broken
            if(params.count() == 3)
            {
                double x = KoUnit::parseValue( params[1] );
                double y = KoUnit::parseValue( params[2] );

                matrix.translate(x, y);
                // oo2 rotates by radians
                matrix.rotate( params[0].toDouble()*180.0/M_PI );
                matrix.translate(-x, -y);
            }
            else
            {
                // oo2 rotates by radians
                matrix.rotate( params[0].toDouble()*180.0/M_PI );
            }
        }
        else if(subtransform[0] == "translate")
        {
            if(params.count() == 2)
            {
                double x = KoUnit::parseValue( params[0] );
                double y = KoUnit::parseValue( params[1] );
                matrix.translate(x, y);
            }
            else    // Spec : if only one param given, assume 2nd param to be 0
                matrix.translate( KoUnit::parseValue( params[0] ) , 0);
        }
        else if(subtransform[0] == "scale")
        {
            if(params.count() == 2)
                matrix.scale(params[0].toDouble(), params[1].toDouble());
            else    // Spec : if only one param given, assume uniform scaling
                matrix.scale(params[0].toDouble(), params[0].toDouble());
        }
        else if(subtransform[0] == "skewx")
            matrix.shear(tan(params[0].toDouble()), 0.0F);
        else if(subtransform[0] == "skewy")
            matrix.shear(tan(params[0].toDouble()), 0.0F);
        else if(subtransform[0] == "skewy")
            matrix.shear(0.0F, tan(params[0].toDouble()));
        else if(subtransform[0] == "matrix")
        {
            if(params.count() >= 6)
                matrix.setMatrix(params[0].toDouble(), params[1].toDouble(), params[2].toDouble(), params[3].toDouble(), KoUnit::parseValue( params[4] ), KoUnit::parseValue( params[5] ) );
        }
    }

    return matrix;
}

void KoShape::saveOdfFrameAttributes(KoShapeSavingContext &context) const {
    saveOdfAttributes(context, FrameAttributes);
    context.addOption(KoShapeSavingContext::FrameOpened);
}

void KoShape::saveOdfAttributes(KoShapeSavingContext &context, int attributes) const {
    if(attributes & OdfMandatories) {
        // all items that should be written to 'draw:frame' and any other 'draw:' object that inherits this shape
        context.xmlWriter().addAttribute( context.isSet( KoShapeSavingContext::PresentationShape ) ?
                                          "presentation:style-name": "draw:style-name",
                                          style( context ) );

        if ( context.isSet( KoShapeSavingContext::DrawId ) )
        {
            context.xmlWriter().addAttribute( "draw:id", context.drawId( this ) );
        }

        if(d->parent && dynamic_cast<KoShapeLayer*> (d->parent))
            context.xmlWriter().addAttribute("draw:layer", d->parent->name());
    }

    // all items after this should not be written out when they have already be written in
    // a 'draw:frame' attribute.
    if(context.isSet(KoShapeSavingContext::FrameOpened)) {
        context.removeOption(KoShapeSavingContext::FrameOpened);
        return;
    }

    if(attributes & OdfSize) {
        QSizeF s( size() );
        context.xmlWriter().addAttributePt( "svg:width", s.width() );
        context.xmlWriter().addAttributePt( "svg:height", s.height() );
        context.xmlWriter().addAttributePt( "svg:x", d->pos.x() );
        context.xmlWriter().addAttributePt( "svg:y", d->pos.y() );
    }

    if(attributes & OdfMandatories) {
        context.xmlWriter().addAttribute("draw:z-index", zIndex());
    }

    if(attributes & OdfTransformation) {
        // just like in shapes; ODF allows you to manipulate the 'matrix' after setting an
        // ofset on the shape (using the x and y positions).   Lets save them here.
        /*
        bool rotate = qAbs(d->angle) > 1E-6;
        bool skew = qAbs(d->shearX) > 1E-6 || qAbs(d->shearY) > 1E-6;
        bool scale = qAbs(d->scaleX - 1) > 1E-6 || qAbs(d->scaleY -1) > 1E-6;

        if(rotate && (skew || scale)) {
            QMatrix matrix; // can't use transformationMatrix() as that includes transformation of the container as well.
            QSizeF size(this->size());
            if ( d->angle != 0 )
            {
                matrix.translate( size.width() / 2.0 * d->scaleX, size.height() / 2.0 * d->scaleY );
                matrix.translate( size.height() / 2.0 * d->shearX, size.width() / 2.0 * d->shearY );
                matrix.rotate( d->angle );
                matrix.translate( -size.width() / 2.0 * d->scaleX, -size.height() / 2.0 * d->scaleY );
                matrix.translate( -size.height() / 2.0 * d->shearX, -size.width() / 2.0 * d->shearY );
            }
            matrix.shear( d->shearX, d->shearY );
            matrix.scale( d->scaleX, d->scaleY );

            QString m = QString( "matrix(0 0 %3 %4 %5pt %6pt)" ).arg( matrix.m11() ).arg( matrix.m12() )
                .arg( matrix.m21() ).arg( matrix.m22() )
                .arg( matrix.dx() ) .arg( matrix.dy() );
            context.xmlWriter().addAttribute( "draw:transform", m );
        }
        else if(rotate || skew || scale) {
            QString transform;
            if(rotate)
                transform = "rotate("+ QString::number(d->angle) +')';
            if(skew)
                transform = "skewX("+ QString::number(d->shearX) +") skewY("+ QString::number(d->shearY) +')';
            if(scale) {
                transform += "scale("+ QString::number(d->scaleX);
                if(d->scaleX != d->scaleY)
                    transform += ','+ QString::number(d->scaleY);
                transform += ')';
            }

            context.xmlWriter().addAttribute( "draw:transform", transform );
        }
        */
        if( ! d->localMatrix.isIdentity() )
        {
            QString m = QString( "matrix(%1 %2 %3 %4 %5pt %6pt)" )
                    .arg( d->localMatrix.m11() ).arg( d->localMatrix.m12() )
                    .arg( d->localMatrix.m21() ).arg( d->localMatrix.m22() )
                    .arg( d->localMatrix.dx() ) .arg( d->localMatrix.dy() );
            context.xmlWriter().addAttribute( "draw:transform", m );
        }
    }
}

// end loading & saving methods


// static
void KoShape::applyConversion(QPainter &painter, const KoViewConverter &converter) {
    double zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    painter.scale(zoomX, zoomY);
}


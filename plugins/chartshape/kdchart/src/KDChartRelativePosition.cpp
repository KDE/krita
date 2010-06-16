/****************************************************************************
 ** Copyright (C) 2007 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Chart library.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License versions 2.0 or 3.0 as published by the Free Software
 ** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file.  Alternatively you may (at
 ** your option) use any later version of the GNU General Public
 ** License if such license has been publicly approved by
 ** Klarälvdalens Datakonsult AB (or its successors, if any).
 ** 
 ** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
 ** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE. Klarälvdalens Datakonsult AB reserves all rights
 ** not expressly granted herein.
 ** 
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 **********************************************************************/

#include "KDChartRelativePosition.h"

#include "KDChartEnums.h"
#include "KDChartMeasure.h"
#include "KDChartPosition.h"
#include "KDChartAbstractArea.h"

#include <QWidget>
#include <QLayout>

#include <KDABLibFakes>

using namespace KDChart;

class RelativePosition::Private {
    friend class ::KDChart::RelativePosition;
public:
    Private();
    ~Private();

private:
    QObject* area;
    PositionPoints points;
    Position position;
    Qt::Alignment alignment;
    Measure horizontalPadding;
    Measure verticalPadding;
    qreal rotation;
};


RelativePosition::Private::Private()
    : area( 0 ),
      alignment( Qt::AlignCenter ),
      rotation( 0 )
{

}

RelativePosition::Private::~Private()
{}



RelativePosition::RelativePosition()
    : _d( new Private )
{

}

RelativePosition::RelativePosition( const RelativePosition& r )
    : _d( new Private( *r._d ) )
{

}

RelativePosition & RelativePosition::operator=( const RelativePosition & other ) {
    RelativePosition copy( other );
    copy.swap( *this );
    return *this;
}

RelativePosition::~RelativePosition()
{
    delete _d;
}

#define d d_func()

void RelativePosition::setReferenceArea( QObject * area ) {
    d->area = area;
    if( area )
        setReferencePoints( PositionPoints() );
}

QObject * RelativePosition::referenceArea() const {
    return d->area;
}

void RelativePosition::setReferencePoints( const PositionPoints& points ){
    d->points = points;
    if( !points.isNull() )
        setReferenceArea( 0 );
}
const PositionPoints RelativePosition::referencePoints() const{
    return d->points;
}

void RelativePosition::setReferencePosition( Position pos ) {
    d->position = pos;
}

void RelativePosition::resetReferencePosition() {
    d->position = Position::Unknown;
}

Position RelativePosition::referencePosition() const {
    return d->position;
}

void RelativePosition::setAlignment( Qt::Alignment align ) {
    d->alignment = align;
}

Qt::Alignment RelativePosition::alignment() const {
    return d->alignment;
}

void RelativePosition::setHorizontalPadding( const Measure & pad ) {
    d->horizontalPadding = pad;
}

Measure RelativePosition::horizontalPadding() const {
    return d->horizontalPadding;
}

void RelativePosition::setVerticalPadding( const Measure & pad ) {
    d->verticalPadding = pad;
}

Measure RelativePosition::verticalPadding() const {
    return d->verticalPadding;
}

void RelativePosition::setRotation( qreal rot ) {
    d->rotation = rot;
}

qreal RelativePosition::rotation() const {
    return d->rotation;
}


const QPointF RelativePosition::referencePoint(qreal* polarDegrees) const
{
    bool useRect = (d->area != 0);
    QRect rect;
    if( useRect ){
        const QWidget* widget = dynamic_cast<const QWidget*>(d->area);
        if( widget ){
            const QLayout * layout = widget->layout();
            rect = layout ? layout->geometry() : widget->geometry();
        }else{
            const AbstractArea* kdcArea = dynamic_cast<const AbstractArea*>(d->area);
            if( kdcArea )
                rect = kdcArea->geometry();
            else
                useRect = false;
        }
    }
    QPointF pt;
    if ( useRect ){
        pt = PositionPoints( rect ).point( d->position );
        if( polarDegrees )
            *polarDegrees = 0.0;
    }else{
        pt = d->points.point( d->position );
        if( polarDegrees )
            *polarDegrees = d->points.degrees( d->position.value() );
    }
    return pt;
}


const QPointF RelativePosition::calculatedPoint( const QSizeF& autoSize ) const
{
    const qreal dx = horizontalPadding().calculatedValue( autoSize, KDChartEnums::MeasureOrientationHorizontal );
    const qreal dy = verticalPadding()  .calculatedValue( autoSize, KDChartEnums::MeasureOrientationVertical );
    qreal polarDegrees;
    QPointF pt( referencePoint( &polarDegrees ) );
    if( polarDegrees == 0.0 ){
        pt += QPointF(dx, dy);
    }else{
        const qreal rad = DEGTORAD( polarDegrees);
        const qreal sinDeg = sin(rad);
        const qreal cosDeg = cos(rad);
        pt.setX( pt.x() + dx * cosDeg + dy * sinDeg );
        pt.setY( pt.y() - dx * sinDeg + dy * cosDeg );
    }
    return pt;
}


bool RelativePosition::operator==( const RelativePosition& r ) const
{
    return  d->area              == r.referenceArea() &&
            d->position          == r.referencePosition() &&
            d->alignment         == r.alignment() &&
            d->horizontalPadding == r.horizontalPadding() &&
            d->verticalPadding   == r.verticalPadding() &&
            d->rotation          == r.rotation() ;
}

#undef d


#if !defined(QT_NO_DEBUG_STREAM)
QDebug operator<<(QDebug dbg, const KDChart::RelativePosition& rp)
{
    dbg << "KDChart::RelativePosition("
	<< "referencearea="<<rp.referenceArea()
	<< "referenceposition="<<rp.referencePosition()
	<< "alignment="<<rp.alignment()
	<< "horizontalpadding="<<rp.horizontalPadding()
	<< "verticalpadding="<<rp.verticalPadding()
	<< "rotation="<<rp.rotation()
	<< ")";
    return dbg;
}
#endif /* QT_NO_DEBUG_STREAM */

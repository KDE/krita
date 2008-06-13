/* This file is part of the KDE project
   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006,2008 Jan Hambrecht <jaham@gmx.net>

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

#include "KoEllipseShape.h"

#include <KoPathPoint.h>
#include <KoShapeSavingContext.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoUnit.h>

#include <math.h>

KoEllipseShape::KoEllipseShape()
: m_startAngle( 0 )
, m_endAngle( 0 )
, m_kindAngle( M_PI )
, m_type( Arc )
{
    m_handles.push_back( QPointF( 100, 50 ) );
    m_handles.push_back( QPointF( 100, 50 ) );
    m_handles.push_back( QPointF( 0, 50 ) );
    QSizeF size( 100, 100 );
    createPath( size );
    m_points = *m_subpaths[0];
    updatePath( size );
}

KoEllipseShape::~KoEllipseShape()
{
}

void KoEllipseShape::saveOdf( KoShapeSavingContext & context ) const
{
    if( isParametricShape() )
    {
        context.xmlWriter().startElement("draw:ellipse");
        saveOdfAttributes( context, OdfAllAttributes );

        switch ( m_type ) {
        case Arc:
            context.xmlWriter().addAttribute( "draw:kind", sweepAngle()==360 ? "full" : "arc" );
            break;
        case Pie:
            context.xmlWriter().addAttribute( "draw:kind", "section" );
            break;
        case Chord:
            context.xmlWriter().addAttribute( "draw:kind", "cut" );
            break;
        default:
            context.xmlWriter().addAttribute( "draw:kind", "full" );
        }
        context.xmlWriter().addAttribute( "draw:start-angle", m_startAngle );
        context.xmlWriter().addAttribute( "draw:end-angle", m_endAngle );
        saveOdfCommonChildElements( context );
        context.xmlWriter().endElement();
    }
    else
        KoPathShape::saveOdf( context );
}

bool KoEllipseShape::loadOdf( const KoXmlElement & element, KoShapeLoadingContext &context )
{
    QSizeF size;
    size.setWidth( KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "width", QString() ) ) );
    size.setHeight( KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "height", QString() ) ) );

    if( element.hasAttributeNS( KoXmlNS::svg, "rx" ) && element.hasAttributeNS( KoXmlNS::svg, "ry" ) )
    {
        double rx = KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "rx" ) );
        double ry = KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "ry" ) );
        size = QSizeF( 2*rx, 2*ry );
    }
    else if( element.hasAttributeNS( KoXmlNS::svg, "r" ) )
    {
        double r = KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "r" ) );
        size = QSizeF( 2*r, 2*r );
    }
    setSize( size );

    QPointF pos;
    pos.setX( KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "x", QString() ) ) );
    pos.setY( KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "y", QString() ) ) );

    if( element.hasAttributeNS( KoXmlNS::svg, "cx" ) && element.hasAttributeNS( KoXmlNS::svg, "cy" ) )
    {
        double cx = KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "cx" ) );
        double cy = KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "cy" ) );
        pos = QPointF( cx - 0.5 * size.width(), cy - 0.5 * size.height() );
    }
    setPosition( pos );

    QString kind = element.attributeNS( KoXmlNS::draw, "kind", "full" );
    if( kind == "section" )
        setType( Pie );
    else if( kind == "cut" )
        setType( Chord );
    else
        setType( Arc );

    setStartAngle( element.attributeNS( KoXmlNS::draw, "start-angle", "0" ).toDouble() );
    setEndAngle( element.attributeNS( KoXmlNS::draw, "end-angle", "360" ).toDouble() );

    loadOdfAttributes( element, context, OdfMandatories | OdfTransformation | OdfAdditionalAttributes | OdfCommonChildElements );

    return true;
}

void KoEllipseShape::setSize( const QSizeF &newSize )
{
    QSizeF oldSize = size();
    QMatrix matrix( newSize.width() / oldSize.width(), 0, 0, newSize.height() / oldSize.height(), 0, 0 );
    m_center = matrix.map( m_center );
    m_radii = matrix.map( m_radii );
    KoParameterShape::setSize( newSize );
}

QPointF KoEllipseShape::normalize()
{
    QPointF offset( KoParameterShape::normalize() );
    QMatrix matrix;
    matrix.translate( -offset.x(), -offset.y() );
    m_center = matrix.map( m_center );
    return offset;
}

void KoEllipseShape::moveHandleAction( int handleId, const QPointF & point, Qt::KeyboardModifiers modifiers )
{
    Q_UNUSED( modifiers );
    QPointF p( point );

    QPointF diff( m_center - point );
    diff.setX( -diff.x() );
    double angle = 0;
    if ( diff.x() == 0 )
    {
        angle = ( diff.y() < 0 ? 270 : 90 ) * M_PI / 180.0;
    }
    else
    {
        diff.setY( diff.y() * m_radii.x() / m_radii.y() );
        angle = atan( diff.y() / diff.x () );
        if ( angle < 0 )
            angle = M_PI + angle;
        if ( diff.y() < 0 )
            angle += M_PI;
    }

    switch ( handleId )
    {
        case 0:
            p = QPointF( m_center + QPointF( cos( angle ) * m_radii.x(), -sin( angle ) * m_radii.y() ) );
            m_startAngle = angle * 180.0 / M_PI;
            m_handles[handleId] = p;
            updateKindHandle();
            break;
        case 1:
            p = QPointF( m_center + QPointF( cos( angle ) * m_radii.x(), -sin( angle ) * m_radii.y() ) );
            m_endAngle = angle * 180.0 / M_PI;
            m_handles[handleId] = p;
            updateKindHandle();
            break;
        case 2:
        {
            QList<QPointF> kindHandlePositions;
            kindHandlePositions.push_back( QPointF( m_center + QPointF( cos( m_kindAngle ) * m_radii.x(), -sin( m_kindAngle ) * m_radii.y() ) ) );
            kindHandlePositions.push_back( m_center );
            kindHandlePositions.push_back( ( m_handles[0] + m_handles[1] ) / 2.0 );

            QPointF diff = m_center * 2.0;
            int handlePos = 0;
            for ( int i = 0; i < kindHandlePositions.size(); ++i )
            {
                QPointF pointDiff( p - kindHandlePositions[i] );
                if ( i == 0 || qAbs( pointDiff.x() ) + qAbs( pointDiff.y() ) < qAbs( diff.x() ) + qAbs( diff.y() ) )
                {
                    diff = pointDiff;
                    handlePos = i;
                }
            }
            m_handles[handleId] = kindHandlePositions[handlePos];
            m_type = KoEllipseType( handlePos );
        } break;
    }
}

void KoEllipseShape::updatePath( const QSizeF &size )
{
    Q_UNUSED( size );
    QPointF startpoint( m_handles[0] );

    QPointF curvePoints[12];

    int pointCnt = arcToCurve( m_radii.x(), m_radii.y(), m_startAngle, sweepAngle() , startpoint, curvePoints );

    int cp = 0;
    m_points[cp]->setPoint( startpoint );
    m_points[cp]->removeControlPoint1();
    for ( int i = 0; i < pointCnt; i += 3 )
    {
        m_points[cp]->setControlPoint2( curvePoints[i] );
        m_points[++cp]->setControlPoint1( curvePoints[i+1] ); 
        m_points[cp]->setPoint( curvePoints[i+2] );
        m_points[cp]->removeControlPoint2();
    }
    if ( m_type == Pie )
    {
        m_points[++cp]->setPoint( m_center );
        m_points[cp]->removeControlPoint1();
        m_points[cp]->removeControlPoint2();
    }
    else if ( m_type == Arc && m_startAngle == m_endAngle )
    {
        m_points[0]->setControlPoint1( m_points[cp]->controlPoint1() );
        m_points[0]->setPoint( m_points[cp]->point() );
        --cp;
    }

    m_subpaths[0]->clear();

    for ( int i = 0; i <= cp; ++i )
    {
        m_points[i]->unsetProperty( KoPathPoint::StopSubpath );
        m_points[i]->unsetProperty( KoPathPoint::CloseSubpath );
        m_subpaths[0]->push_back( m_points[i] );
    }
    m_subpaths[0]->last()->setProperty( KoPathPoint::StopSubpath );
    if( m_type == Arc && m_startAngle != m_endAngle )
    {
        m_subpaths[0]->first()->unsetProperty( KoPathPoint::CloseSubpath );
        m_subpaths[0]->last()->unsetProperty( KoPathPoint::CloseSubpath );
    }
    else
    {
        m_subpaths[0]->first()->setProperty( KoPathPoint::CloseSubpath );
        m_subpaths[0]->last()->setProperty( KoPathPoint::CloseSubpath );
    }

    normalize();
}

void KoEllipseShape::createPath( const QSizeF &size )
{
    clear();
    m_radii = QPointF( size.width() / 2.0, size.height() / 2.0 );
    m_center = QPointF( m_radii.x(), m_radii.y() );
    moveTo( QPointF( size.width(), m_radii.y() ) );
    arcTo( m_radii.x(), m_radii.y(), 0, 360.0 );
    lineTo( QPointF( m_radii.x(), m_radii.y() ) );
    close();
}


void KoEllipseShape::updateKindHandle()
{
   m_kindAngle = ( m_startAngle + m_endAngle ) * M_PI / 360.0;
   if ( m_startAngle > m_endAngle )
   {
       m_kindAngle += M_PI;
   }
   switch ( m_type )
   {
       case Arc:
           m_handles[2] = m_center + QPointF( cos( m_kindAngle ) * m_radii.x(), -sin( m_kindAngle ) * m_radii.y() );
           break;
       case Pie:
           m_handles[2] = m_center;
           break;
       case Chord:
           m_handles[2] = ( m_handles[0] + m_handles[1] ) / 2.0;
           break;
   }
}

void KoEllipseShape::updateAngleHandles()
{
    double startRadian = m_startAngle * M_PI / 180.0;
    double endRadian = m_endAngle * M_PI / 180.0;
    m_handles[0] = m_center + QPointF( cos(startRadian) * m_radii.x(), -sin(startRadian) * m_radii.y());
    m_handles[1] = m_center + QPointF( cos(endRadian) * m_radii.x(), -sin(endRadian) * m_radii.y());
}

double KoEllipseShape::sweepAngle() const
{
    double sAngle =  m_endAngle - m_startAngle;
    // treat also as full circle
    if ( sAngle == 0 || sAngle == -360 )
        sAngle = 360;
    if ( m_startAngle > m_endAngle )
    {
        sAngle = 360 - m_startAngle + m_endAngle;
    }
    return sAngle;
}

void KoEllipseShape::setType( KoEllipseType type )
{
    m_type = type;
    updateKindHandle();
    updatePath( size() );
}

KoEllipseShape::KoEllipseType KoEllipseShape::type() const
{
    return m_type;
}

void KoEllipseShape::setStartAngle( double angle )
{
    m_startAngle = angle;
    updateKindHandle();
    updateAngleHandles();
    updatePath( size() );
}

double KoEllipseShape::startAngle() const
{
    return m_startAngle;
}

void KoEllipseShape::setEndAngle( double angle )
{
    m_endAngle = angle;
    updateKindHandle();
    updateAngleHandles();
    updatePath( size() );
}

double KoEllipseShape::endAngle() const
{
    return m_endAngle;
}

QString KoEllipseShape::pathShapeId() const
{
    return KoEllipseShapeId;
}

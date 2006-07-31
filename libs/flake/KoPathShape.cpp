/* This file is part of the KDE project
   Copyright (C) 2006 Rob Buis <buis@kde.org>
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

#include "KoPathShape.h"

#include <QPainter>
#include <QtDebug>

#include <KoSelection.h>
#include <KoPointerEvent.h>

KoPathShape::KoPathShape()
{
}

void KoPathShape::paint( QPainter &painter, const KoViewConverter &converter)
{
    applyConversion(painter, converter);
    painter.setBrush(background());
    painter.drawPath( m_path );
}

void KoPathShape::paintDecorations(QPainter &painter, KoViewConverter &converter, bool selected) {
    // draw bezier helplines and general points
    if(!selected )
        return;
    applyConversion(painter, converter);

    painter.setBrush( Qt::blue );
    painter.setPen( Qt::blue );
    int mcount = m_path.elementCount();
    for( int i = 0; i < mcount; i++)
    {
        QPainterPath::Element elem = m_path.elementAt( i );
        QRectF r( elem.x - 1, elem.y - 1, 2, 2 );
        painter.drawEllipse( r );
        if(elem.type == QPainterPath::CurveToElement) {
            painter.drawLine( QPointF(m_path.elementAt( i - 1 ).x, m_path.elementAt( i - 1 ).y),
                    QPointF(m_path.elementAt( i ).x, m_path.elementAt( i ).y ));
            painter.drawLine( QPointF(m_path.elementAt( i + 1 ).x, m_path.elementAt( i + 1 ).y),
                    QPointF(m_path.elementAt( i + 2 ).x, m_path.elementAt( i + 2 ).y ));
        }
    }
}

void KoPathShape::moveTo( const QPointF &p )
{
    m_path.moveTo( p );
}

void KoPathShape::lineTo( const QPointF &p )
{
    m_path.lineTo( p );
}

void KoPathShape::curveTo( const QPointF &p1, const QPointF &p2, const QPointF &p )
{
    m_path.cubicTo( p1, p2, p );
}

void close()
{
}

bool KoPathShape::hitTest( const QPointF &position ) const
{
    QPointF point( position * m_invMatrix );

    return m_path.contains( point );
}

QRectF KoPathShape::boundingRect() const
{
    QRectF bb( m_path.boundingRect() );
    bb.moveTopLeft( position() );
    return bb;
}

const QPainterPath KoPathShape::outline() const {
    return m_path;
}

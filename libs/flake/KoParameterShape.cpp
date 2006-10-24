/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

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

#include "KoParameterShape.h"

#include <QDebug>
#include <QPainter>

KoParameterShape::KoParameterShape()
: m_modified( false )
{
}

KoParameterShape::~KoParameterShape()
{
}

void KoParameterShape::moveHandle( int handleId, const QPointF & point, Qt::KeyboardModifiers modifiers )
{
    if ( handleId >= m_handles.size() )
    {
        qWarning() << "handleId out of bounds";
        return;
    }
    // function to do special stuff
    moveHandleAction( handleId, point, modifiers );

    updatePath( size() );
    repaint();
}


int KoParameterShape::handleIdAt( const QRectF & rect ) const
{
    int handle = -1;

    for ( int i = 0; i < m_handles.size(); ++i )
    {
        if ( rect.contains( m_handles.at( i ) ) )
        {
            handle = i;
            break;
        }
    }
    return handle;
}

QPointF KoParameterShape::handlePosition( int handleId )
{
    return m_handles[handleId];
}

void KoParameterShape::paintHandles( QPainter & painter, const KoViewConverter & converter )
{
    applyConversion( painter, converter );

    QWMatrix matrix;
    matrix.rotate( 45.0 );
    QPolygonF poly( converter.viewToDocument( handleRect( QPointF( 0, 0 ) ) ) );
    poly = matrix.map( poly );

    QList<QPointF>::const_iterator it( m_handles.begin() );
    for ( ; it != m_handles.end(); ++it ) 
    {
        QPolygonF p( poly );
        p.translate( *it );
        painter.drawPolygon( p );
    }
}

void KoParameterShape::paintHandle( QPainter & painter, const KoViewConverter & converter, int handleId )
{
    applyConversion( painter, converter );

    QWMatrix matrix;
    matrix.rotate( 45.0 );
    QPolygonF poly( converter.viewToDocument( handleRect( QPointF( 0, 0 ) ) ) );
    poly = matrix.map( poly );
    poly.translate( m_handles[handleId] );
    painter.drawPolygon( poly );
}

void KoParameterShape::resize( const QSizeF &newSize )
{
    QSizeF oldSize = size();
    QMatrix matrix( newSize.width() / oldSize.width(), 0, 0, newSize.height() / oldSize.height(), 0, 0 );

    for( int i = 0; i < m_handles.size(); ++i )
    {
        m_handles[i] = matrix.map( m_handles[i] );
    }

    KoPathShape::resize( newSize );
}

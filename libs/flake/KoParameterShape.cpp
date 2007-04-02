/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>

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

class KoParameterShape::Private {
public:
    Private() : modified(false) {}
    bool modified;
};

KoParameterShape::KoParameterShape()
    : d(new Private())
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

    repaint();
    // function to do special stuff
    moveHandleAction( handleId, documentToShape( point ), modifiers );

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

void KoParameterShape::paintHandles( QPainter & painter, const KoViewConverter & converter, int handleRadius )
{
    applyConversion( painter, converter );

    QMatrix worldMatrix = painter.worldMatrix();
    painter.setMatrix( QMatrix() );

    QMatrix matrix;
    matrix.rotate( 45.0 );
    QPolygonF poly( handleRect( QPointF( 0, 0 ), handleRadius ) );
    poly = matrix.map( poly );

    QList<QPointF>::const_iterator it( m_handles.begin() );
    for ( ; it != m_handles.end(); ++it ) 
    {
        QPointF moveVector = worldMatrix.map( *it );
        poly.translate( moveVector.x(), moveVector.y() );
        painter.drawPolygon( poly );
        poly.translate( -moveVector.x(), -moveVector.y() );
    }
}

void KoParameterShape::paintHandle( QPainter & painter, const KoViewConverter & converter, int handleId, int handleRadius )
{
    applyConversion( painter, converter );

    QMatrix worldMatrix = painter.worldMatrix();
    painter.setMatrix( QMatrix() );

    QMatrix matrix;
    matrix.rotate( 45.0 );
    QPolygonF poly( handleRect( QPointF( 0, 0 ), handleRadius ) );
    poly = matrix.map( poly );
    poly.translate( worldMatrix.map( m_handles[handleId] ) );
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

QPointF KoParameterShape::normalize()
{
    QPointF offset( KoPathShape::normalize() );
    QMatrix matrix;
    matrix.translate( -offset.x(), -offset.y() );

    for( int i = 0; i < m_handles.size(); ++i )
    {
        m_handles[i] = matrix.map( m_handles[i] );
    }

    return offset;
}

bool KoParameterShape::isParametricShape() const {
    return !d->modified;
}

void KoParameterShape::setModified( bool modified ) {
    d->modified = modified;
}

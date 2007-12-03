/* This file is part of the KDE project
 * Copyright (C) 2006 Laurent Montel <montel@kde.org>
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

#include "arrow/KoArrowShape.h"


KoArrowShape::KoArrowShape()
{
    m_type = ArrowLeft;
}

void KoArrowShape::setType(KoArrowType _type)
{
    // TODO check input, and don't accept type if its not in the list of known types.

    //TODO think to clear point if we change type.
    m_type=_type;
    // TODO use real size, not a hardcoded size
    QSizeF size( 100, 100 );
    createPath(size);
}

void KoArrowShape::createPath( const QSizeF &size )
{
    switch(m_type)
    {
    case ArrowRight:
    {
    moveTo( QPointF( 0, size.height()/3));
    lineTo( QPointF( size.width()/2, size.height()/3));
    lineTo( QPointF( size.width()/2, 0));
    lineTo( QPointF( size.width(), size.height()/2));
    lineTo( QPointF( size.width()/2, size.height()));
    lineTo( QPointF( size.width()/2, size.height()*2/3));
    lineTo( QPointF( 0, size.height()*2/3));
    closeMerge();
    break;
    }
    case ArrowLeft:
    {
    moveTo( QPointF( 0, size.height()/2));
    lineTo( QPointF( size.width()/3, 0));
    lineTo( QPointF( size.width()/3, size.height()/3));
    lineTo( QPointF( size.width(), size.height()/3));
    lineTo( QPointF( size.width(), size.height()*2/3));
    lineTo( QPointF( size.width()/3, size.height()*2/3));
    lineTo( QPointF( size.width()/3, size.height()));
    closeMerge();
    break;
    }
    case ArrowBottom:
    {
    moveTo( QPointF(size.width()/3,0));
    lineTo( QPointF(size.width()*2/3,0));
    lineTo( QPointF(size.width()*2/3,size.height()/2));
    lineTo( QPointF(size.width(),size.height()/2));
    lineTo( QPointF(size.width()/2, size.height()));
    lineTo( QPointF(0,size.height()/2));
    lineTo( QPointF(size.width()/3,size.height()/2));
    closeMerge();
    break;
    }
    case ArrowTop:
    {
    moveTo( QPointF(size.width()/2,0));
    lineTo( QPointF(size.width(),size.height()/2));
    lineTo( QPointF(size.width()*2/3,size.height()/2));
    lineTo( QPointF(size.width()*2/3,size.height()));
    lineTo( QPointF(size.width()/3,size.height()));
    lineTo( QPointF(size.width()/3,size.height()/2));
    lineTo( QPointF(0,size.height()/2));
    closeMerge();
    break;
    }
    case ArrowLeftDown:
    {
    closeMerge();
    break;
    }
    case ArrowLeftTop:
    {
    closeMerge();
    break;
    }
    case ArrowRightTop:
    {
    closeMerge();
    break;
    }
    case ArrowRightDown:
    {
    closeMerge();
    break;
    }
    default:
        Q_ASSERT(false); // internal error.
    }
}

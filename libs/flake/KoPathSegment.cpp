/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KoPathSegment.h"
#include "KoPathPoint.h"

class KoPathSegment::Private
{
public:
    Private( KoPathPoint * p1, KoPathPoint * p2 )
    {
        first = p1;
        second = p2;
    }

    KoPathPoint * first;
    KoPathPoint * second;
};

KoPathSegment::KoPathSegment( KoPathPoint * first, KoPathPoint * second )
    : d( new Private( first, second ) )
{
}

KoPathSegment::KoPathSegment( const KoPathSegment & segment )
    : d( new Private(0,0) )
{
   if( ! segment.first() || segment.first()->parent() )
       setFirst( segment.first() );
   else
       setFirst( new KoPathPoint( *segment.first() ) );

   if( ! segment.second() || segment.second()->parent() )
       setSecond( segment.second() );
   else
       setSecond( new KoPathPoint( *segment.second() ) );
}

KoPathSegment & KoPathSegment::operator=( const KoPathSegment &rhs )
{
    if( this == &rhs )
        return (*this);

    if( ! rhs.first() || rhs.first()->parent() )
        setFirst( rhs.first() );
    else
        setFirst( new KoPathPoint( *rhs.first() ) );

    if( ! rhs.second() || rhs.second()->parent() )
        setSecond( rhs.second() );
    else
        setSecond( new KoPathPoint( *rhs.second() ) );

    return (*this);
}

KoPathSegment::~KoPathSegment()
{
    if( d->first && ! d->first->parent() )
        delete d->first;
    if( d->second && ! d->second->parent() )
        delete d->second;
}

KoPathPoint * KoPathSegment::first() const
{
    return d->first;
}

void KoPathSegment::setFirst( KoPathPoint * first )
{
    if( d->first && ! d->first->parent() )
        delete d->first;
    d->first = first;
}

KoPathPoint * KoPathSegment::second() const
{
    return d->second;
}

void KoPathSegment::setSecond( KoPathPoint * second )
{
    if( d->second && ! d->second->parent() )
        delete d->second;
    d->second = second;
}

bool KoPathSegment::isValid() const
{
    return (d->first && d->second);
}

bool KoPathSegment::operator == ( const KoPathSegment &rhs ) const
{
    if( ! isValid() && ! rhs.isValid() )
        return true;
    if( isValid() && ! rhs.isValid() )
        return false;
    if( ! isValid() && rhs.isValid() )
        return false;

    return ( *first() == *rhs.first() &&  *second() == *rhs.second() );
}

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

#ifndef KOPATHSEGMENT_H
#define KOPATHSEGMENT_H

#include "flake_export.h"

class KoPathPoint;

/// A KoPathSegment consist of two neighboring KoPathPoints
class FLAKE_EXPORT KoPathSegment
{
public:
    /** 
    * Creates a new segment from the given path points
    * It takes ownership of the path points which do not have a
    * parent path shape set.
    */
    KoPathSegment( KoPathPoint * first, KoPathPoint * second );

    /// Constructs segment by copying another segment
    KoPathSegment( const KoPathSegment & segment );

    /// Assigns segment
    KoPathSegment& operator=( const KoPathSegment &rhs );

    /// Destroys the path segment
    ~KoPathSegment();

    /// Returns the first point of the segment
    KoPathPoint * first() const;

    /// Sets the first segment point
    void setFirst( KoPathPoint * first );

    /// Returns the second point of the segment
    KoPathPoint * second() const;

    /// Sets the second segment point
    void setSecond( KoPathPoint * second );

    /// Returns if segment is valid, e.g. has two valid points
    bool isValid() const;

    /// Compare operator
    bool operator == ( const KoPathSegment &rhs ) const;
private:
    class Private;
    Private * const d;
};

#endif // KOPATHSEGMENT_H

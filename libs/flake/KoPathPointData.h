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

#ifndef KOPATHPOINTDATA_H
#define KOPATHPOINTDATA_H

#include "KoPathShape.h"

/**
 * @brief Describe a KoPathPoint by a KoPathShape and its indices
 */
class KoPathPointData
{
public:
    /// contructor
    KoPathPointData( KoPathShape * pathShape, const KoPathPointIndex & pathPointIndex )
    : m_pathShape( pathShape )
    , m_pointIndex( pathPointIndex )
    {}

    /// operator used for sorting
    bool operator<( const KoPathPointData & other ) const 
    { 
        return m_pathShape < other.m_pathShape ||
               ( m_pathShape == other.m_pathShape && 
                 ( m_pointIndex.first < other.m_pointIndex.first ||
                   ( m_pointIndex.first == other.m_pointIndex.first &&
                     m_pointIndex.second < other.m_pointIndex.second ) ) );

    }
    bool operator==( const KoPathPointData & other ) const
    {
        return m_pathShape == other.m_pathShape && 
               m_pointIndex.first == other.m_pointIndex.first &&
               m_pointIndex.second == other.m_pointIndex.second;
    }
    /// path shape the path point belongs too
    KoPathShape *m_pathShape;
    /// position of the point in the path shape
    KoPathPointIndex m_pointIndex;
};

#endif

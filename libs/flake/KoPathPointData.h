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
    KoPathPointData(KoPathShape * path, const KoPathPointIndex & pointIndex)
            : pathShape(path)
            , pointIndex(pointIndex) {}

    /// operator used for sorting
    bool operator<(const KoPathPointData & other) const {
        return pathShape < other.pathShape ||
               (pathShape == other.pathShape &&
                (pointIndex.first < other.pointIndex.first ||
                 (pointIndex.first == other.pointIndex.first &&
                  pointIndex.second < other.pointIndex.second)));

    }
    bool operator==(const KoPathPointData & other) const {
        return pathShape == other.pathShape &&
               pointIndex.first == other.pointIndex.first &&
               pointIndex.second == other.pointIndex.second;
    }
    /// path shape the path point belongs too
    KoPathShape *pathShape;
    /// position of the point in the path shape
    KoPathPointIndex pointIndex;
};

#endif

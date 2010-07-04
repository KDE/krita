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

#ifndef KOPOINTGROUP_H
#define KOPOINTGROUP_H

#include "KoPathShape.h"

/**
 * @brief A KoPointGroup represents points in a path that should be treated as one
 *
 * In svg it is possible when you use a close and the create a new subpath not using
 * a moveTo that the new subpath starts at the same point as the last subpath. As
 * every point can only have 2 control points we have this class to group points
 * together which should be handled as one in e.g. a move.
 */
class KoPointGroup
{
public:
    KoPointGroup() {}
    ~KoPointGroup() {}

    /**
     * @brief Add a point to the group
     */
    void add(KoPathPoint *point);
    /**
     * @brief Remove a point from the group
     *
     * This also remove the pointer to the group in the point.
     * When the second last point is removed from the group, the
     * group removes also the last point and deletes itself.
     */
    void remove(KoPathPoint *point);

    void map(const QTransform &matrix);

    /**
     * @brief get The point belonging to the group
     *
     * @return all points of the group
     */
    const QSet<KoPathPoint *> &points() const {
        return m_points;
    }

private:
    QSet<KoPathPoint *> m_points;
};


#endif

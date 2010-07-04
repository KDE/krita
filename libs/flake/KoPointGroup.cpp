/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
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

#include "KoPointGroup.h"
#include "KoPathPoint.h"

void KoPointGroup::add(KoPathPoint * point)
{
    m_points.insert(point);
    point->addToGroup(this);
}

void KoPointGroup::remove(KoPathPoint * point)
{
    if (m_points.remove(point)) {
        point->removeFromGroup();
        if (m_points.size() == 1) {
            (* m_points.begin())->removeFromGroup();
            //commit suicide as it is no longer used
            delete this;
        }
    }
}

void KoPointGroup::map(const QTransform &matrix)
{
    QSet<KoPathPoint *>::iterator it = m_points.begin();
    for (; it != m_points.end(); ++it) {
        (*it)->map(matrix, false);
    }
}


/*
 *  Copyright (c) 2020 Sharaf Zaman <sharafzaz121@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef SVGMESHARRAY_H
#define SVGMESHARRAY_H

#include <QVector>

#include "SvgMeshPatch.h"

class SvgMeshArray
{
public:
    SvgMeshArray();
    ~SvgMeshArray();

    void newRow();

    bool addPatch(QList<QPair<QString, QColor>>& stops, const QPointF initialPoint);

    /// Get the point of a node in mesharray
    SvgMeshStop getStop(const SvgMeshPatch::Type edge, const int row, const int col) const;
    
    /// Get the Path Points for a segment of the meshpatch
    QList<QPointF> getPath(const SvgMeshPatch::Type edge, const int row, const int col) const;


private:
    /// where each vector is a meshrow
    QVector<QVector<SvgMeshPatch*>> m_array;
};

#endif // SVGMESHARRAY_H

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
#include "SvgMeshArray.h"
#include <QDebug>

SvgMeshArray::SvgMeshArray()
{
}

SvgMeshArray::~SvgMeshArray()
{
    for (auto& row: m_array) {
        for (auto& patch: row) {
            delete patch;
        }
    }
}

void SvgMeshArray::newRow()
{
    m_array << QVector<SvgMeshPatch*>();
}

void SvgMeshArray::addPatch(SvgMeshPatch* patch)
{
    if (m_array.size() == 0) {
        m_array.append((QVector<SvgMeshPatch*>() << patch));
        return;
    }
    int lastrowInd = m_array.size() - 1;
    m_array[lastrowInd].append(patch);
}

SvgMeshStop SvgMeshArray::getStop(const SvgMeshPatch::Type edge, const int row, const int col) const
{
    if (!(row < m_array.size() && col < m_array[row].size()
            && row >= 0 && col >= 0)) {
        qWarning() << "Out of bounds";
        return SvgMeshStop();   // programming error
    }

    SvgMeshPatch *patch = m_array[row][col];
    SvgMeshStop *node = patch->getStop(edge);

    if (node != nullptr) {
        return *node;
    }

    switch (patch->countPoints()) {
    case 3:
    case 2:
        if (edge == SvgMeshPatch::Top)
            return getStop(SvgMeshPatch::Left, row - 1, col);
        else if (edge == SvgMeshPatch::Left)
            return getStop(SvgMeshPatch::Bottom, row, col - 1);
    }

    return SvgMeshStop();   // programming error
}


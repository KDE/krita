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

#include <KoPathSegment.h>
#include <kis_global.h>
#include <QDebug>

SvgMeshArray::SvgMeshArray()
{
}

SvgMeshArray::SvgMeshArray(SvgMeshArray& other)
{
    // FIXME The way SvgParser works with gradients is, it frequently destroys the objects holding reference to
    // SvgMeshGradients, so rather than copying we move it. This certainly needs a design refactor.
    m_array = std::move(other.m_array);

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

bool SvgMeshArray::addPatch(QList<QPair<QString, QColor>> stops, const QPointF initialPoint)
{
    // This is function is full of edge-case landmines, please run TestMeshArray after any changes
    if (stops.size() > 4 || stops.size() < 2)
        return false;

    SvgMeshPatch *patch = new SvgMeshPatch(initialPoint);

     m_array.last().append(patch);

    int irow = m_array.size() - 1;
    int icol = m_array.last().size() - 1;

    if (irow == 0 && icol == 0) {
        patch->addStop(stops[0].first, stops[0].second, SvgMeshPatch::Top);
        stops.removeFirst();
    } else if (irow == 0) {
        // For first row, parse patches
        patch->addStop(stops[0].first, stops[0].second, SvgMeshPatch::Top);
        stops.removeFirst();
    } else {
        // path is already defined for rows >= 1
        QColor color = getStop(SvgMeshPatch::Left, irow - 1, icol).color;

        QList<QPointF> points = getPath(SvgMeshPatch::Bottom, irow - 1, icol);
        std::reverse(points.begin(), points.end());

        patch->addStop(points, color, SvgMeshPatch::Top);
    }


    // Right will always be independent
    patch->addStop(stops[0].first, stops[0].second, SvgMeshPatch::Right);
    stops.removeFirst();

    if (icol > 0) {
        patch->addStop(
                stops[0].first,
                stops[0].second,
                SvgMeshPatch::Bottom,
                true, getStop(SvgMeshPatch::Bottom, irow, icol - 1).point);
        stops.removeFirst();
    } else {
        patch->addStop(stops[0].first, stops[0].second, SvgMeshPatch::Bottom);
        stops.removeFirst();
    }

    // last stop
    if (icol == 0) {
        // if stop is in the 0th column, parse path
        patch->addStop(
                stops[0].first,
                stops[0].second,
                SvgMeshPatch::Left,
                true, getStop(SvgMeshPatch::Top, irow, icol).point);
        stops.removeFirst();
    } else {
        QColor color = getStop(SvgMeshPatch::Bottom, irow, icol - 1).color;

        // reuse Right side of the previous patch
        QList<QPointF> points = getPath(SvgMeshPatch::Right, irow, icol - 1);
        std::reverse(points.begin(), points.end());

        patch->addStop(points, color, SvgMeshPatch::Left);
    }
    return true;
}

SvgMeshStop SvgMeshArray::getStop(const SvgMeshPatch::Type edge, const int row, const int col) const
{
    KIS_ASSERT(row < m_array.size() && col < m_array[row].size()
            && row >= 0 && col >= 0);

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
    assert(false);
}

QList<QPointF> SvgMeshArray::getPath(const SvgMeshPatch::Type edge, const int row, const int col) const
{
    KIS_ASSERT(row < m_array.size() && col < m_array[row].size()
            && row >= 0 && col >= 0);

    return m_array[row][col]->getPathSegment(edge).controlPoints();
}

int SvgMeshArray::numRows() const
{
    return m_array.size();
}

int SvgMeshArray::numColumns() const
{
    if (m_array.isEmpty())
        return 0;
    return m_array.first().size();
}

QColor SvgMeshArray::getColor(SvgMeshPatch::Type edge, int row, int col) const
{
    return getStop(edge, row, col).color;
}


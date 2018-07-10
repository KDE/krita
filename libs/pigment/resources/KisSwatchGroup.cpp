/*  This file is part of the KDE project
    Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
    Copyright (c) 2016 L. E. Segovia <leo.segovia@siggraph.org>
    Copyright (c) 2018 Michael Zhou <simerixh@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

 */

#include "KisSwatchGroup.h"

quint32 KisSwatchGroup::DEFAULT_N_COLUMN = 16;

KisSwatchGroup::KisSwatchGroup()
    : m_colorMatrix(DEFAULT_N_COLUMN)
    , m_nColors(0)
{ }

void KisSwatchGroup::setEntry(const KisSwatch &e, int x, int y)
{
    Q_ASSERT(x < m_colorMatrix.size() && x >= 0 && y >= 0);
    if (!checkEntry(x, y)) {
        m_nColors++;
    }
    m_colorMatrix[x][y] = e;
}

bool KisSwatchGroup::checkEntry(int x, int y) const
{
    if (x >= m_colorMatrix.size() || x < 0) {
        return false;
    }
    if (!m_colorMatrix[x].contains(y)) {
        return false;
    }
    return true;
}

bool KisSwatchGroup::removeEntry(int x, int y)
{
    if (m_nColors == 0) {
        return false;
    }

    if (x >= m_colorMatrix.size() || x < 0) {
        return false;
    }

    // QMap::remove returns 1 if key found else 0
    if (m_colorMatrix[x].remove(y)) {
        m_nColors -= 1;
        return true;
    } else {
        return false;
    }
}

void KisSwatchGroup::setColumnCount(int nColumns)
{
    Q_ASSERT(nColumns >= 0);
    if (nColumns < m_colorMatrix.size()) {
        for (int i = m_colorMatrix.size() - 1; i <= nColumns; i-- ) {
            m_nColors -= m_colorMatrix[i].size();
        }
    }
    m_colorMatrix.resize(nColumns);
}

KisSwatch KisSwatchGroup::getEntry(int x, int y) const
{
    Q_ASSERT(checkEntry(x, y));
    return m_colorMatrix[x][y];
}

int KisSwatchGroup::rowCount() const
{
    /*
     * shouldn't have too great a performance impact...
     * add a heap to keep track of last row if there is one
     */

    int res = 0;
    for (const Column &c : m_colorMatrix) {
        if (c.empty()) {
            continue;
        }
        res = res > c.lastKey() ? res : c.lastKey();
    }
    return res + 1;
}

void KisSwatchGroup::addEntry(const KisSwatch &e)
{
    if (columnCount() == 0) {
        setColumnCount(DEFAULT_N_COLUMN);
    }

    if (m_nColors == 0) {
        setEntry(e, 0, 0);
        return;
    }

    int maxY = rowCount() - 1;
    int y = maxY;
    for (int x = m_colorMatrix.size() - 1; x >= 0; x--) {
        if (checkEntry(x, maxY)) {
            // if the last entry's at the rightmost column,
            // add e to the leftmost column of the next row
            if (++x == m_colorMatrix.size()) {
                x = 0;
                y++;
            }
            // else just add it to the right
            setEntry(e, x, y);
            break;
        }
    }
}

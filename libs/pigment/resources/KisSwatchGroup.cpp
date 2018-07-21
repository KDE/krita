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

quint32 KisSwatchGroup::DEFAULT_COLUMN_COUNT = 16;
quint32 KisSwatchGroup::DEFAULT_ROW_COUNT = 20;

KisSwatchGroup::KisSwatchGroup()
    : m_colorMatrix(DEFAULT_COLUMN_COUNT)
    , m_rowCount(DEFAULT_ROW_COUNT)
    , m_colorCount(0)
{ }

void KisSwatchGroup::setEntry(const KisSwatch &e, int x, int y)
{
    Q_ASSERT(x < m_rowCount && x < m_colorMatrix.size() && x >= 0 && y >= 0);
    if (!checkEntry(x, y)) {
        m_colorCount++;
    }
    m_colorMatrix[x][y] = e;
}

bool KisSwatchGroup::checkEntry(int x, int y) const
{
    if (y >= m_rowCount || x >= m_colorMatrix.size() || x < 0) {
        return false;
    }
    if (!m_colorMatrix[x].contains(y)) {
        return false;
    }
    return true;
}

bool KisSwatchGroup::removeEntry(int x, int y)
{
    if (m_colorCount == 0) {
        return false;
    }

    if (x >= m_rowCount || x >= m_colorMatrix.size() || x < 0) {
        return false;
    }

    // QMap::remove returns 1 if key found else 0
    if (m_colorMatrix[x].remove(y)) {
        m_colorCount -= 1;
        return true;
    } else {
        return false;
    }
}

void KisSwatchGroup::setColumnCount(int columnCount)
{
    Q_ASSERT(columnCount >= 0);
    if (columnCount < m_colorMatrix.size()) {
        for (int i = m_colorMatrix.size() - 1; i <= columnCount; i-- ) {
            m_colorCount -= m_colorMatrix[i].size();
        }
    }
    m_colorMatrix.resize(columnCount);
}

KisSwatch KisSwatchGroup::getEntry(int x, int y) const
{
    Q_ASSERT(checkEntry(x, y));
    return m_colorMatrix[x][y];
}

void KisSwatchGroup::addEntry(const KisSwatch &e)
{
    if (columnCount() == 0) {
        setColumnCount(DEFAULT_COLUMN_COUNT);
    }

    if (m_colorCount == 0) {
        setEntry(e, 0, 0);
        return;
    }

    int y = 0;
    for (const Column &c : m_colorMatrix) {
        if (c.isEmpty()) { continue; }
        if (y < c.lastKey()) {
            y = c.lastKey();
        }
    }
    for (int x = m_colorMatrix.size() - 1; x >= 0; x--) {
        if (checkEntry(x, y)) {
            // if the last entry's at the rightmost column,
            // add e to the leftmost column of the next row
            // and increase row count
            if (++x == m_colorMatrix.size()) {
                x = 0;
                y++;
                m_rowCount++;
            }
            // else just add it to the right
            setEntry(e, x, y);
            break;
        }
    }
}

void KisSwatchGroup::setRowCount(int newRowCount)
{
    m_rowCount = newRowCount;
    for (Column &c : m_colorMatrix) {
        for (int k : c.keys()) {
            if (k >= newRowCount) {
                c.remove(k);
                m_colorCount--;
            }
        }
    }
}

QList<KisSwatchGroup::SwatchInfo> KisSwatchGroup::infoList() const
{
    QList<SwatchInfo> res;
    int column = 0;
    for (const Column &c : m_colorMatrix) {
        int row = 0;
        for (const KisSwatch &s : c.values()) {
            SwatchInfo i = {s, column, c.keys()[row++]};
            res.append(i);
        }
        column++;
    }
    return res;
}

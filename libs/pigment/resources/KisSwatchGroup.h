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

#ifndef KISSWATCHGROUP_H
#define KISSWATCHGROUP_H

#include "kritapigment_export.h"
#include <QVector>
#include <QMap> // Used to keep track of the last row. Qt doesn't provide a priority queue...
#include "KisSwatch.h"

/**
 * @brief The KisSwatchGroup class stores a matrix of color swatches
 * swatches can accessed using (x, y) coordinates.
 * x is the column number from left to right and y is the row number from top
 * to bottom.
 * Both x and y start at 0
 * there could be empty entries, so the checkEntry(int, int) method must used
 * whenever you want to get an entry from the matrix
 */
class KRITAPIGMENT_EXPORT KisSwatchGroup
{
private:
    static quint32 DEFAULT_N_COLUMN;

private:
    typedef QMap<int, KisSwatch> Column;

public:
    KisSwatchGroup();

    void setName(const QString &name) { m_name = name; }
    QString name() const { return m_name; }

    void setColumnCount(int nColumns);
    int columnCount() const { return m_colorMatrix.size(); }

    int rowCount() const;
    int colorCount() const { return m_nColors; }

    /**
     * @brief checkEntry
     * checks if position x and y has a valid entry
     * both x and y start from 0
     * @param x
     * @param y
     * @return true if there is a valid entry at position (x, y)
     */
    bool checkEntry(int x, int y) const;
    /**
     * @brief setEntry
     * sets the entry at position (x, y) to be e
     * @param e
     * @param x
     * @param y
     */
    void setEntry(const KisSwatch &e, int x, int y);
    /**
     * @brief getEntry
     * used to get the swatch entry at position (x, y)
     * there is an assertion to make sure that this position isn't empty,
     * so checkEntry(int, int) must be used before this method to ensure
     * a valid entry can be found
     * @param x
     * @param y
     * @return the swatch entry at position (x, y)
     */
    KisSwatch getEntry(int x, int y) const;
    /**
     * @brief removeEntry
     * removes the entry at position (x, y)
     * @param x
     * @param y
     * @return true if these is an entry at (x, y)
     */
    bool removeEntry(int x, int y);
    /**
     * @brief addEntry
     * adds the entry e to the right of the rightmost entry in the last row
     * if the rightmost entry in the last row is in the right most column,
     * add e to the leftmost column of a new row
     *
     * when column is set to 0, resize number of columns to default
     * @param e
     */
    void addEntry(const KisSwatch &e);

    void clear() { m_colorMatrix.clear(); }

private:
    QString m_name;
    QVector<Column> m_colorMatrix;
    int m_nColors;
};

#endif // KISSWATCHGROUP_H

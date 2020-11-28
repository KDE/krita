/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
    SPDX-FileCopyrightText: 2016 L. E. Segovia <amy@amyspark.me>
    SPDX-FileCopyrightText: 2018 Michael Zhou <simerixh@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later

 */

#ifndef KISSWATCHGROUP_H
#define KISSWATCHGROUP_H

#include "KisSwatch.h"

#include "kritapigment_export.h"

#include <QVector>
#include <QList>
#include <QMap>
#include <QScopedPointer>

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
public /* struct */:
    struct SwatchInfo {
        QString group;
        KisSwatch swatch;
        int row;
        int column;
    };

public:
    KisSwatchGroup();
    ~KisSwatchGroup();
    KisSwatchGroup(const KisSwatchGroup &rhs);
    KisSwatchGroup &operator =(const KisSwatchGroup &rhs);

public /* methods */:
    void setName(const QString &name);
    QString name() const;

    void setColumnCount(int columnCount);
    int columnCount() const;

    void setRowCount(int newRowCount);
    int rowCount() const;

    int colorCount() const;

    /**
     * @brief getColors
     * @return the list of colors in this SwatchGroup, in no specific order.
     */
    QList<SwatchInfo> infoList() const;

    /**
     * @brief checkEntry
     * checks if position @p column and @p row has a valid entry
     * both @p column and @p row start from 0
     * @param column
     * @param row
     * @return true if there is a valid entry at position (column, row)
     */
    bool checkEntry(int column, int row) const;
    /**
     * @brief setEntry
     * sets the entry at position (@p column, @p row) to be @p e
     * @param e
     * @param column
     * @param row
     */
    void setEntry(const KisSwatch &e, int column, int row);
    /**
     * @brief getEntry
     * used to get the swatch entry at position (@p column, @p row)
     * there is an assertion to make sure that this position isn't empty,
     * so checkEntry(int, int) must be used before this method to ensure
     * a valid entry can be found
     * @param column
     * @param row
     * @return the swatch entry at position (column, row)
     */
    KisSwatch getEntry(int column, int row) const;
    /**
     * @brief removeEntry
     * removes the entry at position (@p column, @p row)
     * @param column
     * @param row
     * @return true if these is an entry at (column, row)
     */
    bool removeEntry(int column, int row);
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

    void clear();

private /* member variables */:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // KISSWATCHGROUP_H

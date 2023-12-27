/*  This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2005..2022 Halla Rempt <halla@valdyas.org>
 *  SPDX-FileCopyrightText: 2016 L. E. Segovia <amy@amyspark.me>
 *  SPDX-FileCopyrightText: 2018 Michael Zhou <simerixh@gmail.com>
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KISSWATCHGROUP_H
#define KISSWATCHGROUP_H

#include "KisSwatch.h"

#include "kritapigment_export.h"

#include <QSharedPointer>
#include <QVector>
#include <QList>
#include <QMap>
#include <QScopedPointer>

#include <KisPropagateConstWrapper.h>

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

    static int DEFAULT_COLUMN_COUNT;
    static int DEFAULT_ROW_COUNT;

    ~KisSwatchGroup();
    KisSwatchGroup(const KisSwatchGroup &rhs);
    KisSwatchGroup &operator=(const KisSwatchGroup &rhs);

    void setName(const QString &name);
    QString name() const;

    void setRowCount(int newRowCount);
    int rowCount() const;

    int colorCount() const;

    /**
     * @brief getColors
     * @return the list of colors in this SwatchGroup, in no specific order.
     */
    QList<SwatchInfo> infoList() const;

    /**
     * @brief checkSwatch
     * checks if position @p column and @p row has a valid entry
     * both @p column and @p row start from 0
     * @param column
     * @param row
     * @return true if there is a valid entry at position (column, row)
     */
    bool checkSwatchExists(int column, int row) const;

    /**
     * @brief getSwatch
     * used to get the swatch entry at position (@p column, @p row)
     * there is an assertion to make sure that this position isn't empty,
     * so checkEntry(int, int) must be used before this method to ensure
     * a valid entry can be found
     * @param column
     * @param row
     * @return the swatch entry at position (column, row)
     */
    KisSwatch getSwatch(int column, int row) const;

private:

    friend class KoColorSet;
    friend struct AddSwatchCommand;
    friend struct RemoveGroupCommand;
    friend struct RemoveSwatchCommand;
    friend struct AddGroupCommand;
    friend struct ClearCommand;
    friend struct SetColumnCountCommand;
    friend class TestKisSwatchGroup;
    friend class TestKoColorSet;

    // Hidden, you're supposed to go through KoColorSet or KisPaletteModel
    KisSwatchGroup();

    friend class KisPaletteEditor; // Ew, gross! Refactor this when you understand what the PaletteEditor does...

    /**
     * @brief setSwatch
     * sets the entry at position (@p column, @p row) to be @p e
     * @param e
     * @param column
     * @param row
     */
    void setSwatch(const KisSwatch &e, int column, int row);

    /**
     * @brief removeSwatch
     * removes the entry at position (@p column, @p row)
     * @param column
     * @param row
     * @return true if these is an entry at (column, row)
     */
    bool removeSwatch(int column, int row);

    /**
     * @brief addEntry
     * adds the entry e to the right of the rightmost entry in the last row
     * if the rightmost entry in the last row is in the right most column,
     * add e to the leftmost column of a new row
     *
     * when column is set to 0, resize number of columns to default
     * @param e
     */
    QPair<int, int> addSwatch(const KisSwatch &e);

    void clear();


    void setColumnCount(int columnCount);
    int columnCount() const;

    /**
     * The usage of propagate_const makes all 'const' methods of the
     * class reentrant. Which is necessary for, e.g., palettize filter.
     */
    struct Private;
    std::experimental::propagate_const<std::unique_ptr<Private>> d;
};

inline QDebug operator<<(QDebug dbg, const KisSwatchGroup group)
{
    dbg.nospace() << "[Group] Name: " << group.name();
    return dbg.space();
}

typedef QSharedPointer<KisSwatchGroup> KisSwatchGroupSP;


#endif // KISSWATCHGROUP_H

/* This file is part of the KDE project
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2009 Elvis Stansvik <elvstone@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KOTABLECOLUMNANDROWSTYLEMANAGER_H
#define KOTABLECOLUMNANDROWSTYLEMANAGER_H

#include "kritatext_export.h"

#include <QMetaType>
#include <QExplicitlySharedDataPointer>

class KoTableColumnStyle;
class KoTableRowStyle;
class KoTableCellStyle;
class QTextTable;

/**
 * Manages all column and row styles for a single table.
 *
 * It's not managing the lifetime of named styles, which is the job of the KoStyleManager,
 * so you should still register such styles in the styleManager too.
 *
 * The main purpose of this manager is simply to keep track of which styles are in
 * which column (and in which row).
 *
 * It's explicitly shared (for the same table)
 * TODO:
 *  - Eliminate duplicates.
 */
class KRITATEXT_EXPORT KoTableColumnAndRowStyleManager
{
public:
    /// constructor @see getManager for how to create a class the correct way
    explicit KoTableColumnAndRowStyleManager();

    virtual ~KoTableColumnAndRowStyleManager();

    /// Convenience function to get the KoTableColumnAndRowStyleManager for a table (or create one)
    static KoTableColumnAndRowStyleManager getManager(QTextTable *table);

    /// Constructor
    KoTableColumnAndRowStyleManager(const KoTableColumnAndRowStyleManager &rhs);
    /// assign operator
    KoTableColumnAndRowStyleManager &operator=(const KoTableColumnAndRowStyleManager &rhs);

    /**
     * Set the column style for the column \a column to \a columnStyle.
     *
     * @param column the column to set the style for.
     * @param columnStyle a column style.
     */
    void setColumnStyle(int column, const KoTableColumnStyle &columnStyle);

    /**
     * Insert a number of columns before the column \a column to \a columnStyle.
     *
     * @param column the columns are inserted before this column.
     * @param numberColumns how many columns to insert.
     * @param columnStyle the column style of the new columns.
     * @see QTextTable::insertColumns for the analog method for the table data.
     */
    void insertColumns(int column, int numberColumns, const KoTableColumnStyle &columnStyle);

    /**
     * Remove a number of columns  \a column to \a columnStyle.
     *
     * @param column this and possibly following columns are removed.
     * @param numberColumns how many columns to remove.
     * @see QTextTable::removeColumns for the analog method for the table data.
     */
    void removeColumns(int column, int numberColumns);

    /**
     * Get the column style for the column \a column.
     *
     * If you modify it don't forget to set it back here to actually have an effect
     *
     * @param column the column to get the style for.
     * @return the column style.
     */
    KoTableColumnStyle columnStyle(int column) const;

    /**
     * Set the row style for the row \a row to \a rowStyle.
     *
     * @param row the row to set the style for.
     * @param rowStyle a row style.
     */
    void setRowStyle(int row, const KoTableRowStyle &rowStyle);

    /**
     * Insert a number of rows above the row \a row to \a rowStyle.
     *
     * @param row the rows are inserted above this row.
     * @param numberRows how many rows to insert.
     * @param rowStyle the row style of the new rows.
     * @see QTextTable::insertRows for the analog method for the table data.
     */
    void insertRows(int row, int numberRows, const KoTableRowStyle &rowStyle);

    /**
     * Remove a number of rows  \a row to \a rowStyle.
     *
     * @param row this and possibly following rows are removed.
     * @param numberRows how many rows to remove.
     * @see QTextTable::removeRows for the analog method for the table data.
     */
    void removeRows(int row, int numberRows);

    /**
     * Get the row style for the row \a column.
     *
     * If you modify it don't forget to set it back here to actually have an effect
     *
     * @param row the row to get the style for.
     * @return the row style.
     */
    KoTableRowStyle rowStyle(int row) const;

    /**
     * Get the default cell style for the row \a row.
     *
     * @param row the row to get the style for.
     * @return the default cell style for \a row.
     */
    KoTableCellStyle* defaultRowCellStyle(int row) const;

    /**
     * Set the default cell style for the row \a row.
     *
     * @param row the row to set the style to.
     * @return the default cell style for \a row.
     */
    void setDefaultRowCellStyle(int row, KoTableCellStyle* cellStyle);

    /**
     * Get the default cell style for the column \a column.
     *
     * @param column the column to get the style for.
     * @return the default cell style for \a column.
     */
    KoTableCellStyle* defaultColumnCellStyle(int column) const;

    /**
     * Set the default cell style for the column \a column.
     *
     * @param column the column to set the style to.
     * @return the default cell style for \a column.
     */
    void setDefaultColumnCellStyle(int column, KoTableCellStyle* cellStyle);

private:
    class Private;
    QExplicitlySharedDataPointer<Private> d;
};

Q_DECLARE_METATYPE(KoTableColumnAndRowStyleManager)

#endif // KOTABLECOLUMNANDROWSTYLEMANAGER_H


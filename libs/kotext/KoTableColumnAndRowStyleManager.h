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

#include "kotext_export.h"

class KoTableColumnStyle;
class KoTableRowStyle;

/**
 * Manages all column and row styles for a table.
 *
 * TODO:
 *  - Eliminate duplicates.
 *  - Delegate to KoStyleManager for named styles.
 *  - Keep instances instead of pointers to styles after the styles
 *    have been made implicitly shared.
 */
class KOTEXT_EXPORT KoTableColumnAndRowStyleManager
{
public:
    explicit KoTableColumnAndRowStyleManager();
    virtual ~KoTableColumnAndRowStyleManager();

    /**
     * Set the column style for the column \a column to \a columnStyle.
     *
     * @param column the column to set the style for.
     * @param columnStyle a pointer to the column style.
     */
    void setColumnStyle(int column, KoTableColumnStyle *columnStyle);

    /**
     * Get the column style for the column \a column.
     *
     * @param column the column to get the style for.
     * @return the column style.
     */
    KoTableColumnStyle *columnStyle(int column);

    /**
     * Set the row style for the row \a row to \a rowStyle.
     *
     * @param row the row to set the style for.
     * @param rowStyle a pointer to the row style.
     */
    void setRowStyle(int row, KoTableRowStyle *rowStyle);

    /**
     * Get the row style for the row \a column.
     *
     * @param row the row to get the style for.
     * @return the row style.
     */
    KoTableRowStyle *rowStyle(int row);

private:
    class Private;
    Private* const d;
};

#endif // KOTABLECOLUMNANDROWSTYLEMANAGER_H


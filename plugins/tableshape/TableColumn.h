/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
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
#ifndef TABLE_COLUMN_H
#define TABLE_COLUMN_H

#include <QObject>

/**
 * Every column in a table has a column description element
 * <table:table-column>. It is similar to the [XSL] <fo:table-column>
 * element, and its primary use is to reference a table column style that
 * for instance specifies the table column's width. (8.2.1)
 */
class TableColumn : public QObject {

Q_OBJECT

public:

    TableColumn();

    ~TableColumn();

    /// Set the number of times this column style must be displayed. See the
    /// odf spec, 8.2.1, number-columns-repeated. The default is 1, which
    /// means the column description is valid for only one column.
    void setRepeat(int repeat);
    int repeat() const;

private:

    int m_repeated;

};

#endif

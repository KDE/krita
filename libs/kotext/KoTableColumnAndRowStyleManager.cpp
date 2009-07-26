/* This file is part of the KDE project
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
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

#include "KoTableColumnAndRowStyleManager.h"

#include "styles/KoTableColumnStyle.h"
#include "styles/KoTableRowStyle.h"

#include <QVector>
#include <QSet>
#include <QDebug>

class KoTableColumnAndRowStyleManager::Private
{
public:
    Private()  { }
    ~Private() {
        qDeleteAll(tableColumnStylesToDelete);
        qDeleteAll(tableRowStylesToDelete);
    }
    QVector<KoTableColumnStyle *> tableColumnStyles;
    QVector<KoTableRowStyle *> tableRowStyles;
    QSet<KoTableRowStyle *> tableRowStylesToDelete;
    QSet<KoTableColumnStyle *> tableColumnStylesToDelete;
};

KoTableColumnAndRowStyleManager::KoTableColumnAndRowStyleManager()
    : d(new Private())
{
}

KoTableColumnAndRowStyleManager::~KoTableColumnAndRowStyleManager()
{
    delete d;
}

void KoTableColumnAndRowStyleManager::setColumnStyle(int column, KoTableColumnStyle *columnStyle)
{
    Q_ASSERT(columnStyle);
    Q_ASSERT(column >= 0);

    if (column < 0) {
        return;
    }

    if (d->tableColumnStyles.value(column) == columnStyle) {
        return;
    }
    
    d->tableColumnStylesToDelete.insert(columnStyle); // add the style so it can be deleted on destruction

    d->tableColumnStyles.insert(column, columnStyle);
}

KoTableColumnStyle *KoTableColumnAndRowStyleManager::columnStyle(int column)
{
    Q_ASSERT(column >= 0);

    if (column < 0) {
        return 0;
    }

    return d->tableColumnStyles.value(column, 0);
}

void KoTableColumnAndRowStyleManager::setRowStyle(int row, KoTableRowStyle *rowStyle)
{
    Q_ASSERT(rowStyle);
    Q_ASSERT(row >= 0);

    if (row < 0) {
        return;
    }

    if (d->tableRowStyles.value(row) == rowStyle) {
        return;
    }

    d->tableRowStylesToDelete.insert(rowStyle); // add the style so it can be deleted on destruction

    while (row > d->tableRowStyles.size())
        d->tableRowStyles.append(0);
    d->tableRowStyles.insert(row, rowStyle);
}

KoTableRowStyle *KoTableColumnAndRowStyleManager::rowStyle(int row)
{
    Q_ASSERT(row >= 0);

    if (row < 0) {
        return 0;
    }

    return d->tableRowStyles.value(row, 0);
}


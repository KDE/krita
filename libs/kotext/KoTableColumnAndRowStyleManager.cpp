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

#include <QMap>
#include <QDebug>

class KoTableColumnAndRowStyleManager::Private
{
public:
    Private()  { }
    ~Private() {
        qDeleteAll(tableColumnStyles);
    }
    QMap<int, KoTableColumnStyle *> tableColumnStyles;
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

    KoTableColumnStyle *oldColumnStyle = d->tableColumnStyles.value(column);

    if (oldColumnStyle == columnStyle) {
        return;
    }

    delete oldColumnStyle;
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


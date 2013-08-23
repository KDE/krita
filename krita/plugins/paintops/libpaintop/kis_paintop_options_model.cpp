/* This file is part of the KDE project
 * Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2011 Silvio Heinrich <plassyqweb.de>
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

#include "kis_paintop_options_model.h"
#include "kis_paintop_option.h"

KisPaintOpOptionListModel::KisPaintOpOptionListModel(QObject *parent)
    : BaseOptionCategorizedListModel(parent)
{
}

void KisPaintOpOptionListModel::addPaintOpOption(KisPaintOpOption* option, int widgetIndex)
{
    DataItem *item = categoriesMapper()->addEntry(option->category(), KisOptionInfo(option, widgetIndex));

    if (option->isCheckable()) {
        item->setCheckable(true);
    }

    categoriesMapper()->expandAllCategories();
}

bool KisPaintOpOptionListModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    if (!idx.isValid()) return false;

    DataItem *item = categoriesMapper()->itemFromRow(idx.row());
    Q_ASSERT(item);

    if (role == Qt::CheckStateRole && item->checkable()) {
        item->data()->option->setChecked(value.toInt() == Qt::Checked);
    }

    return BaseOptionCategorizedListModel::setData(idx, value, role);
}

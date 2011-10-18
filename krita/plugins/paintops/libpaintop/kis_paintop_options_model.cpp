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

void KisPaintOpOptionListModel::addPaintOpOption(KisPaintOpOption* option, int widgetIndex)
{
    BaseClass::addEntry(option->category(), KisOptionInfo(option, widgetIndex));
}

QString KisPaintOpOptionListModel::categoryToString(const QString& val) const
{
    return val;
}

QString KisPaintOpOptionListModel::entryToString(const KisOptionInfo& val) const
{
    return val.option->label();
}

QVariant KisPaintOpOptionListModel::data(const QModelIndex& idx, int role) const
{
    if (idx.isValid() && role == Qt::CheckStateRole) {
        KisOptionInfo info;

        if (BaseClass::entryAt(info, idx.row()) && info.option->isCheckable())
            return info.option->isChecked() ? Qt::Checked : Qt::Unchecked;

        return QVariant();
    }

    return BaseClass::data(idx, role);
}

bool KisPaintOpOptionListModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    if (idx.isValid() && role == Qt::CheckStateRole) {
        KisOptionInfo info;

        if (BaseClass::entryAt(info, idx.row()) && info.option->isCheckable()) {
            info.option->setChecked(value.toInt() == Qt::Checked);
            return true;
        }

        return false;
    }

    return BaseClass::setData(idx, value, role);
}

Qt::ItemFlags KisPaintOpOptionListModel::flags(const QModelIndex& idx) const
{
    Qt::ItemFlags flags = 0;
    KisOptionInfo info;

    if (idx.isValid() && BaseClass::entryAt(info, idx.row())) {
        if (info.option->isCheckable())
            flags |= Qt::ItemIsUserCheckable;
    }

    return BaseClass::flags(idx) | flags;
}

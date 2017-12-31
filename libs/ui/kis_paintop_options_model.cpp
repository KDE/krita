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

#include <klocalizedstring.h>


KisPaintOpOptionListModel::KisPaintOpOptionListModel(QObject *parent)
    : BaseOptionCategorizedListModel(parent)
{
}

void KisPaintOpOptionListModel::addPaintOpOption(KisPaintOpOption *option, int widgetIndex, const QString &label, KisPaintOpOption::PaintopCategory categoryType)
{

    QString category;
    switch(categoryType) {
    case KisPaintOpOption::GENERAL:
        category = i18nc("option category", "General");
        break;
    case KisPaintOpOption::COLOR:
        category = i18nc("option category", "Color");
        break;
    case KisPaintOpOption::TEXTURE:
        category = i18nc("option category", "Texture");
        break;
    case KisPaintOpOption::FILTER:
        category = i18nc("option category", "Filter");
        break;
    case KisPaintOpOption::MASKING_BRUSH:
        category = i18nc("option category", "Masked Brush");
        break;
    default:
        category = i18n("Unknown");
    };

    DataItem *item = categoriesMapper()->addEntry(category, KisOptionInfo(option, widgetIndex, label));

    if (option->isCheckable()) {
        item->setCheckable(true);
        item->setChecked(option->isChecked());
    }

    categoriesMapper()->expandAllCategories();
}

QVariant KisPaintOpOptionListModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid()) return false;

    DataItem *item = categoriesMapper()->itemFromRow(idx.row());
    Q_ASSERT(item);

    // Lazy fetching of the real checked value (there are no notifications
    // when changing the pointop preset)

    if (role == Qt::CheckStateRole && item->isCheckable()) {
        bool realChecked = item->data()->option->isChecked();

        if (realChecked != item->isChecked()) {
            item->setChecked(realChecked);
        }
    }

    return BaseOptionCategorizedListModel::data(idx, role);
}

bool KisPaintOpOptionListModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    if (!idx.isValid()) return false;

    DataItem *item = categoriesMapper()->itemFromRow(idx.row());
    Q_ASSERT(item);

    if (role == Qt::CheckStateRole && item->isCheckable()) {
        item->data()->option->setChecked(value.toInt() == Qt::Checked);
    }

    return BaseOptionCategorizedListModel::setData(idx, value, role);
}

bool operator==(const KisOptionInfo& a, const KisOptionInfo& b)
{
    if (a.index != b.index) return false;
    if (a.option->objectName() == b.option->objectName())
    if (a.option->category() != b.option->category()) return false;
    if (a.option->isCheckable() != b.option->isCheckable()) return false;
    if (a.option->isChecked() != b.option->isChecked()) return false;
    return true;
}
void KisPaintOpOptionListModel::signalDataChanged(const QModelIndex& index)
{
    emit dataChanged(index,index);
}

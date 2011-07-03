/*
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_categorized_list_view.h"
#include "../kis_categorized_list_model.h"

KisCategorizedListView::KisCategorizedListView(QWidget* parent):
    QListView(parent)
{
    connect(this, SIGNAL(activated(const QModelIndex&)), this, SLOT(slotIndexChanged(const QModelIndex&)));
}

void KisCategorizedListView::slotIndexChanged(const QModelIndex& index)
{
    if(model()->data(index, IsHeaderRole).toBool()) {
        bool expanded = model()->data(index, ExpandCategoryRole).toBool();
        model()->setData(index, !expanded, ExpandCategoryRole);
        emit sigCategoryToggled(index, !expanded);
    }
}

void KisCategorizedListView::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    QListView::dataChanged(topLeft, bottomRight);
    
    int beg = topLeft.row();
    int end = bottomRight.row();
    
    for(; beg<=end; ++beg) {
        QModelIndex index    = model()->index(beg, 0);
        bool        isHeader = model()->data(index, IsHeaderRole).toBool();
        bool        expanded = model()->data(index, ExpandCategoryRole).toBool();
        setRowHidden(beg, !expanded && !isHeader);
    }
}


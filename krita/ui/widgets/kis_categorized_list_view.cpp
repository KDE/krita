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
#include <QMouseEvent>

KisCategorizedListView::KisCategorizedListView(bool useCheckBoxHack, QWidget* parent):
    QListView(parent), m_useCheckBoxHack(useCheckBoxHack)
{
    connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(slotIndexChanged(QModelIndex)));
}

KisCategorizedListView::~KisCategorizedListView()
{
}

void KisCategorizedListView::setModel(QAbstractItemModel* model)
{
    QListView::setModel(model);
    updateRows(0, model->rowCount());
}

void KisCategorizedListView::updateRows(int begin, int end)
{
    for(; begin!=end; ++begin) {
        QModelIndex index    = model()->index(begin, 0);
        bool        isHeader = model()->data(index, IsHeaderRole).toBool();
        bool        expanded = model()->data(index, ExpandCategoryRole).toBool();
        setRowHidden(begin, !expanded && !isHeader);
    }
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
    updateRows(topLeft.row(), bottomRight.row()+1);
}

void KisCategorizedListView::rowsInserted(const QModelIndex& parent, int start, int end)
{
    QListView::rowsInserted(parent, start, end);
    updateRows(0, model()->rowCount());
}

void KisCategorizedListView::mousePressEvent(QMouseEvent* event)
{
    if (m_useCheckBoxHack) {
        QModelIndex index = QListView::indexAt(event->pos());

        if (index.isValid() && (event->pos().x() < 25) && (model()->flags(index) & Qt::ItemIsUserCheckable)) {
            int role = model()->data(index, Qt::CheckStateRole).toInt();
            
            if (role == Qt::Checked) { model()->setData(index, Qt::Unchecked, Qt::CheckStateRole); }
            else                     { model()->setData(index, Qt::Checked  , Qt::CheckStateRole); }
            
            emit sigEntryChecked(index);
            return;
        }
    }
    
    QListView::mousePressEvent(event);
}

void KisCategorizedListView::mouseReleaseEvent(QMouseEvent* event)
{
    QListView::mouseReleaseEvent(event);
}

/*
 * Copyright (C) 2018 Boudewijn Rempt <boud@valdyas.org>
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

#include "KisGridView.h"

struct Q_DECL_HIDDEN KisGridView::Private {

};

KisGridView::KisGridView(QWidget *parent)
    : QAbstractItemView(parent)
    , d(new Private)
{

}

KisGridView::~KisGridView()
{

}

QModelIndex KisGridView::indexAt(const QPoint &point) const
{
    return QModelIndex();
}

void KisGridView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint)
{

}

QRect KisGridView::visualRect(const QModelIndex &index) const
{
    return QRect();
}

void KisGridView::paintEvent(QPaintEvent *event)
{

}


int KisGridView::horizontalOffset() const
{
    return 0;
}

bool KisGridView::isIndexHidden(const QModelIndex &index) const
{
    return true;
}

QModelIndex KisGridView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    return QModelIndex();
}

void KisGridView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags)
{

}

int KisGridView::verticalOffset() const
{
    return 0;
}

QRegion KisGridView::visualRegionForSelection(const QItemSelection &selection) const
{
    return QRegion();
}


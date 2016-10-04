/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com
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

#include "KoResourceItemView.h"
#include <QEvent>
#include <QHelpEvent>

#include <QDebug>

KoResourceItemView::KoResourceItemView(QWidget *parent)
    : KoTableView(parent)
{
    connect(this, SIGNAL(clicked(QModelIndex)), SLOT(slotItemClicked(QModelIndex)));
}

bool KoResourceItemView::viewportEvent(QEvent *event)
{
    if (event->type() == QEvent::ToolTip && model()) {
        QHelpEvent *he = static_cast<QHelpEvent *>(event);
        QStyleOptionViewItem option = viewOptions();
        QModelIndex index = model()->buddy(indexAt(he->pos()));
        if (index.isValid()) {
            option.rect = visualRect(index);
            m_tip.showTip(this, he->pos(), option, index);
            return true;
        }
    }

    return QTableView::viewportEvent(event);
}


void KoResourceItemView::selectionChanged(const QItemSelection &selected, const QItemSelection &/*deselected*/)
{
    if (selected.isEmpty()) {
        emit currentResourceChanged(QModelIndex());
    } else {
        emit currentResourceChanged(selected.indexes().first());
    }
}

void KoResourceItemView::mousePressEvent(QMouseEvent *event)
{
    m_beforeClickIndex = currentIndex();
    KoTableView::mousePressEvent(event);
}

void KoResourceItemView::slotItemClicked(const QModelIndex &index)
{
    if (m_beforeClickIndex == index) {
        emit currentResourceClicked(index);
    }

    m_beforeClickIndex = QModelIndex();
}

void KoResourceItemView::contextMenuEvent(QContextMenuEvent *event)
{
    QTableView::contextMenuEvent(event);
    emit contextMenuRequested(event->globalPos());
}

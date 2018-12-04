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

#include "KisResourceItemView.h"

#include <QEvent>
#include <QHeaderView>
#include <QScroller>
#include <QHelpEvent>
#include <QDebug>

#include <KisKineticScroller.h>

KisResourceItemView::KisResourceItemView(QWidget *parent)
    : QTableView(parent)
{
    setSelectionMode(QAbstractItemView::SingleSelection);
    verticalHeader()->hide();
    horizontalHeader()->hide();
    verticalHeader()->setDefaultSectionSize(20);
    setContextMenuPolicy(Qt::DefaultContextMenu);
    setViewMode(FIXED_COLUMNS);

    QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(this);
    if (scroller) {
        connect(scroller, SIGNAL(stateChanged(QScroller::State)), this, SLOT(slotScrollerStateChange(QScroller::State)));
    }

    connect(this, SIGNAL(clicked(QModelIndex)), SLOT(slotItemClicked(QModelIndex)));
}

bool KisResourceItemView::viewportEvent(QEvent *event)
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

void KisResourceItemView::selectionChanged(const QItemSelection &selected, const QItemSelection &/*deselected*/)
{
    if (selected.isEmpty()) {
        emit currentResourceChanged(QModelIndex());
    } else {
        emit currentResourceChanged(selected.indexes().first());
    }
}

void KisResourceItemView::mousePressEvent(QMouseEvent *event)
{
    m_beforeClickIndex = currentIndex();
    QTableView::mousePressEvent(event);
}

void KisResourceItemView::slotItemClicked(const QModelIndex &index)
{
    if (m_beforeClickIndex == index) {
        emit currentResourceClicked(index);
    }

    m_beforeClickIndex = QModelIndex();
}

void KisResourceItemView::contextMenuEvent(QContextMenuEvent *event)
{
    QTableView::contextMenuEvent(event);
    emit contextMenuRequested(event->globalPos());
}

void KisResourceItemView::resizeEvent(QResizeEvent *event)
{
    QTableView::resizeEvent(event);
    updateView();

    emit sigSizeChanged();
}

void KisResourceItemView::setViewMode(ViewMode mode)
{
    m_viewMode = mode;

    switch (m_viewMode) {
    case FIXED_COLUMNS:
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // Horizontal scrollbar is never needed
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        break;
    case FIXED_ROWS:
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // Vertical scrollbar is never needed
        break;
    default:
        setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }

}

void KisResourceItemView::updateView()
{
    int columnCount = model()->columnCount(QModelIndex());
    int rowCount = model()->rowCount(QModelIndex());
    int rowHeight, columnWidth;

    if (m_viewMode == FIXED_COLUMNS) {
        columnWidth = viewport()->size().width() / columnCount;

        for (int i = 0; i < columnCount; ++i) {
            setColumnWidth(i, columnWidth);
        }
        if (columnCount > 1) {
            for (int i = 0; i < rowCount; ++i) {
                setRowHeight(i, columnWidth);
            }
        }
    } else if (m_viewMode == FIXED_ROWS) {
        if (rowCount == 0) return;  // Don't divide by zero
        rowHeight = viewport()->size().height() / rowCount;

        for (int i = 0; i < rowCount; ++i) {
            setRowHeight(i, rowHeight);
        }
    }
}

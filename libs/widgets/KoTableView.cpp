/*
 * Copyright (C) 2015 Boudewijn Rempt <boud@valdyas.org>
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
#include "KoTableView.h"

#include <QEvent>
#include <QHeaderView>
#include <QtMath>

KoTableView::KoTableView(QWidget *parent)
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
}

void KoTableView::resizeEvent(QResizeEvent *event)
{
    QTableView::resizeEvent(event);
    updateView();

    emit sigSizeChanged();
}

void KoTableView::setViewMode(KoTableView::ViewMode mode)
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

void KoTableView::updateView()
{
    int columnCount = model()->columnCount(QModelIndex());
    int rowCount = model()->rowCount(QModelIndex());
    int rowHeight, columnWidth;

    if (m_viewMode == FIXED_COLUMNS) {
        columnWidth = qFloor(viewport()->size().width() / static_cast<double>(columnCount));

        for (int i = 0; i < columnCount; ++i) {
            setColumnWidth(i, columnWidth);
        }
        // keep aspect ratio always square.
        if (columnCount > 1) {
            for (int i = 0; i < rowCount; ++i) {
                setRowHeight(i, columnWidth);
            }
        }
    } else if (m_viewMode == FIXED_ROWS) {
        if (rowCount == 0) return;  // Don't divide by zero
        rowHeight = qFloor(viewport()->size().height() / static_cast<double>(rowCount));

        for (int i = 0; i < rowCount; ++i) {
            setRowHeight(i, rowHeight);
        }
    }
}

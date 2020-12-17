/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2011 José Luis Vergara <pentalis@gmail.com
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisResourceItemView.h"

#include <QEvent>
#include <QHeaderView>
#include <QScroller>
#include <QHelpEvent>
#include <QDebug>

#include <KisKineticScroller.h>

#include <QtMath>

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
    if (!model()) return true;

    if (event->type() == QEvent::ToolTip) {
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
    }
    else {
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
    if (!model()) return;

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

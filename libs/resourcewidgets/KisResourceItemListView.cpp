/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2019 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisResourceItemListView.h"

#include <QEvent>
#include <QHeaderView>
#include <QScroller>
#include <QHelpEvent>
#include <QDebug>

KisResourceItemListView::KisResourceItemListView(QWidget *parent): QListView(parent)
{
    setSelectionMode(QAbstractItemView::SingleSelection);
    setContextMenuPolicy(Qt::DefaultContextMenu);
    setViewMode(QListView::IconMode);
    setGridSize(QSize(64, 64));
    setIconSize(QSize(64, 64));
    setResizeMode(QListView::Adjust);
    setUniformItemSizes(true);

    QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(this);
    if (scroller) {
        connect(scroller, SIGNAL(stateChanged(QScroller::State)), this, SLOT(slotScrollerStateChange(QScroller::State)));
    }

    connect(this, SIGNAL(clicked(QModelIndex)), SIGNAL(currentResourceClicked(const QModelIndex &)));
}

void KisResourceItemListView::setItemSize(QSize size)
{
    setGridSize(size);
    setIconSize(size);
}

void KisResourceItemListView::setFixedToolTipThumbnailSize(const QSize &size)
{
    m_tip.setFixedToolTipThumbnailSize(size);
}

void KisResourceItemListView::setToolTipShouldRenderCheckers(bool value)
{
    m_tip.setToolTipShouldRenderCheckers(value);
}

void KisResourceItemListView::selectionChanged(const QItemSelection &selected, const QItemSelection &/*deselected*/)
{
    if (selected.isEmpty()) {
        emit currentResourceChanged(QModelIndex());
    }
    else {
        emit currentResourceChanged(selected.indexes().first());
    }
}

void KisResourceItemListView::contextMenuEvent(QContextMenuEvent *event)
{
    QListView::contextMenuEvent(event);
    emit contextMenuRequested(event->globalPos());
}

bool KisResourceItemListView::viewportEvent(QEvent *event)
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

    return QListView::viewportEvent(event);
}

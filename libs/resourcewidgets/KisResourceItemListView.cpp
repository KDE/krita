/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2019 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisResourceItemListView.h"

#include <QEvent>
#include <QHeaderView>
#include <QScroller>
#include <QScrollBar>
#include <QHelpEvent>
#include <QDebug>

#include "KisIconToolTip.h"


struct  Q_DECL_HIDDEN KisResourceItemListView::Private
{
    ListViewMode viewMode = ListViewMode::IconGrid;
    bool strictSelectionMode {false};
    KisIconToolTip tip;

    QScroller* scroller {0};
    QString prev_scrollbar_style;
};

KisResourceItemListView::KisResourceItemListView(QWidget *parent)
    : QListView(parent)
    , m_d(new Private)
{ 
    setSelectionMode(QAbstractItemView::SingleSelection);
    setContextMenuPolicy(Qt::DefaultContextMenu);
    setViewMode(QListView::IconMode);
    setGridSize(QSize(64, 64));
    setIconSize(QSize(64, 64));
    setResizeMode(QListView::Adjust);
    setUniformItemSizes(true);

    m_d->scroller = KisKineticScroller::createPreconfiguredScroller(this);
    if (m_d->scroller) {
        connect(m_d->scroller, SIGNAL(stateChanged(QScroller::State)), this, SLOT(slotScrollerStateChange(QScroller::State)));
    }

    connect(this, SIGNAL(clicked(QModelIndex)), SIGNAL(currentResourceClicked(const QModelIndex &)));

     m_d->prev_scrollbar_style = horizontalScrollBar()->styleSheet();
}

KisResourceItemListView::~KisResourceItemListView()
{
}

void KisResourceItemListView::setListViewMode(ListViewMode viewMode)
{
    m_d->viewMode = viewMode;

    switch (viewMode) {
    case ListViewMode::IconGrid: {
        setViewMode(ViewMode::IconMode);
        setFlow(Flow::LeftToRight);
        setWrapping(true);
        horizontalScrollBar()->setStyleSheet(m_d->prev_scrollbar_style);        
        setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
        break;
    }
    case ListViewMode::IconStripHorizontal: {
        setViewMode(ViewMode::IconMode);
        setFlow(Flow::LeftToRight);
        setWrapping(false);

        // this is the only way to hide it and not have it ocupy space
        horizontalScrollBar()->setStyleSheet("QScrollBar::horizontal {height: 0px;}");
        setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        break;
    }
    case ListViewMode::Detail: {
        setViewMode(ViewMode::ListMode);
        setFlow(Flow::TopToBottom);
        setWrapping(false);
        horizontalScrollBar()->setStyleSheet(m_d->prev_scrollbar_style);
        setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
        break;
    }
    }

    setItemSize(iconSize());
}

void KisResourceItemListView::setItemSize(QSize size)
{
    switch (m_d->viewMode) {
    case ListViewMode::IconGrid: {
        setGridSize(size);
        setIconSize(size);
        break;
    }
    case ListViewMode::IconStripHorizontal: {
        // you can not set the item size in strip mode
        // it is configured automatically based on size
        break;
    }
    case ListViewMode::Detail: {
        const int w = width();
        setGridSize(QSize(w, size.height()));
        setIconSize(QSize(size));
        break;
    }
    }
}

void KisResourceItemListView::setStrictSelectionMode(bool enable)
{
    m_d->strictSelectionMode = enable;
}

void KisResourceItemListView::setFixedToolTipThumbnailSize(const QSize &size)
{
    m_d->tip.setFixedToolTipThumbnailSize(size);
}

void KisResourceItemListView::setToolTipShouldRenderCheckers(bool value)
{
    m_d->tip.setToolTipShouldRenderCheckers(value);
}

void KisResourceItemListView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    // QAbstractItemView moves the current index if the row it points to is removed,
    // which we don't want for strict selections
    QModelIndex current = currentIndex();
    if (selectionMode() == SingleSelection
            && m_d->strictSelectionMode
            && current.isValid()
            && current.row() >= start
            && current.row() <= end) {

        selectionModel()->clear();
    }
    QListView::rowsAboutToBeRemoved(parent, start, end);
}

void KisResourceItemListView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    // base class takes care of viewport updates
    QListView::selectionChanged(selected, deselected);

    if (selected.isEmpty()) {
        emit currentResourceChanged(QModelIndex());
    }
    else {
        emit currentResourceChanged(selected.indexes().first());
    }
}

QItemSelectionModel::SelectionFlags KisResourceItemListView::selectionCommand(const QModelIndex &index, const QEvent *event) const
{
    QItemSelectionModel::SelectionFlags cmd = QListView::selectionCommand(index, event);

    // avoid deselecting the current item by Ctrl-clicking in single selection mode
    if (selectionMode() == SingleSelection
            && m_d->strictSelectionMode
            && cmd.testFlag(QItemSelectionModel::Deselect)) {

        cmd = QItemSelectionModel::NoUpdate;
    }
    return cmd;
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
            m_d->tip.showTip(this, he->pos(), option, index);
            return true;
        }
        m_d->tip.hide();
    }

    return QListView::viewportEvent(event);
}

void KisResourceItemListView::resizeEvent(QResizeEvent *event)
{
    if (m_d->viewMode == ListViewMode::IconStripHorizontal) {
        const int height = event->size().height();
        qDebug("resizeEvent height = %d", height);
        setGridSize(QSize(height, height));
        setIconSize(QSize(height, height));
    }
}

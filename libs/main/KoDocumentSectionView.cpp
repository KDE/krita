/*
  Copyright (c) 2006 Gábor Lehel <illissius@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
#include "KoDocumentSectionView.h"
#include "KoDocumentSectionPropertyAction_p.h"
#include "KoDocumentSectionDelegate.h"
#include "KoDocumentSectionModel.h"

#include <KIconLoader>

#include <QtDebug>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QHelpEvent>
#include <QMenu>
#include <QDrag>
#include <QMouseEvent>
#include <QPersistentModelIndex>
#include <QApplication>
#include <QPainter>
#include <QScrollBar>

class KoDocumentSectionView::Private
{
public:
    Private(): delegate(0), mode(DetailedMode) { }
    KoDocumentSectionDelegate *delegate;
    DisplayMode mode;
    QPersistentModelIndex hovered;
    QPoint lastPos;
};

KoDocumentSectionView::KoDocumentSectionView(QWidget *parent)
    : QTreeView(parent)
    , m_draggingFlag(false)
    , d(new Private)
{
    d->delegate = new KoDocumentSectionDelegate(this, this);
    setMouseTracking(true);
    setVerticalScrollMode(ScrollPerPixel);
    setSelectionMode(SingleSelection);
    setSelectionBehavior(SelectItems);
    header()->hide();
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
}

KoDocumentSectionView::~KoDocumentSectionView()
{
    delete d;
}

void KoDocumentSectionView::setDisplayMode(DisplayMode mode)
{
    if (d->mode != mode) {
        d->mode = mode;
        scheduleDelayedItemsLayout();
    }
}

KoDocumentSectionView::DisplayMode KoDocumentSectionView::displayMode() const
{
    return d->mode;
}

void KoDocumentSectionView::addPropertyActions(QMenu *menu, const QModelIndex &index)
{
    Model::PropertyList list = index.data(Model::PropertiesRole).value<Model::PropertyList>();
    for (int i = 0, n = list.count(); i < n; ++i) {
        if (list.at(i).isMutable) {
            PropertyAction *a = new PropertyAction(i, list.at(i), index, menu);
            connect(a, SIGNAL(toggled(bool, const QPersistentModelIndex&, int)),
                     this, SLOT(slotActionToggled(bool, const QPersistentModelIndex&, int)));
            menu->addAction(a);
        }
    }
}

bool KoDocumentSectionView::viewportEvent(QEvent *e)
{
    if (model()) {
        switch(e->type()) {
        case QEvent::MouseButtonPress: {
            const QPoint pos = static_cast<QMouseEvent*>(e)->pos();
            d->lastPos = pos;
            if (!indexAt(pos).isValid()) {
                return QTreeView::viewportEvent(e);
            }
            QModelIndex index = model()->buddy(indexAt(pos));
            if (d->delegate->editorEvent(e, model(), optionForIndex(index), index)) {
                return true;
            }
        } break;
        case QEvent::Leave: {
            QEvent e(QEvent::Leave);
            d->delegate->editorEvent(&e, model(), optionForIndex(d->hovered), d->hovered);
            d->hovered = QModelIndex();
        } break;
        case QEvent::MouseMove: {
            const QPoint pos = static_cast<QMouseEvent*>(e)->pos();
            QModelIndex hovered = indexAt(pos);
            if (hovered != d->hovered) {
                if (d->hovered.isValid()) {
                    QEvent e(QEvent::Leave);
                    d->delegate->editorEvent(&e, model(), optionForIndex(d->hovered), d->hovered);
                }
                if (hovered.isValid()) {
                    QEvent e(QEvent::Enter);
                    d->delegate->editorEvent(&e, model(), optionForIndex(hovered), hovered);
                }
                d->hovered = hovered;
            }
            /* This is a workaround for a bug in QTreeView that immediately begins a dragging action
            when the mouse lands on the decoration/icon of a different index and moves 1 pixel or more */
            Qt::MouseButtons buttons = static_cast<QMouseEvent*>(e)->buttons();
            if ((Qt::LeftButton | Qt::MidButton) & buttons) {
                if ((pos - d->lastPos).manhattanLength() > qApp->startDragDistance()) {
                    return QTreeView::viewportEvent(e);
                }
                return true;
            }
        } break;
       case QEvent::ToolTip: {
            const QPoint pos = static_cast<QHelpEvent*>(e)->pos();
            if (!indexAt(pos).isValid()) {
                return QTreeView::viewportEvent(e);
            }
            QModelIndex index = model()->buddy(indexAt(pos));
            return d->delegate->editorEvent(e, model(), optionForIndex(index), index);
        } break;
        case QEvent::Resize: {
            scheduleDelayedItemsLayout();
            break;
        }
        default: break;
        }
    }
    return QTreeView::viewportEvent(e);
}

void KoDocumentSectionView::contextMenuEvent(QContextMenuEvent *e)
{
    QTreeView::contextMenuEvent(e);
    QModelIndex i = indexAt(e->pos());
    if (model())
        i = model()->buddy(i);
    showContextMenu(e->globalPos(), i);
}

void KoDocumentSectionView::showContextMenu(const QPoint &globalPos, const QModelIndex &index)
{
    emit contextMenuRequested(globalPos, index);
}

void KoDocumentSectionView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTreeView::currentChanged(current, previous);
    if (current != previous /*&& current.isValid()*/) { //hack?
        Q_ASSERT(!current.isValid() || current.model() == model());
        model()->setData(current, true, Model::ActiveRole);
    }
}

void KoDocumentSectionView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    QTreeView::dataChanged(topLeft, bottomRight);
    for (int x = topLeft.row(); x <= bottomRight.row(); ++x) {
        for (int y = topLeft.column(); y <= bottomRight.column(); ++y) {
            if (topLeft.sibling(x, y).data(Model::ActiveRole).toBool()) {
                setCurrentIndex(topLeft.sibling(x, y));
                return;
            }
        }
    }
}

void KoDocumentSectionView::slotActionToggled(bool on, const QPersistentModelIndex &index, int num)
{
    Model::PropertyList list = index.data(Model::PropertiesRole).value<Model::PropertyList>();
    list[num].state = on;
    const_cast<QAbstractItemModel*>(index.model())->setData(index, QVariant::fromValue(list), Model::PropertiesRole);
}

QStyleOptionViewItem KoDocumentSectionView::optionForIndex(const QModelIndex &index) const
{
    QStyleOptionViewItem option = viewOptions();
    option.rect = visualRect(index);
    if (index == currentIndex())
        option.state |= QStyle::State_HasFocus;
    return option;
}

void KoDocumentSectionView::startDrag(Qt::DropActions supportedActions)
{
    if (displayMode() == KoDocumentSectionView::ThumbnailMode) {
        const QModelIndexList indexes = selectionModel()->selectedIndexes();
        if (!indexes.isEmpty()) {
            QMimeData *data = model()->mimeData(indexes);
            if (!data) {
                return;
            }
            QDrag *drag = new QDrag(this);
            drag->setPixmap(createDragPixmap());
            drag->setMimeData(data);
            //m_dragSource = this;
            drag->exec(supportedActions);
        }
    }
    else {
        QTreeView::startDrag(supportedActions);
    }
}

QPixmap KoDocumentSectionView::createDragPixmap() const
{
    const QModelIndexList selectedIndexes = selectionModel()->selectedIndexes();
    Q_ASSERT(!selectedIndexes.isEmpty());

    const int itemCount = selectedIndexes.count();

    // If more than one item is dragged, align the items inside a
    // rectangular grid. The maximum grid size is limited to 4 x 4 items.
    int xCount = 2;
    int size = 96;
    if (itemCount > 9) {
        xCount = 4;
        size = KIconLoader::SizeLarge;
    }
    else if (itemCount > 4) {
        xCount = 3;
        size = KIconLoader::SizeHuge;
    }
    else if (itemCount < xCount) {
        xCount = itemCount;
    }

    int yCount = itemCount / xCount;
    if (itemCount % xCount != 0) {
        ++yCount;
    }

    if (yCount > xCount) {
        yCount = xCount;
    }

    // Draw the selected items into the grid cells
    QPixmap dragPixmap(xCount * size + xCount - 1, yCount * size + yCount - 1);
    dragPixmap.fill(Qt::transparent);

    QPainter painter(&dragPixmap);
    int x = 0;
    int y = 0;
    foreach (const QModelIndex &selectedIndex, selectedIndexes) {
        const QImage img = selectedIndex.data(int(Model::BeginThumbnailRole) + size).value<QImage>();
        painter.drawPixmap(x, y, QPixmap().fromImage(img.scaled(QSize(size, size), Qt::KeepAspectRatio)));

        x += size + 1;
        if (x >= dragPixmap.width()) {
            x = 0;
            y += size + 1;
        }
        if (y >= dragPixmap.height()) {
            break;
        }
    }

    return dragPixmap;
}

void KoDocumentSectionView::paintEvent(QPaintEvent *event)
{
    event->accept();
    QTreeView::paintEvent(event);

    // Paint the line where the slide should go
    if (isDragging() && (displayMode() == KoDocumentSectionView::ThumbnailMode)) {
        QSize size(visualRect(model()->index(0, 0, QModelIndex())).width(), visualRect(model()->index(0, 0, QModelIndex())).height());
        int numberRow = cursorPageIndex();
        int scrollBarValue = verticalScrollBar()->value();

        QPoint point1(0, numberRow * size.height() - scrollBarValue);
        QPoint point2(size.width(), numberRow * size.height() - scrollBarValue);
        QLineF line(point1, point2);

        QPainter painter(this->viewport());
        QPen pen = QPen(palette().brush(QPalette::Highlight), 8);
        pen.setCapStyle(Qt::RoundCap);
        painter.setPen(pen);
        painter.setOpacity(0.8);
        painter.drawLine(line);
    }
}

void KoDocumentSectionView::dropEvent(QDropEvent *ev)
{
    if (displayMode() == KoDocumentSectionView::ThumbnailMode) {
        setDraggingFlag(false);
        ev->accept();
        clearSelection();

        if (!model()) {
            return;
        }

        int newIndex = cursorPageIndex();
        model()->dropMimeData(ev->mimeData(), ev->dropAction(), newIndex, -1, QModelIndex());
        return;
    }
    QTreeView::dropEvent(ev);
}

int KoDocumentSectionView::cursorPageIndex() const
{
    QSize size(visualRect(model()->index(0, 0, QModelIndex())).width(), visualRect(model()->index(0, 0, QModelIndex())).height());
    int scrollBarValue = verticalScrollBar()->value();

    QPoint cursorPosition = QWidget::mapFromGlobal(QCursor::pos());

    int numberRow = (cursorPosition.y() + scrollBarValue) / size.height();

    //If cursor is at the half button of the page then the move action is performed after the slide, otherwise it is
    //performed before the page
    if (abs((cursorPosition.y() + scrollBarValue) - size.height()*numberRow) > (size.height()/2)) {
        numberRow++;
    }

    if (numberRow > model()->rowCount(QModelIndex())) {
        numberRow = model()->rowCount(QModelIndex());
    }

    return numberRow;
}

void KoDocumentSectionView::dragMoveEvent(QDragMoveEvent *ev)
{
    if (displayMode() == KoDocumentSectionView::ThumbnailMode) {
        ev->accept();
        if (!model()) {
            return;
        }
        QTreeView::dragMoveEvent(ev);
        setDraggingFlag();
        viewport()->update();
        return;
    }
    QTreeView::dragMoveEvent(ev);
}

void KoDocumentSectionView::dragLeaveEvent(QDragLeaveEvent *e)
{
    if (displayMode() == KoDocumentSectionView::ThumbnailMode) {
        setDraggingFlag(false);
        return;
    }
    QTreeView::dragLeaveEvent(e);
}

bool KoDocumentSectionView::isDragging() const
{
    return m_draggingFlag;
}

void KoDocumentSectionView::setDraggingFlag(bool flag)
{
    m_draggingFlag = flag;
}

#include <KoDocumentSectionPropertyAction_p.moc>
#include <KoDocumentSectionView.moc>

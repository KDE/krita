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
#include "NodeView.h"
#include "NodePropertyAction_p.h"
#include "NodeDelegate.h"
#include "NodeViewVisibilityDelegate.h"
#include "kis_node_model.h"
#include "kis_signals_blocker.h"


#include <kconfig.h>
#include <kconfiggroup.h>
#include <kis_icon.h>
#include <ksharedconfig.h>
#include <KisKineticScroller.h>

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
#include <QScroller>

#include "kis_node_view_color_scheme.h"


#ifdef HAVE_X11
#define DRAG_WHILE_DRAG_WORKAROUND
#endif

#ifdef DRAG_WHILE_DRAG_WORKAROUND
#define DRAG_WHILE_DRAG_WORKAROUND_START() d->isDragging = true
#define DRAG_WHILE_DRAG_WORKAROUND_STOP() d->isDragging = false
#else
#define DRAG_WHILE_DRAG_WORKAROUND_START()
#define DRAG_WHILE_DRAG_WORKAROUND_STOP()
#endif


class Q_DECL_HIDDEN NodeView::Private
{
public:
    Private(NodeView* _q)
        : delegate(_q, _q)
        , mode(DetailedMode)
#ifdef DRAG_WHILE_DRAG_WORKAROUND
        , isDragging(false)
#endif
    {
        KSharedConfigPtr config =  KSharedConfig::openConfig();
        KConfigGroup group = config->group("NodeView");
        mode = (DisplayMode) group.readEntry("NodeViewMode", (int)MinimalMode);
    }
    NodeDelegate delegate;
    DisplayMode mode;
    QPersistentModelIndex hovered;
    QPoint lastPos;

#ifdef DRAG_WHILE_DRAG_WORKAROUND
    bool isDragging;
#endif
};


NodeView::NodeView(QWidget *parent)
    : QTreeView(parent)
    , m_draggingFlag(false)
    , d(new Private(this))
{
    setItemDelegateForColumn(0, &d->delegate);

    setMouseTracking(true);
    setSelectionBehavior(SelectRows);
    setDefaultDropAction(Qt::MoveAction);
    setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    header()->hide();
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setAcceptDrops(true);
    setDropIndicatorShown(true);

    {
        QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(this);
        if (scroller) {
            connect(scroller, SIGNAL(stateChanged(QScroller::State)),
                    this, SLOT(slotScrollerStateChanged(QScroller::State)));
        }
    }
}

NodeView::~NodeView()
{
    delete d;
}

void NodeView::setDisplayMode(DisplayMode mode)
{
    if (d->mode != mode) {
        d->mode = mode;
        KSharedConfigPtr config =  KSharedConfig::openConfig();
        KConfigGroup group = config->group("NodeView");
        group.writeEntry("NodeViewMode", (int)mode);
        scheduleDelayedItemsLayout();
    }
}

NodeView::DisplayMode NodeView::displayMode() const
{
    return d->mode;
}

void NodeView::addPropertyActions(QMenu *menu, const QModelIndex &index)
{
    KisBaseNode::PropertyList list = index.data(KisNodeModel::PropertiesRole).value<KisBaseNode::PropertyList>();
    for (int i = 0, n = list.count(); i < n; ++i) {
        if (list.at(i).isMutable) {
            PropertyAction *a = new PropertyAction(i, list.at(i), index, menu);
            connect(a, SIGNAL(toggled(bool,QPersistentModelIndex,int)),
                    this, SLOT(slotActionToggled(bool,QPersistentModelIndex,int)));
            menu->addAction(a);
        }
    }
}

void NodeView::updateNode(const QModelIndex &index)
{
    dataChanged(index, index);
}

QItemSelectionModel::SelectionFlags NodeView::selectionCommand(const QModelIndex &index,
                                                                  const QEvent *event) const
{
    /**
     * Qt has a bug: when we Ctrl+click on an item, the item's
     * selections gets toggled on mouse *press*, whereas usually it is
     * done on mouse *release*.  Therefore the user cannot do a
     * Ctrl+D&D with the default configuration. This code fixes the
     * problem by manually returning QItemSelectionModel::NoUpdate
     * flag when the user clicks on an item and returning
     * QItemSelectionModel::Toggle on release.
     */

    if (event &&
        (event->type() == QEvent::MouseButtonPress ||
         event->type() == QEvent::MouseButtonRelease) &&
        index.isValid()) {

        const QMouseEvent *mevent = static_cast<const QMouseEvent*>(event);

        if (mevent->button() == Qt::RightButton &&
            selectionModel()->selectedIndexes().contains(index)) {

            // Allow calling context menu for multiple layers
            return QItemSelectionModel::NoUpdate;
        }

        if (event->type() == QEvent::MouseButtonPress &&
            (mevent->modifiers() & Qt::ControlModifier)) {

            return QItemSelectionModel::NoUpdate;
        }

        if (event->type() == QEvent::MouseButtonRelease &&
            (mevent->modifiers() & Qt::ControlModifier)) {

            return QItemSelectionModel::Toggle;
        }
    }

    /**
     * Qt 5.6 has a bug: it reads global modifiers, not the ones
     * passed from event.  So if you paste an item using Ctrl+V it'll
     * select multiple layers for you
     */
    Qt::KeyboardModifiers globalModifiers = QApplication::keyboardModifiers();
    if (!event && globalModifiers != Qt::NoModifier) {
        return QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows;
    }

    return QAbstractItemView::selectionCommand(index, event);
}

QRect NodeView::visualRect(const QModelIndex &index) const
{
    QRect rc = QTreeView::visualRect(index);
    if (layoutDirection() == Qt::RightToLeft)
        rc.setRight(width());
    else
        rc.setLeft(0);
    return rc;
}

QRect NodeView::originalVisualRect(const QModelIndex &index) const
{
    return QTreeView::visualRect(index);
}

QModelIndex NodeView::indexAt(const QPoint &point) const
{
    KisNodeViewColorScheme scm;

    QModelIndex index = QTreeView::indexAt(point);
    if (!index.isValid()) {
        // Middle is a good position for both LTR and RTL layouts
        // First reset x, then get the x in the middle
        index = QTreeView::indexAt(point - QPoint(point.x(), 0) + QPoint(width() / 2, 0));
    }

    return index;
}

bool NodeView::viewportEvent(QEvent *e)
{
    if (model()) {
        switch(e->type()) {
        case QEvent::MouseButtonPress: {
            DRAG_WHILE_DRAG_WORKAROUND_STOP();

            const QPoint pos = static_cast<QMouseEvent*>(e)->pos();
            d->lastPos = pos;

            if (!indexAt(pos).isValid()) {
                return QTreeView::viewportEvent(e);
            }
            QModelIndex index = model()->buddy(indexAt(pos));
            if (d->delegate.editorEvent(e, model(), optionForIndex(index), index)) {
                return true;
            }
        } break;
        case QEvent::Leave: {
            QEvent e(QEvent::Leave);
            d->delegate.editorEvent(&e, model(), optionForIndex(d->hovered), d->hovered);
            d->hovered = QModelIndex();
        } break;
        case QEvent::MouseMove: {
#ifdef DRAG_WHILE_DRAG_WORKAROUND
            if (d->isDragging) {
                return false;
            }
#endif

            const QPoint pos = static_cast<QMouseEvent*>(e)->pos();
            QModelIndex hovered = indexAt(pos);
            if (hovered != d->hovered) {
                if (d->hovered.isValid()) {
                    QEvent e(QEvent::Leave);
                    d->delegate.editorEvent(&e, model(), optionForIndex(d->hovered), d->hovered);
                }
                if (hovered.isValid()) {
                    QEvent e(QEvent::Enter);
                    d->delegate.editorEvent(&e, model(), optionForIndex(hovered), hovered);
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
            return d->delegate.editorEvent(e, model(), optionForIndex(index), index);
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

void NodeView::contextMenuEvent(QContextMenuEvent *e)
{
    QTreeView::contextMenuEvent(e);
    QModelIndex i = indexAt(e->pos());
    if (model())
        i = model()->buddy(i);
    showContextMenu(e->globalPos(), i);
}

void NodeView::showContextMenu(const QPoint &globalPos, const QModelIndex &index)
{
    emit contextMenuRequested(globalPos, index);
}

void NodeView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTreeView::currentChanged(current, previous);
    if (current != previous) {
        Q_ASSERT(!current.isValid() || current.model() == model());
        model()->setData(current, true, KisNodeModel::ActiveRole);
    }
}

void NodeView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &/*roles*/)
{
    QTreeView::dataChanged(topLeft, bottomRight);
    for (int x = topLeft.row(); x <= bottomRight.row(); ++x) {
        for (int y = topLeft.column(); y <= bottomRight.column(); ++y) {
            QModelIndex index = topLeft.sibling(x, y);
            if (index.data(KisNodeModel::ActiveRole).toBool()) {
                if (currentIndex() != index) {
                    setCurrentIndex(index);
                }
                return;
            }
        }
    }
}

void NodeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QTreeView::selectionChanged(selected, deselected);
    emit selectionChanged(selectedIndexes());
}

void NodeView::slotActionToggled(bool on, const QPersistentModelIndex &index, int num)
{
    KisBaseNode::PropertyList list = index.data(KisNodeModel::PropertiesRole).value<KisBaseNode::PropertyList>();
    list[num].state = on;
    const_cast<QAbstractItemModel*>(index.model())->setData(index, QVariant::fromValue(list), KisNodeModel::PropertiesRole);
}

QStyleOptionViewItem NodeView::optionForIndex(const QModelIndex &index) const
{
    QStyleOptionViewItem option = viewOptions();
    option.rect = visualRect(index);
    if (index == currentIndex())
        option.state |= QStyle::State_HasFocus;
    return option;
}

void NodeView::startDrag(Qt::DropActions supportedActions)
{
    DRAG_WHILE_DRAG_WORKAROUND_START();

    if (displayMode() == NodeView::ThumbnailMode) {
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

QPixmap NodeView::createDragPixmap() const
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
        size = KisIconUtils::SizeLarge;
    }
    else if (itemCount > 4) {
        xCount = 3;
        size = KisIconUtils::SizeHuge;
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
    Q_FOREACH (const QModelIndex &selectedIndex, selectedIndexes) {
        const QImage img = selectedIndex.data(int(KisNodeModel::BeginThumbnailRole) + size).value<QImage>();
        painter.drawPixmap(x, y, QPixmap().fromImage(img.scaled(QSize(size, size), Qt::KeepAspectRatio, Qt::SmoothTransformation)));

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

void NodeView::resizeEvent(QResizeEvent * event)
{
    KisNodeViewColorScheme scm;
    header()->setStretchLastSection(false);
    header()->setOffset(-scm.visibilityColumnWidth());
    header()->resizeSection(0, event->size().width() - scm.visibilityColumnWidth());
    setIndentation(scm.indentation());
    QTreeView::resizeEvent(event);
}

void NodeView::paintEvent(QPaintEvent *event)
{
    event->accept();
    QTreeView::paintEvent(event);

    // Paint the line where the slide should go
    if (isDragging() && (displayMode() == NodeView::ThumbnailMode)) {
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

void NodeView::drawBranches(QPainter *painter, const QRect &rect,
                               const QModelIndex &index) const
{
    Q_UNUSED(painter);
    Q_UNUSED(rect);
    Q_UNUSED(index);

    /**
     * Noop... Everything is going to be painted by NodeDelegate.
     * So this override basically disables painting of Qt's branch-lines.
     */
}

void NodeView::dropEvent(QDropEvent *ev)
{
    if (displayMode() == NodeView::ThumbnailMode) {
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

    DRAG_WHILE_DRAG_WORKAROUND_STOP();
}

int NodeView::cursorPageIndex() const
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

void NodeView::dragEnterEvent(QDragEnterEvent *ev)
{
    DRAG_WHILE_DRAG_WORKAROUND_START();

    QVariant data = qVariantFromValue(
        static_cast<void*>(const_cast<QMimeData*>(ev->mimeData())));
    model()->setData(QModelIndex(), data, KisNodeModel::DropEnabled);

    QTreeView::dragEnterEvent(ev);
}

void NodeView::dragMoveEvent(QDragMoveEvent *ev)
{
    DRAG_WHILE_DRAG_WORKAROUND_START();

    if (displayMode() == NodeView::ThumbnailMode) {
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

void NodeView::dragLeaveEvent(QDragLeaveEvent *e)
{
    if (displayMode() == NodeView::ThumbnailMode) {
        setDraggingFlag(false);
    } else {
        QTreeView::dragLeaveEvent(e);
    }

    DRAG_WHILE_DRAG_WORKAROUND_STOP();
}

bool NodeView::isDragging() const
{
    return m_draggingFlag;
}

void NodeView::setDraggingFlag(bool flag)
{
    m_draggingFlag = flag;
}

void NodeView::slotUpdateIcons()
{
    d->delegate.slotUpdateIcon();
}

void NodeView::slotScrollerStateChanged(QScroller::State state){
    KisKineticScroller::updateCursor(this, state);
}

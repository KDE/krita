/*
  SPDX-FileCopyrightText: 2020 Saurabh Kumar <saurabhk660@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QDebug>
#include <QPainter>
#include <QPaintEvent>
#include <QMenu>
#include <QProxyStyle>

#include "StoryboardView.h"
#include "StoryboardModel.h"
#include "KisAddRemoveStoryboardCommand.h"

class StoryboardStyle : public QProxyStyle
{
public:
    StoryboardStyle(QStyle *baseStyle = 0) : QProxyStyle(baseStyle) {}

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                       QPainter *painter, const QWidget *widget) const
    {
        if (element == QStyle::PE_IndicatorItemViewItemDrop)
        {
            QColor color(widget->palette().color(QPalette::Highlight).lighter());
            if (option->rect.width() == 0 && option->rect.height() == 0){
                return;
            }
            else if (option->rect.width() == 0) {
                QBrush brush(color);

                QRect r(option->rect);
                r.setLeft(r.left() - 4);
                r.setRight(r.right() + 4);

                painter->fillRect(r, brush);
            }
            else if (option->rect.height() == 0) {
                QBrush brush(color);

                QRect r(option->rect);
                r.setTop(r.top() - 4);
                r.setBottom(r.bottom() + 4);

                painter->fillRect(r, brush);
            }
        }
        else
        {
            QProxyStyle::drawPrimitive(element, option, painter, widget);
        }
    }
};

/**
 * This view draws the children of every index in the first column of 
 * the model inside the parent
 *
 * */

StoryboardView::StoryboardView(QWidget *parent)
    : QListView(parent)
    , m_commentIsVisible(true)
    , m_thumbnailIsVisible(true)
{
    setSelectionBehavior(SelectRows);
    setDefaultDropAction(Qt::MoveAction);
    setResizeMode(QListView::Adjust);
    setUniformItemSizes(true);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    QWidget::setMouseTracking(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setDragEnabled(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setStyle(new StoryboardStyle(this->style()));

    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
                this, SLOT(slotContextMenuRequested(const QPoint &)));

    connect(this, &StoryboardView::clicked,
            this, &StoryboardView::slotItemClicked);
}

StoryboardView::~StoryboardView()
{}

void StoryboardView::paintEvent(QPaintEvent *event)
{
    event->accept();
    QListView::paintEvent(event);

    //ask delegate to draw the child nodes too
    QPainter painter(viewport());
    int itemNum = model()->rowCount();
    for (int row = 0; row < itemNum; row++) {
        QModelIndex index = model()->index(row, 0);
        int childNum = model()->rowCount(index);
        for (int childRow = 0; childRow < childNum; childRow++) {

            QModelIndex childIndex = model()->index(childRow, 0, index);

            QStyleOptionViewItem option;
            if (selectionModel()->isSelected(childIndex)) {
                option.state |= QStyle::State_Selected;
            }
            if (childIndex == selectionModel()->currentIndex()) {
                option.state |= QStyle::State_HasFocus;
            }
            option.font = font();
            option.fontMetrics = fontMetrics();
            option.rect = visualRect(childIndex);
            itemDelegate()->paint(&painter, option, childIndex);
        }
    }
}

QRect StoryboardView::visualRect(const QModelIndex &index) const
{
    /*
     *    fw = fontWidth
     * 
     *  (3*fw+2),        (5*fw+10)  _____ (4*fw+10)
     *    |                |       /
     *    |                |      /
     *   ,_________________________,
     *   |__|_____________|____|___|  ---------->(fontHeight)
     *   |                         |
     *   |                         |
     *   |                         |
     *   |                         |
     *   |_________________________|
     */

    if (!index.isValid() || !index.parent().isValid()) {
        return QListView::visualRect(index);
    }
    else {
        QRect parentRect = visualRect(index.parent());
        parentRect.setTopLeft(parentRect.topLeft() + QPoint(5, 5));
        parentRect.setBottomRight(parentRect.bottomRight() - QPoint(5, 5));
        int fontHeight = fontMetrics().height() + 3;
        int numericFontWidth = fontMetrics().width("0");
        int parentWidth = parentRect.width();
        int childRow = index.row();

        int thumbnailWidth = parentWidth;
        if (m_itemOrientation == Qt::Horizontal) {
            thumbnailWidth = 250;
        }
        switch (childRow)
        {
            case StoryboardItem::FrameNumber:
            {   
                //the frame thumbnail rect
                if (!thumbnailIsVisible()) {
                    parentRect.setSize(QSize(3*numericFontWidth + 2, fontHeight));
                    return parentRect;
                }

                parentRect.setSize(QSize(thumbnailWidth, 120));
                parentRect.translate(0, fontHeight);
                return parentRect;
            }
            case StoryboardItem::ItemName:
            {
                QRect itemNameRect = parentRect;
                itemNameRect.setSize(QSize(thumbnailWidth - (12 * numericFontWidth + 22), fontHeight));
                itemNameRect.moveLeft(parentRect.left() + 3*numericFontWidth + 2);
                return itemNameRect;
            }
            case StoryboardItem::DurationSecond:
            {
                QRect secondRect = parentRect;
                secondRect.setSize(QSize(5 * numericFontWidth + 10, fontHeight));
                secondRect.moveLeft(parentRect.left() + thumbnailWidth - 9*numericFontWidth  -20);
                return secondRect;
            }
            case StoryboardItem::DurationFrame:
            {
                QRect frameRect = parentRect;
                frameRect.setSize(QSize(4 * numericFontWidth + 10, fontHeight));
                frameRect.moveLeft(parentRect.left() + thumbnailWidth - 4*numericFontWidth  - 10);
                return frameRect;
            }
            default:
            {
                //comment rect
                if (!commentIsVisible()) {
                    return QRect();
                }

                int thumbnailheight = thumbnailIsVisible() ? 120 : 0;
                if (m_itemOrientation == Qt::Vertical) {
                    const StoryboardModel* Model = dynamic_cast<const StoryboardModel*>(model());
                    parentRect.setTop(parentRect.top() + thumbnailheight + fontHeight + Model->visibleCommentsUpto(index) * 100);
                    parentRect.setHeight(100);
                    return parentRect;
                }
                else {
                    const StoryboardModel* Model = dynamic_cast<const StoryboardModel*>(model());
                    int numVisibleComments = Model->visibleCommentCount();
                    int commentWidth = 200;
                    if (numVisibleComments) {
                        commentWidth = qMax(200, (viewport()->width() - 250) / numVisibleComments);
                    }
                    parentRect.setSize(QSize(commentWidth, thumbnailheight + fontHeight));
                    parentRect.moveLeft(parentRect.left() + thumbnailWidth + Model->visibleCommentsUpto(index) * commentWidth);
                    return parentRect;
                }
            }
        }
    }
    return QRect();
}

QModelIndex StoryboardView::indexAt(const QPoint &point) const
{
    QModelIndex index = QListView::indexAt(point);
    if (index.isValid()) {
        //look for the index in children of the current index
        int numChild = model()->rowCount(index);
        for (int row = 0; row < numChild; row++) {
            QRect childRect = visualRect(model()->index(row, 0, index));
            if (childRect.contains(point)) {
                return model()->index(row, 0, index);
            }
        }
    }
    return index;
}

void StoryboardView::setItemOrientation(Qt::Orientation orientation)
{
    m_itemOrientation = orientation;
}

Qt::Orientation StoryboardView::itemOrientation()
{
    return m_itemOrientation;
}

bool StoryboardView::commentIsVisible() const
{
    return m_commentIsVisible;
}

bool StoryboardView::thumbnailIsVisible() const
{
    return m_thumbnailIsVisible;
}

void StoryboardView::setCommentVisibility(bool value)
{
    m_commentIsVisible = value;
}

void StoryboardView::setThumbnailVisibility(bool value)
{
    m_thumbnailIsVisible = value;
}

void StoryboardView::slotContextMenuRequested(const QPoint &point)
{
    StoryboardModel* Model = dynamic_cast<StoryboardModel*>(model());
    QMenu contextMenu;
    QModelIndex index = indexAt(point);
    if (!index.isValid()) {
        contextMenu.addAction(i18nc("Add new scene as the last storyboard", "Add Scene"), [this, index, Model] {Model->insertItem(index, false); });
    }
    else if (index.parent().isValid()) {
        index = index.parent();
    }

    if (index.isValid()) {
        contextMenu.addAction(i18nc("Add scene after active scene", "Add Scene After"), [this, index, Model] {Model->insertItem(index, true); });
        if (index.row() > 0) {
            contextMenu.addAction(i18nc("Add scene before active scene", "Add Scene Before"), [this, index, Model] {Model->insertItem(index, false); });
        }
        contextMenu.addAction(i18nc("Remove current scene from storyboards", "Remove Scene"), [this, index, Model] {
            int row = index.row();
            KisRemoveStoryboardCommand *command = new KisRemoveStoryboardCommand(row, Model->getData().at(row), Model);
            Model->removeItem(index, command);
            Model->pushUndoCommand(command);
        });
    }
    contextMenu.exec(viewport()->mapToGlobal(point));
}

void StoryboardView::slotItemClicked(const QModelIndex &clicked)
{
    StoryboardModel* sbModel = dynamic_cast<StoryboardModel*>(model());

    if(sbModel) {
        sbModel->visualizeScene(clicked.parent().isValid() ? clicked.parent() : clicked);
    }
}

void StoryboardView::setCurrentItem(int frame)
{
    const StoryboardModel* Model = dynamic_cast<const StoryboardModel*>(model());
    QModelIndex index = Model->indexFromFrame(frame);
    if (index.isValid()) {
        selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
        selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
        scrollTo(index);
    }
}

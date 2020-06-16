/*
  Copyright (c) 2020 Saurabh Kumar <saurabhk660@gmail.com>

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

#include <QDebug>
#include <QPainter>
#include <QPaintEvent>

#include "storyboardView.h"
#include "storyboardModel.h"

/**
 * This view draws the children of every index in the first column of 
 * the model inside the parent
 *
 * */

StoryboardView::StoryboardView(QWidget *parent)
    :QListView(parent)
{

    setWrapping(true);
    setFlow(QListView::LeftToRight);
    setResizeMode(QListView::Adjust);
    setUniformItemSizes(true);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    QWidget::setMouseTracking(true);

    //make drag and drop work as expected
/*
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    */
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
    for (int row = 0; row < itemNum; row++){
        QModelIndex index = model()->index(row, 0);
        int childNum = model()->rowCount(index);
        for (int childRow = 0; childRow < childNum; childRow++){

            QModelIndex childIndex = model()->index(childRow, 0, index);

            QStyleOptionViewItem option;
            //TO DO: check if the childIndex is in focus
            //TO DO: set proper options for spinbox type indices
            if (selectionModel()->isSelected(childIndex)) {
                option.state |= QStyle::State_Selected;
            }
            if (childIndex == selectionModel()->currentIndex()) {
                option.state |= QStyle::State_HasFocus;
            }
            if (childIndex == m_hoverIndex){
                option.state |= QStyle::State_MouseOver;
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
    if (!index.isValid() || !index.parent().isValid()) {
        return QListView::visualRect(index);
    }
    else {
        QRect parentRect = visualRect(index.parent());
        parentRect.setTopLeft(parentRect.topLeft() + QPoint(5, 5));
        parentRect.setBottomRight(parentRect.bottomRight() - QPoint(5, 5));
        int fontHeight = fontMetrics().height() + 3;
        int numericFontWidth = fontMetrics().width("0");
        int height = parentRect.height();
        int width = parentRect.width();
        int childRow = index.row();
        switch (childRow)
        {
            case 0:
            {   
                //the frame thumbnail rect
                parentRect.setSize(QSize(width, 120));
                parentRect.translate(0, fontHeight);
                return parentRect;
            }
            case 1:
            {
                QRect itemNameRect = parentRect;
                itemNameRect.setSize(QSize(width - (10 * numericFontWidth + 22), fontHeight));
                itemNameRect.moveLeft(parentRect.left() + 3*numericFontWidth + 2);
                return itemNameRect;
            }
            case 2:
            {
                QRect secondRect = parentRect;
                secondRect.setSize(QSize(4 * numericFontWidth + 10, fontHeight));
                secondRect.moveRight(parentRect.right() - 3*numericFontWidth -10);
                return secondRect;
            }
            case 3:
            {
                QRect frameRect = parentRect;
                frameRect.setSize(QSize(3 * numericFontWidth + 10, fontHeight));
                frameRect.moveRight(parentRect.right());
                return frameRect;
            }
            default:
            {
                //comment rect
                const StoryboardModel* Model = dynamic_cast<const StoryboardModel*>(model());
                parentRect.setTop(parentRect.top() + 120 + fontHeight + Model->visibleCommentsUpto(index) * 100);
                parentRect.setHeight(100);
                return parentRect;
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

void StoryboardView::mouseMoveEvent(QMouseEvent *event)
{
    event->accept();
    QListView::mouseMoveEvent(event);

    m_hoverIndex = indexAt(event->pos());
}

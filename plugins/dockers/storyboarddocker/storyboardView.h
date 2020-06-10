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

#ifndef STORYBOARD_VIEW_H
#define STORYBOARD_VIEW_H

#include <QListView>
#include <QScroller>

class QStyleOptionViewItem;
class StoryboardModel;

/**
 * This view draws the children of every index in the first column of 
 * the model inside the parent index
 *
 * */

class StoryboardView: public QListView
{
    Q_OBJECT
public:
    explicit StoryboardView(QWidget *parent = 0);
    ~StoryboardView() override;

    void paintEvent(QPaintEvent *event) override;
    QRect visualRect(const QModelIndex &index) const override;
    QModelIndex indexAt(const QPoint &point) const override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QModelIndex m_hoverIndex;
};

#endif

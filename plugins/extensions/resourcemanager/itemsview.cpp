/*
 *  Copyright (C) 2010 Frederik Gladhorn <gladhorn@kde.org>
 *  Copyright (c) 2017 Aniketh Girish anikethgireesh@gmail.com
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

#include "itemsview_p.h"

#include <QScrollBar>

ItemsView::ItemsView(QWidget *parent)
    :QListView(parent)
{
}

void ItemsView::wheelEvent(QWheelEvent *event)
{
    // this is a workaround because scrolling by mouse wheel is broken in Qt list views for big items
    verticalScrollBar()->setSingleStep(10);
    QListView::wheelEvent(event);
}

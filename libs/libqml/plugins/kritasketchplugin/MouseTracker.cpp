/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "MouseTracker.h"

#include <QQuickItem>
#include <QEvent>
#include <QCoreApplication>
#include <QTouchEvent>

struct TrackedItem {
    TrackedItem(QQuickItem* i, const QPointF& o) : item(i), offset(o) { }

    QQuickItem* item;
    QPointF offset;
};

class MouseTracker::Private
{
public:
    Private()
    { }

    QList<TrackedItem> trackedItems;
};

MouseTracker::MouseTracker(QObject* parent)
    : QObject(parent), d(new Private)
{
    QCoreApplication::instance()->installEventFilter(this);
}

MouseTracker::~MouseTracker()
{
    delete d;
}

void MouseTracker::addItem(QQuickItem* item, const QPointF& offset)
{
    d->trackedItems.append(TrackedItem(item, offset));
}

void MouseTracker::removeItem(QQuickItem* item)
{
    for(int i = 0; i < d->trackedItems.length(); ++i) {
        if(d->trackedItems.at(i).item == item) {
            d->trackedItems.removeAt(i);
            break;
        }
    }
}

bool MouseTracker::eventFilter(QObject* target, QEvent* event)
{
    Q_UNUSED(target)
    if (d->trackedItems.count() > 0) {
        switch(event->type()) {
    // QT5TODO
//             case QEvent::GraphicsSceneMouseMove: {
//                 QGraphicsSceneMouseEvent* mevent = static_cast<QGraphicsSceneMouseEvent*>(event);
//                 Q_FOREACH(const TrackedItem& item, d->trackedItems) {
//                     item.item->setPos(mevent->scenePos() + item.offset);
//                 }
//                 return false;
//             }
//             case QEvent::TouchUpdate: {
//                 QTouchEvent* tevent = static_cast<QTouchEvent*>(event);
//                 QTouchEvent::TouchPoint primary = tevent->touchPoints().at(0);
//                 Q_FOREACH(const TrackedItem& item, d->trackedItems) {
//                     item.item->setPos(primary.scenePos() + item.offset);
//                 }
//                 return false;
//             }
//             case QEvent::DragMove: {
//                 QDragMoveEvent* mevent = static_cast<QDragMoveEvent*>(event);
//                 Q_FOREACH(const TrackedItem& item, d->trackedItems) {
//                     item.item->setPos(mevent->pos() + item.offset);
//                 }
//                 return false;
//             }
            default: ;
        }
    }
    return false;
}

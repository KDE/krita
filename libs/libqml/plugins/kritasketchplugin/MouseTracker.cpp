/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
    Q_UNUSED(target);
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

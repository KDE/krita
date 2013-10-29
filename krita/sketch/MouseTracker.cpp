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

#include <QDeclarativeItem>
#include <QEvent>
#include <QCoreApplication>
#include <QGraphicsSceneMouseEvent>
#include <QTouchEvent>

MouseTracker::MouseTracker(QObject* parent)
    : QObject(parent)
{
    QCoreApplication::instance()->installEventFilter(this);
}

MouseTracker::~MouseTracker()
{

}

void MouseTracker::addItem(QDeclarativeItem* item)
{
    m_trackedItems.append(item);
}

void MouseTracker::removeItem(QDeclarativeItem* item)
{
    m_trackedItems.removeOne(item);
}

bool MouseTracker::eventFilter(QObject* target, QEvent* event)
{
    Q_UNUSED(target)
    if (m_trackedItems.count() > 0) {
        switch(event->type()) {
            case QEvent::GraphicsSceneMouseMove: {
                QGraphicsSceneMouseEvent* mevent = static_cast<QGraphicsSceneMouseEvent*>(event);
                foreach(QDeclarativeItem* item, m_trackedItems) {
                    item->setPos(mevent->scenePos());
                }
                return false;
            }
            case QEvent::TouchUpdate: {
                QTouchEvent* tevent = static_cast<QTouchEvent*>(event);
                QTouchEvent::TouchPoint primary;
                foreach(const QTouchEvent::TouchPoint& point, tevent->touchPoints()) {
                    if (point.isPrimary()) {
                        primary = point;
                        break;
                    }
                }
                foreach(QDeclarativeItem* item, m_trackedItems) {
                    item->setPos(primary.scenePos());
                }
                return false;
            }
            case QEvent::DragMove: {
                QDragMoveEvent* mevent = static_cast<QDragMoveEvent*>(event);
                foreach(QDeclarativeItem* item, m_trackedItems) {
                    item->setPos(mevent->pos());
                }
                return false;
            }
            default: ;
        }
    }
    return false;
}

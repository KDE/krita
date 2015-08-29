/* This file is part of the KDE project
 * Copyright (C) 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

#include "SimpleTouchArea.h"
#include <input/kis_tablet_event.h>
#include <QTouchEvent>
#include <QApplication>

SimpleTouchArea::SimpleTouchArea(QDeclarativeItem* parent)
    : QDeclarativeItem(parent)
{
    setAcceptTouchEvents(true);
}

SimpleTouchArea::~SimpleTouchArea()
{
}

bool SimpleTouchArea::event(QEvent* event)
{
    switch(static_cast<int>(event->type())) {
        case KisTabletEvent::TabletPressEx:
        case KisTabletEvent::TabletReleaseEx:
        case KisTabletEvent::TabletMoveEx:
            event->ignore();
            return true;
        default:
            break;
    }
    return QDeclarativeItem::event(event);
}

bool SimpleTouchArea::sceneEvent(QEvent* event)
{
    switch (event->type())
    {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    case QEvent::TabletPress:
    case QEvent::GraphicsSceneMousePress:
        event->accept();
        return true;
    default:
        break;
    }

    return QDeclarativeItem::sceneEvent(event);
}


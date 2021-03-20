/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "SimpleTouchArea.h"
#include <QEvent>


SimpleTouchArea::SimpleTouchArea(QQuickItem* parent)
    : QQuickItem(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
}

SimpleTouchArea::~SimpleTouchArea()
{
}

bool SimpleTouchArea::event(QEvent* event)
{
    switch(static_cast<int>(event->type())) {
        default:
            break;
    }
    return QQuickItem::event(event);
}

void SimpleTouchArea::touchEvent(QTouchEvent* event)
{
    switch (event->type())
    {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    case QEvent::TabletPress:
        event->accept();
        return;
    default:
        break;
    }

    QQuickItem::touchEvent(event);
}

/* This file is part of the KDE project

   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006 Casper Boemann Rasmussen <cbr@boemann.dk>
   Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
   
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
 * Boston, MA 02110-1301, USA.
*/

#include "KoPointerEvent.h"
#include "KoInputDeviceHandlerEvent.h"
#include <QTabletEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QGraphicsSceneMouseEvent>

class KoPointerEvent::Private
{
public:
    Private()
            : tabletEvent(0), mouseEvent(0), wheelEvent(0), gsMouseEvent(0)
            , gsWheelEvent(0), deviceEvent(0), tabletButton(Qt::NoButton)
            , globalPos(0, 0), pos(0, 0), posZ(0), rotationX(0), rotationY(0)
            , rotationZ(0) {}
    QTabletEvent * tabletEvent;
    QMouseEvent * mouseEvent;
    QWheelEvent * wheelEvent;
    QGraphicsSceneMouseEvent * gsMouseEvent;
    QGraphicsSceneWheelEvent * gsWheelEvent;
    KoInputDeviceHandlerEvent * deviceEvent;
    Qt::MouseButton tabletButton;
    QPoint globalPos, pos;
    int posZ;
    int rotationX, rotationY, rotationZ;
};

KoPointerEvent::KoPointerEvent(QMouseEvent *ev, const QPointF &pnt)
        : point(pnt),
        m_event(ev),
        d(new Private())
{
    Q_ASSERT(m_event);
    d->mouseEvent = ev;
}

KoPointerEvent::KoPointerEvent(QGraphicsSceneMouseEvent *ev, const QPointF &pnt)
        : point(pnt),
        m_event(ev),
        d(new Private())
{
    Q_ASSERT(m_event);
    d->gsMouseEvent = ev;
}

KoPointerEvent::KoPointerEvent(QGraphicsSceneWheelEvent *ev, const QPointF &pnt)
        : point(pnt),
        m_event(ev),
        d(new Private())
{
    Q_ASSERT(m_event);
    d->gsWheelEvent = ev;
}

KoPointerEvent::KoPointerEvent(QTabletEvent *ev, const QPointF &pnt)
        : point(pnt),
        m_event(ev),
        d(new Private())
{
    Q_ASSERT(m_event);
    d->tabletEvent = ev;
}

KoPointerEvent::KoPointerEvent(QWheelEvent *ev, const QPointF &pnt)
        : point(pnt),
        m_event(ev),
        d(new Private())
{
    Q_ASSERT(m_event);
    d->wheelEvent = ev;
}

KoPointerEvent::KoPointerEvent(KoInputDeviceHandlerEvent * ev, int x, int y, int z, int rx, int ry, int rz)
        : m_event(ev)
        , d(new Private())
{
    Q_ASSERT(m_event);
    d->deviceEvent = ev;
    d->pos = QPoint(x, y);
    d->posZ = z;
    d->rotationX = rx;
    d->rotationY = ry;
    d->rotationZ = rz;
}

KoPointerEvent::KoPointerEvent(KoPointerEvent *event, const QPointF &point)
    : point(point)
    , m_event(event->m_event)
    , d(new Private(*(event->d)))
{
    Q_ASSERT(m_event);
}

KoPointerEvent::~KoPointerEvent()
{
    delete d;
}

Qt::MouseButton KoPointerEvent::button() const
{
    if (d->mouseEvent)
        return d->mouseEvent->button();
    else if (d->tabletEvent)
        return d->tabletButton;
    else if (d->deviceEvent)
        return d->deviceEvent->button();
    else if (d->gsMouseEvent)
        return d->gsMouseEvent->button();
    else
        return Qt::NoButton;
}

Qt::MouseButtons KoPointerEvent::buttons() const
{
    if (d->mouseEvent)
        return d->mouseEvent->buttons();
    else if (d->wheelEvent)
        return d->wheelEvent->buttons();
    else if (d->tabletEvent)
        return d->tabletButton;
    else if (d->deviceEvent)
        return d->deviceEvent->buttons();
    else if (d->gsMouseEvent)
        return d->gsMouseEvent->buttons();
    else if (d->gsWheelEvent)
        return d->gsWheelEvent->buttons();
    return Qt::NoButton;
}

QPoint KoPointerEvent::globalPos() const
{
    if (d->mouseEvent)
        return d->mouseEvent->globalPos();
    else if (d->wheelEvent)
        return d->wheelEvent->globalPos();
    else if (d->tabletEvent)
        return d->tabletEvent->globalPos();
    else if (d->gsMouseEvent)
        return d->gsMouseEvent->screenPos();
    else if (d->gsWheelEvent)
        return d->gsWheelEvent->screenPos();
    else
        return d->globalPos;
}

QPoint KoPointerEvent::pos() const
{
    if (d->mouseEvent)
        return d->mouseEvent->pos();
    else if (d->wheelEvent)
        return d->wheelEvent->pos();
    else if (d->tabletEvent)
        return d->tabletEvent->pos();
    else if (d->gsMouseEvent)
        return d->gsMouseEvent->pos().toPoint();
    else if (d->gsWheelEvent)
        return d->gsWheelEvent->pos().toPoint();
    else
        return d->pos;
}

qreal KoPointerEvent::pressure() const
{
    if (d->tabletEvent)
        return d->tabletEvent->pressure();
    else
        return 0.5;
}

qreal KoPointerEvent::rotation() const
{
    if (d->tabletEvent)
        return d->tabletEvent->rotation();
    else
        return 0.0;
}

qreal KoPointerEvent::tangentialPressure() const
{
    if (d->tabletEvent)
        return d->tabletEvent->tangentialPressure();
    else
        return 0.0;
}

int KoPointerEvent::x() const
{
    if (d->tabletEvent)
        return d->tabletEvent->x();
    if (d->wheelEvent)
        return d->wheelEvent->x();
    else if (d->mouseEvent)
        return d->mouseEvent->x();
    else
        return pos().x();
}

int KoPointerEvent::xTilt() const
{
    if (d->tabletEvent)
        return d->tabletEvent->xTilt();
    else
        return 0;
}

int KoPointerEvent::y() const
{
    if (d->tabletEvent)
        return d->tabletEvent->y();
    if (d->wheelEvent)
        return d->wheelEvent->y();
    else if (d->mouseEvent)
        return d->mouseEvent->y();
    else
        return pos().y();
}

int KoPointerEvent::yTilt() const
{
    if (d->tabletEvent)
        return d->tabletEvent->yTilt();
    else
        return 0;
}

int KoPointerEvent::z() const
{
    if (d->tabletEvent)
        return d->tabletEvent->z();
    else if (d->deviceEvent)
        return d->posZ;
    else
        return 0;
}

int KoPointerEvent::delta() const
{
    if (d->wheelEvent)
        return d->wheelEvent->delta();
    else if (d->gsWheelEvent)
        return d->gsWheelEvent->delta();
    else
        return 0;
}

int KoPointerEvent::rotationX() const
{
    return d->rotationX;
}

int KoPointerEvent::rotationY() const
{
    return d->rotationY;
}

int KoPointerEvent::rotationZ() const
{
    return d->rotationZ;
}

Qt::Orientation KoPointerEvent::orientation() const
{
    if (d->wheelEvent)
        return d->wheelEvent->orientation();
    else if (d->gsWheelEvent)
        return d->gsWheelEvent->orientation();
    else
        return Qt::Horizontal;
}

void KoPointerEvent::setTabletButton(Qt::MouseButton button)
{
    d->tabletButton = button;
}

Qt::KeyboardModifiers KoPointerEvent::modifiers() const
{
    if (d->tabletEvent)
        return d->tabletEvent->modifiers();
    else if (d->mouseEvent)
        return d->mouseEvent->modifiers();
    else if (d->wheelEvent)
        return d->wheelEvent->modifiers();
    else if (d->deviceEvent)
        return d->deviceEvent->modifiers();
    else if (d->gsMouseEvent)
        return d->gsMouseEvent->modifiers();
    else if (d->gsWheelEvent)
        return d->gsWheelEvent->modifiers();
    else
        return Qt::NoModifier;
}

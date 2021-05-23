/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2006 Thorsten Zachmann <zachmann@kde.org>
   SPDX-FileCopyrightText: 2006 C. Boemann Rasmussen <cbo@boemann.dk>
   SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KoPointerEvent.h"
#include "KoInputDeviceHandlerEvent.h"
#include <QTabletEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>

class Q_DECL_HIDDEN KoPointerEvent::Private
{
public:
    Private()
        : tabletEvent(0)
        , mouseEvent(0)
        , touchEvent(0)
        , deviceEvent(0)
        , tabletButton(Qt::NoButton)
        , globalPos(0, 0)
        , pos(0, 0)
        , posZ(0)
        , rotationX(0)
        , rotationY(0)
        , rotationZ(0)
    {}

    QTabletEvent *tabletEvent;
    QMouseEvent *mouseEvent;
    QTouchEvent *touchEvent;
    KoInputDeviceHandlerEvent *deviceEvent;
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

KoPointerEvent::KoPointerEvent(QTabletEvent *ev, const QPointF &pnt)
    : point(pnt),
      m_event(ev),
      d(new Private())
{
    Q_ASSERT(m_event);
    d->tabletEvent = ev;
}

KoPointerEvent::KoPointerEvent(QTouchEvent* ev, const QPointF &pnt)
    : point(pnt)
    , m_event(ev)
    , d(new Private)
{
    Q_ASSERT(m_event);
    d->touchEvent = ev;
    d->pos = ev->touchPoints().at(0).pos().toPoint();
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
    , touchPoints(event->touchPoints)
    , m_event(event->m_event)
    , d(new Private(*(event->d)))
{
    Q_ASSERT(m_event);
}

KoPointerEvent::KoPointerEvent(const KoPointerEvent &rhs)
    : point(rhs.point)
    , touchPoints(rhs.touchPoints)
    , m_event(rhs.m_event)
    , d(new Private(*rhs.d))
{
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
    else
        return Qt::NoButton;
}

Qt::MouseButtons KoPointerEvent::buttons() const
{
    if (d->mouseEvent)
        return d->mouseEvent->buttons();
    else if (d->tabletEvent)
        return d->tabletButton;
    else if (d->deviceEvent)
        return d->deviceEvent->buttons();
    return Qt::NoButton;
}

QPoint KoPointerEvent::globalPos() const
{
    if (d->mouseEvent)
        return d->mouseEvent->globalPos();
    else if (d->tabletEvent)
        return d->tabletEvent->globalPos();
    else
        return d->globalPos;
}

QPoint KoPointerEvent::pos() const
{
    if (d->mouseEvent)
        return d->mouseEvent->pos();
    else if (d->tabletEvent)
        return d->tabletEvent->pos();
    else
        return d->pos;
}

qreal KoPointerEvent::pressure() const
{
    if (d->tabletEvent)
        return d->tabletEvent->pressure();
    else
        return 1.0;
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
        return std::fmod((d->tabletEvent->tangentialPressure() - (-1.0)) / (1.0 - (-1.0)), 2.0);
    else
        return 0.0;
}

int KoPointerEvent::x() const
{
    if (d->tabletEvent)
        return d->tabletEvent->x();
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

ulong KoPointerEvent::time() const
{
    return static_cast<QInputEvent*>(m_event)->timestamp();
}

bool KoPointerEvent::isTabletEvent()
{
    return dynamic_cast<QTabletEvent*>(m_event) != 0;
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
    else if (d->deviceEvent)
        return d->deviceEvent->modifiers();
    else
        return Qt::NoModifier;
}

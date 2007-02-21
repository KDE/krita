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

#include <QTabletEvent>
#include <QMouseEvent>
#include <QWheelEvent>

class KoPointerEvent::Private {
public:
    Private() : tabletEvent(0), mouseEvent(0), wheelEvent(0) {}
    QTabletEvent * tabletEvent;
    QMouseEvent * mouseEvent;
    QWheelEvent * wheelEvent;
};

KoPointerEvent::KoPointerEvent( QMouseEvent *ev, const QPointF &pnt )
    : point( pnt ),
    m_event(ev),
    d(new Private())
{
    d->mouseEvent = ev;
}

KoPointerEvent::KoPointerEvent( QTabletEvent *ev, const QPointF &pnt )
    : point( pnt ),
    m_event(ev),
    d(new Private())
{
    d->tabletEvent = ev;
}

KoPointerEvent::KoPointerEvent( QWheelEvent *ev, const QPointF &pnt )
    : point( pnt ),
    m_event(ev),
    d(new Private())
{
    d->wheelEvent = ev;
}

Qt::MouseButton KoPointerEvent::button () const
{
    if (d->mouseEvent)
        return d->mouseEvent->button();
    else
        return Qt::NoButton;
}

Qt::MouseButtons KoPointerEvent::buttons () const
{
    if (d->mouseEvent)
        return d->mouseEvent->buttons();
    else if (d->wheelEvent)
        return d->wheelEvent->buttons();
    else
        return Qt::NoButton;
}

const QPoint & KoPointerEvent::globalPos()
{
    if (d->mouseEvent)
        return d->mouseEvent->globalPos();
    else if (d->wheelEvent)
        return d->wheelEvent->globalPos();
    else
        return d->tabletEvent->globalPos();
}

const QPoint & KoPointerEvent::pos () const
{
    if (d->mouseEvent)
        return d->mouseEvent->pos();
    else if (d->wheelEvent)
        return d->wheelEvent->pos();
    else
        return d->tabletEvent->pos();
}

qreal KoPointerEvent::pressure () const
{
    if (d->tabletEvent)
        return d->tabletEvent->pressure();
    else
        return 0.5;
}

qreal KoPointerEvent::rotation () const
{
    if (d->tabletEvent)
        return d->tabletEvent->rotation();
    else
        return 0.0;
}

qreal KoPointerEvent::tangentialPressure () const
{
    if (d->tabletEvent)
        return d->tabletEvent->tangentialPressure();
    else
        return 0.0;
}

int KoPointerEvent::x () const
{
    if (d->tabletEvent)
        return d->tabletEvent->x();
    if (d->wheelEvent)
        return d->wheelEvent->x();
    else
        return d->mouseEvent->x();
}

int KoPointerEvent::xTilt () const
{
    if (d->tabletEvent)
        return d->tabletEvent->xTilt();
    else
        return 0;
}

int KoPointerEvent::y () const
{
    if (d->tabletEvent)
        return d->tabletEvent->y();
    if (d->wheelEvent)
        return d->wheelEvent->y();
    else
        return d->mouseEvent->y();
}

int KoPointerEvent::yTilt () const
{
    if (d->tabletEvent)
        return d->tabletEvent->yTilt();
    else
        return 0;
}

int KoPointerEvent::z () const
{
    if (d->tabletEvent)
        return d->tabletEvent->z();
    else
        return 0;
}

int KoPointerEvent::delta() const
{
    if (d->wheelEvent)
        return d->wheelEvent->delta();
    else
        return 0;
}

Qt::Orientation KoPointerEvent::orientation() const
{
    if (d->wheelEvent)
        return d->wheelEvent->orientation();
    else
        return Qt::Horizontal;
}

/* This file is part of the KDE project

   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006 Casper Boemann Rasmussen <cbr@boemann.dk>
   Copyright (C) 2006 Thomas Zander <zander@kde.org>

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


KoPointerEvent::KoPointerEvent( QMouseEvent *ev, const QPointF &pnt )
: point( pnt )
, m_tabletEvent( 0 )
, m_mouseEvent( ev )
, m_wheelEvent( 0 )
, m_event( ev )
{
}

KoPointerEvent::KoPointerEvent( QTabletEvent *ev, const QPointF &pnt )
: point( pnt )
, m_tabletEvent( ev )
, m_mouseEvent( 0 )
, m_wheelEvent( 0 )
, m_event( ev )
{
}

KoPointerEvent::KoPointerEvent( QWheelEvent *ev, const QPointF &pnt )
: point( pnt )
, m_tabletEvent( 0 )
, m_mouseEvent( 0 )
, m_wheelEvent( ev )
, m_event( ev )
{
}

Qt::MouseButton KoPointerEvent::button () const
{
    if (m_mouseEvent)
        return m_mouseEvent->button();
    else
        return Qt::NoButton;
}

Qt::MouseButtons KoPointerEvent::buttons () const
{
    if (m_mouseEvent)
        return m_mouseEvent->buttons();
    else if (m_wheelEvent)
        return m_wheelEvent->buttons();
    else
        return Qt::NoButton;
}

const QPoint & KoPointerEvent::globalPos()
{
    if (m_mouseEvent)
        return m_mouseEvent->globalPos();
    else if (m_wheelEvent)
        return m_wheelEvent->globalPos();
    else
        return m_tabletEvent->globalPos();
}

const QPoint & KoPointerEvent::pos () const
{
    if (m_mouseEvent)
        return m_mouseEvent->pos();
    else if (m_wheelEvent)
        return m_wheelEvent->pos();
    else
        return m_tabletEvent->pos();
}

qreal KoPointerEvent::pressure () const
{
    if (m_tabletEvent)
        return m_tabletEvent->pressure();
    else
        return 0.5;
}

qreal KoPointerEvent::rotation () const
{
    if (m_tabletEvent)
        return m_tabletEvent->rotation();
    else
        return 0.0;
}

qreal KoPointerEvent::tangentialPressure () const
{
    if (m_tabletEvent)
        return m_tabletEvent->tangentialPressure();
    else
        return 0.0;
}

int KoPointerEvent::x () const
{
    if (m_tabletEvent)
        return m_tabletEvent->x();
    if (m_wheelEvent)
        return m_wheelEvent->x();
    else
        return m_mouseEvent->x();
}

int KoPointerEvent::xTilt () const
{
    if (m_tabletEvent)
        return m_tabletEvent->xTilt();
    else
        return 0;
}

int KoPointerEvent::y () const
{
    if (m_tabletEvent)
        return m_tabletEvent->y();
    if (m_wheelEvent)
        return m_wheelEvent->y();
    else
        return m_mouseEvent->y();
}

int KoPointerEvent::yTilt () const
{
    if (m_tabletEvent)
        return m_tabletEvent->yTilt();
    else
        return 0;
}

int KoPointerEvent::z () const
{
    if (m_tabletEvent)
        return m_tabletEvent->z();
    else
        return 0;
}

int KoPointerEvent::delta() const
{
    if (m_wheelEvent)
        return m_wheelEvent->delta();
    else
        return 0;
}

Qt::Orientation KoPointerEvent::orientation() const
{
    if (m_wheelEvent)
        return m_wheelEvent->orientation();
    else
        return Qt::Horizontal;
}

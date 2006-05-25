/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006 Casper Boemann Rasmussen <cbr@boemann.dk>

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

#ifndef KOGFXEVENT_H
#define KOGFXEVENT_H

#include <QMouseEvent>

/**
 * The event used in Flake to have both the original (canvas based) position as
 * well as the normalized position.
 */
class KoGfxEvent
{
public:
    /**
     * Constructor.
     * @param ev the mouse event that is the base of this event.
     * @param pnt the zoomed pointed in the normal coordiante system.
     */
    KoGfxEvent( QMouseEvent *ev, const QPointF &pnt )
    : point( pnt )
    , m_event( ev )
    {}

    /**
     * For classes that are handed this event, you can choose to accept (default) this event.
     * Acceptance signifies that you have handled this event and found it usefull, the effect
     * of that will be that the event will not be handled to other event handlers.
     */
    void accept() { m_event->accept(); }
    /**
     * For classes that are handed this event, you can choose to ignore this event.
     * Ignoring this event means you have not handled it and want to allow other event
     * handlers to try to handle it.
     */
    void ignore() { m_event->ignore(); }
    /**
     * Return the modifiers.
     */
    Qt::KeyboardModifiers modifiers () const { return m_event->modifiers(); }


    /// return if the event has been accepted.
    bool isAccepted () const { return m_event->isAccepted(); }
    /// return if this event was spontaneous (see QMouseEvent::spontaneous())
    bool spontaneous () const { return m_event->spontaneous(); }

    /// return button pressed (see QMouseEvent::button());
    Qt::MouseButton button () const { return m_event->button(); }
    /// return buttons pressed (see QMouseEvent::buttons());
    Qt::MouseButtons buttons () const { return m_event->buttons(); }

    /// The point in normal space.
    const QPointF &point;
private:
    QMouseEvent *const m_event;
};

#endif /* KOGFXEVENT_H */


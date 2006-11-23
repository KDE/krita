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

#ifndef KOGFXEVENT_H
#define KOGFXEVENT_H

#include <QTabletEvent>
#include <QMouseEvent>
#include <QWheelEvent>

#include <koffice_export.h>

/**
 * KoPointerEvent is a synthetic event that can be built from a mouse
 * or a tablet event. In addition to always providing tools with tablet
 * pressure characteristics, KoPointerEvent has both the original 
 * (canvas based) position as well as the normalized position, that is,
 * the position of the event _in_ the document coordinates.
 */
class FLAKE_EXPORT KoPointerEvent
{
public:
    /**
     * Constructor.
     *
     * @param ev the mouse event that is the base of this event.
     * @param pnt the zoomed point in the normal coordinate system.
     */
    KoPointerEvent( QMouseEvent *ev, const QPointF &pnt )
        : point( pnt )
        , m_tabletEvent( 0 )
        , m_mouseEvent( ev )
        , m_wheelEvent( 0 )
        , m_event( ev )
    {
    }

    /**
     * Constructor.
     * 
     * @param ev the tablet event that is the base of this event.
     * @param pnt the zoomed point in the normal coordiante system.
     */
    KoPointerEvent( QTabletEvent *ev, const QPointF &pnt )
        : point( pnt )
        , m_tabletEvent( ev )
        , m_mouseEvent( 0 )
        , m_wheelEvent( 0 )
        , m_event( ev )
    {
    }    

    /**
     * Constructor.
     * 
     * @param ev the tablet event that is the base of this event.
     * @param pnt the zoomed point in the normal coordiante system.
     */
    KoPointerEvent( QWheelEvent *ev, const QPointF &pnt )
        : point( pnt )
        , m_tabletEvent( 0 )
        , m_mouseEvent( 0 )
        , m_wheelEvent( ev )
        , m_event( ev )
    {
    }    
    /**
     * For classes that are handed this event, you can choose to accept (default) this event.
     * Acceptance signifies that you have handled this event and found it useful, the effect
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
     * Returns the keyboard modifier flags that existed immediately before the event occurred.
     * See also QApplication::keyboardModifiers().
     */
    Qt::KeyboardModifiers modifiers () const { return m_event->modifiers(); }

    /// return if the event has been accepted.
    bool isAccepted () const { return m_event->isAccepted(); }

    /// return if this event was spontaneous (see QMouseEvent::spontaneous())
    bool spontaneous () const { return m_event->spontaneous(); }

    /// return button pressed (see QMouseEvent::button());
    Qt::MouseButton button () const 
        { 
            if (m_mouseEvent)
                return m_mouseEvent->button(); 
            else
                return Qt::NoButton;
        }

    /// return buttons pressed (see QMouseEvent::buttons());
    Qt::MouseButtons buttons () const 
        { 
            if (m_mouseEvent)
                return m_mouseEvent->buttons(); 
            else if (m_wheelEvent)
                return m_wheelEvent->buttons();
            else
                return Qt::NoButton;
        }

    // Not needed, since we send the event to the right tool. Enable
    // as soon as there are tools that do different things depending
    // on the type of event. I don't think we checked in Krita tools
    // for these values.
    //
    //TabletDevice device () const 
    //PointerType pointerType () const 
    //qint64 uniqueId () const 
     
    /// Return the position screen coordinates
    const QPoint & globalPos()
        {
            if (m_mouseEvent)
                return m_mouseEvent->globalPos();
            else if (m_wheelEvent)
                return m_wheelEvent->globalPos();
            else
                return m_tabletEvent->globalPos();
        }

    /// return the position in widget coordinates
    const QPoint & pos () const 
        {
            if (m_mouseEvent)
                return m_mouseEvent->pos();
            else if (m_wheelEvent)
                return m_wheelEvent->pos();
            else
                return m_tabletEvent->pos();
        }

    /// return the pressure (or a default value)
    qreal pressure () const 
        {
            if (m_tabletEvent) 
                return m_tabletEvent->pressure();
            else 
                return 1.0;
        }

    /// return the rotation (or a default value)
    qreal rotation () const 
        {
            if (m_tabletEvent)
                return m_tabletEvent->rotation();
            else 
                return 0.0;
        }

    /**
     * return the tangential pressure  (or a default value)
     * This is typically given by a finger wheel on an airbrush tool. The range 
     * is from -1.0 to 1.0. 0.0 indicates a neutral position. Current airbrushes can 
     * only move in the positive direction from the neutral position. If the device 
     * does not support tangential pressure, this value is always 0.0.
     */
    qreal tangentialPressure () const 
        {
            if (m_tabletEvent)
                return m_tabletEvent->tangentialPressure();
            else
                return 0.0;
        }

    /// Return the x position in widget coordinates.
    int x () const 
        {
            if (m_tabletEvent)
                return m_tabletEvent->x();
            if (m_wheelEvent)
                return m_wheelEvent->x();
            else
                return m_mouseEvent->x();
        }
    
    /**
     * Returns the angle between the device (a pen, for example) and the 
     * perpendicular in the direction of the x axis. Positive values are 
     * towards the tablet's physical right. The angle is in the range -60 
     * to +60 degrees. The default value is 0.
     */
    int xTilt () const 
        {
            if (m_tabletEvent)
                return m_tabletEvent->xTilt();
            else
                return 0;
        }

    /// Return the y position in widget coordinates.
    int y () const 
        {
            if (m_tabletEvent)
                return m_tabletEvent->y();
            if (m_wheelEvent)
                return m_wheelEvent->y();
            else
                return m_mouseEvent->y();
        }

    /**
     * Returns the angle between the device (a pen, for example) and the 
     * perpendicular in the direction of the x axis. Positive values are 
     * towards the tablet's physical right. The angle is in the range -60 
     * to +60 degrees. The default value is 0.
     */
    int yTilt () const
        {
            if (m_tabletEvent)
                return m_tabletEvent->yTilt();
            else
                return 0;
        }

    /**
     * Returns the z position of the device. Typically this is represented 
     * by a wheel on a 4D Mouse. If the device does not support a Z-axis, 
     * this value is always zero. This is <em>not</em> the same as pressure.
     */
    int z () const
        {
            if (m_tabletEvent)
                return m_tabletEvent->z();
            else
                return 0;
        }

    /**
     * Returns the distance that the wheel is rotated, in eights of a degree, or 0 otherwise.
     * @return the distance of rotation.
     * @see orientation()
     */
    int delta() const
        {
            if (m_wheelEvent)
                return m_wheelEvent->delta();
            else
                return 0;
        }

    /**
     * Returns the orientation of the delta.
     */
    Qt::Orientation orientation() const
        {
            if (m_wheelEvent)
                return m_wheelEvent->orientation();
            else
                return Qt::Horizontal;
        }

    /// The point in normal space.
    const QPointF &point;

private:
    QTabletEvent * m_tabletEvent;
    QMouseEvent * m_mouseEvent;
    QWheelEvent * m_wheelEvent;
    QInputEvent * m_event;
};

#endif /* KOGFXEVENT_H */


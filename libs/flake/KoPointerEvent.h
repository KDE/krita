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

#ifndef KOPOINTEREVENT_H
#define KOPOINTEREVENT_H

#include <QInputEvent>

class QTabletEvent;
class QMouseEvent;
class QWheelEvent;
class KoInputDeviceHandlerEvent;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneWheelEvent;

#include "flake_export.h"

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
     * @param event the mouse event that is the base of this event.
     * @param point the zoomed point in the normal coordinate system.
     */
    KoPointerEvent(QMouseEvent *event, const QPointF &point);

    /**
     * Constructor.
     *
     * @param event the mouse event that is the base of this event.
     * @param point the zoomed point in the normal coordinate system.
     */
    KoPointerEvent(QGraphicsSceneMouseEvent *event, const QPointF &point);

    /**
     * Constructor.
     *
     * @param event the mouse event that is the base of this event.
     * @param point the zoomed point in the normal coordinate system.
     */
    KoPointerEvent(QGraphicsSceneWheelEvent *event, const QPointF &point);

    /**
     * Constructor.
     *
     * @param event the tablet event that is the base of this event.
     * @param point the zoomed point in the normal coordinate system.
     */
    KoPointerEvent(QTabletEvent *event, const QPointF &point);

    /**
     * Constructor.
     *
     * @param event the tablet event that is the base of this event.
     * @param point the zoomed point in the normal coordinate system.
     */
    KoPointerEvent(QWheelEvent *event, const QPointF &point);

    KoPointerEvent(KoInputDeviceHandlerEvent *event, int x, int y, int z = 0, int rx = 0, int ry = 0, int rz = 0);

    KoPointerEvent(KoPointerEvent *event, const QPointF& point);

    ~KoPointerEvent();

    /**
     * For classes that are handed this event, you can choose to accept (default) this event.
     * Acceptance signifies that you have handled this event and found it useful, the effect
     * of that will be that the event will not be handled to other event handlers.
     */
    inline void accept() {
        m_event->accept();
    }

    /**
     * For classes that are handed this event, you can choose to ignore this event.
     * Ignoring this event means you have not handled it and want to allow other event
     * handlers to try to handle it.
     */
    inline void ignore() {
        m_event->ignore();
    }

    /**
     * Returns the keyboard modifier flags that existed immediately before the event occurred.
     * See also QApplication::keyboardModifiers().
     */
    Qt::KeyboardModifiers modifiers() const;

    /// return if the event has been accepted.
    inline bool isAccepted() const {
        return m_event->isAccepted();
    }

    /// return if this event was spontaneous (see QMouseEvent::spontaneous())
    inline bool spontaneous() const {
        return m_event->spontaneous();
    }

    /// return button pressed (see QMouseEvent::button());
    Qt::MouseButton button() const;

    /// return buttons pressed (see QMouseEvent::buttons());
    Qt::MouseButtons buttons() const;

    /// Return the position screen coordinates
    QPoint globalPos() const;

    /// return the position in widget coordinates
    QPoint pos() const;

    /**
     * return the pressure (or a default value). The range is 0.0 - 1.0
     * and the default pressure (this is the pressure that will be given
     * when you use something like the mouse) is 0.5
     */
    qreal pressure() const;

    /// return the rotation (or a default value)
    qreal rotation() const;

    /**
     * return the tangential pressure  (or a default value)
     * This is typically given by a finger wheel on an airbrush tool. The range
     * is from -1.0 to 1.0. 0.0 indicates a neutral position. Current airbrushes can
     * only move in the positive direction from the neutral position. If the device
     * does not support tangential pressure, this value is always 0.0.
     */
    qreal tangentialPressure() const;

    /**
     * Return the x position in widget coordinates.
     * @see point
     */
    int x() const;

    /**
     * Returns the angle between the device (a pen, for example) and the
     * perpendicular in the direction of the x axis. Positive values are
     * towards the tablet's physical right. The angle is in the range -60
     * to +60 degrees. The default value is 0.
     */
    int xTilt() const;

    /**
     * Return the y position in widget coordinates.
     * @see point
     */
    int y() const;

    /**
     * Returns the angle between the device (a pen, for example) and the
     * perpendicular in the direction of the x axis. Positive values are
     * towards the tablet's physical right. The angle is in the range -60
     * to +60 degrees. The default value is 0.
     */
    int yTilt() const;

    /**
     * Returns the z position of the device. Typically this is represented
     * by a wheel on a 4D Mouse. If the device does not support a Z-axis,
     * this value is always zero. This is <em>not</em> the same as pressure.
     */
    int z() const;

    /**
     * Returns the distance that the wheel is rotated, in eights of a degree, or 0 otherwise.
     * @return the distance of rotation.
     * @see orientation()
     */
    int delta() const;

    /**
     * Returns the rotation around the X-axis. If the device does not support
     * this, the value is always zero.
     */
    int rotationX() const;

    /**
     * Returns the rotation around the X-axis. If the device does not support
     * this, the value is always zero.
     */
    int rotationY() const;

    /**
     * Returns the rotation around the Z-axis. If the device does not support
     * this, the value is always zero.
     */
    int rotationZ() const;

    /**
     * Returns the orientation of the delta.
     */
    Qt::Orientation orientation() const;

    /// The point in document coordinates.
    const QPointF point;

protected:
    friend class KoToolProxy;
    /// called by KoToolProxy to set which button was pressed.
    void setTabletButton(Qt::MouseButton button);
private:
    // for the d-pointer police; we want to make accessors to the event inline, so this one stays here.
    QEvent *m_event;

    class Private;
    Private * const d;
};

#endif


/* This file is part of the KDE project
 * Copyright (c) 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KODEVICEEVENT_H
#define KODEVICEEVENT_H

#include "flake_export.h"

#include <QtGui/QInputEvent>

class KoPointerEvent;

/**
 * Base class for events from custom input devices.
 */
class FLAKE_EXPORT KoDeviceEvent : public QInputEvent
{
public:
    enum Type {
        ButtonPressed = QEvent::User + 2008,  ///< a button was pressed
        ButtonReleased, ///< a button was released
        PositionChanged   ///< the position has changed
    };

    /// Constructs a new device event of the given type
    KoDeviceEvent(Type type);

    virtual ~KoDeviceEvent();

    /// Creates and returns a KoPointerEvent for tools to consume
    virtual KoPointerEvent * pointerEvent() = 0;

    /// The device button which caused the event, Qt::NoButton if it is a move event
    Qt::MouseButton button() const;
    /// The device button state when the event was generated
    Qt::MouseButtons buttons() const;

    /// Sets the device button causing the event
    void setButton(Qt::MouseButton);
    /// Sets the device button state
    void setButtons(Qt::MouseButtons);

protected:
    KoPointerEvent *m_event;

private:
    class Private;
    Private * const d;
};

#endif // KODEVICEEVENT_H

/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOINPUTDEVICEHANDLEREVENT_H
#define KOINPUTDEVICEHANDLEREVENT_H

#include "kritaflake_export.h"

#include <QInputEvent>

class KoPointerEvent;

/**
 * Base class for events from custom input devices.
 */
class KRITAFLAKE_EXPORT KoInputDeviceHandlerEvent : public QInputEvent
{
public:
    enum Type {
        ButtonPressed = QEvent::User + 2008,  ///< a button was pressed
        ButtonReleased, ///< a button was released
        PositionChanged   ///< the position has changed
    };

    /// Constructs a new device event of the given type
    explicit KoInputDeviceHandlerEvent(Type type);

    ~KoInputDeviceHandlerEvent() override;

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

#endif // KOINPUTDEVICEHANDLEREVENT_H

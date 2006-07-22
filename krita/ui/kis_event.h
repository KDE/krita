/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_EVENT_H_
#define KIS_EVENT_H_

#include <QEvent>

#include "kis_point.h"
#include "kis_input_device.h"

/**
 * This class wrap the events from the mouse and the tablet and works in float precision.
 */
class KRITAUI_EXPORT KisEvent {
public:
    enum enumEventType {
        UnknownEvent,
        MoveEvent,
        ButtonPressEvent,
        ButtonReleaseEvent,
        DoubleClickEvent
    };

    KisEvent() : m_type(UnknownEvent), m_device(KisInputDevice::unknown()) {}
    KisEvent(enumEventType type, KisInputDevice device, const KisPoint& pos, const KisPoint& globalPos, 
             double pressure, double xTilt, double yTilt, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
        : m_type(type), m_device(device), m_pos(pos), m_globalPos(globalPos), m_pressure(pressure), 
          m_xTilt(xTilt), m_yTilt(yTilt), m_buttons(buttons), m_modifiers(modifiers) {}

    enumEventType type() const { return m_type; }
    KisInputDevice device() const { return m_device; }
    /**
     * @return the position in widget coordinates.
     */
    KisPoint pos() const { return m_pos; }
    /**
     * Convenient function that retun the x-coordinate of the event (in the widget coordinates)
     */
    double x() const { return m_pos.x(); }
    /**
     * Convenient function that retun the y-coordinate of the event (in the widget coordinates)
     */
    double y() const { return m_pos.y(); }
    /**
     * @return the position in the sreen coordinate
     */
    KisPoint globalPos() const { return m_globalPos; }
    /**
     * @return the pressure from the table.
     */
    double pressure() const { return m_pressure; }
    /**
     * @return the tilt of the stylet around the x-axis
     */
    double xTilt() const { return m_xTilt; }
    /**
     * @return the tilt of the stylet around the y-axis
     */
    double yTilt() const { return m_yTilt; }
    Qt::MouseButtons buttons() const { return m_buttons; }
    Qt::KeyboardModifiers modifiers() const { return m_modifiers; }

protected:
    enumEventType m_type;
    KisInputDevice m_device;
    KisPoint m_pos;
    KisPoint m_globalPos;
    double m_pressure;
    double m_xTilt;
    double m_yTilt;
    Qt::MouseButtons m_buttons;
    Qt::KeyboardModifiers m_modifiers;
};

#endif // KIS_EVENT_H_


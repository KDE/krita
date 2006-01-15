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
#ifndef KIS_BUTTON_EVENT_H_
#define KIS_BUTTON_EVENT_H_

#include "kis_event.h"

class KisButtonEvent : public KisEvent {
    typedef KisEvent super;
public:
    Qt::ButtonState button() const { return m_button; }
    
protected:
    KisButtonEvent() {}
    KisButtonEvent(enumEventType type,
                   KisInputDevice device,
                   const KisPoint& pos,
                   const KisPoint& globalPos,
                   double pressure,
                   double xTilt, double yTilt,
                   Qt::ButtonState button,
                   Qt::ButtonState state)
    : super(type, device, pos, globalPos, pressure, xTilt, yTilt, state)
    , m_button(button) {}

    Qt::ButtonState m_button;
};

#endif // KIS_BUTTON_EVENT_H_


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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef KIS_MOVE_EVENT_H_
#define KIS_MOVE_EVENT_H_

#include "kis_event.h"

class KisMoveEvent : public KisEvent {
	typedef KisEvent super;
public:
	KisMoveEvent() {}
	KisMoveEvent(enumInputDevice device, const KisPoint& pos, const KisPoint& globalPos, double pressure, double xTilt, double yTilt, Qt::ButtonState state) : super(MoveEvent, device, pos, globalPos, pressure, xTilt, yTilt, state) {}
};

#endif // KIS_MOVE_EVENT_H_


/*
 *  Copyright (C) 2017 Bernhard Liebl <poke1024@gmx.de>
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
 *
 */

#include "kis_native_gesture_shortcut.h"

#include <QNativeGestureEvent>

class KisNativeGestureShortcut::Private
{
public:
	Private() { }

	Qt::NativeGestureType type;
};

KisNativeGestureShortcut::KisNativeGestureShortcut(KisAbstractInputAction* action, int index, Qt::NativeGestureType type)
	: KisAbstractShortcut(action, index), d(new Private)
{
	d->type = type;
}

KisNativeGestureShortcut::~KisNativeGestureShortcut()
{
	delete d;
}

int KisNativeGestureShortcut::priority() const
{
	return 0;
}

bool KisNativeGestureShortcut::match(QNativeGestureEvent* event)
{
	//printf("checking NativeGesture against KisNativeGestureShortcut %d %d\n", (int)event->gestureType(), (int)d->type);
	return event->gestureType() == d->type;
}

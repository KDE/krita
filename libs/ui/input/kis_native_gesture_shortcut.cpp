/*
 *  SPDX-FileCopyrightText: 2017 Bernhard Liebl <poke1024@gmx.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

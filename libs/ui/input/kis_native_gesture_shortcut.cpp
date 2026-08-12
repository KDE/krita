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

    Type type = PinchNavigation;
};

KisNativeGestureShortcut::KisNativeGestureShortcut(KisAbstractInputAction* action, int index, Type type)
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
    if (d->type == PinchNavigation) {
        return event->gestureType() == Qt::PanNativeGesture || event->gestureType() == Qt::ZoomNativeGesture
            || event->gestureType() == Qt::RotateNativeGesture;
    } else if (d->type == SmartZoomNativeGesture) {
        return event->gestureType() == Qt::SmartZoomNativeGesture;
    }

    return false;
}

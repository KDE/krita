/*
 *  SPDX-FileCopyrightText: 2017 Bernhard Liebl <poke1024@gmx.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#ifndef KISNATIVEGESTURESHORTCUT_H
#define KISNATIVEGESTURESHORTCUT_H

#include "kis_abstract_shortcut.h"

class QNativeGestureEvent;
class KisNativeGestureShortcut : public KisAbstractShortcut
{
public:
	KisNativeGestureShortcut(KisAbstractInputAction* action, int index, Qt::NativeGestureType type);
	~KisNativeGestureShortcut() override;

	int priority() const override;

	bool match(QNativeGestureEvent* event);

private:
	class Private;
	Private * const d;
};

#endif // KISNATIVEGESTURESHORTCUT_H

/*
 *  SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisTouchpadScrollShortcut.h"

#include <QWheelEvent>

KisTouchpadScrollShortcut::KisTouchpadScrollShortcut(KisAbstractInputAction* action, int index)
    : KisAbstractShortcut(action, index)
{
}

KisTouchpadScrollShortcut::~KisTouchpadScrollShortcut()
{
}

int KisTouchpadScrollShortcut::priority() const
{
	return 0;
}

bool KisTouchpadScrollShortcut::match(QWheelEvent* event)
{
    return event->phase() != Qt::NoScrollPhase;
}

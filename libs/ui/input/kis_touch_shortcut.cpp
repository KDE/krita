/*
 *  This file is part of the KDE project
 *  SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "kis_touch_shortcut.h"
#include "kis_abstract_input_action.h"
#include "kis_config.h"

#include <QTouchEvent>

class KisTouchShortcut::Private
{
public:
    Private(GestureAction type)
        : minTouchPoints(0)
        , maxTouchPoints(0)
        , type(type)
        , disableOnTouchPainting(false)
    { }

    int minTouchPoints;
    int maxTouchPoints;
    GestureAction type;
    bool disableOnTouchPainting;
};

KisTouchShortcut::KisTouchShortcut(KisAbstractInputAction* action, int index, GestureAction type)
    : KisAbstractShortcut(action, index)
    , d(new Private(type))
{

}

KisTouchShortcut::~KisTouchShortcut()
{
    delete d;
}

int KisTouchShortcut::priority() const
{
    return action()->priority();
}

void KisTouchShortcut::setMinimumTouchPoints(int min)
{
    d->minTouchPoints = min;
}

void KisTouchShortcut::setMaximumTouchPoints(int max)
{
    d->maxTouchPoints = max;
}

void KisTouchShortcut::setDisableOnTouchPainting(bool disableOnTouchPainting)
{
    d->disableOnTouchPainting = disableOnTouchPainting;
}

bool KisTouchShortcut::matchTapType(QTouchEvent *event)
{
    return matchTouchPoint(event)
#ifndef Q_OS_MACOS
        && (d->type >= KisShortcutConfiguration::OneFingerTap && d->type <= KisShortcutConfiguration::FiveFingerTap)
#endif
        ;
}

bool KisTouchShortcut::matchDragType(QTouchEvent *event)
{
    return matchTouchPoint(event)
#ifndef Q_OS_MACOS
        && (d->type >= KisShortcutConfiguration::OneFingerDrag && d->type <= KisShortcutConfiguration::FiveFingerDrag)
#endif
        ;
}

bool KisTouchShortcut::matchTouchPoint(QTouchEvent *event)
{
    return (!d->disableOnTouchPainting || KisConfig(true).disableTouchOnCanvas())
        && event->touchPoints().count() >= d->minTouchPoints && event->touchPoints().count() <= d->maxTouchPoints;
}

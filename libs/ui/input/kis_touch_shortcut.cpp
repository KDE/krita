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
        , isTouchPainting(false)
    { }

    int minTouchPoints;
    int maxTouchPoints;
    GestureAction type;
    bool disableOnTouchPainting;
    bool isTouchPainting;
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
    return d->isTouchPainting ? std::numeric_limits<int>::max() : action()->priority();
}

bool KisTouchShortcut::isHoldType() const
{
#ifdef Q_OS_MACOS
    return false; // No equivalent gestures on macOS.
#else
    return d->type == KisShortcutConfiguration::OneFingerHold;
#endif
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

void KisTouchShortcut::setIsTouchPainting(bool value)
{
    d->isTouchPainting = value;
}

bool KisTouchShortcut::isAvailable(KisInputActionGroupsMask mask) const
{
    if (d->isTouchPainting && KisConfig(true).disableTouchOnCanvas()) {
        return false;
    }

    // if (d->disableOnTouchPainting && !KisConfig(true).disableTouchOnCanvas()) {
    //     return false;
    // }

    return KisAbstractShortcut::isAvailable(mask);
}

bool KisTouchShortcut::matchTapType(QTouchEvent *event, Qt::TouchPointStates allowedStates)
{
    return matchTouchPoint(event, allowedStates)
#ifndef Q_OS_MACOS
        && (d->type >= KisShortcutConfiguration::OneFingerTap && d->type <= KisShortcutConfiguration::FiveFingerTap)
#endif
        ;
}

bool KisTouchShortcut::matchDragType(QTouchEvent *event, Qt::TouchPointStates allowedStates)
{
    return matchTouchPoint(event, allowedStates)
#ifndef Q_OS_MACOS
        && (d->type >= KisShortcutConfiguration::OneFingerDrag && d->type <= KisShortcutConfiguration::FiveFingerDrag)
#endif
        ;
}

bool KisTouchShortcut::matchHoldType(QTouchEvent *event, Qt::TouchPointStates allowedStates)
{
    return isHoldType() && matchTouchPoint(event, allowedStates);
}

int KisTouchShortcut::countTouchPoints(QTouchEvent *event, Qt::TouchPointStates allowedStates)
{
    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto points = event->touchPoints();
    using TouchPoint = QTouchEvent::TouchPoint;
#else
    auto points = event->points();
    using TouchPoint = QEventPoint;
#endif

    const int count =
        std::count_if(points.begin(), points.end(), [=] (const TouchPoint &point) {
            auto state = static_cast<Qt::TouchPointState>(point.state());
            return allowedStates.testFlag(state);
        });

    return count;
}

bool KisTouchShortcut::matchTouchPoint(QTouchEvent *event, Qt::TouchPointStates allowedStates)
{
    const int numStillActivePoints = countTouchPoints(event, allowedStates);

    return /*(!d->disableOnTouchPainting || KisConfig(true).disableTouchOnCanvas())
        &&*/ numStillActivePoints >= d->minTouchPoints && numStillActivePoints <= d->maxTouchPoints;
}

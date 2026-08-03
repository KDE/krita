/*
 *  This file is part of the KDE project
 *  SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "kis_touch_shortcut.h"

#include <kis_algebra_2d.h>
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
        , isTouchPainting(false)
        , minDragThreshold(16)
    { }

    int minTouchPoints;
    int maxTouchPoints;
    GestureAction type;
    bool isTouchPainting;
    qreal minDragThreshold;
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

qreal KisTouchShortcut::minDragThreshold() const
{
    return d->minDragThreshold;
}

void KisTouchShortcut::setMinDragThreshold(qreal value)
{
    d->minDragThreshold = value;
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

void KisTouchShortcut::setIsTouchPainting(bool value)
{
    d->isTouchPainting = value;
}

bool KisTouchShortcut::isAvailable(KisInputActionGroupsMask mask) const
{
    if (d->isTouchPainting && KisConfig(true).disableTouchOnCanvas()) {
        return false;
    }

    return KisAbstractShortcut::isAvailable(mask);
}

bool KisTouchShortcut::matchTapType(const QTouchEvent *event, Qt::TouchPointStates allowedStates)
{
    return matchTouchPoint(event, allowedStates)
#ifndef Q_OS_MACOS
        && (d->type >= KisShortcutConfiguration::OneFingerTap && d->type <= KisShortcutConfiguration::FiveFingerTap)
#endif
        ;
}

bool KisTouchShortcut::matchDragType(const QTouchEvent *event, Qt::TouchPointStates allowedStates)
{
    return touchDragDistance(event, allowedStates) > d->minDragThreshold &&
        matchTouchPoint(event, allowedStates)
#ifndef Q_OS_MACOS
        && (d->type >= KisShortcutConfiguration::OneFingerDrag && d->type <= KisShortcutConfiguration::FiveFingerDrag)
#endif
        ;
}

bool KisTouchShortcut::matchHoldType(const QTouchEvent *event, Qt::TouchPointStates allowedStates)
{
    return isHoldType() && matchTouchPoint(event, allowedStates);
}

// TODO: const
int KisTouchShortcut::countTouchPoints(const QTouchEvent *event, Qt::TouchPointStates allowedStates)
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

qreal KisTouchShortcut::touchDragDistance(const QTouchEvent *event, Qt::TouchPointStates allowedStates)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto points = event->touchPoints();
    using TouchPoint = QTouchEvent::TouchPoint;
#else
    auto points = event->points();
    using TouchPoint = QEventPoint;
#endif

    return std::sqrt(std::accumulate(points.begin(), points.end(), qreal(0),
        [&] (qreal maxOffsetSq, const TouchPoint &point) {
            auto state = static_cast<Qt::TouchPointState>(point.state());
            if (!allowedStates.testFlag(state)) {
                return maxOffsetSq;
            }

            return std::max(maxOffsetSq, KisAlgebra2D::normSquared(point.pos() - point.startPos()));
        }));
}

bool KisTouchShortcut::matchTouchPoint(const QTouchEvent *event, Qt::TouchPointStates allowedStates)
{
    const int numStillActivePoints = countTouchPoints(event, allowedStates);

    return numStillActivePoints >= d->minTouchPoints && numStillActivePoints <= d->maxTouchPoints;
}

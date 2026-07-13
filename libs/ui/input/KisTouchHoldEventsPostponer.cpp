/*
 *  SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisTouchHoldEventsPostponer.h"

#include <QTimer>

#include <kis_assert.h>
#include <input/kis_touch_shortcut.h>

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include <KoPointerEvent.h>
#else
#include <QTouchEvent>
#endif


KisTouchHoldEventsPostponer::KisTouchHoldEventsPostponer(qreal maxHoldDistance, int holdTimeout)
    : m_maxHoldDistance(maxHoldDistance)
{
    connect(&m_timer, &QTimer::timeout, this, &KisTouchHoldEventsPostponer::slotHoldCompletionTimeout);

    m_timer.setSingleShot(true);
    m_timer.setTimerType(Qt::CoarseTimer);
    m_timer.setInterval(holdTimeout);
    m_timer.start();
}

KisTouchHoldEventsPostponer::~KisTouchHoldEventsPostponer()
{
    qDeleteAll(m_postponedEvents);
}

KisTouchHoldEventsPostponer::State KisTouchHoldEventsPostponer::state() const
{
    return m_state;
}

void KisTouchHoldEventsPostponer::pushThrough(QTouchEvent *event)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_state == WaitingForHold);

    if (qualifyForPostponing(event)) {
        enqueueEvent(event);
    } else {
        // enqueue the event to make sure it is delivered via
        // the "deliver all the postponed events" mechanism
        enqueueEvent(event);
        cancelHoldWait();
    }
}

void KisTouchHoldEventsPostponer::cancelHoldWait()
{
    m_state = HoldCancelled;
    m_timer.stop();
}

QVector<QTouchEvent*> KisTouchHoldEventsPostponer::postponedEvents()
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_state != WaitingForHold);
    return m_postponedEvents;
}

void KisTouchHoldEventsPostponer::slotHoldCompletionTimeout()
{
    // it might happen that the timer event has been emitted before
    // a cancellation touch event was delivered, then the timer event
    // may come "late"
    if (m_state != WaitingForHold) {
        // the caller should have processed all the postponed events in this case
        KIS_SAFE_ASSERT_RECOVER_NOOP(m_postponedEvents.isEmpty());
        return;
    }

    m_state = HoldCompleted;
    Q_EMIT sigHoldCompleted();
}

bool KisTouchHoldEventsPostponer::qualifyForPostponing(QTouchEvent *event)
{
    const int numPressedPoints = KisTouchShortcut::countTouchPoints(event, KisTouchShortcut::pressedOnlyTouchStates());
    if (numPressedPoints > 1)
        return false;

    const qreal pressDistance = KisTouchShortcut::touchDragDistance(event, KisTouchShortcut::pressedOnlyTouchStates());
    if (pressDistance > m_maxHoldDistance)
        return false;

    return true;
}

void KisTouchHoldEventsPostponer::enqueueEvent(QTouchEvent *event)
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    // TODO: change QScopedPointer -> std::unique_ptr
    QScopedPointer<QEvent> clonedTouchEvent;
    KoPointerEvent::copyQtPointerEvent(event, clonedTouchEvent);
    m_postponedEvents.append(static_cast<QTouchEvent *>(clonedTouchEvent.take()));
#else
    m_postponedEvents.append(static_cast<QTouchEvent *>(event->clone()));
#endif
}

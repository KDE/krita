/*
 *  SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISTOUCHHOLDEVENTSPOSTPONER_H
#define KISTOUCHHOLDEVENTSPOSTPONER_H

#include <QTimer>
#include <kritaui_export.h>

class QTouchEvent;

/**
 * The object to store and manage postponed events while waiting for
 * a touch-hold shortcut to trigger. Currently, it supports single-
 * finger touch actions only.
 *
 * Usage:
 *
 *    \code{.cpp}
 *
 *    // in the class
 *    std::optional<KisTouchHoldEventsPostponer> postponer;
 *
 *    // on action start
 *    postponer.emplace(20, 1000);
 *
 *    // on any touch event (including the first)
 *    if (postponer) {
 *        postponer->pushThrough(event);
 *        if (postponer->state() == KisTouchHoldEventsPostponer::HoldCancelled) {
 *            Q_FOREACH (QTouchEvent *event, postponer->postponedEvents()) {
 *                // ... deliver the event ...
 *            }
 *            postponer.reset();
 *        }
 *    }
 *
 *    // on sigHoldCompleted()
 *    QTouchEvent *tiggeringEvent = postponer->postponedEvents().front();
 *    // ... start operation using the very first as the rule ...
 *
 *    \endcode
 */
class KRITAUI_EXPORT KisTouchHoldEventsPostponer : public QObject
{
    Q_OBJECT
public:
    enum State {
        WaitingForHold, /// the postponer is still accumulating the events
        HoldCompleted, /// the hold action has been detected, the accumulated events should be dropped and the hold action should be started
        HoldCancelled /// hold action has been cancelled, the accumulated events should be fired to the consumers
    };
public:
    /**
     * Createa  touch-hold postponer with \p maxHoldDistance used
     * as a threshold for hold/drag and \p holdTimeout as a timeout
     * to consider the hold operation completed
     */
    KisTouchHoldEventsPostponer(qreal maxHoldDistance, int holdTimeout);
    KisTouchHoldEventsPostponer(const KisTouchHoldEventsPostponer &rhs) = delete;
    ~KisTouchHoldEventsPostponer();

    /**
     * \return the current state of the postponer
     */
    State state() const;

    /**
     * Push a new event through the postponer. The method should be called in
     * `WaitingForHold` state only. In all other states the postponer is not
     * considered as active anymore.
     *
     * If \p event does not qualify as an event for touch-hold, then the state
     * of the postponer will be switched into `HoldCancelled`, which should be
     * handled by the called on exiting pushThrough().
     *
     * Note: the \p event that caused the state transition into `HoldCancelled`
     * state will still be added to the queue, so that it could be managed
     * uniformely with the rest of the postponed events.
     */
    void pushThrough(QTouchEvent *event);

    /**
     * Cancel waiting for a touch hold. The state will transition into `HoldCancelled`
     */
    void cancelHoldWait();

    /**
     * \return the clones of all the postponed events, potentially including the one
     * that caused the cancellation of the hold-wait. The postponer **owns** all the
     * postponed events.
     */
    QVector<QTouchEvent*> postponedEvents();

Q_SIGNALS:
    /**
     * Emitted when the postponed has detected a touch-and-hold action
     */
    void sigHoldCompleted();

private Q_SLOTS:
    void slotHoldCompletionTimeout();

private:
    /**
     * \return if \p event can be considered as a touch-hold event
     *
     * It is currently hardcoded to support only single-finger touch-holds.
     */
    bool qualifyForPostponing(QTouchEvent *event);

    /**
     * Make a clone of the event and add to the queue. Can be removed after
     * we stop supporting Qt5.
     */
    void enqueueEvent(QTouchEvent *event);

private:
    QTimer m_timer;
    State m_state { WaitingForHold };
    QVector<QTouchEvent*> m_postponedEvents;
    qreal m_maxHoldDistance {20.0} ;
};

#endif // KISTOUCHHOLDEVENTSPOSTPONER_H

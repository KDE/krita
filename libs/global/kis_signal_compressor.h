/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SIGNAL_COMPRESSOR_H
#define __KIS_SIGNAL_COMPRESSOR_H

#include <QObject>
#include "kritaglobal_export.h"

#include <QElapsedTimer>
#include <functional>

class QTimer;

/**
 * Sets a timer to delay or throttle activation of a Qt slot. One example of
 * where this is used is to limit the amount of expensive redraw activity on the
 * canvas.
 *
 * There are four behaviors to choose from.
 *
 * POSTPONE resets the timer after each call. Therefore if the calls are made
 * quickly enough, the timer will never be activated.
 *
 * FIRST_ACTIVE_POSTPONE_NEXT emits the first signal and postpones all
 * the other actions like in POSTPONE. This mode is
 * used e.g. in move/remove layer functionality. If you remove a
 * single layer, you'll see the result immediately. But if you want to
 * remove multiple layers, you should wait until all the actions are
 * finished.
 *
 * FIRST_ACTIVE emits the timeout() event immediately and sets a timer of
 * duration \p delay. If the compressor is triggered during this time, it will
 * wait until the end of the delay period to fire the signal. Further events are
 * ignored until the timer elapses. Think of it as a queue with size 1, and
 * where the leading element is popped every \p delay ms.
 *
 * FIRST_INACTIVE emits the timeout() event at the end of a timer of duration \p
 * delay ms. The compressor becomes inactive and all events are ignored until
 * the timer has elapsed.
 *
 * The current implementation allows the timeout() to be delayed by up to 2 times
 * \p delay in certain situations (for details see cpp file).
 */
class KRITAGLOBAL_EXPORT KisSignalCompressor : public QObject
{
    Q_OBJECT

public:
    enum Mode {
        POSTPONE, /* Calling start() resets the timer to \p delay ms */
        FIRST_ACTIVE_POSTPONE_NEXT, /* emits the first signal and postpones all the next ones */
        FIRST_ACTIVE, /* Emit timeout() signal immediately. Throttle further timeout() to rate of one per \p delay ms */
        FIRST_INACTIVE, /* Set a timer \p delay ms, emit timeout() when it elapses. Ignore all events meanwhile. */
        UNDEFINED /* KisSignalCompressor is created without an explicit mode */
    };

    enum SlowHandlerMode {
        PRECISE_INTERVAL, /* Interval of timeout is forced to \p delay ms, whatever time the handler of timeout() takes */
        ADDITIVE_INTERVAL /* When the handler of timeout() is slow, the timeout delay is increased to the (delay + handler_time) */
    };

public:
    KisSignalCompressor();
    KisSignalCompressor(int delay, Mode mode, QObject *parent = 0);
    KisSignalCompressor(int delay, Mode mode, SlowHandlerMode slowHandlerMode, QObject *parent = 0);
    bool isActive() const;
    void setMode(Mode mode);

    int delay() const;

    void setIdleCallback();
    void setDelay(std::function<bool()> idleCallback, int idleDelay, int timeout);

public Q_SLOTS:
    void setDelay(int delay);
    void start();
    void stop();

private Q_SLOTS:
    void slotTimerExpired();

Q_SIGNALS:
    void timeout();

private:
    bool tryEmitOnTick(bool isFromTimer);
    bool tryEmitSignalSafely();
    void setDelayImpl(int delay);

private:
    QTimer *m_timer = 0;
    Mode m_mode = UNDEFINED;
    SlowHandlerMode m_slowHandlerMode = PRECISE_INTERVAL;
    bool m_signalsPending = false;
    QElapsedTimer m_lastEmittedTimer;
    int m_isEmitting = 0;
    int m_timeout = 0;
    std::function<bool()> m_idleCallback;
};

#endif /* __KIS_SIGNAL_COMPRESSOR_H */

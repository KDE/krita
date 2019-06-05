/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_SIGNAL_COMPRESSOR_H
#define __KIS_SIGNAL_COMPRESSOR_H

#include <QObject>
#include "kritaglobal_export.h"

class KisRelaxedTimer;

/**
 * Sets a timer to delay or throttle activation of a Qt slot. One example of
 * where this is used is to limit the amount of expensive redraw activity on the
 * canvas.
 *
 * There are three behaviors to choose from.
 *
 * POSTPONE resets the timer after each call. Therefore if the calls are made
 * quickly enough, the timer will never be activated.
 *
 * FIRST_ACTIVE_POSTPONE_NEXT emits the first signal and postpones all
 * the other actions the other action like in POSTPONE. This mode is
 * used e.g.  in move/remove layer functionality. If you remove a
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

public:
    KisSignalCompressor();
    KisSignalCompressor(int delay, Mode mode, QObject *parent = 0);
    bool isActive() const;
    void setMode(Mode mode);


public Q_SLOTS:
    void setDelay(int delay);
    void start();
    void stop();

private Q_SLOTS:
    void slotTimerExpired();

Q_SIGNALS:
    void timeout();

private:
    KisRelaxedTimer *m_timer;
    Mode m_mode;
    bool m_gotSignals;
};

#endif /* __KIS_SIGNAL_COMPRESSOR_H */

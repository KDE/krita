/*
 *  Copyright (c) 2017 Bernhard Liebl <poke1024@gmx.de>
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

#ifndef __KIS_RELAXED_TIMER_H
#define __KIS_RELAXED_TIMER_H

#include <QObject>
#include <QBasicTimer>
#include <QElapsedTimer>

#include "kritaglobal_export.h"

/**
 * A timer using an interface like QTimer that relaxes the given interval callback
 * time guarantees and minimizes internal timer restarts by keeping one long-running
 * repeating timer.
 *
 * Users can use this just like a QTimer. The difference is that KisRelaxedTimer will
 * relax the callback guarantee time as follows: timeouts will never happen earlier
 * than \p interval ms, but may well happen only after 2 * \p interval ms (whereas
 * QTimer guarantees a fixed interval of \p interval ms).
 *
 * The rationale for using this is that stopping and starting timers can produce a
 * measurable performance overhead. KisRelaxedTimer removes that overhead.
 */
class KRITAGLOBAL_EXPORT KisRelaxedTimer : public QObject
{
    Q_OBJECT

public:
    KisRelaxedTimer(QObject *parent = nullptr);

    void start();

    inline void stop() {
        m_emitOnTimeTick = 0;
    }

    void setInterval(int interval);
    void setSingleShot(bool singleShot);

    inline bool isActive() const {
        return m_emitOnTimeTick >= m_nextTimerTickSeqNo;
    }

    int remainingTime() const;

Q_SIGNALS:
    void timeout();

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    int m_interval;
    bool m_singleShot;

    QBasicTimer m_timer;
    qint64 m_nextTimerTickSeqNo;
    qint64 m_emitOnTimeTick;

    QElapsedTimer m_elapsed;

protected:
    class IsEmitting {
    public:
        IsEmitting(KisRelaxedTimer &timer) : m_timer(timer) {
            timer.m_isEmitting = true;
        }

        ~IsEmitting() {
            m_timer.m_isEmitting = false;
        }

    private:
        KisRelaxedTimer &m_timer;
    };

    friend class IsEmitting;

    bool m_isEmitting;
};

#endif /* __KIS_RELAXED_TIMER_H */

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
#include "krita_export.h"

class QTimer;

class KRITAIMAGE_EXPORT KisSignalCompressor : public QObject
{
    Q_OBJECT

public:
    enum Mode {
        POSTPONE, /* every start() porspones event by \p delay ms */
        FIRST_ACTIVE, /* fist call to start() emits a signal, the latter will happen not earlier after \p delay ms */
        FIRST_INACTIVE /* the first signal will be emitted not earlier that after \p delay ms after the first call to start() */
    };

public:
    KisSignalCompressor(int delay, Mode mode, QObject *parent = 0);
    bool isActive() const;
    void setDelay(int delay);

public Q_SLOTS:
    void start();
    void stop();

private Q_SLOTS:
    void slotTimerExpired();

Q_SIGNALS:
    void timeout();

private:
    QTimer *m_timer;
    Mode m_mode;
    bool m_gotSignals;
};

#endif /* __KIS_SIGNAL_COMPRESSOR_H */

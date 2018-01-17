/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_TIMED_SIGNAL_THRESHOLD_H
#define __KIS_TIMED_SIGNAL_THRESHOLD_H

#include "kritaimage_export.h"
#include <QScopedPointer>
#include <QObject>


/**
 * Emits the timeout() signal if and only if the flow of start()
 * events has been coming for a consecutive \p delay of milliseconds.
 * If the events were not coming for \p cancelDelay of milliseconds the
 * counting is dropped and the new period is started.
 */
class KRITAIMAGE_EXPORT KisTimedSignalThreshold : public QObject
{
    Q_OBJECT
public:
    KisTimedSignalThreshold(int delay, int cancelDelay = -1, QObject *parent = 0);
    ~KisTimedSignalThreshold() override;

public Q_SLOTS:
    /**
     * Stops counting and emits the signal forcefully
     */
    void forceDone();

    /**
     * Start/continue counting and if the signal flow is stable enough
     * (longer than \p delay and shorter than \p cancelDelay), the
     * timeout signal in emitted.
     */
    void start();

    /**
     * Stops counting the signals flow
     */
    void stop();

    /**
     * Enable or disable emitting the signal
     */
    void setEnabled(bool value);


    /**
     * The peiod of time, after which the signal will be emitted
     */
    void setDelayThreshold(int delay, int cancelDelay = -1);

Q_SIGNALS:
    void timeout();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_TIMED_SIGNAL_THRESHOLD_H */

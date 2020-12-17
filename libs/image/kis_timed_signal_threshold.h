/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

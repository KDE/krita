/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef __KISSCOPEDPERFORMANCELOGGER_H_
#define __KISSCOPEDPERFORMANCELOGGER_H_

#include <QElapsedTimer>
#include <QString>

#include "kis_debug.h"
#include "kritaglobal_export.h"

class KRITAGLOBAL_EXPORT KisScopedPerformanceLogger
{
    Q_DISABLE_COPY_MOVE(KisScopedPerformanceLogger)
public:
    explicit KisScopedPerformanceLogger(const QString &title)
        : m_title(title)
    {
        if (dbgPerformanceIsEnabled()) {
            m_timer.start();
        }
    }

    ~KisScopedPerformanceLogger()
    {
        if (m_timer.isValid()) {
            // We'll format this as fractional milliseconds so that both short
            // and long durations are reasonably comprehensible without having
            // to decipher excessively long numbers.
            qint64 nsecs = m_timer.nsecsElapsed();
            dbgPerformance.noquote() << QStringLiteral("%1: %2.%3 ms")
                                            .arg(m_title)
                                            .arg(nsecs / 1000000LL)
                                            .arg(nsecs % 1000000LL, 6, 10, QChar('0'));
        }
    }

private:
    QString m_title;
    QElapsedTimer m_timer;
};

#endif

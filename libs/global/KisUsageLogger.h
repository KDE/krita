/*
 *  SPDX-FileCopyrightText: 2019 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISUSAGELOGGER_H
#define KISUSAGELOGGER_H

#include <QString>
#include <QScopedPointer>

#include "kritaglobal_export.h"

/**
 * @brief The KisUsageLogger class logs messages to a logfile
 */
class KRITAGLOBAL_EXPORT KisUsageLogger
{

public:

    KisUsageLogger();
    ~KisUsageLogger();

    static void initialize();
    static void close();

    /// basic system information
    ///    (there is other information spreaded in the code
    ///     check usages of writeSysInfo for details)
    static QString basicSystemInfo();

    /// Logs with date/time
    static void log(const QString &message);

    /// Writes without date/time
    static void write(const QString &message);

    /// Writes to the system information file and Krita log
    static void writeSysInfo(const QString &message);

    static void writeHeader();

    /// Returns information about all available screens
    static QString screenInformation();

private:

    void rotateLog();

    Q_DISABLE_COPY(KisUsageLogger)

    struct Private;
    const QScopedPointer<Private> d;

    static const QString s_sectionHeader;
    static const int s_maxLogs {10};

};

#endif // KISUSAGELOGGER_H

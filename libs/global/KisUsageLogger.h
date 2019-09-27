/*
 *  Copyright (c) 2019 Boudewijn Rempt <boud@valdyas.org>
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

    /// Logs with date/time
    static void log(const QString &message);

    /// Writes without date/time
    static void write(const QString &message);


    static void writeHeader();

private:

    void rotateLog();

    Q_DISABLE_COPY(KisUsageLogger)

    struct Private;
    const QScopedPointer<Private> d;

    static const QString s_sectionHeader;
    static const int s_maxLogs {10};

};

#endif // KISUSAGELOGGER_H

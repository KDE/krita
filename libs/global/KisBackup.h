/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 1999 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2006 Jaison Lee <lee.jaison@gmail.com>
    SPDX-FileCopyrightText: 2011 Romain Perier <bambi@ubuntu.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KISBACKUP_H
#define KISBACKUP_H

#include <QString>
#include <kritaglobal_export.h>

class KRITAGLOBAL_EXPORT KisBackup
{
public:

    static bool backupFile(const QString &filename, const QString &backupDir = QString());
    static bool simpleBackupFile(const QString &filename, const QString &backupDir = QString(), const QString &backupExtension = QStringLiteral("~"));
    static bool numberedBackupFile(const QString &filename,
                                   const QString &backupDir = QString(),
                                   const QString &backupExtension = QStringLiteral("~"),
                                   const uint maxBackups = 10);
};

#endif

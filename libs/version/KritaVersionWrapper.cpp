/*
 *  SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <KritaVersionWrapper.h>

#include <kritaversion.h>
#include <kritagitversion.h>

QString KritaVersionWrapper::versionString(bool checkGit)
{
    QString kritaVersion = QStringLiteral(KRITA_VERSION_STRING);
    QString version = kritaVersion;

    if (checkGit) {
#ifdef KRITA_GIT_SHA1_STRING
        QString gitVersion = QStringLiteral(KRITA_GIT_SHA1_STRING);
        version = QStringLiteral("%1 (git %2)").arg(kritaVersion, gitVersion);
#endif
    }
    return version;
}

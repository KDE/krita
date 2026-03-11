/*
 *  SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KritaVersionWrapper.h"

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

bool KritaVersionWrapper::isDevelopersBuild()
{
    // Qt6 is not considered stable yet, don't present it as such.
#if defined(KRITA_STABLE) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return false;
#else
    return true;
#endif
}

/* This file is part of the KDE project
SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>
SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>

SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef SYSTEMINFORMATION_P_H
#define SYSTEMINFORMATION_P_H

#include <QString>

namespace SystemInformation {
    QString userName();
    QString operatingSystemVersion();
}

#if !defined(Q_OS_WIN)
#include <pwd.h>
#include <unistd.h>
#include <sys/utsname.h>
inline QString SystemInformation::userName()
{
    struct passwd *p = getpwuid(getuid());
    return QString::fromLatin1(p->pw_name);
}

inline QString SystemInformation::operatingSystemVersion()
{
    struct utsname unameBuf;
    uname(&unameBuf);
    return QString::fromUtf8(unameBuf.sysname) +
        QStringLiteral(" (") + QString::fromUtf8(unameBuf.machine) + QLatin1String(") ") +
        QStringLiteral("release ") + QString::fromUtf8(unameBuf.release);
}
#else
#include <QSysInfo>
#include <qt_windows.h>
#define SECURITY_WIN32
#include <security.h>
//#include <secext.h> // GetUserNameEx

inline QString SystemInformation::userName()
{
    WCHAR nameBuffer[256];
    DWORD bufsize = 256;
    if (!GetUserNameExW(NameDisplay, nameBuffer, &bufsize)) {
        return QStringLiteral("Unknown User"); //should never happen (translate?)
    }
    return QString::fromWCharArray(nameBuffer);
}

static inline QString windowsVersionString() {
    QString productVersion = QSysInfo::productVersion();
    if (productVersion == "unknown") return QStringLiteral("Unknown Windows");
    return "Windows " + productVersion;
}

inline QString SystemInformation::operatingSystemVersion()
{
    SYSTEM_INFO info;
    GetNativeSystemInfo(&info);
    QString arch;
    switch (info.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_AMD64:
    case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64:
        arch = QStringLiteral(" (x86_64)");
        break;
    case PROCESSOR_ARCHITECTURE_INTEL:
        arch = QStringLiteral(" (x86)");
        break;
    case PROCESSOR_ARCHITECTURE_ARM:
        arch = QStringLiteral(" (ARM)");
        break;
    default:
        arch = QStringLiteral(" (unknown architecture)");
    }
    QString winVer;
    //TODO: handle Service packs?
    return windowsVersionString() + arch;
}

#endif

#endif // SYSTEMINFORMATION_P_H

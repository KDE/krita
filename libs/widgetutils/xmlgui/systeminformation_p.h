/* This file is part of the KDE project
Copyright (C) 1999 David Faure <faure@kde.org>
Copyright (C) 2014 Alex Richardson <arichardson.kde@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
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
    switch (QSysInfo::windowsVersion()) {
    case QSysInfo::WV_XP: return QStringLiteral("Windows XP");
    case QSysInfo::WV_2003: return QStringLiteral("Windows 2003");
    case QSysInfo::WV_VISTA: return QStringLiteral("Windows Vista");
    case QSysInfo::WV_WINDOWS7: return QStringLiteral("Windows 7");
    case QSysInfo::WV_WINDOWS8: return QStringLiteral("Windows 8");
    case QSysInfo::WV_WINDOWS8_1: return QStringLiteral("Windows 8.1");
    default: return QStringLiteral("Unknown Windows");
    }
}

inline QString SystemInformation::operatingSystemVersion()
{
    SYSTEM_INFO info;
    GetNativeSystemInfo(&info);
    QString arch;
    switch (info.dwProcessorType) {
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

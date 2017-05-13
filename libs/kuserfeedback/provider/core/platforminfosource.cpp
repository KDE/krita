/*
    Copyright (C) 2016 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "platforminfosource.h"

#include <QSysInfo>
#include <QVariant>

using namespace UserFeedback;

PlatformInfoSource::PlatformInfoSource() :
    AbstractDataSource(QStringLiteral("platform"))
{
}

QString PlatformInfoSource::description() const
{
    return tr("Type and version of the operating system.");
}

QVariant PlatformInfoSource::data()
{
    QVariantMap m;
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID))
    // on Linux productType() is the distro name
    m.insert(QStringLiteral("os"), QStringLiteral("linux"));

    // openSUSE Tumbleweed has the current date as version number, that is a bit too precise for us
    if (QSysInfo::productType() == QLatin1String("opensuse") && QSysInfo::productVersion().startsWith(QLatin1String("20")))
        m.insert(QStringLiteral("version"), QString(QSysInfo::productType() + QLatin1String("-tumbleweed")));
    else
        m.insert(QStringLiteral("version"), QString(QSysInfo::productType() + QLatin1Char('-') + QSysInfo::productVersion()));
#else
    m.insert(QStringLiteral("os"), QSysInfo::productType());
    m.insert(QStringLiteral("version"), QSysInfo::productVersion());
#endif
#else

    // Qt4 and Qt5 < 5.4
#ifdef Q_OS_LINUX
    m.insert(QStringLiteral("os"), QStringLiteral("linux"));
    m.insert(QStringLiteral("version"), QStringLiteral("unknown")); // TODO could be done by reading /etc/os-release
#elif defined(Q_OS_WIN32)
    m.insert(QStringLiteral("os"), QStringLiteral("windows"));
    switch (QSysInfo::windowsVersion()) {
        case QSysInfo::WV_NT: m.insert(QStringLiteral("version"), QStringLiteral("4.0")); break;
        case QSysInfo::WV_2000: m.insert(QStringLiteral("version"), QStringLiteral("5.0")); break;
        case QSysInfo::WV_XP: m.insert(QStringLiteral("version"), QStringLiteral("5.1")); break;
        case QSysInfo::WV_2003: m.insert(QStringLiteral("version"), QStringLiteral("5.2")); break;
        case QSysInfo::WV_VISTA: m.insert(QStringLiteral("version"), QStringLiteral("6.0")); break;
        case QSysInfo::WV_WINDOWS7: m.insert(QStringLiteral("version"), QStringLiteral("6.1")); break;
        case QSysInfo::WV_WINDOWS8: m.insert(QStringLiteral("version"), QStringLiteral("6.2")); break;
#if QT_VERSION > QT_VERSION_CHECK(4, 8, 5)
        case QSysInfo::WV_WINDOWS8_1: m.insert(QStringLiteral("version"), QStringLiteral("6.3")); break;
#endif
        default: m.insert(QStringLiteral("version"), QStringLiteral("unknown"));
    }
#elif defined(Q_OS_MAC)
    m.insert(QStringLiteral("os"), QStringLiteral("mac"));
    switch (QSysInfo::MacintoshVersion) {
        case QSysInfo::MV_10_3: m.insert(QStringLiteral("version"), QStringLiteral("10.3")); break;
        case QSysInfo::MV_10_4: m.insert(QStringLiteral("version"), QStringLiteral("10.4")); break;
        case QSysInfo::MV_10_5: m.insert(QStringLiteral("version"), QStringLiteral("10.5")); break;
        case QSysInfo::MV_10_6: m.insert(QStringLiteral("version"), QStringLiteral("10.6")); break;
        case QSysInfo::MV_10_7: m.insert(QStringLiteral("version"), QStringLiteral("10.7")); break;
        case QSysInfo::MV_10_8: m.insert(QStringLiteral("version"), QStringLiteral("10.8")); break;
        case QSysInfo::MV_10_9: m.insert(QStringLiteral("version"), QStringLiteral("10.9")); break;
        default: m.insert(QStringLiteral("version"), QStringLiteral("unknown"));
    }
#else
    m.insert(QStringLiteral("os"), QStringLiteral("unusual"));
#endif

#endif
    return m;
}

/* This file is part of the KDE project
   Copyright (C) 2017 Alexey Kapustin <akapust1n@mail.ru>


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
 * Boston, MA 02110-1301, USA.
*/
#include "general_info_source.h"

#include <QDebug>
#include <QSysInfo>
#include <QThread>
#include <QVariant>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
typedef QSharedPointer<QSettings> QSettingsPtr;
using namespace UserFeedback;
using namespace KUserFeedback;

TelemetryGeneralInfoSource::TelemetryGeneralInfoSource()
    : AbstractDataSource(QStringLiteral("general"), Provider::DetailedSystemInformation)
{
}

QString TelemetryGeneralInfoSource::description() const
{
    return QObject::tr("The general info about Krita.");
}

QVariant TelemetryGeneralInfoSource::data()
{
    QVariantMap m;
    QLocale l = QLocale::system().name();;
    QString version = qApp->applicationVersion();
    qDebug() << version;
    version = version.left(version.indexOf(" "));
    qDebug() << version;
    m.insert(QStringLiteral("appVersion"), version);

    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    const QDir configDir(configPath);
    if (!configDir.exists()) {
        configDir.mkpath(QStringLiteral("."));
    }
    QSettingsPtr settings = QSettingsPtr(new QSettings(configPath + QStringLiteral("/klanguageoverridesrc"), QSettings::IniFormat));
    settings->beginGroup(QStringLiteral("Language"));
    m.insert(QStringLiteral("appLanguage"), settings->value(qAppName()).toString());
    m.insert(QStringLiteral("systemLanguage"),l.name());
    return m;
}

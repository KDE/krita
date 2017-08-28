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
#include "fatal_assert_info_source.h"
#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <QVariantMap>
using namespace UserFeedback;
using namespace KUserFeedback;

UserFeedback::TelemetryFatalAssertInfoSource::TelemetryFatalAssertInfoSource()
    : AbstractDataSource(QStringLiteral("asserts"), Provider::DetailedSystemInformation)
{
}

QString UserFeedback::TelemetryFatalAssertInfoSource::description() const
{
    return QObject::tr("Information about fatal asserts");
}

QVariant UserFeedback::TelemetryFatalAssertInfoSource::data()
{
    QVariantMap m;
    KConfigGroup configGroup =  KSharedConfig::openConfig()->group("KisAsserts");

    QString assertText = configGroup.readEntry("FatalAssertion", "Empty");
    if(assertText == "Empty" )
        throw NoFatalError();
    m.insert(QStringLiteral("assertText"), assertText);
    configGroup.writeEntry("FatalAssertion", "Empty");

    QString assertFile = configGroup.readEntry("FatalFile", "Empty");
    m.insert(QStringLiteral("assertFile"), assertFile);
    configGroup.writeEntry("FatalFile", "Empty");

    QString assertLine = configGroup.readEntry("FatalLine", "Empty");
    m.insert(QStringLiteral("assertLine"), assertLine.toInt());
    configGroup.writeEntry("FatalLine", "Empty");
    m.insert(QStringLiteral("count"), 1);
    m.insert(QStringLiteral("isFatal"), true);

    return m;

}

const char *NoFatalError::what() const  throw()
{
    return "no fatal error \n";
}

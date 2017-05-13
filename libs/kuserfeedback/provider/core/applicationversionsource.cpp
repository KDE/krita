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

#include "applicationversionsource.h"

#include <QCoreApplication>
#include <QVariant>

using namespace UserFeedback;

ApplicationVersionSource::ApplicationVersionSource() :
    AbstractDataSource(QStringLiteral("applicationVersion"))
{
}

QString ApplicationVersionSource::description() const
{
    return tr("The version of the application.");
}

QVariant ApplicationVersionSource::data()
{
    if (QCoreApplication::applicationVersion().isEmpty())
        return QVariant();

    QVariantMap m;
    m.insert(QStringLiteral("value"), QCoreApplication::applicationVersion());
    return m;
}

/*
    Copyright (C) 2017 Volker Krause <vkrause@kde.org>

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

#include "localeinfosource.h"

#include <QLocale>
#include <QVariant>

using namespace UserFeedback;

LocaleInfoSource::LocaleInfoSource()
    : AbstractDataSource(QStringLiteral("locale"))
{
}

QString LocaleInfoSource::description() const
{
    return tr("The current region and language settings.");
}

QVariant LocaleInfoSource::data()
{
    QLocale l;
    QVariantMap m;
    m.insert(QStringLiteral("region"), QLocale::countryToString(l.country()));
    m.insert(QStringLiteral("language"), QLocale::languageToString(l.language()));
    return m;
}

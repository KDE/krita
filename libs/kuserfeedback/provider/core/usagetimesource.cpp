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

#include "usagetimesource.h"
#include "abstractdatasource_p.h"
#include "provider_p.h"

#include <QVariant>

using namespace UserFeedback;

namespace UserFeedback {
class UsageTimeSourcePrivate : public AbstractDataSourcePrivate
{
public:
    UsageTimeSourcePrivate() : provider(nullptr) {}
    ProviderPrivate *provider;
};
}

UsageTimeSource::UsageTimeSource() :
    AbstractDataSource(QStringLiteral("usageTime"), new UsageTimeSourcePrivate)
{
}

QString UsageTimeSource::description() const
{
    return tr("The total amount of time the application has been used.");
}

QVariant UsageTimeSource::data()
{
    Q_D(UsageTimeSource);
    Q_ASSERT(d->provider);

    QVariantMap m;
    m.insert(QStringLiteral("value"), d->provider->currentApplicationTime());
    return m;
}

void UsageTimeSource::setProvider(ProviderPrivate* p)
{
    Q_D(UsageTimeSource);
    d->provider = p;
}

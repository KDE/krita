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

#include "startcountsource.h"
#include "abstractdatasource_p.h"
#include "provider_p.h"

#include <QVariant>

using namespace UserFeedback;

namespace UserFeedback {
class StartCountSourcePrivate : public AbstractDataSourcePrivate
{
public:
    StartCountSourcePrivate() : provider(nullptr) {}
    ProviderPrivate *provider;
};
}

StartCountSource::StartCountSource() :
    AbstractDataSource(QStringLiteral("startCount"), new StartCountSourcePrivate)
{
}

QString StartCountSource::description() const
{
    return tr("How often the application has been started.");
}

QVariant StartCountSource::data()
{
    Q_D(StartCountSource);
    Q_ASSERT(d->provider);

    QVariantMap m;
    m.insert(QStringLiteral("value"), d->provider->startCount);
    return m;
}

void StartCountSource::setProvider(ProviderPrivate *p)
{
    Q_D(StartCountSource);
    d->provider = p;
}

/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KoInteractionStrategyFactory.h"

#include <QString>

struct KoInteractionStrategyFactory::Private
{
    int priority = 0;
    QString id;
};

KoInteractionStrategyFactory::KoInteractionStrategyFactory(int priority, const QString &id)
    : m_d(new Private)
{
    m_d->priority = priority;
    m_d->id = id;
}

KoInteractionStrategyFactory::~KoInteractionStrategyFactory()
{
}

QString KoInteractionStrategyFactory::id() const
{
    return m_d->id;
}

int KoInteractionStrategyFactory::priority() const
{
    return m_d->priority;
}

bool KoInteractionStrategyFactory::compareLess(KoInteractionStrategyFactorySP f1, KoInteractionStrategyFactorySP f2)
{
    return f1->priority() < f2->priority();
}


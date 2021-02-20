/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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


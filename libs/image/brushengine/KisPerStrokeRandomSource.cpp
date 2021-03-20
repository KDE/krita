/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisPerStrokeRandomSource.h"

#include <QHash>
#include <QMutex>
#include <QMutexLocker>

#include <boost/random/taus88.hpp>
#include <boost/random/uniform_smallint.hpp>
#include <boost/random/normal_distribution.hpp>

struct KisPerStrokeRandomSource::Private
{
    Private(int _seed)
        : seed(_seed)
    {
        boost::taus88 tempGenerator(seed);
        generatorMax = tempGenerator.max();
    }

    Private(const Private &rhs)
        : seed(rhs.seed),
          generatorMax(rhs.generatorMax),
          valuesCache(rhs.valuesCache)
    {
    }

    qint64 fetchInt(const QString &key);

    int seed = 0;
    qint64 generatorMax = 0;
    QHash<QString, qint64> valuesCache;
    QMutex mutex;
};

KisPerStrokeRandomSource::KisPerStrokeRandomSource()
    : m_d(new Private(qrand()))
{

}

KisPerStrokeRandomSource::KisPerStrokeRandomSource(const KisPerStrokeRandomSource &rhs)
    : KisShared(),
      m_d(new Private(*rhs.m_d))
{
}

KisPerStrokeRandomSource::~KisPerStrokeRandomSource()
{
}


qint64 KisPerStrokeRandomSource::Private::fetchInt(const QString &key)
{
    QMutexLocker l(&mutex);

    auto it = valuesCache.find(key);
    if (it != valuesCache.end()) {
        return it.value();
    }

    boost::taus88 oneTimeGenerator(seed + qHash(key));
    const qint64 newValue = oneTimeGenerator();

    valuesCache.insert(key, newValue);

    return newValue;
}

int KisPerStrokeRandomSource::generate(const QString &key, int min, int max) const
{
    return min + m_d->fetchInt(key) % (max - min);
}

qreal KisPerStrokeRandomSource::generateNormalized(const QString &key) const
{
    return qreal(m_d->fetchInt(key)) / m_d->generatorMax;
}


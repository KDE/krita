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


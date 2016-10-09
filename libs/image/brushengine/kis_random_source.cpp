/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_random_source.h"

#include <boost/random/taus88.hpp>
#include <boost/random/uniform_smallint.hpp>
#include <boost/random/normal_distribution.hpp>
#include <time.h>
#include <ctime>
#include <//qDebug>

struct KisRandomSource::Private
{
    Private()
        {}

    Private(int seed)
        : uniformSource(seed) {}

    /**
     * Taus88's numbers are not too random, but it works fast and it
     * can be copied very quickly (three 32-bit integers only).
     *
     * Average cycle: 2^88 steps
     */
    boost::taus88 uniformSource;
};


KisRandomSource::KisRandomSource()
    : m_d(new Private)
{
    int r = qrand();
    m_d->uniformSource = boost::taus88(r);
    //qDebug() << "new kisrandomsource()" << this << "random seed" << r; 
}

KisRandomSource::KisRandomSource(int seed)
    : m_d(new Private(seed))
{
    //qDebug() << "new kisrandomsource(seed)" << this << "seed" << seed;
}

KisRandomSource::KisRandomSource(const KisRandomSource &rhs)
    : KisShared(),
      m_d(new Private(*rhs.m_d))
{

    //qDebug() << "copy kisrandomsource from" << &rhs << "to" << this;
}

KisRandomSource& KisRandomSource::operator=(const KisRandomSource &rhs)
{
    if (this != &rhs) {
        //qDebug() << "operator= kisrandomsource from" << &rhs << "to" << this;
        *m_d = *rhs.m_d;
    }

    return *this;
}

KisRandomSource::~KisRandomSource()
{
}

qint64 KisRandomSource::generate() const
{
    qint64 v = m_d->uniformSource();
    //qDebug() << "generate" << v;
    return v;
}

int KisRandomSource::generate(int min, int max) const
{
    boost::uniform_smallint<int> smallint(min, max);
    int i = smallint(m_d->uniformSource);
    //qDebug() << this << "generate min" << min << "max" << max << "result" << i;
    return i;
}

qreal KisRandomSource::generateNormalized() const
{
    const qint64 v = m_d->uniformSource();
    const qint64 max = m_d->uniformSource.max();

    //qDebug() << "generateNormalized" << qreal(v) / max;

    return qreal(v) / max;
}

qreal KisRandomSource::generateGaussian(qreal mean, qreal sigma) const
{
    boost::normal_distribution<qreal> normal(mean, sigma);
    qreal v = normal(m_d->uniformSource);
    //qDebug() << "generateGaussian mean" << mean << "sigma" << sigma << "result" << v;
    return v;
}

/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_stroke_random_source.h"

struct KisStrokeRandomSource::Private
{
    Private()
        : levelOfDetail(0),
          lod0RandomSource(new KisRandomSource()),
          lodNRandomSource(new KisRandomSource(*lod0RandomSource)),
          lod0PerStrokeRandomSource(new KisPerStrokeRandomSource()),
          lodNPerStrokeRandomSource(new KisPerStrokeRandomSource(*lod0PerStrokeRandomSource))
    {
    }

    int levelOfDetail;
    KisRandomSourceSP lod0RandomSource;
    KisRandomSourceSP lodNRandomSource;

    KisPerStrokeRandomSourceSP lod0PerStrokeRandomSource;
    KisPerStrokeRandomSourceSP lodNPerStrokeRandomSource;
};


KisStrokeRandomSource::KisStrokeRandomSource()
    : m_d(new Private)
{
}

KisStrokeRandomSource::KisStrokeRandomSource(const KisStrokeRandomSource &rhs)
    : m_d(new Private(*rhs.m_d))
{
}

KisStrokeRandomSource& KisStrokeRandomSource::operator=(const KisStrokeRandomSource &rhs)
{
    if (&rhs != this) {
        *m_d = *rhs.m_d;
    }

    return *this;
}

KisStrokeRandomSource::~KisStrokeRandomSource()
{
}

KisRandomSourceSP KisStrokeRandomSource::source() const
{
    return m_d->levelOfDetail ? m_d->lodNRandomSource : m_d->lod0RandomSource;
}

KisPerStrokeRandomSourceSP KisStrokeRandomSource::perStrokeSource() const
{
    return m_d->levelOfDetail ? m_d->lodNPerStrokeRandomSource : m_d->lod0PerStrokeRandomSource;
}


int KisStrokeRandomSource::levelOfDetail() const
{
    return m_d->levelOfDetail;
}

void KisStrokeRandomSource::setLevelOfDetail(int value)
{
    m_d->levelOfDetail = value;
}

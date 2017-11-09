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

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

#include "kis_lod_capable_layer_offset.h"

#include "kis_lod_transform.h"


struct KisLodCapableLayerOffset::Private
{
    Private(KisDefaultBoundsBaseSP _defaultBounds)
        : defaultBounds(_defaultBounds),
          x(0),
          y(0),
          lodX(0),
          lodY(0)
    {
    }

    KisDefaultBoundsBaseSP defaultBounds;

    int x;
    int y;

    int lodX;
    int lodY;
};


KisLodCapableLayerOffset::KisLodCapableLayerOffset(KisDefaultBoundsBaseSP defaultBounds)
    : m_d(new Private(defaultBounds))
{
}

KisLodCapableLayerOffset::KisLodCapableLayerOffset(const KisLodCapableLayerOffset &rhs)
    : m_d(new Private(*rhs.m_d))
{
}

KisLodCapableLayerOffset& KisLodCapableLayerOffset::operator=(const KisLodCapableLayerOffset &rhs)
{
    if (this != &rhs) {
        *m_d = *rhs.m_d;
    }

    return *this;
}

KisLodCapableLayerOffset::~KisLodCapableLayerOffset()
{
}

int KisLodCapableLayerOffset::x() const
{
    const int newLod = m_d->defaultBounds->currentLevelOfDetail();
    return newLod > 0 ? m_d->lodX : m_d->x;
}

int KisLodCapableLayerOffset::y() const
{
    const int newLod = m_d->defaultBounds->currentLevelOfDetail();
    return newLod > 0 ? m_d->lodY : m_d->y;
}

void KisLodCapableLayerOffset::setX(int value)
{
    const int newLod = m_d->defaultBounds->currentLevelOfDetail();
    (newLod > 0 ? m_d->lodX : m_d->x) = value;
}

void KisLodCapableLayerOffset::setY(int value)
{
    const int newLod = m_d->defaultBounds->currentLevelOfDetail();
    (newLod > 0 ? m_d->lodY : m_d->y) = value;
}

void KisLodCapableLayerOffset::syncLodOffset()
{
    const int newLod = m_d->defaultBounds->currentLevelOfDetail();

    m_d->lodX = KisLodTransform::coordToLodCoord(m_d->x, newLod);
    m_d->lodY = KisLodTransform::coordToLodCoord(m_d->y, newLod);
}

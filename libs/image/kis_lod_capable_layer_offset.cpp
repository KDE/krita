/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

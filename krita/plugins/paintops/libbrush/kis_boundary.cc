/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#include "kis_boundary.h"
#include <QPixmap>
#include <QPainter>
#include <QList>

#include "KoColorSpace.h"
#include "kis_paint_device.h"
#include "kis_iterators_pixel.h"

struct KisBoundary::Private {
    bool isDark(quint8 val);
    KisPaintDeviceSP m_device;
    int m_fuzzyness;

    PointPairListList m_horSegments;
    PointPairListList m_vertSegments;

};

KisBoundary::KisBoundary(KisPaintDeviceSP dev) : d(new Private)
{
    d->m_device = dev;
    d->m_fuzzyness = 255 / 2;
}

KisBoundary::~KisBoundary()
{
}

bool KisBoundary::Private::isDark(quint8 val)
{
    return val < m_fuzzyness;
}

void KisBoundary::generateBoundary(int w, int h)
{
    if (!d->m_device)
        return;

    const KoColorSpace* cs = d->m_device->colorSpace();

    // Horizontal
    for (int currentY = - 1; currentY < h; currentY++) {

        KisHLineConstIteratorPixel topIt = d->m_device->createHLineIterator(0, currentY, w);
        KisHLineConstIteratorPixel botIt = d->m_device->createHLineIterator(0, currentY + 1, w);

        bool darkTop;
        bool darkBot;

        d->m_horSegments.append(QList<PointPair>());

        while (!topIt.isDone()) {
            darkTop = cs->alpha(topIt.rawData());
            darkBot = cs->alpha(botIt.rawData());
            if (darkTop != darkBot) {
                // detected a change
                d->m_horSegments.back().append(qMakePair(QPointF(botIt.x(), botIt.y()), 1));
            }
            ++topIt;
            ++botIt;
        }
    }

    // Vertical
    for (int currentX = - 1; currentX < w; currentX++) {
        KisVLineConstIteratorPixel leftIt = d->m_device->createVLineIterator(currentX, 0, h);
        KisVLineConstIteratorPixel rightIt = d->m_device->createVLineIterator(currentX + 1, 0, h);
        bool darkLeft;
        bool darkRight;

        d->m_vertSegments.append(QList<PointPair>());

        while (!leftIt.isDone()) {
            darkLeft = cs->alpha(leftIt.rawData());
            darkRight = cs->alpha(rightIt.rawData());
            if (darkLeft != darkRight) {
                // detected a change
                d->m_vertSegments.back().append(qMakePair(QPointF(rightIt.x(), rightIt.y()), 1));
            }
            ++leftIt;
            ++rightIt;
        }
    }
}

const KisBoundary::PointPairListList& KisBoundary::horizontalSegment() const
{
    return d->m_horSegments;
}

const KisBoundary::PointPairListList& KisBoundary::verticalSegment() const
{
    return d->m_vertSegments;
}


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
#include "kis_fixed_paint_device.h"
#include "kis_iterators_pixel.h"

struct KisBoundary::Private {
    bool isDark(quint8 val);
    KisFixedPaintDeviceSP m_device;
    int m_fuzzyness;

    PointPairListList m_horSegments;
    PointPairListList m_vertSegments;

};

KisBoundary::KisBoundary(KisFixedPaintDeviceSP dev) : d(new Private)
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
    int pixelSize = d->m_device->pixelSize();

    // Yes, we start looking before the begin of the data. There we return the default pixel,
    // which is transparent.
    quint8* dataPointer = d->m_device->data();
    quint8* dataPointerTop = d->m_device->data() - w * pixelSize;
    quint8* dataPointerBot = d->m_device->data();
    // Horizontal
    for (int currentY = -1; currentY < h; currentY++) {

        d->m_horSegments.append(QList<PointPair>());

        for (int currentX = 0; currentX < w; currentX++) {

            bool darkTop;
            bool darkBot;

            if (dataPointerTop < dataPointer) {
                darkTop = OPACITY_TRANSPARENT;
            } else {
                darkTop = cs->alpha(dataPointerTop);
            }
            darkBot = cs->alpha(dataPointerBot);

            if (darkTop != darkBot) {
                // detected a change
                d->m_horSegments.back().append(qMakePair(QPointF(currentX, currentY + 1), 1));
            }

            dataPointerTop++;
            dataPointerBot++;
        }
    }

    // Vertical
    for (int currentX = - 1; currentX < w; currentX++) {

        bool darkLeft;
        bool darkRight;

        d->m_vertSegments.append(QList<PointPair>());

        for (int currentY = 0; currentY < h; currentY++) {

            quint8* dataPointerLeft = d->m_device->data() + (h * pixelSize) + (currentX * pixelSize);
            quint8* dataPointerRight = dataPointerTop - pixelSize;

            darkLeft = cs->alpha(dataPointerLeft);
            darkRight = cs->alpha(dataPointerRight);

            if (darkLeft != darkRight) {
                // detected a change
                d->m_vertSegments.back().append(qMakePair(QPointF(currentX, currentY), 1));
            }
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


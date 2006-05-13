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
#include <QPixmap>
#include <QPainter>
//Added by qt3to4:
#include <Q3ValueList>

#include "kis_colorspace.h"
#include "kis_iterators_pixel.h"
#include "kis_paint_device.h"
#include "kis_boundary.h"

KisBoundary::KisBoundary(KisPaintDevice* dev) {
    m_device = dev;
    m_fuzzyness = 255 / 2;
}

bool KisBoundary::isDark(quint8 val) {
    return val < m_fuzzyness;
}

void KisBoundary::generateBoundary(int w, int h) {
    if (!m_device)
        return;

    KisColorSpace* cs = m_device->colorSpace();

    // Horizontal
    for (int currentY = - 1; currentY < h; currentY++) {
        KisHLineIteratorPixel topIt = m_device->createHLineIterator(0, currentY, w, false);
        KisHLineIteratorPixel botIt = m_device->createHLineIterator(0, currentY + 1, w, false);
        bool darkTop;
        bool darkBot;

        m_horSegments.append(Q3ValueList<PointPair>());

        while (!topIt.isDone()) {
            darkTop = cs->getAlpha(topIt.rawData());
            darkBot = cs->getAlpha(botIt.rawData());
            if (darkTop != darkBot) {
                // detected a change
                m_horSegments.back().append(qMakePair(KisPoint(botIt.x(), botIt.y()), 1));
            }
            ++topIt;
            ++botIt;
        }
    }

    // Vertical
    for (int currentX = - 1; currentX < w; currentX++) {
        KisVLineIteratorPixel leftIt = m_device->createVLineIterator(currentX, 0, h, false);
        KisVLineIteratorPixel rightIt = m_device->createVLineIterator(currentX + 1, 0, h, false);
        bool darkLeft;
        bool darkRight;

        m_vertSegments.append(Q3ValueList<PointPair>());

        while (!leftIt.isDone()) {
            darkLeft = cs->getAlpha(leftIt.rawData());
            darkRight = cs->getAlpha(rightIt.rawData());
            if (darkLeft != darkRight) {
                // detected a change
                m_vertSegments.back().append(qMakePair(KisPoint(rightIt.x(), rightIt.y()), 1));
            }
            ++leftIt;
            ++rightIt;
        }
    }
}


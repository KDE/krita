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
#include <qpixmap.h>
#include <qpainter.h>

#include "kis_colorspace.h"
#include "kis_iterators_pixel.h"

#include "kis_boundary.h"

KisBoundary::KisBoundary(KisPaintDeviceImpl* dev) {
    m_device = dev;
    m_fuzzyness = 255 / 2;
}

bool KisBoundary::isDark(Q_UINT8 val) {
    return val < m_fuzzyness;
}

void KisBoundary::generateBoundary(int w, int h) {
    if (!m_device)
        return;

    KisColorSpace* cs = m_device -> colorSpace();

    // Horizontal
    for (int currentY = - 1; currentY < h; currentY++) {
        KisHLineIteratorPixel topIt = m_device -> createHLineIterator(0, currentY, w, false);
        KisHLineIteratorPixel botIt = m_device -> createHLineIterator(0, currentY + 1, w, false);
        bool darkTop;
        bool darkBot;

        m_horSegments.append(QValueList<PointPair>());

        while (!topIt.isDone()) {
            darkTop = cs -> getAlpha(topIt.rawData());
            darkBot = cs -> getAlpha(botIt.rawData());
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
        KisVLineIteratorPixel leftIt = m_device -> createVLineIterator(currentX, 0, h, false);
        KisVLineIteratorPixel rightIt = m_device -> createVLineIterator(currentX + 1, 0, h, false);
        bool darkLeft;
        bool darkRight;

        m_vertSegments.append(QValueList<PointPair>());

        while (!leftIt.isDone()) {
            darkLeft = cs -> getAlpha(leftIt.rawData());
            darkRight = cs -> getAlpha(rightIt.rawData());
            if (darkLeft != darkRight) {
                // detected a change
                m_vertSegments.back().append(qMakePair(KisPoint(rightIt.x(), rightIt.y()), 1));
            }
            ++leftIt;
            ++rightIt;
        }
    }
}

QPixmap KisBoundary::pixmap(int w, int h) {
    QPixmap target(w, h);
    QPainter painter(&target);

    painter.eraseRect(0, 0, w, h);

    paint(painter);

    painter.end();
    return target;
}

void KisBoundary::paint(QPainter& painter) {
    QValueList< QValueList<PointPair> >::const_iterator it = m_horSegments.constBegin();
    QValueList< QValueList<PointPair> >::const_iterator end = m_horSegments.constEnd();
    
    // Horizontal
    while (it != end) {
        QValueList<PointPair>::const_iterator lineIt = (*it).constBegin();
        QValueList<PointPair>::const_iterator lineEnd = (*it).constEnd();
        while (lineIt != lineEnd) {
            int x1 = (*lineIt).first.floorX();
            int y = (*lineIt).first.floorY();
            int x2 = x1 + (*lineIt).second;

            painter.drawLine(x1, y, x2, y);
            painter.drawPoint(x2, y);

            ++lineIt;
        }
        ++it;
    }
    
    // Vertical
    it = m_vertSegments.constBegin();
    end = m_vertSegments.constEnd();
    
    while (it != end) {
        QValueList<PointPair>::const_iterator lineIt = (*it).constBegin();
        QValueList<PointPair>::const_iterator lineEnd = (*it).constEnd();
        while (lineIt != lineEnd) {
            int x = (*lineIt).first.floorX();
            int y1 = (*lineIt).first.floorY();
            int y2 = y1 + (*lineIt).second;

            painter.drawLine(x, y1, x, y2);
            painter.drawPoint(x, y2);

            ++lineIt;
        }
        ++it;
    }
}


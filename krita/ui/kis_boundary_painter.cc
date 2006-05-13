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

#include "kis_boundary.h"
#include "kis_boundary_painter.h"
#include "kis_canvas.h"
#include "kis_canvas_painter.h"

QPixmap KisBoundaryPainter::createPixmap(const KisBoundary& boundary, int w, int h)
{
    QPixmap target(w, h);
    KisCanvasPainter painter(&target);

    painter.eraseRect(0, 0, w, h);

    paint(boundary, painter);

    painter.end();
    return target;
}

void KisBoundaryPainter::paint(const KisBoundary& boundary, KisCanvasPainter& painter)
{
    KisBoundary::PointPairListList::const_iterator it = boundary.m_horSegments.constBegin();
    KisBoundary::PointPairListList::const_iterator end = boundary.m_horSegments.constEnd();
    
    // Horizontal
    while (it != end) {
        KisBoundary::PointPairList::const_iterator lineIt = (*it).constBegin();
        KisBoundary::PointPairList::const_iterator lineEnd = (*it).constEnd();
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
    it = boundary.m_vertSegments.constBegin();
    end = boundary.m_vertSegments.constEnd();
    
    while (it != end) {
        KisBoundary::PointPairList::const_iterator lineIt = (*it).constBegin();
        KisBoundary::PointPairList::const_iterator lineEnd = (*it).constEnd();
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


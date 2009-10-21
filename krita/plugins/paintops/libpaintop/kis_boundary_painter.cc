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

#include "kis_boundary_painter.h"
#include <QPixmap>
#include <QPainter>

#include <KoViewConverter.h>

#include "kis_boundary.h"
#include <kis_image.h>

void KisBoundaryPainter::paint(const KisBoundary* boundary, KisImageWSP image, QPainter& painter, const KoViewConverter &converter)
{
    KisBoundary::PointPairListList::const_iterator it = boundary->horizontalSegment().constBegin();
    KisBoundary::PointPairListList::const_iterator end = boundary->horizontalSegment().constEnd();

    // Horizontal
    while (it != end) {
        KisBoundary::PointPairList::const_iterator lineIt = (*it).constBegin();
        KisBoundary::PointPairList::const_iterator lineEnd = (*it).constEnd();
        while (lineIt != lineEnd) {
            int x1 = static_cast<int>((*lineIt).first.x());
            int y = static_cast<int>((*lineIt).first.y());
            int x2 = x1 + (*lineIt).second;

            QPointF p1 = converter.documentToView(image->pixelToDocument(QPoint(x1, y)));
            QPointF p2 = converter.documentToView(image->pixelToDocument(QPoint(x2, y)));
            painter.drawLine(p1, p2);
            painter.drawPoint(p2);

            ++lineIt;
        }
        ++it;
    }

    // Vertical
    it = boundary->verticalSegment().constBegin();
    end = boundary->verticalSegment().constEnd();

    while (it != end) {
        KisBoundary::PointPairList::const_iterator lineIt = (*it).constBegin();
        KisBoundary::PointPairList::const_iterator lineEnd = (*it).constEnd();
        while (lineIt != lineEnd) {
            int x = static_cast<int>((*lineIt).first.x());
            int y1 = static_cast<int>((*lineIt).first.y());
            int y2 = y1 + (*lineIt).second;

            QPointF p1 = converter.documentToView(image->pixelToDocument(QPoint(x, y1)));
            QPointF p2 = converter.documentToView(image->pixelToDocument(QPoint(x, y2)));
            painter.drawLine(p1, p2);
            painter.drawPoint(p2);

            ++lineIt;
        }
        ++it;
    }
}


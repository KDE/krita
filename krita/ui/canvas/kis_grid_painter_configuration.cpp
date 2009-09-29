/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include "canvas/kis_grid_painter_configuration.h"

#include "kis_config.h"

#include <QBrush>
#include <QPen>

Qt::PenStyle gs2style(quint32 s)
{
    switch (s) {
    case 1:
        return Qt::DashLine;
    case 2:
        return Qt::DotLine;
    case 3:
        return Qt::DashDotLine;
    case 4:
        return Qt::DashDotDotLine;
    default:
        return Qt::SolidLine;
    }
}

QPen KisGridPainterConfiguration::mainPen()
{
    KisConfig cfg;
    return QPen(cfg.getGridMainColor(), 1, gs2style(cfg.getGridMainStyle()));
}
QPen KisGridPainterConfiguration::subdivisionPen()
{
    KisConfig cfg;
    return QPen(cfg.getGridSubdivisionColor(), 1, gs2style(cfg.getGridSubdivisionStyle()));
}


#if 0

#include <config-opengl.h>

#ifdef HAVE_OPENGL
#include <qgl.h>
#endif

#include <KoViewConverter.h>

KisGridDrawer::KisGridDrawer(KisDoc2* doc, const KoViewConverter * viewConverter)
        : m_doc(doc),
        m_viewConverter(viewConverter)
{
}

Qt::PenStyle KisGridDrawer::gs2style(quint32 s)
{
    switch (s) {
    case 1:
        return Qt::DashLine;
    case 2:
        return Qt::DotLine;
    case 3:
        return Qt::DashDotLine;
    case 4:
        return Qt::DashDotDotLine;
    default:
        return Qt::SolidLine;
    }
}

#define pixelToView(point) \
    m_viewConverter->documentToView(image->pixelToDocument(point))


void KisGridDrawer::drawPerspectiveGrid(KisImageWSP image, const QRect& wr, const KisSubPerspectiveGrid* grid)
{
    Q_UNUSED(wr);

    KisConfig cfg;
    QPen mainPen = QPen(cfg.getGridMainColor(), 1, gs2style(cfg.getGridMainStyle()));
    QPen subdivisionPen =  QPen(cfg.getGridSubdivisionColor(), 1, gs2style(cfg.getGridSubdivisionStyle()));
    setPen(subdivisionPen);
    // 1 -> top-left corner
    // 2 -> top-right corner
    // 3 -> bottom-right corner
    // 4 -> bottom-left corner
    // d12 line from top-left to top-right
    // note that the notion of top-left is purely theorical
    KisPerspectiveMath::LineEquation d12 = KisPerspectiveMath::computeLineEquation(grid->topLeft().data(), grid->topRight().data()) ;
    QPointF v12 = QPointF(*grid->topLeft() - *grid->topRight());
    v12.setX(v12.x() / grid->subdivisions()); v12.setY(v12.y() / grid->subdivisions());
    KisPerspectiveMath::LineEquation d23 = KisPerspectiveMath::computeLineEquation(grid->topRight().data(), grid->bottomRight().data());
    QPointF v23 = QPointF(*grid->topRight() - *grid->bottomRight());
    v23.setX(v23.x() / grid->subdivisions()); v23.setY(v23.y() / grid->subdivisions());
    KisPerspectiveMath::LineEquation d34 = KisPerspectiveMath::computeLineEquation(grid->bottomRight().data(), grid->bottomLeft().data());
    KisPerspectiveMath::LineEquation d41 = KisPerspectiveMath::computeLineEquation(grid->bottomLeft().data(), grid->topLeft().data());

    QPointF horizVanishingPoint = KisPerspectiveMath::computeIntersection(d12, d34);
    QPointF vertVanishingPoint = KisPerspectiveMath::computeIntersection(d23, d41);

    for (int i = 1; i < grid->subdivisions(); i ++) {
        QPointF pol1 = *grid->topRight() + i * v12;
        KisPerspectiveMath::LineEquation d1 = KisPerspectiveMath::computeLineEquation(&pol1, &vertVanishingPoint);
        QPointF pol1b =  KisPerspectiveMath::computeIntersection(d1, d34);
        drawLine(pixelToView(pol1.toPoint()), pixelToView(pol1b.toPoint()));

        QPointF pol2 = *grid->bottomRight() + i * v23;
        KisPerspectiveMath::LineEquation d2 = KisPerspectiveMath::computeLineEquation(&pol2, &horizVanishingPoint);
        QPointF pol2b = KisPerspectiveMath::computeIntersection(d2, d41);
        drawLine(pixelToView(pol2.toPoint()), pixelToView(pol2b.toPoint()));
    }
    setPen(mainPen);
    drawLine(pixelToView(*grid->topLeft()), pixelToView(*grid->topRight()));
    drawLine(pixelToView(*grid->topRight()), pixelToView(*grid->bottomRight()));
    drawLine(pixelToView(*grid->bottomRight()), pixelToView(*grid->bottomLeft()));
    drawLine(pixelToView(*grid->bottomLeft()), pixelToView(*grid->topLeft()));
}

#undef pixelToView
#define pixelToView(point) \
    m_viewConverter->documentToView( m_doc->image()->pixelToDocument(point))

void KisGridDrawer::drawGrid(const QRectF& area)
{
//     if(!m_doc->gridData().showGrid())
//         return;

    KisConfig cfg;

    quint32 offsetx = cfg.getGridOffsetX();
    quint32 offsety = cfg.getGridOffsetY();
    quint32 hspacing = cfg.getGridHSpacing(); // m_doc->gridData().gridX(); // use koffice grid when KOffice grid is on par with Krita grid, and is configurable by whatever mean Krita has to manipulate the grid
    quint32 vspacing = cfg.getGridVSpacing(); // m_doc->gridData().gridY(); // use koffice grid when KOffice grid is on par with Krita grid, and is configurable by whatever mean Krita has to manipulate the grid
    quint32 subdivision = cfg.getGridSubdivisions() - 1;

    // Draw vertical line
    QPen mainPen = QPen(cfg.getGridMainColor(), 1, gs2style(cfg.getGridMainStyle()));
    QPen subdivisionPen = QPen(cfg.getGridSubdivisionColor(), 1, gs2style(cfg.getGridSubdivisionStyle()));
    quint32 i = subdivision - (offsetx / hspacing) % (subdivision + 1);

    QPointF bottomRight = m_doc->image()->documentToPixel(area.bottomRight());
    QPointF topLeft = m_doc->image()->documentToPixel(area.topLeft());

    double x = offsetx % hspacing;
    while (x <= bottomRight.x()) {
        if (i == subdivision) {
            setPen(mainPen);
            i = 0;
        } else {
            setPen(subdivisionPen);
            i++;
        }
        if (x >= topLeft.x()) {
            // Always draw the full line otherwise the line stippling varies
            // with the location of area and we get glitchy patterns.
            drawLine(pixelToView(QPointF(x, topLeft.y())), pixelToView(QPointF(x, bottomRight.y())));
        }
        x += hspacing;
    }
    // Draw horizontal line
    i = subdivision - (offsety / vspacing) % (subdivision + 1);
    double y = offsety % vspacing;
    while (y <= bottomRight.y()) {
        if (i == subdivision) {
            setPen(mainPen);
            i = 0;
        } else {
            setPen(subdivisionPen);
            i++;
        }
        if (y >= topLeft.y()) {
            drawLine(pixelToView(QPointF(topLeft.x(), y)), pixelToView(QPointF(bottomRight.x(), y)));
        }
        y += vspacing;
    }
}

QPainterGridDrawer::QPainterGridDrawer(KisDoc2* doc, const KoViewConverter * viewConverter)
        : KisGridDrawer(doc, viewConverter),
        m_painter(0)
{
}

#endif

/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006,2008 Cyrille Berger <cberger@cberger.net>
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

#include "canvas/kis_perspective_grid_manager.h"


#include <kaction.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>

#include <KoViewConverter.h>

#include "canvas/kis_canvas2.h"
#include "kis_config.h"
#include "kis_image.h"
#include "kis_perspective_grid.h"
#include "canvas/kis_grid_painter_configuration.h"
#include "kis_view2.h"
#include "kis_canvas_resource_provider.h"

KisPerspectiveGridManager::KisPerspectiveGridManager(KisView2 * parent)
        : KisCanvasDecoration("perspectiveGrid", i18n("Perspective grid"), parent)
        , m_toggleEdition(false)
        , m_view(parent)
{
}


KisPerspectiveGridManager::~KisPerspectiveGridManager()
{

}

void KisPerspectiveGridManager::updateGUI()
{
    KisImageWSP image = m_view->image();


    if (image) {
        KisPerspectiveGrid* pGrid = image->perspectiveGrid();
        m_toggleGrid->setEnabled(pGrid->hasSubGrids());
    }
}

void KisPerspectiveGridManager::setup(KActionCollection * collection)
{


    m_toggleGrid  = new KToggleAction(i18n("Show Perspective Grid"), this);
    collection->addAction("view_toggle_perspective_grid", m_toggleGrid);
    connect(m_toggleGrid, SIGNAL(triggered()), this, SLOT(toggleVisibility()));

    m_toggleGrid->setCheckedState(KGuiItem(i18n("Hide Perspective Grid")));
    m_toggleGrid->setChecked(false);
    m_gridClear  = new KAction(i18n("Clear Perspective Grid"), this);
    collection->addAction("view_clear_perspective_grid", m_gridClear);
    connect(m_gridClear, SIGNAL(triggered()), this, SLOT(clearPerspectiveGrid()));
}

void KisPerspectiveGridManager::clearPerspectiveGrid()
{
    KisImageWSP image = m_view->image();
    if (image) {
        image->perspectiveGrid()->clearSubGrids();
        m_view->canvas()->update();
        m_toggleGrid->setChecked(false);
        m_toggleGrid->setEnabled(false);
    }
}

void KisPerspectiveGridManager::startEdition()
{
    m_toggleEdition = true;
    m_toggleGrid->setEnabled(false);
}

void KisPerspectiveGridManager::stopEdition()
{
    m_toggleEdition = false;
    if (m_view->resourceProvider()->currentImage()->perspectiveGrid()->hasSubGrids()) {
        m_toggleGrid->setEnabled(true);
        m_toggleGrid->setChecked(true);
    } else {
        m_toggleGrid->setChecked(false);
    }
}

#define pixelToView(point) \
    converter.documentToView(image->pixelToDocument(point))

void KisPerspectiveGridManager::drawDecoration(QPainter& gc, const QPoint& documentOffset, const QRect& area, const KoViewConverter &converter)
{
    Q_UNUSED(documentOffset);
    Q_UNUSED(area);

    KisImageWSP image = m_view->resourceProvider()->currentImage();


    if (image && !m_toggleEdition) {
        KisPerspectiveGrid* pGrid = image->perspectiveGrid();

        KisConfig cfg;
        QPen mainPen = KisGridPainterConfiguration::mainPen();
        QPen subdivisionPen = KisGridPainterConfiguration::subdivisionPen();

        for (QList<KisSubPerspectiveGrid*>::const_iterator it = pGrid->begin(); it != pGrid->end(); ++it) {
            const KisSubPerspectiveGrid* grid = *it;
            gc.setPen(subdivisionPen);
            // 1 -> top-left corner
            // 2 -> top-right corner
            // 3 -> bottom-right corner
            // 4 -> bottom-left corner
            // d12 line from top-left to top-right
            // note that the notion of top-left is purely theorical
            LineEquation d12 = LineEquation::Through(toKisVector2D(*grid->topLeft()), toKisVector2D(*grid->topRight()));
            QPointF v12 = QPointF(*grid->topLeft() - *grid->topRight());
            v12.setX(v12.x() / grid->subdivisions()); v12.setY(v12.y() / grid->subdivisions());
            LineEquation d23 = LineEquation::Through(toKisVector2D(*grid->topRight()), toKisVector2D(*grid->bottomRight()));
            QPointF v23 = QPointF(*grid->topRight() - *grid->bottomRight());
            v23.setX(v23.x() / grid->subdivisions()); v23.setY(v23.y() / grid->subdivisions());
            LineEquation d34 = LineEquation::Through(toKisVector2D(*grid->bottomRight()), toKisVector2D(*grid->bottomLeft()));
            LineEquation d41 = LineEquation::Through(toKisVector2D(*grid->bottomLeft()), toKisVector2D(*grid->topLeft()));

            KisVector2D horizVanishingPoint = d12.intersection(d34);
            KisVector2D vertVanishingPoint = d23.intersection(d41);

            for (int i = 1; i < grid->subdivisions(); i ++) {
                KisVector2D pol1 = toKisVector2D(*grid->topRight() + i * v12);
                LineEquation d1 = LineEquation::Through(pol1, vertVanishingPoint);
                KisVector2D pol1b =  d1.intersection(d34);
                gc.drawLine(pixelToView(toQPointF(pol1)), pixelToView(toQPointF(pol1b)));

                KisVector2D pol2 = toKisVector2D(*grid->bottomRight() + i * v23);
                LineEquation d2 = LineEquation::Through(pol2, horizVanishingPoint);
                KisVector2D pol2b = d2.intersection(d41);
                gc.drawLine(pixelToView(toQPointF(pol2)), pixelToView(toQPointF(pol2b)));
            }
            gc.setPen(mainPen);
            gc.drawLine(pixelToView(*grid->topLeft()), pixelToView(*grid->topRight()));
            gc.drawLine(pixelToView(*grid->topRight()), pixelToView(*grid->bottomRight()));
            gc.drawLine(pixelToView(*grid->bottomRight()), pixelToView(*grid->bottomLeft()));
            gc.drawLine(pixelToView(*grid->bottomLeft()), pixelToView(*grid->topLeft()));

        }
    }
}


#include "kis_perspective_grid_manager.moc"

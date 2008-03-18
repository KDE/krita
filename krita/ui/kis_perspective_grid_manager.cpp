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

#include "kis_perspective_grid_manager.h"


#include <kaction.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>

#include <KoViewConverter.h>

#include "kis_canvas2.h"
#include "kis_config.h"
#include "kis_image.h"
#include "kis_perspective_grid.h"
#include "kis_grid_painter_configuration.h"
#include "kis_view2.h"
#include "kis_resource_provider.h"

KisPerspectiveGridManager::KisPerspectiveGridManager(KisView2 * parent)
    : KisCanvasDecoration(parent)
    , m_toggleEdition(false)
    , m_view(parent)
{
}


KisPerspectiveGridManager::~KisPerspectiveGridManager()
{

}

void KisPerspectiveGridManager::updateGUI()
{
    KisImageSP image = m_view->image();


    if (image ) {
        KisPerspectiveGrid* pGrid = image->perspectiveGrid();
        m_toggleGrid->setEnabled( pGrid->hasSubGrids());
    }
}

void KisPerspectiveGridManager::setup(KActionCollection * collection)
{


    m_toggleGrid  = new KToggleAction(i18n("Show Perspective Grid"), this);
    collection->addAction("view_toggle_perspective_grid", m_toggleGrid );
    connect(m_toggleGrid, SIGNAL(triggered()), this, SLOT(toggleVisibility()));

    m_toggleGrid->setCheckedState(KGuiItem(i18n("Hide Perspective Grid")));
    m_toggleGrid->setChecked(false);
    m_gridClear  = new KAction(i18n("Clear Perspective Grid"), this);
    collection->addAction("view_clear_perspective_grid", m_gridClear );
    connect(m_gridClear, SIGNAL(triggered()), this, SLOT(clearPerspectiveGrid()));
}

void KisPerspectiveGridManager::clearPerspectiveGrid()
{
    KisImageSP image = m_view->image();
    if (image ) {
        image->perspectiveGrid()->clearSubGrids();
        m_view->canvas()->update();
        m_toggleGrid->setChecked(false);
        m_toggleGrid->setEnabled(false);
    }
}

void KisPerspectiveGridManager::startEdition()
{
    m_toggleEdition = true;
    m_toggleGrid->setEnabled( false );
}

void KisPerspectiveGridManager::stopEdition()
{
    m_toggleEdition = false;
    m_toggleGrid->setEnabled( true );
}

#define pixelToView(point) \
    converter.documentToView(image->pixelToDocument(point))

void KisPerspectiveGridManager::drawDecoration(QPainter& gc, const QRect& area, const KoViewConverter &converter)
{

    KisImageSP image = m_view->resourceProvider()->currentImage();


    if (image && m_toggleEdition) {
        KisPerspectiveGrid* pGrid = image->perspectiveGrid();

        KisConfig cfg;
        QPen mainPen = KisGridPainterConfiguration::mainPen();
        QPen subdivisionPen = KisGridPainterConfiguration::subdivisionPen();

        for( QList<KisSubPerspectiveGrid*>::const_iterator it = pGrid->begin(); it != pGrid->end(); ++it)
        {
            const KisSubPerspectiveGrid* grid = *it;
            gc.setPen(subdivisionPen );
            // 1 -> top-left corner
            // 2 -> top-right corner
            // 3 -> bottom-right corner
            // 4 -> bottom-left corner
            // d12 line from top-left to top-right
            // note that the notion of top-left is purely theorical
            KisPerspectiveMath::LineEquation d12 = KisPerspectiveMath::computeLineEquation( grid->topLeft().data(), grid->topRight().data() ) ;
            QPointF v12 = QPointF(*grid->topLeft() - *grid->topRight());
            v12.setX( v12.x() / grid->subdivisions()); v12.setY( v12.y() / grid->subdivisions() );
            KisPerspectiveMath::LineEquation d23 = KisPerspectiveMath::computeLineEquation( grid->topRight().data(), grid->bottomRight().data() );
            QPointF v23 = QPointF(*grid->topRight() - *grid->bottomRight());
            v23.setX( v23.x() / grid->subdivisions()); v23.setY( v23.y() / grid->subdivisions() );
            KisPerspectiveMath::LineEquation d34 = KisPerspectiveMath::computeLineEquation( grid->bottomRight().data(), grid->bottomLeft().data() );
            KisPerspectiveMath::LineEquation d41 = KisPerspectiveMath::computeLineEquation( grid->bottomLeft().data(), grid->topLeft().data() );

            QPointF horizVanishingPoint = KisPerspectiveMath::computeIntersection(d12,d34);
            QPointF vertVanishingPoint = KisPerspectiveMath::computeIntersection(d23,d41);

            for(int i = 1; i < grid->subdivisions(); i ++)
            {
                QPointF pol1 = *grid->topRight() + i * v12;
                KisPerspectiveMath::LineEquation d1 = KisPerspectiveMath::computeLineEquation( &pol1, &vertVanishingPoint );
                QPointF pol1b =  KisPerspectiveMath::computeIntersection(d1,d34);
                gc.drawLine( pixelToView(pol1.toPoint()), pixelToView(pol1b.toPoint() ));
        
                QPointF pol2 = *grid->bottomRight() + i * v23;
                KisPerspectiveMath::LineEquation d2 = KisPerspectiveMath::computeLineEquation( &pol2, &horizVanishingPoint );
                QPointF pol2b = KisPerspectiveMath::computeIntersection(d2,d41);
                gc.drawLine( pixelToView(pol2.toPoint()), pixelToView(pol2b.toPoint()) );
            }
            gc.setPen(mainPen);
            gc.drawLine( pixelToView( *grid->topLeft()), pixelToView(  *grid->topRight() ) );
            gc.drawLine( pixelToView( *grid->topRight()), pixelToView( *grid->bottomRight()) );
            gc.drawLine( pixelToView( *grid->bottomRight()), pixelToView( *grid->bottomLeft()) );
            gc.drawLine( pixelToView( *grid->bottomLeft()), pixelToView( *grid->topLeft()) );

        }
    }
}


#include "kis_perspective_grid_manager.moc"

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

#include "kis_perspective_grid_manager.h"

#include <kaction.h>
#include <klocale.h>

#include "kis_image.h"
#include "kis_grid_drawer.h"
#include "kis_view.h"

KisPerspectiveGridManager::KisPerspectiveGridManager(KisView * parent)
    : QObject(), m_view(parent)
{
    m_grid.topLeft = QPoint( 10, 20 );
    m_grid.topRight = QPoint( 40, 20 );
    m_grid.bottomLeft = QPoint( 50, 60 );
    m_grid.bottomRight = QPoint( 80, 50 );
    
}


KisPerspectiveGridManager::~KisPerspectiveGridManager()
{
    
}

void KisPerspectiveGridManager::updateGUI()
{

}

void KisPerspectiveGridManager::setup(KActionCollection * collection)
{
    m_toggleGrid = new KToggleAction(i18n("Show Perspective Grid"), "", this, SLOT(toggleGrid()), collection, "view_toggle_perspective_grid");
    m_toggleGrid->setCheckedState(KGuiItem(i18n("Hide Perspective Grid")));
    m_toggleGrid->setChecked(false);
}

void KisPerspectiveGridManager::toggleGrid()
{
    m_view->updateCanvas();
}

void KisPerspectiveGridManager::drawGrid(QRect wr, QPainter *p, bool openGL )
{
    KisImageSP image = m_view->canvasSubject()->currentImg();

    if (image) {
        if (m_toggleGrid->isChecked())
        {
            GridDrawer *gridDrawer = 0;

            if (openGL) {
                gridDrawer = new OpenGLGridDrawer();
            } else {
                Q_ASSERT(p);

                if (p) {
                    gridDrawer = new QPainterGridDrawer(p);
                }
            }

            Q_ASSERT(gridDrawer != 0);

            if (gridDrawer) {
                gridDrawer->drawPerspectiveGrid(image, wr, m_grid );
                delete gridDrawer;
            }
        }
    }
}


#include "kis_perspective_grid_manager.moc"

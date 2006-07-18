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
#include <kmessagebox.h>

#include "kis_image.h"
#include "kis_grid_drawer.h"
#include "kis_perspective_grid.h"
#include "kis_view.h"

KisPerspectiveGridManager::KisPerspectiveGridManager(KisView * parent)
    : QObject(), m_view(parent)
{
    
}


KisPerspectiveGridManager::~KisPerspectiveGridManager()
{
    
}

void KisPerspectiveGridManager::updateGUI()
{

}

void KisPerspectiveGridManager::setup(KActionCollection * collection)
{
    kdDebug() << "KisPerspectiveGridManager::setup(KActionCollection * collection)" << endl;
    m_toggleGrid = new KToggleAction(i18n("Show Perspective Grid"), "", this, SLOT(toggleGrid()), collection, "view_toggle_perspective_grid");
    m_toggleGrid->setCheckedState(KGuiItem(i18n("Hide Perspective Grid")));
    m_toggleGrid->setChecked(false);
}

void KisPerspectiveGridManager::toggleGrid()
{
    KisImageSP image = m_view->canvasSubject()->currentImg();

    
    if (image && m_toggleGrid->isChecked()) {
        KisPerspectiveGrid* pGrid = image->perspectiveGrid();

        if(!pGrid->hasSubGrids())
        {
            KMessageBox::error(0, i18n("Before displaying the perspective grid, you need to initialize it with the perspective grid tool"), i18n("No perspective grid to display") );
            m_toggleGrid->setChecked(false);
        }
    }
    m_view->updateCanvas();
}

void KisPerspectiveGridManager::drawGrid(QRect wr, QPainter *p, bool openGL )
{
    KisImageSP image = m_view->canvasSubject()->currentImg();

    
    if (image && m_toggleGrid->isChecked()) {
        KisPerspectiveGrid* pGrid = image->perspectiveGrid();

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

        for( QValueList<KisSubPerspectiveGrid*>::const_iterator it = pGrid->begin(); it != pGrid->end(); ++it)
        {
            gridDrawer->drawPerspectiveGrid(image, wr, *it );
        }
        delete gridDrawer;
    }
}


#include "kis_perspective_grid_manager.moc"

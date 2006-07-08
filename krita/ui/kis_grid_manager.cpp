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

#include "kis_grid_manager.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_GL
#include <qgl.h>
#endif

#include <qradiobutton.h>

#include <kaction.h>
#include <kdialogbase.h>
#include <klocale.h>


#include "kis_config.h"
#include "kis_grid_drawer.h"
#include "kis_image.h"
#include "kis_view.h"

KisGridManager::KisGridManager(KisView * parent)
    : QObject(parent), m_view(parent)
{

}

KisGridManager::~KisGridManager()
{

}

void KisGridManager::setup(KActionCollection * collection)
{
    m_toggleGrid = new KToggleAction(i18n("Show Grid"), "", this, SLOT(toggleGrid()), collection, "view_toggle_grid");
    m_toggleGrid->setCheckedState(KGuiItem(i18n("Hide Grid")));
    m_toggleGrid->setChecked(false);

    // Fast grid config
    m_gridFastConfig1x1 = new KAction(i18n("1x1"), 0, "", this, SLOT(fastConfig1x1()), collection, "view_fast_grid_1x1");
    m_gridFastConfig2x2 = new KAction(i18n("2x2"), 0, "", this, SLOT(fastConfig2x2()), collection, "view_fast_grid_2x2");
    m_gridFastConfig5x5 = new KAction(i18n("5x5"), 0, "", this, SLOT(fastConfig5x5()), collection, "view_fast_grid_5x5");
    m_gridFastConfig10x10 = new KAction(i18n("10x10"), 0, "", this, SLOT(fastConfig10x10()), collection, "view_fast_grid_10x10");
    m_gridFastConfig20x20 = new KAction(i18n("20x20"), 0, "", this, SLOT(fastConfig20x20()), collection, "view_fast_grid_20x20");
    m_gridFastConfig40x40 = new KAction(i18n("40x40"), 0, "", this, SLOT(fastConfig40x40()), collection, "view_fast_grid_40x40");
}

void KisGridManager::updateGUI()
{

}

void KisGridManager::toggleGrid()
{
    m_view->updateCanvas();
}

void KisGridManager::fastConfig1x1()
{
    KisConfig cfg;
    cfg.setGridHSpacing(1);
    cfg.setGridVSpacing(1);
    m_view->updateCanvas();
}

void KisGridManager::fastConfig2x2()
{
    KisConfig cfg;
    cfg.setGridHSpacing(2);
    cfg.setGridVSpacing(2);
    m_view->updateCanvas();
}

void KisGridManager::fastConfig5x5()
{
    KisConfig cfg;
    cfg.setGridHSpacing(5);
    cfg.setGridVSpacing(5);
    m_view->updateCanvas();
}

void KisGridManager::fastConfig10x10()
{
    KisConfig cfg;
    cfg.setGridHSpacing(10);
    cfg.setGridVSpacing(10);
    m_view->updateCanvas();
}

void KisGridManager::fastConfig20x20()
{
    KisConfig cfg;
    cfg.setGridHSpacing(20);
    cfg.setGridVSpacing(20);
    m_view->updateCanvas();
}

void KisGridManager::fastConfig40x40()
{
    KisConfig cfg;
    cfg.setGridHSpacing(40);
    cfg.setGridVSpacing(40);
    m_view->updateCanvas();
}

void KisGridManager::drawGrid(QRect wr, QPainter *p, bool openGL)
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
                gridDrawer->drawGrid(image, wr);
                delete gridDrawer;
            }
        }
    }
}

#include "kis_grid_manager.moc"

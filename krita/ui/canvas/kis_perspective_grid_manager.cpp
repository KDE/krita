/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006,2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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
#include <ktoggleaction.h>
#include <kactioncollection.h>

#include "canvas/kis_canvas2.h"
#include "kis_config.h"
#include "kis_image.h"
#include "kis_perspective_grid.h"
#include "KisViewManager.h"
#include "kis_perspective_grid_decoration.h"
#include "KisView.h"


/***************************************************************/
/*                 KisPerspectiveGridManager                   */
/***************************************************************/

KisPerspectiveGridManager::KisPerspectiveGridManager(KisViewManager * parent) : QObject(parent)
    , m_imageView(0)
{
}


KisPerspectiveGridManager::~KisPerspectiveGridManager()
{
}

void KisPerspectiveGridManager::updateGUI()
{
    if (m_imageView) {
        KisImageWSP image = m_imageView->image();

        if (image) {
            KisPerspectiveGrid* pGrid = image->perspectiveGrid();
            m_toggleGrid->setEnabled(pGrid->hasSubGrids());
            m_toggleGrid->setChecked(decoration()->visible());
            m_gridClear->setEnabled(pGrid->hasSubGrids());
        }
    } else {
        m_toggleGrid->setEnabled(false);
        m_gridClear->setEnabled(false);
    }
}

void KisPerspectiveGridManager::setup(KActionCollection * collection)
{
    m_toggleGrid  = new KToggleAction(i18n("Show Perspective Grid"), this);
    collection->addAction("view_toggle_perspective_grid", m_toggleGrid);

    m_toggleGrid->setCheckedState(KGuiItem(i18n("Hide Perspective Grid")));
    m_toggleGrid->setChecked(false);
    m_gridClear  = new KAction(i18n("Clear Perspective Grid"), this);
    collection->addAction("view_clear_perspective_grid", m_gridClear);
    connect(m_gridClear, SIGNAL(triggered()), this, SLOT(clearPerspectiveGrid()));
    updateGUI();
}

void KisPerspectiveGridManager::clearPerspectiveGrid()
{
    if (m_imageView) {
        KisImageWSP image = m_imageView->image();
        if (image) {
            image->perspectiveGrid()->clearSubGrids();
            m_imageView->canvasBase()->canvasWidget()->update();
            updateGUI();
        }
    }
}

void KisPerspectiveGridManager::startEdition()
{
    m_toggleGrid->setEnabled(false);
}

void KisPerspectiveGridManager::stopEdition()
{
    updateGUI();
}

void KisPerspectiveGridManager::setView(QPointer<KisView> imageView)
{
    m_toggleGrid->disconnect();

    m_imageView = imageView;
    if (m_imageView && !decoration()) {
        KisPerspectiveGridDecoration* newDecoration = new KisPerspectiveGridDecoration(m_imageView);
        newDecoration->setVisible(true);
        m_imageView->canvasBase()->addDecoration(newDecoration);
    }
    connect(m_toggleGrid, SIGNAL(triggered()), decoration(), SLOT(toggleVisibility()));
    updateGUI();
}

KisPerspectiveGridDecoration* KisPerspectiveGridManager::decoration()
{
    if (m_imageView && m_imageView->canvasBase()) {
        return qobject_cast<KisPerspectiveGridDecoration*>(m_imageView->canvasBase()->decoration("perspectiveGrid"));
    }
    return 0;
}



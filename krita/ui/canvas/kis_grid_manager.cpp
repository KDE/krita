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

#include <QAction>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <klocalizedstring.h>

#include <kis_icon.h>

#include "kis_coordinates_converter.h"
#include "kis_config.h"
#include "kis_grid_painter_configuration.h"
#include "kis_grid_decoration.h"
#include "kis_image.h"
#include "KisViewManager.h"
#include "KisDocument.h"
#include "KisView.h"

KisGridManager::KisGridManager(KisViewManager * parent) : QObject(parent)
{

}

KisGridManager::~KisGridManager()
{

}

void KisGridManager::setup(KisActionManager* actionManager)
{
    m_toggleGrid = actionManager->createAction("view_grid");
    m_toggleGrid->setActivationFlags(KisAction::ACTIVE_NODE);

    m_toggleSnapToGrid  = actionManager->createAction("view_snap_to_grid");
    m_toggleSnapToGrid->setActivationFlags(KisAction::ACTIVE_NODE);
    connect(m_toggleSnapToGrid, SIGNAL(triggered()), this, SLOT(toggleSnapToGrid()));
}

void KisGridManager::updateGUI()
{

}

void KisGridManager::setView(QPointer<KisView> imageView)
{
    if (m_imageView) {
        m_toggleGrid->disconnect();
        m_gridDecoration = 0;
    }

    m_imageView = imageView;

    if (imageView) {
        m_gridDecoration = qobject_cast<KisGridDecoration*>(imageView->canvasBase()->decoration("grid"));
        if (!m_gridDecoration) {
            m_gridDecoration = new KisGridDecoration(imageView);
            imageView->canvasBase()->addDecoration(m_gridDecoration);
        }
        checkVisibilityAction(m_gridDecoration->visible());
        connect(m_toggleGrid, SIGNAL(triggered()), m_gridDecoration, SLOT(toggleVisibility()));
    }
}

void KisGridManager::checkVisibilityAction(bool check)
{
    m_toggleGrid->setChecked(check);
}

void KisGridManager::toggleSnapToGrid()
{
    if (m_imageView) {
        m_imageView->document()->gridData().setSnapToGrid(m_toggleSnapToGrid->isChecked());
        m_imageView->canvasBase()->updateCanvas();
    }
}


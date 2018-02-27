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

#include "kis_canvas2.h"
#include "kis_coordinates_converter.h"
#include "kis_config.h"
#include "kis_grid_decoration.h"
#include "kis_image.h"
#include "KisViewManager.h"
#include "KisDocument.h"
#include "KisView.h"
#include "kis_grid_config.h"
#include "kis_signals_blocker.h"


KisGridManager::KisGridManager(KisViewManager * parent)
    : QObject(parent)
{

}

KisGridManager::~KisGridManager()
{

}

void KisGridManager::setGridConfig(const KisGridConfig &config)
{
    setGridConfigImpl(config, true);
}

void KisGridManager::setGridConfigImpl(const KisGridConfig &config, bool /*emitModified*/)
{
    if (!m_imageView) return;

    config.saveStaticData();
    m_imageView->document()->setGridConfig(config);

    m_gridDecoration->setGridConfig(config);
    m_gridDecoration->setVisible(config.showGrid());

    m_toggleGrid->setChecked(config.showGrid());
    m_toggleSnapToGrid->setChecked(config.snapToGrid());
}

void KisGridManager::setup(KisActionManager* actionManager)
{
    m_toggleGrid = actionManager->createAction("view_grid");
    connect(m_toggleGrid, SIGNAL(toggled(bool)), this, SLOT(slotChangeGridVisibilityTriggered(bool)));

    m_toggleSnapToGrid  = actionManager->createAction("view_snap_to_grid");
    connect(m_toggleSnapToGrid, SIGNAL(toggled(bool)), this, SLOT(slotSnapToGridTriggered(bool)));
}

void KisGridManager::updateGUI()
{

}

void KisGridManager::setView(QPointer<KisView> imageView)
{
    if (m_imageView) {
        m_gridDecoration = 0;
    }

    m_imageView = imageView;

    if (imageView) {
        m_gridDecoration = qobject_cast<KisGridDecoration*>(imageView->canvasBase()->decoration("grid").data());
        if (!m_gridDecoration) {
            m_gridDecoration = new KisGridDecoration(imageView);
            imageView->canvasBase()->addDecoration(m_gridDecoration);
        }

        KisGridConfig config = imageView->document()->gridConfig();
        setGridConfigImpl(config, false);

        KisSignalsBlocker blocker(m_toggleGrid, m_toggleSnapToGrid);
        Q_UNUSED(blocker);
        m_toggleGrid->setChecked(config.showGrid());
        m_toggleSnapToGrid->setChecked(config.snapToGrid());
    }
}

void KisGridManager::slotChangeGridVisibilityTriggered(bool value)
{
    if (!m_imageView) return;

    KisGridConfig config = m_imageView->document()->gridConfig();
    config.setShowGrid(value);

    setGridConfig(config);
    emit sigRequestUpdateGridConfig(config);
}

void KisGridManager::slotSnapToGridTriggered(bool value)
{
    if (!m_imageView) return;

    KisGridConfig config = m_imageView->document()->gridConfig();
    config.setSnapToGrid(value);

    setGridConfig(config);
    emit sigRequestUpdateGridConfig(config);
}


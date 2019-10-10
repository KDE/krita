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
#include <kis_signal_auto_connection.h>

struct KisGridManager::Private
{
    KisAction *toggleGrid;
    KisAction* toggleSnapToGrid;

    QPointer<KisView> imageView;
    KisGridDecoration* gridDecoration;

    bool blockModifiedSignal;
    KisSignalAutoConnectionsStore connections;
};

KisGridManager::KisGridManager(KisViewManager * parent)
    : QObject(parent)
    , m_d(new Private)
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
    if (!m_d->imageView) return;

    config.saveStaticData();
    m_d->imageView->document()->setGridConfig(config);

    m_d->gridDecoration->setGridConfig(config);
    m_d->gridDecoration->setVisible(config.showGrid());

    m_d->toggleGrid->setChecked(config.showGrid());
    m_d->toggleSnapToGrid->setChecked(config.snapToGrid());
}

void KisGridManager::setup(KisActionManager* actionManager)
{
    m_d->toggleGrid = actionManager->createAction("view_grid");
    connect(m_d->toggleGrid, SIGNAL(toggled(bool)), this, SLOT(slotChangeGridVisibilityTriggered(bool)));

    m_d->toggleSnapToGrid  = actionManager->createAction("view_snap_to_grid");
    connect(m_d->toggleSnapToGrid, SIGNAL(toggled(bool)), this, SLOT(slotSnapToGridTriggered(bool)));
}

void KisGridManager::updateGUI()
{

}

void KisGridManager::setView(QPointer<KisView> imageView)
{
    if (m_d->imageView) {
        m_d->connections.clear();
        m_d->gridDecoration = 0;
    }

    m_d->imageView = imageView;

    if (imageView) {
        if (!imageView->document()) {
            return;
        }

        m_d->gridDecoration = qobject_cast<KisGridDecoration*>(imageView->canvasBase()->decoration("grid").data());
        if (!m_d->gridDecoration) {
            m_d->gridDecoration = new KisGridDecoration(imageView);
            imageView->canvasBase()->addDecoration(m_d->gridDecoration);
        }
        m_d->connections.addConnection(imageView->document(), SIGNAL(sigGridConfigChanged(KisGridConfig)),
                                       this, SIGNAL(sigRequestUpdateGridConfig(KisGridConfig)));

        KisGridConfig config = imageView->document()->gridConfig();
        setGridConfigImpl(config, false);

        KisSignalsBlocker blocker(m_d->toggleGrid, m_d->toggleSnapToGrid);
        Q_UNUSED(blocker);
        m_d->toggleGrid->setChecked(config.showGrid());
        m_d->toggleSnapToGrid->setChecked(config.snapToGrid());
    }
}

void KisGridManager::slotChangeGridVisibilityTriggered(bool value)
{
    if (!m_d->imageView) return;

    KisGridConfig config = m_d->imageView->document()->gridConfig();
    config.setShowGrid(value);

    setGridConfig(config);
    emit sigRequestUpdateGridConfig(config);
}

void KisGridManager::slotSnapToGridTriggered(bool value)
{
    if (!m_d->imageView) return;

    KisGridConfig config = m_d->imageView->document()->gridConfig();
    config.setSnapToGrid(value);

    setGridConfig(config);
    emit sigRequestUpdateGridConfig(config);
}


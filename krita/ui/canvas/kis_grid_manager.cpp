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

#include <kaction.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <klocale.h>

#include <KoIcon.h>

#include "kis_coordinates_converter.h"
#include "kis_config.h"
#include "kis_grid_painter_configuration.h"
#include "kis_grid_decoration.h"
#include "kis_image.h"
#include "KisViewManager.h"
#include "KisDocument.h"
#include "KisView.h"
#include "kis_action.h"

KisGridManager::KisGridManager(KisViewManager * parent) : QObject(parent)
{

}

KisGridManager::~KisGridManager()
{

}

void KisGridManager::setup(KisActionManager* actionManager)
{
    //there is no grid by default
//     m_view->document()->gridData().setShowGrid(false);
// 
//     KisConfig config;
//     m_view->document()->gridData().setGrid(config.getGridHSpacing(), config.getGridVSpacing());

    m_toggleGrid = new KisAction(koIcon("view-grid"), i18n("Show Grid"), 0);
    m_toggleGrid->setCheckable(true);
    m_toggleGrid->setActivationFlags(KisAction::ACTIVE_NODE);
    m_toggleGrid->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Apostrophe));
    actionManager->addAction("view_grid", m_toggleGrid);

    m_toggleSnapToGrid  = new KisAction(i18n("Snap To Grid"), this);
    m_toggleSnapToGrid->setCheckable(true);
    m_toggleSnapToGrid->setActivationFlags(KisAction::ACTIVE_NODE);
    m_toggleSnapToGrid->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Semicolon));
    actionManager->addAction("view_snap_to_grid", m_toggleSnapToGrid);
    connect(m_toggleSnapToGrid, SIGNAL(triggered()), this, SLOT(toggleSnapToGrid()));

    // Fast grid config
    m_gridFastConfig1x1  = new KisAction(i18n("1x1"), this);
    m_gridFastConfig1x1->setActivationFlags(KisAction::ACTIVE_NODE);
    actionManager->addAction("view_fast_grid_1x1", m_gridFastConfig1x1);
    connect(m_gridFastConfig1x1, SIGNAL(triggered()), this, SLOT(fastConfig1x1()));

    m_gridFastConfig2x2  = new KisAction(i18n("2x2"), this);
    m_gridFastConfig2x2->setActivationFlags(KisAction::ACTIVE_NODE);
    actionManager->addAction("view_fast_grid_2x2", m_gridFastConfig2x2);
    connect(m_gridFastConfig2x2, SIGNAL(triggered()), this, SLOT(fastConfig2x2()));

    m_gridFastConfig4x4  = new KisAction(i18n("4x4"), this);
    m_gridFastConfig4x4->setActivationFlags(KisAction::ACTIVE_NODE);
    actionManager->addAction("view_fast_grid_4x4", m_gridFastConfig4x4);
    connect(m_gridFastConfig4x4, SIGNAL(triggered()), this, SLOT(fastConfig4x4()));
    
    m_gridFastConfig5x5  = new KisAction(i18n("5x5"), this);
    m_gridFastConfig5x5->setActivationFlags(KisAction::ACTIVE_NODE);
    actionManager->addAction("view_fast_grid_5x5", m_gridFastConfig5x5);
    connect(m_gridFastConfig5x5, SIGNAL(triggered()), this, SLOT(fastConfig5x5()));

    m_gridFastConfig8x8  = new KisAction(i18n("8x8"), this);
    m_gridFastConfig8x8->setActivationFlags(KisAction::ACTIVE_NODE);
    actionManager->addAction("view_fast_grid_8x8", m_gridFastConfig8x8);
    connect(m_gridFastConfig8x8, SIGNAL(triggered()), this, SLOT(fastConfig8x8()));
        
    m_gridFastConfig10x10  = new KisAction(i18n("10x10"), this);
    m_gridFastConfig10x10->setActivationFlags(KisAction::ACTIVE_NODE);
    actionManager->addAction("view_fast_grid_10x10", m_gridFastConfig10x10);
    connect(m_gridFastConfig10x10, SIGNAL(triggered()), this, SLOT(fastConfig10x10()));

    m_gridFastConfig16x16  = new KisAction(i18n("16x16"), this);
    m_gridFastConfig16x16->setActivationFlags(KisAction::ACTIVE_NODE);
    actionManager->addAction("view_fast_grid_16x16", m_gridFastConfig16x16);
    connect(m_gridFastConfig16x16, SIGNAL(triggered()), this, SLOT(fastConfig16x16()));

    m_gridFastConfig20x20  = new KisAction(i18n("20x20"), this);
    m_gridFastConfig20x20->setActivationFlags(KisAction::ACTIVE_NODE);
    actionManager->addAction("view_fast_grid_20x20", m_gridFastConfig20x20);
    connect(m_gridFastConfig20x20, SIGNAL(triggered()), this, SLOT(fastConfig20x20()));

    m_gridFastConfig32x32  = new KisAction(i18n("32x32"), this);
    m_gridFastConfig32x32->setActivationFlags(KisAction::ACTIVE_NODE);
    actionManager->addAction("view_fast_grid_32x32", m_gridFastConfig32x32);
    connect(m_gridFastConfig32x32, SIGNAL(triggered()), this, SLOT(fastConfig32x32()));
    
    m_gridFastConfig40x40  = new KisAction(i18n("40x40"), this);
    m_gridFastConfig40x40->setActivationFlags(KisAction::ACTIVE_NODE);
    actionManager->addAction("view_fast_grid_40x40", m_gridFastConfig40x40);
    connect(m_gridFastConfig40x40, SIGNAL(triggered()), this, SLOT(fastConfig40x40()));
    
    m_gridFastConfig64x64  = new KisAction(i18n("64x64"), this);
    m_gridFastConfig64x64->setActivationFlags(KisAction::ACTIVE_NODE);
    actionManager->addAction("view_fast_grid_64x64", m_gridFastConfig64x64);
    connect(m_gridFastConfig64x64, SIGNAL(triggered()), this, SLOT(fastConfig64x64()));
}

void KisGridManager::updateGUI()
{

}

void KisGridManager::setView(QPointer< KisView > imageView)
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

void KisGridManager::setFastConfig(int size)
{
    if (m_imageView) {
        m_imageView->document()->gridData().setGrid(size, size);
        m_imageView->canvasBase()->updateCanvas();
    }
}

void KisGridManager::fastConfig1x1()
{
    setFastConfig(1);
}

void KisGridManager::fastConfig2x2()
{
    setFastConfig(2);
}

void KisGridManager::fastConfig5x5()
{
    setFastConfig(5);
}

void KisGridManager::fastConfig10x10()
{
    setFastConfig(10);
}

void KisGridManager::fastConfig20x20()
{
    setFastConfig(20);
}

void KisGridManager::fastConfig40x40()
{
    setFastConfig(40);
}

void KisGridManager::fastConfig4x4()
{
    setFastConfig(4);
}

void KisGridManager::fastConfig8x8()
{
    setFastConfig(4);
}

void KisGridManager::fastConfig16x16()
{
    setFastConfig(16);
}

void KisGridManager::fastConfig32x32()
{
    setFastConfig(32);
}

void KisGridManager::fastConfig64x64()
{
    setFastConfig(64);
}


#include "kis_grid_manager.moc"

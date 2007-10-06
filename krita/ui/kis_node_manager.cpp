/*
 *  Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_node_manager.h"

#include <kactioncollection.h>

#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoShape.h>
#include <KoShapeLayer.h>

#include <kis_types.h>
#include <kis_node.h>
#include <kis_selection.h>

#include "kis_canvas2.h"
#include "kis_resource_provider.h"
#include "kis_view2.h"
#include "kis_doc2.h"

struct KisNodeManager::Private {
    KisView2 * view;
    KisDoc2 * doc;
    KisNodeSP activeNode;
};

KisNodeManager::KisNodeManager( KisView2 * view, KisDoc2 * doc )
    : m_d( new Private() )
{
    m_d->view = view;
    m_d->doc = doc;
}

void KisNodeManager::setup(KActionCollection * actionCollection)
{
}

KisNodeManager::~KisNodeManager()
{
    delete m_d;
}

void KisNodeManager::updateGUI()
{
    // enable/disable all relevant actions
}


KisNodeSP KisNodeManager::activeNode()
{
    return m_d->activeNode;
}


void KisNodeManager::activateNode( KisNodeSP node )
{

    Q_ASSERT( node );

    // XXX: Set the selection on the shape manager to the active layer
    // and set call KoSelection::setActiveLayer( KoShapeLayer* layer )
    // with the parent of the active layer.

    Q_ASSERT( m_d->view );
    Q_ASSERT( m_d->view->canvasBase() );
    Q_ASSERT( m_d->view->canvasBase()->globalShapeManager() );

    KoSelection * selection = m_d->view->canvasBase()->globalShapeManager()->selection();
    Q_ASSERT( selection );

    KoShape * shape = m_d->view->document()->shapeForNode( node );
    Q_ASSERT( shape );

    selection->deselectAll();
    selection->select(shape);

    Q_ASSERT( node->parent() );
    KoShape * parentShape = m_d->view->document()->shapeForNode( static_cast<KisNode*>( node->parent().data() ) );
    Q_ASSERT( parentShape );

    KoShapeLayer * shapeLayer = dynamic_cast<KoShapeLayer*>( parentShape );
    Q_ASSERT( shapeLayer );

    // So the KoShapeController class can set the right parent on
    // layers we add.
    selection->setActiveLayer( shapeLayer );

    m_d->activeNode = node;
    emit sigNodeActivated( node );
    nodesUpdated();
}

void KisNodeManager::nodesUpdated()
{

    KisNodeSP node = activeNode();
    if (!node) return;

    m_d->view->updateGUI();
    m_d->view->resourceProvider()->slotNodeActivated( node );

}

#include "kis_node_manager.moc"


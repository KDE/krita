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
#include <kis_layer.h>
#include <kis_mask.h>
#include <kis_image.h>

#include "kis_canvas2.h"
#include "kis_resource_provider.h"
#include "kis_view2.h"
#include "kis_doc2.h"
#include "kis_mask_manager.h"
#include "kis_layer_manager.h"
#include "kis_selection_manager.h"

struct KisNodeManager::Private {

    ~Private()
    {
        delete layerManager;
        delete maskManager;
    }

    KisView2 * view;
    KisDoc2 * doc;
    KisLayerManager * layerManager;
    KisMaskManager * maskManager;
    KisNodeSP activeNode;
};

KisNodeManager::KisNodeManager( KisView2 * view, KisDoc2 * doc )
    : m_d( new Private() )
{
    m_d->view = view;
    m_d->doc = doc;
    m_d->layerManager = new KisLayerManager( view, doc );
    m_d->maskManager = new KisMaskManager( view );
}

KisNodeManager::~KisNodeManager()
{
    delete m_d;
}
void KisNodeManager::setup(KActionCollection * actionCollection)
{
    m_d->layerManager->setup( actionCollection );
    m_d->maskManager->setup( actionCollection );

}

void KisNodeManager::updateGUI()
{
    // enable/disable all relevant actions
    m_d->layerManager->updateGUI();
    m_d->maskManager->updateGUI();
}


KisNodeSP KisNodeManager::activeNode()
{
    return m_d->activeNode;
}

KisPaintDeviceSP KisNodeManager::activePaintDevice()
{
    if ( m_d->maskManager->activeMask() ) {
        return m_d->maskManager->activeDevice();
    }
    else {
        return m_d->layerManager->activeDevice();
    }
}

const KoColorSpace* KisNodeManager::activeColorSpace()
{
    Q_ASSERT(m_d->maskManager);
    
    if ( m_d->maskManager->activeDevice() ) {
        Q_ASSERT(m_d->maskManager->activeDevice());
        return m_d->maskManager->activeDevice()->colorSpace();
    }
    else {
        Q_ASSERT(m_d->layerManager);
        Q_ASSERT(m_d->layerManager->activeLayer());
        if(m_d->layerManager->activeLayer()->parentLayer())
            return m_d->layerManager->activeLayer()->parentLayer()->colorSpace();
        else
            return m_d->view->image()->colorSpace();
    }
}


KisLayerManager * KisNodeManager::layerManager()
{
    return m_d->layerManager;
}

KisMaskManager * KisNodeManager::maskManager()
{
    return m_d->maskManager;
}

bool allowAsChild( const QString & parentType, const QString & childType )
{
    // XXX_NODE: do we want to allow masks to apply on masks etc? Selections on masks?
    if ( parentType == "KisPaintLayer" || parentType == "KisAdjustmentLayer" || parentType == "KisShapeLayer" ) {
        if (childType == "KisFilterMask" || childType == "KisTransformationMask" || childType == "KisTransparencyMask" || childType == "KisSelectionMask") {
            return true;
        }
        return false;
    }
    else if ( parentType == "KisGroupLayer" ) {
        return true;
    }
    else if ( parentType == "KisFilterMask" || parentType == "KisTransformationMask" || parentType == "KisTransparencyMask" || parentType == "KisSelectionMask")
    {
        return false;
    }

    return true;
}

void KisNodeManager::getNewNodeLocation(const QString & nodeType, KisNodeSP &parent, KisNodeSP &above)
{
    KisNodeSP root = m_d->view->image()->root();
    KisNodeSP active = activeNode();
    if (!active)
        active = root->firstChild();
    
    // Find the first node above the current node that can have the desired
    // layer type as child. XXX_NODE: disable the menu entries for node types
    // that are not compatible with the active node type.
    while (active) {
        if ( allowAsChild(active->metaObject()->className(), nodeType) ) {
            parent = active;
            if ( activeNode()->parent() == parent ) {
                above = activeNode();
            }
            else {
                above = parent->firstChild();
            }
            return;
        }
        active = active->parent();
    }
    parent = root;
    above = parent->firstChild();
}

void KisNodeManager::createNode( const QString & nodeType)
{
    
    KisNodeSP parent;
    KisNodeSP above;

    getNewNodeLocation(nodeType, parent, above);

    // XXX: make factories for this kind of stuff,
    //      with a registry

    if ( nodeType == "KisPaintLayer" ) {
        m_d->layerManager->addLayer( parent, above );
    }
    else if ( nodeType == "KisGroupLayer" ) {
        m_d->layerManager->addGroupLayer( parent, above );
    }
    else if ( nodeType == "KisAdjustmentLayer" ) {
        m_d->layerManager->addAdjustmentLayer( parent, above );
    }
    else if ( nodeType == "KisShapeLayer" ) {
        m_d->layerManager->addShapeLayer( parent, above );
    }
    else if ( nodeType == "KisCloneLayer" ) {
        m_d->layerManager->addCloneLayer( parent, above );
    }
    else if ( nodeType == "KisTransparencyMask" ) {
        m_d->maskManager->createTransparencyMask( parent, above );
    }
    else if ( nodeType == "KisFilterMask" ) {
        m_d->maskManager->createFilterMask( parent, above );
    }
    else if ( nodeType == "KisTransformationMask" ) {
        m_d->maskManager->createTransformationMask( parent, above );
    }
    else if ( nodeType == "KisSelectionMask" ) {
        m_d->maskManager->createSelectionMask( parent, above );
    }

}

void KisNodeManager::activateNode( KisNodeSP node )
{
    Q_ASSERT( node );
    // XXX: Set the selection on the shape manager to the active layer
    // and set call KoSelection::setActiveLayer( KoShapeLayer* layer )
    // with the parent of the active layer.
    dbgUI << " activated node: " << node->name() ;

    Q_ASSERT( m_d->view );
    Q_ASSERT( m_d->view->canvasBase() );
    Q_ASSERT( m_d->view->canvasBase()->globalShapeManager() );

    KoSelection * selection = m_d->view->canvasBase()->globalShapeManager()->selection();
    Q_ASSERT( selection );

    KoShape * shape = m_d->view->document()->shapeForNode( node );
    if ( !shape ) {
        shape = m_d->view->document()->addShape( node->parent() );
    }

    selection->deselectAll();
    selection->select(shape);

    Q_ASSERT( node->parent() );
    
    KoShape * parentShape = m_d->view->document()->shapeForNode( node->parent() );
    if (!parentShape) {
        parentShape = m_d->view->document()->addShape( node->parent() );
    }

    m_d->activeNode = node;
    if ( KisLayerSP layer = dynamic_cast<KisLayer*>( node.data() ) ) {
        m_d->maskManager->activateMask( 0 );
        m_d->layerManager->activateLayer( layer );
    }
    else if ( KisMaskSP mask = dynamic_cast<KisMask*>( node.data() ) ) {
        m_d->maskManager->activateMask( mask );
        // XXX_NODE: for now, masks cannot be nested.
        m_d->layerManager->activateLayer( static_cast<KisLayer*>( node->parent().data() ) );
    }
    emit sigNodeActivated( node );
    nodesUpdated();
}

void KisNodeManager::nodesUpdated()
{
    KisNodeSP node = activeNode();
    if (!node) return;

    m_d->layerManager->layersUpdated();
    m_d->maskManager->masksUpdated();

    m_d->view->updateGUI();
    m_d->view->resourceProvider()->slotNodeActivated( node );
    m_d->view->selectionManager()->selectionChanged();

}

void KisNodeManager::nodeProperties( KisNodeSP node )
{
    if ( node->inherits( "KisLayer" ) ) {
        m_d->layerManager->layerProperties();
    }
    else if ( node->inherits( "KisMask") ) {
        m_d->maskManager->maskProperties();
    }
}

void KisNodeManager::nodeOpacityChanged( double opacity, bool final)
{
    m_d->layerManager->layerOpacity( opacity, final );
}

void KisNodeManager::nodeCompositeOpChanged( const KoCompositeOp* op )
{
    m_d->layerManager->layerCompositeOp( op );
}

void KisNodeManager::duplicateActiveNode( KisNodeSP node )
{
    if (node->inherits("KisLayer")) {
        m_d->layerManager->layerDuplicate();
    }
    else if (node->inherits("KisMask")) {
        m_d->maskManager->duplicateMask();
    }
}

void KisNodeManager::raiseNode()
{
    // The user sees the layer stack topsy-turvy, as a tree with the
    // root at the bottom instead of on top.    
    KisNodeSP node = activeNode();
    if (node->inherits("KisLayer")) {
        m_d->layerManager->layerLower();
    }
    else if (node->inherits("KisMask")) {
        m_d->maskManager->lowerMask();
    }
}

void KisNodeManager::lowerNode()
{
    // The user sees the layer stack topsy-turvy, as a tree with the
    // root at the bottom instead of on top.
    KisNodeSP node = activeNode();
    
    if (node->inherits("KisLayer")) {
        m_d->layerManager->layerRaise();
    }
    else if (node->inherits("KisMask")) {
        m_d->maskManager->raiseMask();
    }
}
    
void KisNodeManager::nodeToTop()
{
    KisNodeSP node = activeNode();
    if (node->inherits("KisLayer")) {
        m_d->layerManager->layerBack();
    }
    else if (node->inherits("KisMask")) {
        m_d->maskManager->maskToBottom();
    }

}

void KisNodeManager::nodeToBottom()
{
    KisNodeSP node = activeNode();
    if (node->inherits("KisLayer")) {
        m_d->layerManager->layerLower();
    }
    else if (node->inherits("KisMask")) {
        m_d->maskManager->maskToTop();
    }
}
    

#include "kis_node_manager.moc"


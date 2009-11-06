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
#include <kaction.h>

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

#include "canvas/kis_canvas2.h"
#include "kis_canvas_resource_provider.h"
#include "kis_view2.h"
#include "kis_doc2.h"
#include "kis_mask_manager.h"
#include "kis_group_layer.h"
#include "kis_layer_manager.h"
#include "kis_selection_manager.h"
#include "kis_node_commands_adapter.h"

struct KisNodeManager::Private {

    ~Private() {
        delete layerManager;
        delete maskManager;
    }

    KisView2 * view;
    KisDoc2 * doc;
    KisLayerManager * layerManager;
    KisMaskManager * maskManager;
    KisNodeSP activeNode;
    KisNodeManager* self;
    KisNodeCommandsAdapter* commandsAdapter;
    void slotLayersChanged(KisGroupLayerSP);
};

void KisNodeManager::Private::slotLayersChanged(KisGroupLayerSP layer)
{
    self->activateNode(layer->at(0));
}

KisNodeManager::KisNodeManager(KisView2 * view, KisDoc2 * doc)
        : m_d(new Private())
{
    m_d->view = view;
    m_d->doc = doc;
    m_d->layerManager = new KisLayerManager(view, doc);
    m_d->maskManager = new KisMaskManager(view);
    m_d->self = this;
    m_d->commandsAdapter = new KisNodeCommandsAdapter(view);
    connect(m_d->view->image(), SIGNAL(sigPostLayersChanged(KisGroupLayerSP)), SLOT(slotLayersChanged(KisGroupLayerSP)));
}

KisNodeManager::~KisNodeManager()
{
    delete m_d->commandsAdapter;
    delete m_d;
}
void KisNodeManager::setup(KActionCollection * actionCollection)
{
    m_d->layerManager->setup(actionCollection);
    m_d->maskManager->setup(actionCollection);

    KAction * action  = new KAction(KIcon("view-split-left-right"), i18n("Mirror Horizontally"), this);
    actionCollection->addAction("mirrorX", action);
    connect(action, SIGNAL(triggered()), this, SLOT(mirrorNodeX()));

    action  = new KAction(KIcon("view-split-top-bottom"), i18n("Mirror Vertically"), this);
    actionCollection->addAction("mirrorY", action);
    connect(action, SIGNAL(triggered()), this, SLOT(mirrorNodeY()));
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

const KoColorSpace* KisNodeManager::activeColorSpace()
{
    Q_ASSERT(m_d->maskManager);

    if (m_d->maskManager->activeDevice()) {
//        Q_ASSERT(m_d->maskManager->activeDevice());
        return m_d->maskManager->activeDevice()->colorSpace();
    } else {
        Q_ASSERT(m_d->layerManager);
        Q_ASSERT(m_d->layerManager->activeLayer());
        if (m_d->layerManager->activeLayer()->parentLayer())
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

bool allowAsChild(const QString & parentType, const QString & childType)
{
    // XXX_NODE: do we want to allow masks to apply on masks etc? Selections on masks?
    if (parentType == "KisPaintLayer" || parentType == "KisAdjustmentLayer" || parentType == "KisShapeLayer" || parentType == "KisGeneratorLayer" || parentType == "KisCloneLayer") {
        if (childType == "KisFilterMask" || childType == "KisTransformationMask" || childType == "KisTransparencyMask" || childType == "KisSelectionMask") {
            return true;
        }
        return false;
    } else if (parentType == "KisGroupLayer") {
        return true;
    } else if (parentType == "KisFilterMask" || parentType == "KisTransformationMask" || parentType == "KisTransparencyMask" || parentType == "KisSelectionMask") {
        return false;
    }

    return true;
}

void KisNodeManager::getNewNodeLocation(const QString & nodeType, KisNodeSP &parent, KisNodeSP &above, KisNodeSP _activeNode)
{
    KisNodeSP root = m_d->view->image()->root();
    if (!_activeNode)
        _activeNode = root->firstChild();
    KisNodeSP active = _activeNode;
    // Find the first node above the current node that can have the desired
    // layer type as child. XXX_NODE: disable the menu entries for node types
    // that are not compatible with the active node type.
    while (active) {
        if (allowAsChild(active->metaObject()->className(), nodeType)) {
            parent = active;
            if (_activeNode->parent() == parent) {
                above = _activeNode;
            } else {
                above = parent->firstChild();
            }
            return;
        }
        active = active->parent();
    }
    parent = root;
    above = parent->firstChild();
}

void KisNodeManager::addNode(KisNodeSP node, KisNodeSP activeNode)
{
    KisNodeSP parent;
    KisNodeSP above;

    getNewNodeLocation(node->metaObject()->className(), parent, above, activeNode);
    m_d->commandsAdapter->addNode(node, parent, above);
    node->setDirty(node->extent());
}

void KisNodeManager::insertNode(KisNodeSP node, KisNodeSP parent, int index)
{
    if (allowAsChild(parent->metaObject()->className(), node->metaObject()->className())) {
        m_d->commandsAdapter->addNode(node, parent, index);
    }
}

void KisNodeManager::moveNode(KisNodeSP node, KisNodeSP activeNode)
{
    KisNodeSP parent;
    KisNodeSP above;

    getNewNodeLocation(node->metaObject()->className(), parent, above, activeNode);
    m_d->commandsAdapter->moveNode(node, parent, above);
    node->setDirty(node->extent());
}

void KisNodeManager::moveNodeAt(KisNodeSP node, KisNodeSP parent, int index)
{
    if (allowAsChild(parent->metaObject()->className(), node->metaObject()->className())) {
        m_d->commandsAdapter->moveNode(node, parent, index);
    }
}


void KisNodeManager::createNode(const QString & nodeType)
{

    KisNodeSP parent;
    KisNodeSP above;

    getNewNodeLocation(nodeType, parent, above, activeNode());

    // XXX: make factories for this kind of stuff,
    //      with a registry

    if (nodeType == "KisPaintLayer") {
        m_d->layerManager->addLayer(parent, above);
    } else if (nodeType == "KisGroupLayer") {
        m_d->layerManager->addGroupLayer(parent, above);
    } else if (nodeType == "KisAdjustmentLayer") {
        m_d->layerManager->addAdjustmentLayer(parent, above);
    } else if (nodeType == "KisGeneratorLayer") {
        m_d->layerManager->addGeneratorLayer(parent, above);
    } else if (nodeType == "KisShapeLayer") {
        m_d->layerManager->addShapeLayer(parent, above);
    } else if (nodeType == "KisCloneLayer") {
        m_d->layerManager->addCloneLayer(parent, above);
    } else if (nodeType == "KisTransparencyMask") {
        m_d->maskManager->createTransparencyMask(parent, above);
    } else if (nodeType == "KisFilterMask") {
        m_d->maskManager->createFilterMask(parent, above);
    } else if (nodeType == "KisTransformationMask") {
        m_d->maskManager->createTransformationMask(parent, above);
    } else if (nodeType == "KisSelectionMask") {
        m_d->maskManager->createSelectionMask(parent, above);
    }

}

void KisNodeManager::activateNode(KisNodeSP node)
{
    Q_ASSERT(m_d->view);
    Q_ASSERT(m_d->view->canvasBase());
    Q_ASSERT(m_d->view->canvasBase()->globalShapeManager());

    if (node && m_d->activeNode) {
        if (node == m_d->activeNode) {
            return;
        }
    }

    // Set the selection on the shape manager to the active layer
    // and set call KoSelection::setActiveLayer( KoShapeLayer* layer )
    // with the parent of the active layer.
    KoSelection * selection = m_d->view->canvasBase()->globalShapeManager()->selection();
    Q_ASSERT(selection);
    selection->deselectAll();

    if (! node) {
        selection->setActiveLayer(0);
        emit sigNodeActivated(0);
        m_d->activeNode = 0;
        m_d->maskManager->activateMask(0);
        m_d->layerManager->activateLayer(0);
    } else {
        KoShape * shape = m_d->view->document()->shapeForNode(node);
        if (!shape) {
            shape = m_d->view->document()->addShape(node);
        }
#if 0
        KoShape * parentShape = shape->parent();
        if (!parentShape) {
            parentShape = m_d->view->document()->addShape(node->parent());
        }
#endif

        selection->select(shape);
        KoShapeLayer * shapeLayer = dynamic_cast<KoShapeLayer*>(shape);
        Q_ASSERT(shapeLayer);
        if (shapeLayer) {
            shapeLayer->setGeometryProtected(node->userLocked());
            shapeLayer->setVisible(node->visible());
            selection->setActiveLayer(shapeLayer);
        }

        m_d->activeNode = node;
        if (KisLayerSP layer = dynamic_cast<KisLayer*>(node.data())) {
            m_d->maskManager->activateMask(0);
            m_d->layerManager->activateLayer(layer);
        } else if (KisMaskSP mask = dynamic_cast<KisMask*>(node.data())) {
            m_d->maskManager->activateMask(mask);
            // XXX_NODE: for now, masks cannot be nested.
            m_d->layerManager->activateLayer(static_cast<KisLayer*>(node->parent().data()));
        }
        emit sigNodeActivated(node);

    }
    nodesUpdated();
}

void KisNodeManager::nodesUpdated()
{
    KisNodeSP node = activeNode();
    if (!node) return;

    m_d->layerManager->layersUpdated();
    m_d->maskManager->masksUpdated();

    m_d->view->updateGUI();
    m_d->view->selectionManager()->selectionChanged();

}

KisPaintDeviceSP KisNodeManager::activePaintDevice()
{
    return m_d->maskManager->activeMask() ?
        m_d->maskManager->activeDevice() :
        m_d->layerManager->activeDevice();
}

void KisNodeManager::nodeProperties(KisNodeSP node)
{
    if (node->inherits("KisLayer")) {
        m_d->layerManager->layerProperties();
    } else if (node->inherits("KisMask")) {
        m_d->maskManager->maskProperties();
    }
}

qint32 KisNodeManager::convertOpacityToInt(qreal opacity)
{
    /**
     * Scales opacity from the range 0...1
     * to the integer range 0...255
     */

    return qMin(255, int(opacity * 2.55 + 0.5));
}

void KisNodeManager::setNodeOpacity(KisNodeSP node, qint32 opacity,
                                    bool finalChange)
{
    if (!node) return;
    if (node->opacity() == opacity) return;

    if (!finalChange) {
        node->setOpacity(opacity);
        node->setDirty();
    } else {
        m_d->commandsAdapter->setOpacity(node, opacity);
    }
}

void KisNodeManager::setNodeCompositeOp(KisNodeSP node,
                                        const KoCompositeOp* compositeOp)
{
    if (!node) return;
    if (node->compositeOp() == compositeOp) return;

    m_d->commandsAdapter->setCompositeOp(node, compositeOp);
}

void KisNodeManager::nodeOpacityChanged(qreal opacity, bool finalChange)
{
    KisNodeSP node = activeNode();

    setNodeOpacity(node, convertOpacityToInt(opacity), finalChange);
}

void KisNodeManager::nodeCompositeOpChanged(const KoCompositeOp* op)
{
    KisNodeSP node = activeNode();

    setNodeCompositeOp(node, op);
}

void KisNodeManager::duplicateActiveNode()
{
    KisNodeSP node = activeNode();

    // FIXME: can't imagine how it may happen
    Q_ASSERT(node);

    if (node->inherits("KisLayer")) {
        m_d->layerManager->layerDuplicate();
    } else if (node->inherits("KisMask")) {
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
    } else if (node->inherits("KisMask")) {
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
    } else if (node->inherits("KisMask")) {
        m_d->maskManager->raiseMask();
    }
}

void KisNodeManager::nodeToTop()
{
    KisNodeSP node = activeNode();
    if (node->inherits("KisLayer")) {
        m_d->layerManager->layerBack();
    } else if (node->inherits("KisMask")) {
        m_d->maskManager->maskToBottom();
    }

}

void KisNodeManager::nodeToBottom()
{
    KisNodeSP node = activeNode();
    if (node->inherits("KisLayer")) {
        m_d->layerManager->layerLower();
    } else if (node->inherits("KisMask")) {
        m_d->maskManager->maskToTop();
    }
}

void KisNodeManager::removeNode(KisNodeSP node)
{
//     QRect bounds = node->exactBounds();
    m_d->commandsAdapter->removeNode(node);
//     m_image->rootLayer()->setDirty(bounds);
}

void KisNodeManager::mirrorNodeX()
{
    KisNodeSP node = activeNode();
    if (node->inherits("KisLayer")) {
        m_d->layerManager->mirrorLayerX();
    } else if (node->inherits("KisMask")) {
        m_d->maskManager->mirrorMaskX();
    }
}

void KisNodeManager::mirrorNodeY()
{
    KisNodeSP node = activeNode();
    if (node->inherits("KisLayer")) {
        m_d->layerManager->mirrorLayerY();
    } else if (node->inherits("KisMask")) {
        m_d->maskManager->mirrorMaskY();
    }
}

#include "kis_node_manager.moc"


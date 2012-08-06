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

#include <KoIcon.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoShape.h>
#include <KoShapeLayer.h>

#include <kis_types.h>
#include <kis_node.h>
#include <kis_selection.h>
#include <kis_selection_mask.h>
#include <kis_layer.h>
#include <kis_mask.h>
#include <kis_image.h>

#include "canvas/kis_canvas2.h"
#include "kis_shape_controller.h"
#include "kis_canvas_resource_provider.h"
#include "kis_view2.h"
#include "kis_doc2.h"
#include "kis_mask_manager.h"
#include "kis_group_layer.h"
#include "kis_layer_manager.h"
#include "kis_selection_manager.h"
#include "kis_node_commands_adapter.h"
#include "kis_mirror_visitor.h"

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

    bool activateNodeImpl(KisNodeSP node);
};

bool KisNodeManager::Private::activateNodeImpl(KisNodeSP node)
{
    Q_ASSERT(view);
    Q_ASSERT(view->canvasBase());
    Q_ASSERT(view->canvasBase()->globalShapeManager());

    if (node && node == activeNode) {
        return false;
    }

    // Set the selection on the shape manager to the active layer
    // and set call KoSelection::setActiveLayer( KoShapeLayer* layer )
    // with the parent of the active layer.
    KoSelection *selection = view->canvasBase()->globalShapeManager()->selection();
    Q_ASSERT(selection);
    selection->deselectAll();

    if (!node) {
        selection->setActiveLayer(0);
        activeNode = 0;
        maskManager->activateMask(0);
        layerManager->activateLayer(0);
    } else {

        KoShape * shape = view->document()->shapeForNode(node);
        Q_ASSERT(shape);

        selection->select(shape);
        KoShapeLayer * shapeLayer = dynamic_cast<KoShapeLayer*>(shape);

        Q_ASSERT(shapeLayer);
//         shapeLayer->setGeometryProtected(node->userLocked());
//         shapeLayer->setVisible(node->visible());
        selection->setActiveLayer(shapeLayer);


        activeNode = node;
        if (KisLayerSP layer = dynamic_cast<KisLayer*>(node.data())) {
            maskManager->activateMask(0);
            layerManager->activateLayer(layer);
        } else if (KisMaskSP mask = dynamic_cast<KisMask*>(node.data())) {
            maskManager->activateMask(mask);
            // XXX_NODE: for now, masks cannot be nested.
            layerManager->activateLayer(static_cast<KisLayer*>(node->parent().data()));
        }

    }
    return true;
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

    KisShapeController *shapeController =
        dynamic_cast<KisShapeController*>(doc->shapeController());
    Q_ASSERT(shapeController);

    connect(shapeController, SIGNAL(sigActivateNode(KisNodeSP)), SLOT(slotNonUiActivatedNode(KisNodeSP)));
    connect(m_d->layerManager, SIGNAL(sigLayerActivated(KisLayerSP)), SIGNAL(sigLayerActivated(KisLayerSP)));
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

    KAction * action  = new KAction(koIcon("object-flip-horizontal"), i18n("Mirror Horizontally"), this);
    actionCollection->addAction("mirrorX", action);
    connect(action, SIGNAL(triggered()), this, SLOT(mirrorNodeX()));

    action  = new KAction(koIcon("object-flip-vertical"), i18n("Mirror Vertically"), this);
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

KisLayerSP KisNodeManager::activeLayer()
{
    return m_d->layerManager->activeLayer();
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

bool allowAsChild(const QString & parentType, const QString & childType)
{
    // XXX_NODE: do we want to allow masks to apply on masks etc? Selections on masks?
    if (parentType == "KisPaintLayer" || parentType == "KisAdjustmentLayer" || parentType == "KisShapeLayer" || parentType == "KisGeneratorLayer" || parentType == "KisCloneLayer") {
        if (childType == "KisFilterMask" || childType == "KisTransparencyMask" || childType == "KisSelectionMask") {
            return true;
        }
        return false;
    } else if (parentType == "KisGroupLayer") {
        return true;
    } else if (parentType == "KisFilterMask" || parentType == "KisTransparencyMask" || parentType == "KisSelectionMask") {
        return false;
    }

    return true;
}

void KisNodeManager::getNewNodeLocation(KisNodeSP node, KisNodeSP& parent, KisNodeSP& above, KisNodeSP _activeNode)
{
    KisNodeSP root = m_d->view->image()->root();
    if (!_activeNode)
        _activeNode = root->firstChild();
    KisNodeSP active = _activeNode;
    // Find the first node above the current node that can have the desired
    // layer type as child. XXX_NODE: disable the menu entries for node types
    // that are not compatible with the active node type.
    while (active) {
        if (active->allowAsChild(node)) {
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

void KisNodeManager::moveNodeAt(KisNodeSP node, KisNodeSP parent, int index)
{
    if (parent->allowAsChild(node)) {
        if (node->inherits("KisSelectionMask") && parent->inherits("KisLayer")) {
            KisSelectionMask *m = dynamic_cast<KisSelectionMask*>(node.data());
            KisLayer *l = dynamic_cast<KisLayer*>(parent.data());
            KisSelectionMaskSP selMask = l->selectionMask();
            if (m && m->active() && l && l->selectionMask())
                selMask->setActive(false);
        }
        m_d->commandsAdapter->moveNode(node, parent, index);
    }
}

void KisNodeManager::addNodeDirect(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis)
{
    Q_ASSERT(parent->allowAsChild(node));
    m_d->commandsAdapter->addNode(node, parent, aboveThis);
}

void KisNodeManager::moveNodeDirect(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis)
{
    Q_ASSERT(parent->allowAsChild(node));
    m_d->commandsAdapter->moveNode(node, parent, aboveThis);
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
    } else if (nodeType == "KisSelectionMask") {
        m_d->maskManager->createSelectionMask(parent, above);
    }

}

void KisNodeManager::slotNonUiActivatedNode(KisNodeSP node)
{
    if(m_d->activateNodeImpl(node)) {
        emit sigUiNeedChangeActiveNode(node);
        emit sigNodeActivated(node);
        nodesUpdated();
    }
}

void KisNodeManager::slotUiActivatedNode(KisNodeSP node)
{
    if(m_d->activateNodeImpl(node)) {
        emit sigNodeActivated(node);
        nodesUpdated();
    }
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
    //do not delete root layer
    if(node->parent()==0)
        return;

    m_d->commandsAdapter->removeNode(node);
}

void KisNodeManager::mirrorNodeX()
{
    KisNodeSP node = activeNode();

    QString commandName;
    if (node->inherits("KisLayer")) {
        commandName = i18n("Mirror Layer X");
    } else if (node->inherits("KisMask")) {
        commandName = i18n("Mirror Mask X");
    }
    mirrorNode(node, commandName, Qt::Horizontal);
}

void KisNodeManager::mirrorNodeY()
{
    KisNodeSP node = activeNode();

    QString commandName;
    if (node->inherits("KisLayer")) {
        commandName = i18n("Mirror Layer Y");
    } else if (node->inherits("KisMask")) {
        commandName = i18n("Mirror Mask Y");
    }
    mirrorNode(node, commandName, Qt::Vertical);
}

void KisNodeManager::mergeLayerDown()
{
    m_d->layerManager->mergeLayer();
}

void KisNodeManager::rotate(double radians)
{
    // XXX: implement rotation for masks as well
    m_d->layerManager->rotateLayer(radians);

}


void KisNodeManager::rotate180()
{
    rotate(M_PI);
}

void KisNodeManager::rotateLeft90()
{
   rotate(M_PI / 2 - 2*M_PI); 
}

void KisNodeManager::rotateRight90()
{
    rotate(M_PI / 2);
}

void KisNodeManager::shear(double angleX, double angleY)
{
    // XXX: implement shear for masks as well
    m_d->layerManager->shearLayer(angleX, angleY);
}

void KisNodeManager::scale(double sx, double sy, KisFilterStrategy *filterStrategy)
{
    // XXX: implement scale for masks as well
    m_d->layerManager->scaleLayer(sx, sy, filterStrategy);
}

void KisNodeManager::mirrorNode(KisNodeSP node, const QString& commandName, Qt::Orientation orientation)
{
    m_d->view->image()->undoAdapter()->beginMacro(commandName);

    KisMirrorVisitor visitor(m_d->view->image(), orientation);
    node->accept(visitor);

    m_d->view->image()->undoAdapter()->endMacro();
    m_d->doc->setModified(true);

    if (node->inherits("KisLayer")) {
        m_d->layerManager->layersUpdated();
    } else if (node->inherits("KisMask")) {
        m_d->maskManager->masksUpdated();
    }
    m_d->view->canvas()->update();
}

#include "kis_node_manager.moc"


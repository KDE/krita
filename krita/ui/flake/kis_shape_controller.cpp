/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_shape_controller.h"


#include <klocale.h>

#include <KoShape.h>
#include <KoShapeContainer.h>
#include <KoShapeManager.h>
#include <KoShapeRegistry.h>
#include <KoCanvasBase.h>
#include <KoToolManager.h>
#include <KoView.h>
#include <KoSelection.h>
#include <KoShapeLayer.h>
#include <KoPathShape.h>
#include <KoColorSpaceConstants.h>
#include <KoCanvasController.h>

#include "kis_node_manager.h"
#include "kis_shape_selection.h"
#include "kis_selection.h"
#include "kis_selection_component.h"
#include "kis_adjustment_layer.h"
#include "kis_clone_layer.h"
#include "canvas/kis_canvas2.h"
#include "kis_doc2.h"
#include "kis_image.h"
#include "kis_group_layer.h"
#include "kis_layer_container_shape.h"
#include "kis_node_shape.h"
#include "kis_name_server.h"
#include "kis_mask.h"
#include "kis_shape_layer.h"
#include "kis_view2.h"
#include "kis_node.h"

#include <KoTextDocumentLayout.h>
#include <KoTextShapeData.h>
#include <KoDocumentResourceManager.h>
#include <KoDataCenterBase.h>
#include <commands/kis_image_layer_add_command.h>
#include <kis_undo_adapter.h>

typedef QMap<KisNodeSP, KoShape*> KisNodeMap;

class KisShapeController::Private
{
public:
    Private(KisShapeController *parent) : q(parent) {}

    KisImageWSP image;
    KisNodeSP rootNode;

    KisNodeMap nodeShapes; // maps from krita/image layers to shapes
    KisDoc2 *doc;
    KisNameServer *nameServer;
    QMap<QString, KoDataCenterBase*> dataCenterMap;

    void addNodeShape(KisNodeSP node);
    void removeNodeShape(KisNodeSP node);

private:
    KisShapeController *q;
};

void KisShapeController::Private::addNodeShape(KisNodeSP node)
{
    Q_ASSERT(!nodeShapes.contains(node));

    KoShapeContainer *parent = 0;

    if(nodeShapes.contains(node->parent())) {
        parent = dynamic_cast<KoShapeContainer*>(nodeShapes[node->parent()]);
    }


    KoShape *shape;

    if (node->inherits("KisGroupLayer")) {
        shape = new KisLayerContainerShape(parent, static_cast<KisGroupLayer*>(node.data()));
    } else if (node->inherits("KisShapeLayer")) {
        KisShapeLayer *shapeLayer = static_cast<KisShapeLayer*>(node.data());
        q->connect(shapeLayer, SIGNAL(selectionChanged(QList<KoShape*>)),
                   q, SIGNAL(selectionChanged()));

        shape = shapeLayer;
    } else {
        shape = new KisNodeShape(parent, static_cast<KisLayer*>(node.data()));
    }

    shape->setParent(parent);
    nodeShapes[node] = shape;

    KisNodeSP childNode = node->firstChild();
    while (childNode) {
        addNodeShape(childNode);
        childNode = childNode->nextSibling();
    }
}

void KisShapeController::Private::removeNodeShape(KisNodeSP node)
{
    KisNodeSP childNode = node->firstChild();
    while (childNode) {
        removeNodeShape(childNode);
        childNode = childNode->nextSibling();
    }

    Q_ASSERT(nodeShapes.contains(node));

    KoShape *nodeShape = nodeShapes[node];
    nodeShapes.remove(node);

    KisShapeLayer *shapeLayer = dynamic_cast<KisShapeLayer*>(nodeShape);
    if (shapeLayer) {
        shapeLayer->disconnect(q);
        nodeShape->setParent(0);
    } else {
        delete nodeShape;
    }
}

KisShapeController::KisShapeController(KisDoc2 * doc, KisNameServer *nameServer)
        : QObject(doc)
        , m_d(new Private(this))
{
    m_d->doc = doc;
    m_d->nameServer = nameServer;
    m_d->image = 0;
    resourceManager()->setUndoStack(doc->undoStack());
}


KisShapeController::~KisShapeController()
{
    setImage(0);

    // XXX: deleting the undoStack of the document while the document is being deleted is dangerous
    m_d->dataCenterMap.remove("UndoStack");
    qDeleteAll(m_d->dataCenterMap);

    delete m_d;
}

void KisShapeController::setImage(KisImageWSP image)
{
    if (m_d->image.isValid()) {
        m_d->image->disconnect(this);
        m_d->removeNodeShape(m_d->rootNode);

        Q_ASSERT(m_d->nodeShapes.empty());
        m_d->nodeShapes.clear();

        m_d->image = 0;
        m_d->rootNode = 0;
    }

    if (image) {
        m_d->rootNode = image->rootLayer();
        m_d->addNodeShape(m_d->rootNode);

        connect(image, SIGNAL(sigNodeHasBeenAdded(KisNode *, int)), SLOT(slotNodeAdded(KisNode*, int)));
        connect(image, SIGNAL(sigAboutToRemoveANode(KisNode *, int)), SLOT(slotNodeRemoved(KisNode*, int)));
        connect(image, SIGNAL(sigLayersChanged(KisGroupLayerSP)), this, SLOT(slotLayersChanged(KisGroupLayerSP)));
    }

    m_d->image = image;
}

static inline bool belongsToShapeSelection(KoShape* shape) {
    return dynamic_cast<KisShapeSelectionMarker*>(shape->userData());
}

void KisShapeController::addShape(KoShape* shape)
{
    if (!m_d->image) return;

    Q_ASSERT(m_d->image);

    /**
     * Krita layers have their own creation path.
     * It goes through slotNodeAdded()
     */
    Q_ASSERT(shape->shapeId() != KIS_NODE_SHAPE_ID  &&
             shape->shapeId() != KIS_SHAPE_LAYER_ID  &&
             shape->shapeId() != KIS_LAYER_CONTAINER_ID);


    KisCanvas2 *canvas = dynamic_cast<KisCanvas2*>(KoToolManager::instance()->activeCanvasController()->canvas());
    Q_ASSERT(canvas);

    if (belongsToShapeSelection(shape)) {

        KisSelectionSP selection = canvas->view()->selection();
        if (selection) {
            if (!selection->shapeSelection()) {
                selection->setShapeSelection(new KisShapeSelection(m_d->image, selection));
            }
            KisShapeSelection * shapeSelection = static_cast<KisShapeSelection*>(selection->shapeSelection());
            shapeSelection->addShape(shape);
        }

    } else {
        KisShapeLayer *shapeLayer = dynamic_cast<KisShapeLayer*>(shape->parent());

        if (!shapeLayer) {
            KisLayerContainerShape *container =
                dynamic_cast<KisLayerContainerShape*>(shapeForNode(m_d->image->rootLayer().data()));

            shapeLayer = new KisShapeLayer(container, this, m_d->image,
                                           i18n("Vector Layer %1", m_d->nameServer->number()),
                                           OPACITY_OPAQUE_U8);

            m_d->image->undoAdapter()->addCommand(new KisImageLayerAddCommand(m_d->image, shapeLayer, m_d->image->rootLayer(), m_d->image->rootLayer()->childCount()));
            canvas->view()->nodeManager()->activateNode(shapeLayer);
        }

        shapeLayer->addShape(shape);
    }

    m_d->doc->setModified(true);
}

void KisShapeController::removeShape(KoShape* shape)
{
    // Nodes have their own way of death through slotNodeRemoved()
    Q_ASSERT(!dynamic_cast<KisNodeShape*>(shape) &&
             !dynamic_cast<KisLayerContainerShape*>(shape));

    // Remove children shapes if any
    KoShapeContainer * container = dynamic_cast<KoShapeContainer*>(shape);
    if (container) {
        foreach(KoShape * child, container->shapes()) {
            removeShape(child);
        }
    }

    shape->setParent(0);

    m_d->doc->setModified(true);
}

QMap<QString, KoDataCenterBase *> KisShapeController::dataCenterMap() const
{
    return m_d->dataCenterMap;
}


void KisShapeController::setInitialShapeForView(KisView2 * view)
{
    if (!m_d->image) return;

    if (! m_d->nodeShapes.isEmpty()) {
        Q_ASSERT(view->canvasBase());
        Q_ASSERT(view->canvasBase()->shapeManager());
        KoSelection *selection = view->canvasBase()->shapeManager()->selection();
        if (selection) {
            selection->select(m_d->nodeShapes.values().first());
            KoToolManager::instance()->switchToolRequested(KoToolManager::instance()->preferredToolForSelection(selection->selectedShapes()));
        }
    }
}

void KisShapeController::slotNodeAdded(KisNode *parentNode, int index)
{
    if (!parentNode) return;

    KisNodeSP node = parentNode->at(index);
    m_d->addNodeShape(node);
}

void KisShapeController::slotNodeRemoved(KisNode *parentNode, int index)
{
    if (!parentNode) return;

    KisNodeSP node = parentNode->at(index);
    m_d->removeNodeShape(node);
}

void KisShapeController::slotLayersChanged(KisGroupLayerSP rootLayer)
{
    Q_UNUSED(rootLayer);

    setImage(m_d->image);
}

KoShape * KisShapeController::shapeForNode(KisNodeSP node) const
{
    Q_ASSERT(m_d->nodeShapes.contains(node));
    return m_d->nodeShapes[node];
}


int KisShapeController::layerMapSize()
{
    return m_d->nodeShapes.size();
}


#include "kis_shape_controller.moc"

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
#include "kis_node_shape.h"
#include "kis_node_shapes_graph.h"
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


struct KisShapeController::Private
{
public:
    KisImageWSP image;
    KisNodeSP savedRootNode;

    KisDoc2 *doc;
    KisNameServer *nameServer;
    QMap<QString, KoDataCenterBase*> dataCenterMap;

    KisNodeShapesGraph shapesGraph;
};

KisShapeController::KisShapeController(KisDoc2 * doc, KisNameServer *nameServer)
        : QObject(doc)
        , m_d(new Private())
{
    m_d->doc = doc;
    m_d->nameServer = nameServer;
    m_d->image = 0;
    resourceManager()->setUndoStack(doc->undoStack());

    connect(this, SIGNAL(sigContinueAddNode(KisNodeSP, KisNodeSP, KisNodeSP)),
            SLOT(slotContinueAddNode(KisNodeSP, KisNodeSP, KisNodeSP)));
    connect(this, SIGNAL(sigContinueMoveNode(KisNodeSP, KisNodeSP, KisNodeSP)),
            SLOT(slotContinueMoveNode(KisNodeSP, KisNodeSP, KisNodeSP)));
    connect(this, SIGNAL(sigContinueRemoveNode(KisNodeSP)),
            SLOT(slotContinueRemoveNode(KisNodeSP)));
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
        slotRemoveNode(m_d->savedRootNode);

        Q_ASSERT(!m_d->shapesGraph.shapesCount());
    }
    m_d->image = 0;
    m_d->savedRootNode = 0;

    if (image) {
        m_d->image = image;
        m_d->savedRootNode = m_d->image->root();
        slotNodeAdded(m_d->savedRootNode);

        connect(image, SIGNAL(sigNodeAddedAsync(KisNodeSP)),
                SLOT(slotNodeAdded(KisNodeSP)), Qt::DirectConnection);
        connect(image, SIGNAL(sigNodeMovedAsync(KisNodeSP)),
                SLOT(slotNodeMoved(KisNodeSP)), Qt::DirectConnection);
        connect(image, SIGNAL(sigRemoveNodeAsync(KisNodeSP)),
                SLOT(slotRemoveNode(KisNodeSP)), Qt::DirectConnection);
        connect(image, SIGNAL(sigLayersChangedAsync()),
                SLOT(slotLayersChanged()), Qt::DirectConnection);
    }
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
             shape->shapeId() != KIS_SHAPE_LAYER_ID);


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
            KoShapeLayer *rootLayer = shapeForNode(m_d->image->rootLayer().data());
            shapeLayer = new KisShapeLayer(rootLayer, this, m_d->image,
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
    /**
     * Krita layers have their own destruction path.
     * It goes through slotRemoveNode()
     */
    Q_ASSERT(shape->shapeId() != KIS_NODE_SHAPE_ID  &&
             shape->shapeId() != KIS_SHAPE_LAYER_ID);


#if 0
    // Remove children shapes if any
    KoShapeContainer * container = dynamic_cast<KoShapeContainer*>(shape);
    if (container) {
        foreach(KoShape * child, container->shapes()) {
            removeShape(child);
        }
    }
#endif

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

    KisNodeSP rootNode = m_d->image->root();

    if (m_d->shapesGraph.containsNode(rootNode)) {
        Q_ASSERT(view->canvasBase());
        Q_ASSERT(view->canvasBase()->shapeManager());
        KoSelection *selection = view->canvasBase()->shapeManager()->selection();
        if (selection) {
            selection->select(m_d->shapesGraph.nodeToShape(rootNode));
            KoToolManager::instance()->switchToolRequested(KoToolManager::instance()->preferredToolForSelection(selection->selectedShapes()));
        }
    }
}

void KisShapeController::slotLayersChanged()
{
    setImage(m_d->image);
}

void KisShapeController::slotNodeAdded(KisNodeSP node)
{
    emit sigContinueAddNode(node, node->parent(), node->prevSibling());

    KisNodeSP childNode = node->firstChild();
    while (childNode) {
        slotNodeAdded(childNode);
        childNode = childNode->nextSibling();
    }
}

void KisShapeController::slotNodeMoved(KisNodeSP node)
{
    emit sigContinueMoveNode(node, node->parent(), node->prevSibling());
}

void KisShapeController::slotRemoveNode(KisNodeSP node)
{
    emit sigContinueRemoveNode(node);
}

void KisShapeController::slotContinueAddNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis)
{
    KisNodeShape *newShape =
        m_d->shapesGraph.addNode(node, parent, aboveThis);

    KisShapeLayer *shapeLayer = dynamic_cast<KisShapeLayer*>(node.data());
    if (shapeLayer) {
        /**
         * Forward signals for global shape manager
         * \see comment in the constructor of KisCanvas2
         */
        connect(shapeLayer, SIGNAL(selectionChanged()),
                SIGNAL(selectionChanged()));
        connect(shapeLayer, SIGNAL(currentLayerChanged(const KoShapeLayer*)),
                SIGNAL(currentLayerChanged(const KoShapeLayer*)));
        ((KoShapeLayer*)shapeLayer)->setParent(newShape);
    }
}

void KisShapeController::slotContinueMoveNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis)
{
    m_d->shapesGraph.moveNode(node, parent, aboveThis);
}

void KisShapeController::slotContinueRemoveNode(KisNodeSP node)
{
    KisShapeLayer *shapeLayer = dynamic_cast<KisShapeLayer*>(node.data());
    if (shapeLayer) {
        shapeLayer->disconnect(this);
        ((KoShapeLayer*)shapeLayer)->setParent(0);
    }

    m_d->shapesGraph.removeNode(node);
}

KoShapeLayer* KisShapeController::shapeForNode(KisNodeSP node) const
{
    return m_d->shapesGraph.nodeToShape(node);
}

KisNodeDummy* KisShapeController::dummyForNode(KisNodeSP node) const
{
    return m_d->shapesGraph.nodeToDummy(node);
}

int KisShapeController::layerMapSize()
{
    return m_d->shapesGraph.shapesCount();
}


#include "kis_shape_controller.moc"

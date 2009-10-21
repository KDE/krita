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
#include <KoUndoStack.h>
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
#include "kis_layermap_visitor.h"
#include "kis_node_shape.h"
#include "kis_name_server.h"
#include "kis_mask.h"
#include "kis_shape_layer.h"
#include "kis_view2.h"
#include "kis_node.h"

#include <KoTextDocumentLayout.h>
#include <KoTextShapeData.h>
#include <KoDataCenter.h>
#include <commands/kis_image_layer_add_command.h>
#include <kis_undo_adapter.h>

typedef QMap<KisNodeSP, KoShape*> KisNodeMap;

class KisShapeController::Private
{
public:
    KisImageWSP image;
    KisNodeMap nodeShapes; // maps from krita/image layers to shapes
    KisDoc2 * doc;
    KisNameServer * nameServer;
    QMap<QString, KoDataCenter *>  dataCenterMap;
    bool selectionShapeToBeAdded;

    void removeShapeFromMap(KoShape*);
    void removeShapeAndChildrenFromMap(KoShape*);
};


void KisShapeController::Private::removeShapeFromMap(KoShape* shape)
{
    KisNodeMap::iterator begin = nodeShapes.begin();
    KisNodeMap::iterator end = nodeShapes.end();
    KisNodeMap::iterator it = begin;
    while (it != end) {
        if (it.value() == shape) {
            dbgKrita << "Going to delete node " << it.key() << " with shape " << it.value() << ", because it is the same as " << shape;
            nodeShapes.remove(it.key());
            break;
        }
        ++it;
    }
}


void KisShapeController::Private::removeShapeAndChildrenFromMap(KoShape* shape)
{
    KoShapeContainer * parent = dynamic_cast<KoShapeContainer*>(shape);
    if (parent) {
        foreach(KoShape * child, parent->childShapes()) {
            removeShapeFromMap(child);
        }
    }
    removeShapeFromMap(shape);
}

KisShapeController::KisShapeController(KisDoc2 * doc, KisNameServer *nameServer)
        : QObject(doc)
        , m_d(new Private)
{
    m_d->doc = doc;
    m_d->nameServer = nameServer;
    m_d->image = 0;
    m_d->selectionShapeToBeAdded = false;
    // Ask every shapefactory to populate the dataCenterMap
    QList<KoShapeFactory*> shapeFactories = KoShapeRegistry::instance()->values();
    foreach(KoShapeFactory* shapeFactory, shapeFactories) {
        shapeFactory->populateDataCenterMap(m_d->dataCenterMap);
    }

    m_d->dataCenterMap["UndoStack"] = doc->undoStack();
}


KisShapeController::~KisShapeController()
{
    dbgUI << "Deleting the KisShapeController. There are" << m_d->nodeShapes.size() << " shapes";
    /*
        XXX: leak!

        foreach( KoShape* shape, m_d->nodeShapes ) {
            removeShape( shape);
            delete shape; // XXX: What happes with stuff on the
                          // clipboard? And how about undo information?
        }
        m_d->nodeShapes.clear();
    // XXX: deleting the undoStack of the document while the document is being deleted is dangerous
    */
    m_d->dataCenterMap.remove("UndoStack");
    qDeleteAll(m_d->dataCenterMap);

    delete m_d;
}

void KisShapeController::setImage(KisImageWSP image)
{
    dbgUI << ppVar(image);
    if (m_d->image) {
        m_d->image->disconnect(this);
        // First clear the current set of shapes away
        foreach(KoShape* shape, m_d->nodeShapes) {
            removeShape(shape);
            // clipboard? And how about undo information?

        }
#ifdef __GNUC__
#warning "KisShapeController::setImage: FIXME someone clever should know how to fix the shape corresponding to the root layer and delete that"
#endif
        m_d->nodeShapes.clear();

    }

    if (image) {
        m_d->image = image;

        dbgUI << "New root layer is " << m_d->image->rootLayer();
        KisLayerMapVisitor v(m_d->nodeShapes);
        m_d->image->rootLayer()->accept(v);
        m_d->nodeShapes = v.layerMap();

        foreach(KoShape* shape, m_d->nodeShapes) {
            KisShapeLayer * shapeLayer = dynamic_cast<KisShapeLayer*>(shape);
            if (shapeLayer) {
                connect(shapeLayer, SIGNAL(selectionChanged(QList<KoShape*>)),
                        KoToolManager::instance(), SLOT(selectionChanged(QList<KoShape*>)));
            }
        }

        foreach(KoView *view, m_d->doc->views()) {
            KisCanvas2 *canvas = ((KisView2*)view)->canvasBase();
            foreach(KoShape* shape, m_d->nodeShapes) {
                canvas->shapeManager()->add(shape);
            }
            canvas->canvasWidget()->update();
        }

        connect(m_d->image, SIGNAL(sigNodeHasBeenAdded(KisNode *, int)), SLOT(slotNodeAdded(KisNode*, int)));
        connect(m_d->image, SIGNAL(sigAboutToRemoveANode(KisNode *, int)), SLOT(slotNodeRemoved(KisNode*, int)));
        connect(m_d->image, SIGNAL(sigLayersChanged(KisGroupLayerSP)), this, SLOT(slotLayersChanged(KisGroupLayerSP)));
    }
}

void KisShapeController::removeShape(KoShape* shape)
{

    // Also remove all the children, if any
    {
        KoShapeContainer * parent = dynamic_cast<KoShapeContainer*>(shape);
        if (parent) {
            foreach(KoShape * child, parent->childShapes()) {
                removeShape(child);
            }
        }
    }


    KisCanvas2 * canvas = 0;
    KisSelectionSP selection = 0;

    if ((shape->shapeId() == KIS_NODE_SHAPE_ID
            || shape->shapeId() == KIS_SHAPE_LAYER_ID
            || shape->shapeId() == KIS_LAYER_CONTAINER_ID) // Those shape can be in a selection
            && (KoToolManager::instance()->activeCanvasController()
                && KoToolManager::instance()->activeCanvasController()->canvas()) // FIXME don't we check twice for the same thing ?
            && (canvas =  dynamic_cast<KisCanvas2*>(KoToolManager::instance()->activeCanvasController()->canvas()))
            && (selection = canvas->view()->selection())) {
        // Has a selection, be in a seclection, remove it from there
        if (selection->hasShapeSelection()) {
            KisShapeSelection * shapeSelection = static_cast<KisShapeSelection*>(selection->shapeSelection());
            shapeSelection->removeChild(shape);
        }
    } else {
        KisShapeLayer * shapeLayer = dynamic_cast<KisShapeLayer*>(shape->parent());

        // XXX: What happens if the shape is added embedded in another
        // shape?
        if (shapeLayer)
            shapeLayer->removeChild(shape);
    }

    m_d->removeShapeFromMap(shape);

    m_d->doc->setModified(true);
}

void KisShapeController::prepareAddingSelectionShape()
{
    m_d->selectionShapeToBeAdded = true;
}

void KisShapeController::addShape(KoShape* shape)
{
    if (!m_d->image) return;

    // If the parent of the added shape has is a selection, has a selection
    // or if the image has a selection, add the shape to the selection, instead
    // of to a shape layer
    KisCanvas2 * canvas = dynamic_cast<KisCanvas2*>(KoToolManager::instance()->activeCanvasController()->canvas());

    // Only non-krita shapes get added through this method; krita
    // layer shapes are added to kisimage and then end up in
    // slotLayerAdded
    if (shape->shapeId() != KIS_NODE_SHAPE_ID  &&
            shape->shapeId() != KIS_SHAPE_LAYER_ID  &&
            shape->shapeId() != KIS_LAYER_CONTAINER_ID) {

        if (m_d->selectionShapeToBeAdded) {
            // There's a selection active. that means that all shapes get added to the active selection,
            // instead of to a shape layer or a newly created shape layer.
            KisSelectionSP selection;
            if (canvas && (selection = canvas->view()->selection())) {
                if (!selection->shapeSelection()) {
                    selection->setShapeSelection(new KisShapeSelection(m_d->image, selection));
                }
                KisShapeSelection * shapeSelection = static_cast<KisShapeSelection*>(selection->shapeSelection());
                shapeSelection->addChild(shape);
                /*
                            foreach( KoView *view, m_d->doc->views() ) {
                                KisCanvas2 *canvas = static_cast<KisView2*>(view)->canvasBase();
                                canvas->globalShapeManager()->add(shape);
                            }*/
            }
            m_d->selectionShapeToBeAdded = false;

        } else {
            // An ordinary shape, if the active layer is a KisShapeLayer,
            // add it there, otherwise, create a new KisShapeLayer on top
            // of the active layer.

            // Check whether the shape is part of a layer -- that would be our
            // shape layers. The parent is set by KoShapeController using the
            // KoSelection object returned by the KoShapeManager that is
            // returned by KoCanvasBase -- and the shape manager in the
            // KisShapeLayer always sets the layer as parent using
            // KoSelection::setActiveLayer. The inheritance is:
            // KisShapeLayer::KoShapeLayer::KoShapeContainer::KoShape.

            KisShapeLayer * shapeLayer = dynamic_cast<KisShapeLayer*>(shape->parent());
            dbgUI << "shape:" << shape;
            dbgUI << "shape parent:" << shape->parent();
            dbgUI << "shape layer:" << shapeLayer;

            if (!shapeLayer) {
                // There is no parent layer set, which means that when
                // dropping, there was no shape layer active. Create one
                // and add it on top of the image stack.

                KisLayerContainerShape * container =
                    dynamic_cast<KisLayerContainerShape*>(shapeForNode(m_d->image->rootLayer().data()));

                dbgUI << "container:" << container;
                shapeLayer = new KisShapeLayer(container,
                                               this,
                                               m_d->image,
                                               i18n("Flake shapes %1", m_d->nameServer->number()),
                                               OPACITY_OPAQUE);

                // Add the shape layer to the image. The image then emits
                // a signal that is caught by us (the document) and the
                // layerbox and makes sure the new layer is in the
                // layer-shape map and in the layerbox
                m_d->image->undoAdapter()->addCommand(new KisImageLayerAddCommand(m_d->image, shapeLayer, m_d->image->rootLayer(), m_d->image->rootLayer()->childCount()));

                if (canvas) {
                    canvas->view()->nodeManager()->activateNode(shapeLayer);
                }

            }
            // TODO: refactor after 2.0
            connect(shapeLayer, SIGNAL(selectionChanged(QList<KoShape*>)),
                    this, SLOT(slotNotifySelectionChanged(QList<KoShape*>)));
            connect(shapeLayer, SIGNAL(selectionChanged(QList<KoShape*>)),
                    KoToolManager::instance(), SLOT(selectionChanged(QList<KoShape*>)));

            // XXX: What happens if the shape is added embedded in another
            // shape?
            if (shapeLayer)
                shapeLayer->addChild(shape);

        }
    } else {
        warnKrita << "Eeek -- we tried to add a krita layer shape without going through KisImage";
    }

    m_d->doc->setModified(true);
}

QMap<QString, KoDataCenter *> KisShapeController::dataCenterMap() const
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

void KisShapeController::slotNodeAdded(KisNode* parentNode, int index)
{
    if (!parentNode) return;

    KisNodeSP node = parentNode->at(index);

    // Check whether the layer is already in the map
    if (m_d->nodeShapes.contains(node)) {
        dbgUI << "The document already contains node" << node->name();
        return;
    }

    // Get the parent -- there is always one, and it should be in the nodemap already
    dbgUI << ppVar(parentNode) << ppVar(shapeForNode(parentNode)) << ppVar(node->name()) << ppVar(parentNode->name());
    KoShapeContainer * parent = dynamic_cast<KoShapeContainer*>(shapeForNode(parentNode));
    Q_ASSERT(parent);

    KoShape * shape = 0;

    dbgUI << "Going to add node of type " << node->metaObject()->className();
    if (node->inherits("KisGroupLayer")) {
        shape = new KisLayerContainerShape(parent, static_cast<KisGroupLayer*>(node.data()));
    } else if (node->inherits("KisPaintLayer")  ||
               node->inherits("KisAdjustmentLayer") ||
               node->inherits("KisCloneLayer") ||
               node->inherits("KisGeneratorLayer") ||
               node->inherits("KisMask")) {
        shape = new KisNodeShape(parent, static_cast<KisLayer*>(node.data()));
    } else if (node->inherits("KisShapeLayer")) {
        shape = static_cast<KisShapeLayer*>(node.data());
    }

    Q_ASSERT(shape);

    // Put the layer in the right place in the hierarchy
    shape->setParent(parent);
    parent->addChild(shape);

    m_d->nodeShapes[node] = shape;

    foreach(KoView *view, m_d->doc->views()) {
        KisCanvas2 *canvas = ((KisView2*)view)->canvasBase();
        canvas->globalShapeManager()->add(shape);
        canvas->canvasWidget()->update();
    }
    dbgUI << "Added " << node << " as shape " << shape << " to the document";
}

void KisShapeController::slotNodeRemoved(KisNode* parent, int index)
{
    KisNodeSP node = parent->at(index);
    dbgKrita << "Going to remove node " << node << " from parent " << parent;
    KoShape * shape = shapeForNode(node);
    dbgKrita << "Going to remove node " << node << " from parent " << parent << "( shape: " << shape << ")";
    m_d->removeShapeAndChildrenFromMap(shapeForNode(node));
}

void KisShapeController::slotLayersChanged(KisGroupLayerSP rootLayer)
{
    Q_UNUSED(rootLayer);

    setImage(m_d->image);
}

KoShape * KisShapeController::shapeForNode(KisNodeSP node) const
{
    if (m_d->nodeShapes.contains(node))
        return m_d->nodeShapes[node];
    else {
        dbgUI << "KisShapeController::shapeForNode does not find a shape for node " << node << ", this should never happen!";
        return 0;
    }
}


int KisShapeController::layerMapSize()
{
    return m_d->nodeShapes.size();
}

void KisShapeController::slotNotifySelectionChanged(QList<KoShape*> shapes)
{
    foreach(KoView *view, m_d->doc->views()) {
        KisCanvas2 *canvas = ((KisView2*)view)->canvasBase();
        canvas->globalShapeManager()->selection()->deselectAll();
        foreach(KoShape* shape, shapes) {
            canvas->globalShapeManager()->selection()->select(shape);
        }
    }
}

#include "kis_shape_controller.moc"

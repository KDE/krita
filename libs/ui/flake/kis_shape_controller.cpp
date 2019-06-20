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


#include <klocalizedstring.h>

#include <KoShape.h>
#include <KoShapeContainer.h>
#include <KoShapeManager.h>
#include <KoCanvasBase.h>
#include <KoToolManager.h>
#include <KisView.h>
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
#include "KisDocument.h"
#include "kis_image.h"
#include "kis_group_layer.h"
#include "kis_node_shape.h"
#include "kis_node_shapes_graph.h"
#include "kis_name_server.h"
#include "kis_mask.h"
#include "kis_shape_layer.h"
#include "KisViewManager.h"
#include "kis_node.h"

#include <KoDocumentResourceManager.h>
#include <KoDataCenterBase.h>
#include <commands/kis_image_layer_add_command.h>
#include <kis_undo_adapter.h>
#include "KoSelectedShapesProxy.h"
#include "kis_signal_auto_connection.h"


struct KisShapeController::Private
{
public:
    KisDocument *doc;
    KisNameServer *nameServer;
    KisSignalAutoConnectionsStore imageConnections;

    KisNodeShapesGraph shapesGraph;
};

KisShapeController::KisShapeController(KisDocument *doc, KisNameServer *nameServer)
    : KisDummiesFacadeBase(doc)
    , m_d(new Private())
{
    m_d->doc = doc;
    m_d->nameServer = nameServer;
    resourceManager()->setUndoStack(doc->undoStack());
}


KisShapeController::~KisShapeController()
{
    KisNodeDummy *node = m_d->shapesGraph.rootDummy();
    if (node) {
        m_d->shapesGraph.removeNode(node->node());
    }

    delete m_d;
}

void KisShapeController::slotUpdateDocumentResolution()
{
    const qreal pixelsPerInch = m_d->doc->image()->xRes() * 72.0;
    resourceManager()->setResource(KoDocumentResourceManager::DocumentResolution, pixelsPerInch);
}

void KisShapeController::slotUpdateDocumentSize()
{
    resourceManager()->setResource(KoDocumentResourceManager::DocumentRectInPixels, m_d->doc->image()->bounds());
}

void KisShapeController::addNodeImpl(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis)
{
    KisNodeShape *newShape =
        m_d->shapesGraph.addNode(node, parent, aboveThis);
    // XXX: what are we going to do with this shape?
    Q_UNUSED(newShape);

    KisShapeLayer *shapeLayer = dynamic_cast<KisShapeLayer*>(node.data());
    if (shapeLayer) {
        /**
         * Forward signals for global shape manager
         * \see comment in the constructor of KisCanvas2
         */
        connect(shapeLayer, SIGNAL(selectionChanged()),
                SIGNAL(selectionChanged()));
        connect(shapeLayer->shapeManager(), SIGNAL(selectionContentChanged()),
                SIGNAL(selectionContentChanged()));
        connect(shapeLayer, SIGNAL(currentLayerChanged(const KoShapeLayer*)),
                SIGNAL(currentLayerChanged(const KoShapeLayer*)));
    }
}

void KisShapeController::removeNodeImpl(KisNodeSP node)
{
    KisShapeLayer *shapeLayer = dynamic_cast<KisShapeLayer*>(node.data());
    if (shapeLayer) {
        shapeLayer->disconnect(this);
    }

    m_d->shapesGraph.removeNode(node);
}

bool KisShapeController::hasDummyForNode(KisNodeSP node) const
{
    return m_d->shapesGraph.containsNode(node);
}

KisNodeDummy* KisShapeController::dummyForNode(KisNodeSP node) const
{
    return m_d->shapesGraph.nodeToDummy(node);
}

KisNodeDummy* KisShapeController::rootDummy() const
{
    return m_d->shapesGraph.rootDummy();
}

int KisShapeController::dummiesCount() const
{
    return m_d->shapesGraph.shapesCount();
}

static inline bool belongsToShapeSelection(KoShape* shape) {
    return dynamic_cast<KisShapeSelectionMarker*>(shape->userData());
}

void KisShapeController::addShapes(const QList<KoShape*> shapes)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(!shapes.isEmpty());

    KisCanvas2 *canvas = dynamic_cast<KisCanvas2*>(KoToolManager::instance()->activeCanvasController()->canvas());
    KIS_SAFE_ASSERT_RECOVER_RETURN(canvas);

    const KoShape *baseShapeParent = shapes.first()->parent();
    const bool baseBelongsToSelection = belongsToShapeSelection(shapes.first());
    bool allSameParent = true;
    bool allSameBelongsToShapeSelection = true;
    bool hasNullParent = false;

    Q_FOREACH (KoShape *shape, shapes) {
        hasNullParent |= !shape->parent();
        allSameParent &= shape->parent() == baseShapeParent;
        allSameBelongsToShapeSelection &= belongsToShapeSelection(shape) == baseBelongsToSelection;
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN(!baseBelongsToSelection || allSameBelongsToShapeSelection);

    if (!allSameParent || hasNullParent) {
        if (baseBelongsToSelection && allSameBelongsToShapeSelection) {
            KisSelectionSP selection = canvas->viewManager()->selection();
            if (selection) {
                if (!selection->shapeSelection()) {
                    selection->setShapeSelection(new KisShapeSelection(this, image(), selection));
                }
                KisShapeSelection * shapeSelection = static_cast<KisShapeSelection*>(selection->shapeSelection());

                Q_FOREACH(KoShape *shape, shapes) {
                    shapeSelection->addShape(shape);
                }
            }
        } else {
            KisShapeLayer *shapeLayer =
                dynamic_cast<KisShapeLayer*>(
                    canvas->selectedShapesProxy()->selection()->activeLayer());

            if (!shapeLayer) {
                shapeLayer = new KisShapeLayer(this, image(),
                                               i18n("Vector Layer %1", m_d->nameServer->number()),
                                               OPACITY_OPAQUE_U8);

                image()->undoAdapter()->addCommand(new KisImageLayerAddCommand(image(), shapeLayer, image()->rootLayer(), image()->rootLayer()->childCount()));
            }

            QRectF updateRect;

            Q_FOREACH(KoShape *shape, shapes) {
                shapeLayer->addShape(shape);
                updateRect |= shape->boundingRect();
            }

            canvas->shapeManager()->update(updateRect);
        }
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


    QRectF updateRect = shape->boundingRect();
    shape->setParent(0);

    KisCanvas2 *canvas = dynamic_cast<KisCanvas2*>(KoToolManager::instance()->activeCanvasController()->canvas());
    KIS_SAFE_ASSERT_RECOVER_RETURN(canvas);
    canvas->shapeManager()->update(updateRect);

    m_d->doc->setModified(true);
}

QRectF KisShapeController::documentRectInPixels() const
{
    return m_d->doc->image()->bounds();
}

qreal KisShapeController::pixelsPerInch() const
{
    return m_d->doc->image()->xRes() * 72.0;
}

void KisShapeController::setInitialShapeForCanvas(KisCanvas2 *canvas)
{
    if (!image()) return;

    KisNodeSP rootNode = image()->root();

    if (m_d->shapesGraph.containsNode(rootNode)) {
        Q_ASSERT(canvas);
        Q_ASSERT(canvas->shapeManager());
        KoSelection *selection = canvas->shapeManager()->selection();
        if (selection && m_d->shapesGraph.nodeToShape(rootNode)) {
            selection->select(m_d->shapesGraph.nodeToShape(rootNode));
            KoToolManager::instance()->switchToolRequested(KoToolManager::instance()->preferredToolForSelection(selection->selectedShapes()));
        }
    }
}

void KisShapeController::setImage(KisImageWSP image)
{
    m_d->imageConnections.clear();

    m_d->imageConnections.addConnection(m_d->doc->image(), SIGNAL(sigResolutionChanged(double, double)), this, SLOT(slotUpdateDocumentResolution()));
    m_d->imageConnections.addConnection(m_d->doc->image(), SIGNAL(sigSizeChanged(QPointF, QPointF)), this, SLOT(slotUpdateDocumentSize()));

    slotUpdateDocumentResolution();
    slotUpdateDocumentSize();

    KisDummiesFacadeBase::setImage(image);
}

KoShapeLayer* KisShapeController::shapeForNode(KisNodeSP node) const
{
    if (node) {
        return m_d->shapesGraph.nodeToShape(node);
    }
    return 0;
}


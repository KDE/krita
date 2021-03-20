/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

#include "KoAddRemoveShapeCommands.h"


struct KisShapeController::Private
{
public:
    KisNameServer *nameServer;
    KisSignalAutoConnectionsStore imageConnections;

    KisNodeShapesGraph shapesGraph;
};

KisShapeController::KisShapeController(KisNameServer *nameServer, KUndo2Stack *undoStack, QObject *parent)
    : KisDummiesFacadeBase(parent)
    , m_d(new Private())
{
    m_d->nameServer = nameServer;
    resourceManager()->setUndoStack(undoStack);
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
    KisImageSP image = this->image();

    if (image) {
        const qreal pixelsPerInch = image->xRes() * 72.0;
        resourceManager()->setResource(KoDocumentResourceManager::DocumentResolution, pixelsPerInch);
    }
}

void KisShapeController::slotUpdateDocumentSize()
{
    KisImageSP image = this->image();

    if (image) {
        resourceManager()->setResource(KoDocumentResourceManager::DocumentRectInPixels, image->bounds());
    }
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

KoShapeContainer *KisShapeController::createParentForShapes(const QList<KoShape *> shapes, KUndo2Command *parentCommand)
{
    KoShapeContainer *resultParent = 0;
    KisCommandUtils::CompositeCommand *resultCommand =
        new KisCommandUtils::CompositeCommand(parentCommand);

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!shapes.isEmpty(), resultParent);
    Q_FOREACH (KoShape *shape, shapes) {
        KIS_SAFE_ASSERT_RECOVER_BREAK(!shape->parent());
    }

    KisCanvas2 *canvas = dynamic_cast<KisCanvas2*>(KoToolManager::instance()->activeCanvasController()->canvas());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(canvas, resultParent);

    const bool baseBelongsToSelection = belongsToShapeSelection(shapes.first());
    bool allSameBelongsToShapeSelection = true;

    Q_FOREACH (KoShape *shape, shapes) {
        allSameBelongsToShapeSelection &= belongsToShapeSelection(shape) == baseBelongsToSelection;
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!baseBelongsToSelection || allSameBelongsToShapeSelection, resultParent);

    if (baseBelongsToSelection && allSameBelongsToShapeSelection) {
        KisSelectionSP selection = canvas->viewManager()->selection();
        if (selection) {
            KisSelectionComponent* shapeSelectionComponent = selection->shapeSelection();

            if (!shapeSelectionComponent) {
                shapeSelectionComponent = new KisShapeSelection(this, image(), selection);
                resultCommand->addCommand(selection->convertToVectorSelection(shapeSelectionComponent));
            }

            KisShapeSelection * shapeSelection = static_cast<KisShapeSelection*>(shapeSelectionComponent);
            resultParent = shapeSelection;
        }
    } else {
        KisShapeLayer *shapeLayer =
                dynamic_cast<KisShapeLayer*>(
                    canvas->selectedShapesProxy()->selection()->activeLayer());

        if (!shapeLayer) {
            shapeLayer = new KisShapeLayer(this, image(),
                                           i18n("Vector Layer %1", m_d->nameServer->number()),
                                           OPACITY_OPAQUE_U8);

            resultCommand->addCommand(
                        new KisImageLayerAddCommand(image(),
                                                    shapeLayer,
                                                    image()->rootLayer(),
                                                    image()->rootLayer()->childCount()));
        }

        resultParent = shapeLayer;
    }

    return resultParent;
}

QRectF KisShapeController::documentRectInPixels() const
{
    KisImageSP image = this->image();
    return image ? image->bounds() : QRect(0, 0, 666, 777);
}

qreal KisShapeController::pixelsPerInch() const
{
    KisImageSP image = this->image();
    return image ? image->xRes() * 72.0 : 72.0;
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

    if (image) {
        m_d->imageConnections.addConnection(image, SIGNAL(sigResolutionChanged(double, double)), this, SLOT(slotUpdateDocumentResolution()));
        m_d->imageConnections.addConnection(image, SIGNAL(sigSizeChanged(QPointF, QPointF)), this, SLOT(slotUpdateDocumentSize()));
    }

    KisDummiesFacadeBase::setImage(image);

    slotUpdateDocumentResolution();
    slotUpdateDocumentSize();
}

KoShapeLayer* KisShapeController::shapeForNode(KisNodeSP node) const
{
    if (node) {
        return m_d->shapesGraph.nodeToShape(node);
    }
    return 0;
}


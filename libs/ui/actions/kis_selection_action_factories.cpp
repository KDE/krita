/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_selection_action_factories.h"

#include <QMimeData>

#include <klocalizedstring.h>
#include <kundo2command.h>

#include <KisMainWindow.h>
#include <KisDocument.h>
#include <KisPart.h>
#include <KoPathShape.h>
#include <KoShapeController.h>
#include <KoShapeRegistry.h>
#include <KoCompositeOpRegistry.h>
#include <KoShapeManager.h>
#include <KoSelection.h>
#include <KoDocumentResourceManager.h>
#include <KoShapeStroke.h>
#include <KoDocumentInfo.h>
#include <KoCanvasBase.h>

#include "KisViewManager.h"
#include "kis_canvas_resource_provider.h"
#include "kis_clipboard.h"
#include "kis_pixel_selection.h"
#include "kis_paint_layer.h"
#include "kis_image.h"
#include "KisImageBarrierLock.h"
#include "kis_fill_painter.h"
#include "kis_transaction.h"
#include "kis_iterator_ng.h"
#include "kis_processing_applicator.h"
#include "kis_group_layer.h"
#include "commands/kis_selection_commands.h"
#include "commands/kis_image_layer_add_command.h"
#include "kis_tool_proxy.h"
#include "kis_canvas2.h"
#include "kis_canvas_controller.h"
#include "kis_selection_manager.h"
#include "commands_new/kis_transaction_based_command.h"
#include "kis_selection_filters.h"
#include "kis_shape_selection.h"
#include "kis_shape_layer.h"
#include <kis_shape_controller.h>
#include "kis_image_animation_interface.h"
#include "kis_time_span.h"
#include "kis_keyframe_channel.h"
#include "kis_node_manager.h"
#include "kis_layer_utils.h"
#include <kis_selection_mask.h>
#include <KisReferenceImagesLayer.h>

#include <processing/fill_processing_visitor.h>
#include <kis_stroke_strategy_undo_command_based.h>
#include <commands_new/kis_processing_command.h>
#include <commands_new/kis_update_command.h>
#include <kis_selection_tool_helper.h>

#include "kis_figure_painting_tool_helper.h"
#include "kis_update_outline_job.h"

namespace ActionHelper {

    void trimDevice(KisViewManager *view,
                        KisPaintDeviceSP device,
                        bool makeSharpClip = false,
                        const KisTimeSpan &range = KisTimeSpan())
    {
        Q_UNUSED(range); // TODO: Allow multiple frame operation across a timespan.

        KisImageWSP image = view->image();
        if (!image) return;

        KisSelectionSP selection = view->selection();

        QRect rc = (selection) ? selection->selectedExactRect() : image->bounds();

        const KoColorSpace *cs = device->colorSpace();

        // We need to allow for trimming from non-transparent defaultPixel layers.
        // Default color should be phased out of use when the area in question is not aligned with image bounds.
        // Otherwise, we can maintain default pixel.
        const bool hasNonTransparentDefaultPixel = device->defaultPixel() != KoColor::createTransparent(device->colorSpace());
        const bool needsTransparentPixel = selection && rc != image->bounds() && hasNonTransparentDefaultPixel;

        if (selection) {
            // Apply selection mask.
            KisPaintDeviceSP selectionProjection = selection->projection();
            const KoColorSpace *selCs = selection->projection()->colorSpace();

            KisSequentialIterator layerIt(device, rc);
            KisSequentialConstIterator selectionIt(selectionProjection, rc);

            while (layerIt.nextPixel() && selectionIt.nextPixel()) {

                /**
                 * Sharp method is an exact reverse of COMPOSITE_OVER
                 * so if you cover the cut/copied piece over its source
                 * you get an exactly the same image without any seams
                 */
                if (makeSharpClip) {
                    qreal dstAlpha = cs->opacityF(layerIt.rawData());
                    qreal sel = selCs->opacityF(selectionIt.oldRawData());
                    qreal newAlpha = sel * dstAlpha / (1.0 - dstAlpha + sel * dstAlpha);
                    float mask = newAlpha / dstAlpha;

                    cs->applyAlphaNormedFloatMask(layerIt.rawData(), &mask, 1);
                } else {
                    cs->applyAlphaU8Mask(layerIt.rawData(), selectionIt.oldRawData(), 1);
                }
            }
        }



        if ( needsTransparentPixel ) {
            device->setDefaultPixel(KoColor::createTransparent(device->colorSpace()));
            device->purgeDefaultPixels();
        }

        device->crop(rc);
    }

    KisImageSP makeImage(KisViewManager *view, KisNodeList nodes)
    {
        KisImageWSP image = view->image();

        KisImageSP clipImage = new KisImage(0, image->width(), image->height(), image->colorSpace(), "ClipImage");
        clipImage->setResolution(image->xRes(), image->yRes());
        Q_FOREACH (KisNodeSP node, nodes) {
            clipImage->addNode(node, clipImage->root());
        }

        clipImage->refreshGraphAsync();
        clipImage->waitForDone();

        return clipImage;
    }
}

void KisSelectAllActionFactory::run(KisViewManager *view)
{
    KisImageWSP image = view->image();
    if (!image) return;

    if (view->canvasBase()->toolProxy()->selectAll()) {
        return;
    }

    KisProcessingApplicator *ap = beginAction(view, kundo2_i18n("Select All"));

    ap->applyCommand(new KisCommandUtils::LambdaCommand(
        [image] () {
            return !image->globalSelection() ?
                new KisSetEmptyGlobalSelectionCommand(image) : 0;
        }),
        KisStrokeJobData::SEQUENTIAL,
        KisStrokeJobData::EXCLUSIVE);

    struct SelectAll : public KisTransactionBasedCommand {
        SelectAll(KisImageSP image) : m_image(image) {}
        KisImageSP m_image;
        KUndo2Command* paint() override {
            KisSelectionSP selection = m_image->globalSelection();
            KisSelectionTransaction transaction(selection->pixelSelection());
            selection->pixelSelection()->clear();
            selection->pixelSelection()->select(m_image->bounds());
            return transaction.endAndTake();
        }
    };

    ap->applyCommand(new SelectAll(image),
                     KisStrokeJobData::SEQUENTIAL,
                     KisStrokeJobData::EXCLUSIVE);

    endAction(ap, KisOperationConfiguration(id()).toXML());
}

void KisDeselectActionFactory::run(KisViewManager *view)
{
    KisImageWSP image = view->image();
    if (!image) return;

    if (view->canvasBase()->toolProxy()->hasSelection()) {
        // see KisCutCopyActionFactory::run
        KisImageBarrierLock lock(image, std::try_to_lock);
        if (!lock.owns_lock()) return;

        view->canvasBase()->toolProxy()->deselect();
        return;
    }

    KUndo2Command *cmd = new KisDeselectActiveSelectionCommand(view->selection(), image);

    KisProcessingApplicator *ap = beginAction(view, cmd->text());
    ap->applyCommand(cmd, KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);
    endAction(ap, KisOperationConfiguration(id()).toXML());
}

void KisReselectActionFactory::run(KisViewManager *view)
{
    KisImageWSP image = view->image();
    if (!image) return;

    KUndo2Command *cmd = new KisReselectActiveSelectionCommand(view->activeNode(), image);

    KisProcessingApplicator *ap = beginAction(view, cmd->text());
    ap->applyCommand(cmd, KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);
    endAction(ap, KisOperationConfiguration(id()).toXML());
}

void KisFillActionFactory::run(const QString &fillSource, KisViewManager *view)
{
    KisNodeSP node = view->activeNode();
    if (!node || !node->hasEditablePaintDevice()) return;

    KisImageWSP image = view->image();

    KisSelectionSP selection = view->selection();

    bool usePattern = false;
    bool useBgColor = false;
    if (fillSource.contains("pattern")) {
        usePattern = true;
    } else if (fillSource.contains("bg")) {
        useBgColor = true;
    }

    KisResourcesSnapshotSP resources =
        new KisResourcesSnapshot(image, node, view->canvasResourceProvider()->resourceManager());
    if (!fillSource.contains("opacity")) {
        resources->setOpacity(1.0);
    }

    KisStrokeStrategyUndoCommandBased *strategy =
            new KisStrokeStrategyUndoCommandBased(
                kundo2_i18n("Flood Fill Layer"), false, image.data()
            );
    strategy->setSupportsWrapAroundMode(true);
    KisStrokeId fillStrokeId = image->startStroke(strategy);
    KIS_SAFE_ASSERT_RECOVER_RETURN(fillStrokeId);

    QSharedPointer<QRect> dirtyRect = QSharedPointer<QRect>(new QRect);

    FillProcessingVisitor *visitor =  new FillProcessingVisitor(nullptr, selection, resources);
    visitor->setSeedPoint(QPoint(0, 0));
    visitor->setSelectionOnly(true);
    visitor->setOutDirtyRect(dirtyRect);
    visitor->setUsePattern(usePattern);
    visitor->setUseBgColor(useBgColor);

    image->addJob(
        fillStrokeId,
        new KisStrokeStrategyUndoCommandBased::Data(
            KUndo2CommandSP(new KisProcessingCommand(visitor, node)),
            false,
            KisStrokeJobData::SEQUENTIAL,
            KisStrokeJobData::EXCLUSIVE
        )
    );

    image->addJob(
        fillStrokeId,
        new KisStrokeStrategyUndoCommandBased::Data(
            KUndo2CommandSP(new KisUpdateCommand(node, dirtyRect, image.data())),
            false,
            KisStrokeJobData::SEQUENTIAL,
            KisStrokeJobData::EXCLUSIVE
        )
    );

    image->endStroke(fillStrokeId);

    view->canvasResourceProvider()->slotPainting();
}

void KisClearActionFactory::run(KisViewManager *view)
{
    // XXX: "Add saving of XML data for Clear action"

    view->canvasBase()->toolProxy()->deleteSelection();
}

void KisImageResizeToSelectionActionFactory::run(KisViewManager *view)
{
    // XXX: "Add saving of XML data for Image Resize To Selection action"

    KisSelectionSP selection = view->selection();
    if (!selection) return;

    view->image()->cropImage(selection->selectedExactRect());
}

void KisCutCopyActionFactory::run(Flags flags, KisViewManager *view)
{
    KisImageSP image = view->image();
    if (!image) return;

    if (!view->blockUntilOperationsFinished(image)) return;

    // Reference layers is a fake node, so it isn't added to the layer stack, this results in KisSelectedShapesProxy not
    // being aware of the active shapeManager and its selected shapes.
    const auto currentToolHasSelection =
        view->canvasBase()->toolProxy()->hasSelection();

    const bool haveShapesSelected =
            view->selectionManager()->haveShapesSelected();

    KisSelectionSP selection = view->selection();

    const bool skipShapes = selection && !currentToolHasSelection;

    if (!skipShapes && !flags.testFlag(SharpClip) && (haveShapesSelected || currentToolHasSelection)) {
        // XXX: "Add saving of XML data for Cut/Copy of shapes"

        /**
         * Make sure that we use tryBarrierLock() here because it does **not**
         * cause requestStrokeEnd() to be called in the tools, hence does not
         * prevent disruptions in the text tool.
         */
        KisImageBarrierLock lock(image, std::try_to_lock);
        if (!lock.owns_lock()) return;

        if (flags & CutClip) {
            view->canvasBase()->toolProxy()->cut();
        } else {
            view->canvasBase()->toolProxy()->copy();
        }
    } else if (selection) {
        KisNodeList selectedNodes = view->nodeManager()->selectedNodes();

        KisNodeList masks;
        Q_FOREACH (KisNodeSP node, selectedNodes) {
            if (node->inherits("KisMask")) {
                masks.append(node);
            }
        }

        selectedNodes = KisLayerUtils::sortAndFilterMergeableInternalNodes(selectedNodes);

        KisNodeList nodes;
        Q_FOREACH (KisNodeSP node, selectedNodes) {
            KisNodeSP dupNode;
            if (node->inherits("KisShapeLayer") || node->inherits("KisFileLayer")) {
                KisPaintDeviceSP dev = new KisPaintDevice(*node->projection());
                // might have to change node's name (vector to paint layer)
                dupNode = new KisPaintLayer(image, node->name(), node->opacity(), dev);
            } else {
                dupNode = node->clone();
            }
            nodes.append(dupNode);
        }

        Q_FOREACH (KisNodeSP node, nodes) {
            KisLayerUtils::recursiveApplyNodes(node, [image, view, flags] (KisNodeSP node) {
                if (node && node->paintDevice()) {
                    node->paintDevice()->burnKeyframe();
                }

                KisTimeSpan range;

                KisKeyframeChannel *channel = node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
                if (channel) {
                    const int currentTime = image->animationInterface()->currentTime();
                    range = channel->affectedFrames(currentTime);
                }

                if (node && node->paintDevice() && !node->inherits("KisMask")) {
                    ActionHelper::trimDevice(view, node->paintDevice(), flags.testFlag(SharpClip), range);
                }
            });
        }

        KisImageSP tempImage = ActionHelper::makeImage(view, nodes);
        KisClipboard::instance()->setLayers(nodes, tempImage);


        KUndo2MagicString actionName = flags & CutClip ?
                    kundo2_i18n("Cut") :
                    kundo2_i18n("Copy");
        KisProcessingApplicator *ap = beginAction(view, actionName);

        if (flags & CutClip) {
            selectedNodes.append(masks);
            Q_FOREACH (KisNodeSP node, selectedNodes) {
                KisLayerUtils::recursiveApplyNodes(node, [selection, masks, ap] (KisNodeSP node){

                    if (!node->hasEditablePaintDevice()) {
                        return;
                    }

                    // applied on masks if selected explicitly (when CTRL-X(cut) is used for deletion)
                    if (node->inherits("KisMask") && !masks.contains(node)) {
                        return;
                    }

                    struct ClearSelection : public KisTransactionBasedCommand {
                        ClearSelection(KisNodeSP node, KisSelectionSP sel)
                            : m_node(node), m_sel(sel) {}
                        KisNodeSP m_node;
                        KisSelectionSP m_sel;

                        KUndo2Command* paint() override {
                            KisSelectionSP cutSelection = m_sel;
                            // Shrinking the cutting area was previously used
                            // for getting seamless cut-paste. Now we use makeSharpClip
                            // instead.
                            // QRect originalRect = cutSelection->selectedExactRect();
                            // static const int preciseSelectionThreshold = 16;
                            //
                            // if (originalRect.width() > preciseSelectionThreshold ||
                            //     originalRect.height() > preciseSelectionThreshold) {
                            //     cutSelection = new KisSelection(*m_sel);
                            //     delete cutSelection->flatten();
                            //
                            //     KisSelectionFilter* filter = new KisShrinkSelectionFilter(1, 1, false);
                            //
                            //     QRect processingRect = filter->changeRect(originalRect);
                            //     filter->process(cutSelection->pixelSelection(), processingRect);
                            // }

                            KisTransaction transaction(m_node->paintDevice());
                            m_node->paintDevice()->clearSelection(cutSelection);
                            m_node->setDirty(cutSelection->selectedRect());
                            return transaction.endAndTake();
                        }
                    };

                    KUndo2Command *command = new ClearSelection(node, selection);
                    ap->applyCommand(command, KisStrokeJobData::CONCURRENT, KisStrokeJobData::NORMAL);

                });
            }
        }

        KisOperationConfiguration config(id());
        config.setProperty("will-cut", flags.testFlag(CutClip));
        config.setProperty("use-sharp-clip", flags.testFlag(SharpClip));
        endAction(ap, config.toXML());
    } else if (!flags.testFlag(SharpClip)) {
        if (flags & CutClip) {
            view->nodeManager()->cutLayersToClipboard();
        } else {
            view->nodeManager()->copyLayersToClipboard();
        }
    }
}

void KisCopyMergedActionFactory::run(KisViewManager *view)
{
    KisImageWSP image = view->image();
    if (!image) return;
    if (!view->blockUntilOperationsFinished(image)) return;

    image->barrierLock();
    KisPaintDeviceSP dev = new KisPaintDevice(*image->root()->projection());
    ActionHelper::trimDevice(view, dev);

    KisNodeSP node = new KisPaintLayer(image, "Projection", OPACITY_OPAQUE_U8, dev);
    KisNodeList nodes{node};

    KisImageSP tempImage = ActionHelper::makeImage(view, nodes);
    KisClipboard::instance()->setLayers(nodes, tempImage);
    image->unlock();

    KisProcessingApplicator *ap = beginAction(view, kundo2_i18n("Copy Merged"));
    endAction(ap, KisOperationConfiguration(id()).toXML());
}

void KisInvertSelectionOperation::runFromXML(KisViewManager* view, const KisOperationConfiguration& config)
{
    KisSelectionFilter* filter = new KisInvertSelectionFilter();

    runFilter(filter, view, config);
}

void KisSelectionToVectorActionFactory::run(KisViewManager *view)
{
    KisSelectionSP selection = view->selection();

    if (selection->hasShapeSelection()) {
        view->showFloatingMessage(i18nc("floating message",
                                        "Selection is already in a vector format "),
                                  QIcon(), 2000, KisFloatingMessage::Low);
        return;
    }

    if (!selection->outlineCacheValid()) {
        view->image()->addSpontaneousJob(new KisUpdateOutlineJob(selection, false, Qt::transparent));
        if (!view->blockUntilOperationsFinished(view->image())) {
            return;
        }
    }

    QPainterPath selectionOutline = selection->outlineCache();
    QTransform transform = view->canvasBase()->coordinatesConverter()->imageToDocumentTransform();

    KoShape *shape = KoPathShape::createShapeFromPainterPath(transform.map(selectionOutline));
    shape->setShapeId(KoPathShapeId);

    /**
     * Mark a shape that it belongs to a shape selection
     */
    if(!shape->userData()) {
        shape->setUserData(new KisShapeSelectionMarker);
    }

    KisProcessingApplicator *ap = beginAction(view, kundo2_i18n("Convert to Vector Selection"));

    ap->applyCommand(view->canvasBase()->shapeController()->addShape(shape, 0),
                     KisStrokeJobData::SEQUENTIAL,
                     KisStrokeJobData::EXCLUSIVE);

    endAction(ap, KisOperationConfiguration(id()).toXML());
}

void KisSelectionToRasterActionFactory::run(KisViewManager *view)
{
    KisSelectionSP selection = view->selection();

    if (!selection->hasShapeSelection()) {
        view->showFloatingMessage(i18nc("floating message",
                                        "Selection is already in a raster format "),
                                  QIcon(), 2000, KisFloatingMessage::Low);
        return;
    }

    KisProcessingApplicator *ap = beginAction(view, kundo2_i18n("Convert to Vector Selection"));

    struct RasterizeSelection : public KisTransactionBasedCommand {
        RasterizeSelection(KisSelectionSP sel)
            : m_sel(sel) {}
        KisSelectionSP m_sel;

        KUndo2Command* paint() override {
            // just create an empty transaction: it will rasterize the
            // selection and Q_EMIT the necessary signals

            KisTransaction transaction(m_sel->pixelSelection());
            return transaction.endAndTake();
        }
    };

    ap->applyCommand(new RasterizeSelection(selection),
                     KisStrokeJobData::SEQUENTIAL,
                     KisStrokeJobData::EXCLUSIVE);

    endAction(ap, KisOperationConfiguration(id()).toXML());
}

void KisShapesToVectorSelectionActionFactory::run(KisViewManager* view)
{
    const QList<KoShape*> originalShapes = view->canvasBase()->shapeManager()->selection()->selectedShapes();

    bool hasSelectionShapes = false;
    QList<KoShape*> clonedShapes;

    Q_FOREACH (KoShape *shape, originalShapes) {
        if (dynamic_cast<KisShapeSelectionMarker*>(shape->userData())) {
            hasSelectionShapes = true;
            continue;
        }

        clonedShapes << shape->cloneShapeAndBakeAbsoluteTransform();
    }

    if (clonedShapes.isEmpty()) {
        if (hasSelectionShapes) {
            view->showFloatingMessage(i18nc("floating message",
                                            "The shape already belongs to a selection"),
                                      QIcon(), 2000, KisFloatingMessage::Low);
        }
        return;
    }

    KisSelectionToolHelper helper(view->canvasBase(), kundo2_i18n("Convert shapes to vector selection"));
    helper.addSelectionShapes(clonedShapes);
}

void KisSelectionToShapeActionFactory::run(KisViewManager *view)
{
    KisSelectionSP selection = view->selection();
    if (!selection->outlineCacheValid()) {
        return;
    }

    QPainterPath selectionOutline = selection->outlineCache();
    QTransform transform = view->canvasBase()->coordinatesConverter()->imageToDocumentTransform();

    KoShape *shape = KoPathShape::createShapeFromPainterPath(transform.map(selectionOutline));
    shape->setShapeId(KoPathShapeId);

    KoColor fgColor = view->canvasBase()->resourceManager()->resource(KoCanvasResource::ForegroundColor).value<KoColor>();
    KoShapeStrokeSP border(new KoShapeStroke(1.0, fgColor.toQColor()));
    shape->setStroke(border);

    KUndo2Command *cmd = view->canvasBase()->shapeController()->addShapeDirect(shape, 0);
    KisProcessingApplicator::runSingleCommandStroke(view->image(), cmd);
}

void KisStrokeSelectionActionFactory::run(KisViewManager *view, const StrokeSelectionOptions& params)
{
    KisImageWSP image = view->image();
    if (!image) {
        return;
    }

    KisSelectionSP selection = view->selection();
    if (!selection) {
        return;
    }

    int size = params.lineSize;

    KisPixelSelectionSP pixelSelection = selection->projection();
    if (!pixelSelection->outlineCacheValid()) {
        pixelSelection->recalculateOutlineCache();
    }

    QPainterPath outline = pixelSelection->outlineCache();
    QColor color = params.color.toQColor();

    KisNodeSP currentNode = view->canvasResourceProvider()->resourceManager()->resource(KoCanvasResource::CurrentKritaNode).value<KisNodeWSP>();
    if (!currentNode->inherits("KisShapeLayer") && currentNode->paintDevice()) {
        KoCanvasResourceProvider * rManager = view->canvasResourceProvider()->resourceManager();
        KisToolShapeUtils::StrokeStyle strokeStyle =  KisToolShapeUtils::StrokeStyleForeground;
        KisToolShapeUtils::FillStyle fillStyle = params.fillStyle();

        KisFigurePaintingToolHelper helper(kundo2_i18n("Draw Polyline"),
                                       image,
                                       currentNode,
                                       rManager ,
                                       strokeStyle,
                                       fillStyle);
        helper.setFGColorOverride(params.color);
        helper.setSelectionOverride(0);
        QPen pen(Qt::red, size);
        pen.setJoinStyle(Qt::RoundJoin);

        if (fillStyle != KisToolShapeUtils::FillStyleNone) {
            helper.paintPainterPathQPenFill(outline, pen, params.fillColor);
        }
        else {
            helper.paintPainterPathQPen(outline, pen, params.fillColor);
        }
    }
    else if (currentNode->inherits("KisShapeLayer")) {

        QTransform transform = view->canvasBase()->coordinatesConverter()->imageToDocumentTransform();

        KoShape *shape = KoPathShape::createShapeFromPainterPath(transform.map(outline));
        shape->setShapeId(KoPathShapeId);

        KoShapeStrokeSP border(new KoShapeStroke(size, color));
        shape->setStroke(border);

        KUndo2Command *cmd = view->canvasBase()->shapeController()->addShapeDirect(shape, 0);
        KisProcessingApplicator::runSingleCommandStroke(view->image(), cmd);
    }
}

void KisStrokeBrushSelectionActionFactory::run(KisViewManager *view, const StrokeSelectionOptions& params)
{
    KisImageWSP image = view->image();
    if (!image) {
        return;
    }

    KisSelectionSP selection = view->selection();
    if (!selection) {
        return;
    }

    KisPixelSelectionSP pixelSelection = selection->projection();
    if (!pixelSelection->outlineCacheValid()) {
        pixelSelection->recalculateOutlineCache();
    }

    KisNodeSP currentNode = view->canvasResourceProvider()->resourceManager()->resource(KoCanvasResource::CurrentKritaNode).value<KisNodeWSP>();
    if (!currentNode->inherits("KisShapeLayer") && currentNode->paintDevice())
    {
        KoCanvasResourceProvider * rManager = view->canvasResourceProvider()->resourceManager();
        QPainterPath outline = pixelSelection->outlineCache();
        KisToolShapeUtils::StrokeStyle strokeStyle =  KisToolShapeUtils::StrokeStyleForeground;
        KisToolShapeUtils::FillStyle fillStyle =  KisToolShapeUtils::FillStyleNone;
        KoColor color = params.color;

        KisFigurePaintingToolHelper helper(kundo2_i18n("Draw Polyline"),
                                       image,
                                       currentNode,
                                       rManager,
                                       strokeStyle,
                                       fillStyle);
        helper.setFGColorOverride(color);
        helper.setSelectionOverride(0);
        helper.paintPainterPath(outline);
    }
}

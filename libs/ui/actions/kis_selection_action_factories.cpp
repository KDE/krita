/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisViewManager.h"
#include "kis_canvas_resource_provider.h"
#include "kis_clipboard.h"
#include "kis_pixel_selection.h"
#include "kis_paint_layer.h"
#include "kis_image.h"
#include "kis_image_barrier_locker.h"
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
#include "kis_time_range.h"
#include "kis_keyframe_channel.h"


#include <processing/fill_processing_visitor.h>
#include <kis_selection_tool_helper.h>

#include "kis_figure_painting_tool_helper.h"
#include "kis_update_outline_job.h"

namespace ActionHelper {

    void copyFromDevice(KisViewManager *view,
                        KisPaintDeviceSP device,
                        bool makeSharpClip = false,
                        const KisTimeRange &range = KisTimeRange())
    {
        KisImageWSP image = view->image();
        if (!image) return;

        KisSelectionSP selection = view->selection();

        QRect rc = (selection) ? selection->selectedExactRect() : image->bounds();

        KisPaintDeviceSP clip = new KisPaintDevice(device->colorSpace());
        Q_CHECK_PTR(clip);

        const KoColorSpace *cs = clip->colorSpace();

        // TODO if the source is linked... copy from all linked layers?!?

        // Copy image data
        KisPainter::copyAreaOptimized(QPoint(), device, clip, rc);

        if (selection) {
            // Apply selection mask.
            KisPaintDeviceSP selectionProjection = selection->projection();
            KisHLineIteratorSP layerIt = clip->createHLineIteratorNG(0, 0, rc.width());
            KisHLineConstIteratorSP selectionIt = selectionProjection->createHLineIteratorNG(rc.x(), rc.y(), rc.width());

            const KoColorSpace *selCs = selection->projection()->colorSpace();

            for (qint32 y = 0; y < rc.height(); y++) {

                for (qint32 x = 0; x < rc.width(); x++) {

                    /**
                     * Sharp method is an exact reverse of COMPOSITE_OVER
                     * so if you cover the cut/copied piece over its source
                     * you get an exactly the same image without any seams
                     */
                    if (makeSharpClip) {
                        qreal dstAlpha = cs->opacityF(layerIt->rawData());
                        qreal sel = selCs->opacityF(selectionIt->oldRawData());
                        qreal newAlpha = sel * dstAlpha / (1.0 - dstAlpha + sel * dstAlpha);
                        float mask = newAlpha / dstAlpha;

                        cs->applyAlphaNormedFloatMask(layerIt->rawData(), &mask, 1);
                    } else {
                        cs->applyAlphaU8Mask(layerIt->rawData(), selectionIt->oldRawData(), 1);
                    }

                    layerIt->nextPixel();
                    selectionIt->nextPixel();
                }
                layerIt->nextRow();
                selectionIt->nextRow();
            }
        }

        KisClipboard::instance()->setClip(clip, rc.topLeft(), range);
    }

}

void KisSelectAllActionFactory::run(KisViewManager *view)
{
    KisImageWSP image = view->image();
    if (!image) return;

    KisProcessingApplicator *ap = beginAction(view, kundo2_i18n("Select All"));

    if (!image->globalSelection()) {
        ap->applyCommand(new KisSetEmptyGlobalSelectionCommand(image),
                         KisStrokeJobData::SEQUENTIAL,
                         KisStrokeJobData::EXCLUSIVE);
    }

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

    KisSelectionSP selection = view->selection();
    QRect selectedRect = selection ?
                         selection->selectedRect() : view->image()->bounds();
    Q_UNUSED(selectedRect);
    KisPaintDeviceSP filled = node->paintDevice()->createCompositionSourceDevice();
    Q_UNUSED(filled);
    bool usePattern = false;
    bool useBgColor = false;

    if (fillSource.contains("pattern")) {
        usePattern = true;
    } else if (fillSource.contains("bg")) {
        useBgColor = true;
    }

    KisProcessingApplicator applicator(view->image(), node,
                                       KisProcessingApplicator::NONE,
                                       KisImageSignalVector() << ModifiedSignal,
                                       kundo2_i18n("Flood Fill Layer"));

    KisResourcesSnapshotSP resources =
        new KisResourcesSnapshot(view->image(), node, view->canvasResourceProvider()->resourceManager());
    if (!fillSource.contains("opacity")) {
        resources->setOpacity(1.0);
    }

    KisProcessingVisitorSP visitor =
        new FillProcessingVisitor(QPoint(0, 0), // start position
                                  selection,
                                  resources,
                                  false, // fast mode
                                  usePattern,
                                  true, // fill only selection,
                                  0, // feathering radius
                                  0, // sizemod
                                  80, // threshold,
                                  false, // unmerged
                                  useBgColor);

    applicator.applyVisitor(visitor,
                            KisStrokeJobData::SEQUENTIAL,
                            KisStrokeJobData::EXCLUSIVE);

    applicator.end();

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

void KisCutCopyActionFactory::run(bool willCut, bool makeSharpClip, KisViewManager *view)
{
    KisImageSP image = view->image();
    if (!image) return;

    bool haveShapesSelected = view->selectionManager()->haveShapesSelected();

    if (haveShapesSelected) {
        // XXX: "Add saving of XML data for Cut/Copy of shapes"

        KisImageBarrierLocker locker(image);
        if (willCut) {
            view->canvasBase()->toolProxy()->cut();
        } else {
            view->canvasBase()->toolProxy()->copy();
        }
    } else {
        KisNodeSP node = view->activeNode();
        if (!node) return;

        KisSelectionSP selection = view->selection();
        if (selection.isNull()) return;

        {
            KisImageBarrierLocker locker(image);
            KisPaintDeviceSP dev = node->paintDevice();
            if (!dev) {
                dev = node->projection();
            }

            if (!dev) {
                view->showFloatingMessage(
                    i18nc("floating message when cannot copy from a node",
                          "Cannot copy pixels from this type of layer "),
                    QIcon(), 3000, KisFloatingMessage::Medium);

                return;
            }

            if (dev->exactBounds().isEmpty()) {
                view->showFloatingMessage(
                    i18nc("floating message when copying empty selection",
                          "Selection is empty: no pixels were copied "),
                    QIcon(), 3000, KisFloatingMessage::Medium);

                return;
            }

            KisTimeRange range;

            KisKeyframeChannel *channel = node->getKeyframeChannel(KisKeyframeChannel::Content.id());
            if (channel) {
                const int currentTime = image->animationInterface()->currentTime();
                range = channel->affectedFrames(currentTime);
            }

            ActionHelper::copyFromDevice(view, dev, makeSharpClip, range);
        }

        KUndo2Command *command = 0;

        if (willCut && node->hasEditablePaintDevice()) {
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

            command = new ClearSelection(node, selection);
        }

        KUndo2MagicString actionName = willCut ?
                    kundo2_i18n("Cut") :
                    kundo2_i18n("Copy");
        KisProcessingApplicator *ap = beginAction(view, actionName);

        if (command) {
            ap->applyCommand(command,
                             KisStrokeJobData::SEQUENTIAL,
                             KisStrokeJobData::NORMAL);
        }

        KisOperationConfiguration config(id());
        config.setProperty("will-cut", willCut);
        endAction(ap, config.toXML());
    }
}

void KisCopyMergedActionFactory::run(KisViewManager *view)
{
    KisImageWSP image = view->image();
    if (!image) return;
    if (!view->blockUntilOperationsFinished(image)) return;

    image->barrierLock();
    KisPaintDeviceSP dev = image->root()->projection();
    ActionHelper::copyFromDevice(view, dev);
    image->unlock();

    KisProcessingApplicator *ap = beginAction(view, kundo2_i18n("Copy Merged"));
    endAction(ap, KisOperationConfiguration(id()).toXML());
}

void KisPasteNewActionFactory::run(KisViewManager *viewManager)
{
    Q_UNUSED(viewManager);

    KisPaintDeviceSP clip = KisClipboard::instance()->clip(QRect(), true);
    if (!clip) return;

    QRect rect = clip->exactBounds();
    if (rect.isEmpty()) return;

    KisDocument *doc = KisPart::instance()->createDocument();
    doc->documentInfo()->setAboutInfo("title", i18n("Untitled"));
    KisImageSP image = new KisImage(doc->createUndoStore(),
                                    rect.width(),
                                    rect.height(),
                                    clip->colorSpace(),
                                    i18n("Pasted"));
    KisPaintLayerSP layer =
        new KisPaintLayer(image.data(), image->nextLayerName() + " " + i18n("(pasted)"),
                          OPACITY_OPAQUE_U8, clip->colorSpace());

    KisPainter::copyAreaOptimized(QPoint(), clip, layer->paintDevice(), rect);

    image->addNode(layer.data(), image->rootLayer());
    doc->setCurrentImage(image);
    KisPart::instance()->addDocument(doc);

    KisMainWindow *win = viewManager->mainWindow();
    win->addViewAndNotifyLoadingCompleted(doc);
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
            // selection and emit the necessary signals

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
        clonedShapes << shape->cloneShape();
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

    KoColor fgColor = view->canvasBase()->resourceManager()->resource(KoCanvasResourceProvider::ForegroundColor).value<KoColor>();
    KoShapeStrokeSP border(new KoShapeStroke(1.0, fgColor.toQColor()));
    shape->setStroke(border);

    view->document()->shapeController()->addShape(shape);
}

void KisStrokeSelectionActionFactory::run(KisViewManager *view, StrokeSelectionOptions params)
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

    KisNodeSP currentNode = view->canvasResourceProvider()->resourceManager()->resource(KisCanvasResourceProvider::CurrentKritaNode).value<KisNodeWSP>();
    if (!currentNode->inherits("KisShapeLayer") && currentNode->paintDevice()) {
        KoCanvasResourceProvider * rManager = view->canvasResourceProvider()->resourceManager();
        KisPainter::StrokeStyle strokeStyle =  KisPainter::StrokeStyleBrush;
        KisPainter::FillStyle fillStyle =  params.fillStyle();

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

        if (fillStyle != KisPainter::FillStyleNone) {
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

        view->document()->shapeController()->addShape(shape);
    }
    image->setModified();

}

void KisStrokeBrushSelectionActionFactory::run(KisViewManager *view, StrokeSelectionOptions params)
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

    KisNodeSP currentNode = view->canvasResourceProvider()->resourceManager()->resource(KisCanvasResourceProvider::CurrentKritaNode).value<KisNodeWSP>();
    if (!currentNode->inherits("KisShapeLayer") && currentNode->paintDevice())
    {
        KoCanvasResourceProvider * rManager = view->canvasResourceProvider()->resourceManager();
        QPainterPath outline = pixelSelection->outlineCache();
        KisPainter::StrokeStyle strokeStyle =  KisPainter::StrokeStyleBrush;
        KisPainter::FillStyle fillStyle =  KisPainter::FillStyleNone;
        KoColor color = params.color;

        KisFigurePaintingToolHelper helper(kundo2_i18n("Draw Polyline"),
                                       image,
                                       currentNode,
                                       rManager ,
                                       strokeStyle,
                                       fillStyle);
        helper.setFGColorOverride(color);
        helper.setSelectionOverride(0);
        helper.paintPainterPath(outline);
        image->setModified();
    }
}

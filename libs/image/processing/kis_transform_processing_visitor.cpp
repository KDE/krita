/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_transform_processing_visitor.h"

#include "klocalizedstring.h"

#include <KoUpdater.h>

#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_selection.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_clone_layer.h"
#include "kis_adjustment_layer.h"
#include "generator/kis_generator_layer.h"

#include "kis_transparency_mask.h"
#include "kis_filter_mask.h"
#include "kis_transform_mask.h"
#include "kis_selection_mask.h"
#include "lazybrush/kis_colorize_mask.h"

#include "kis_external_layer_iface.h"

#include "kis_transaction.h"
#include "kis_undo_adapter.h"
#include "kis_transform_worker.h"
#include "commands_new/kis_node_move_command2.h"

#include "kis_do_something_command.h"
#include <kis_transform_mask_params_interface.h>


KisTransformProcessingVisitor::
KisTransformProcessingVisitor(qreal  xscale, qreal  yscale,
                              qreal  xshear, qreal  yshear,
                              const QPointF &shearOrigin,
                              qreal angle,
                              qint32  tx, qint32  ty,
                              KisFilterStrategy *filter,
                              const QTransform &shapesCorrection)
    : m_sx(xscale), m_sy(yscale)
    , m_tx(tx), m_ty(ty)
    , m_shearx(xshear), m_sheary(yshear)
    , m_shearOrigin(shearOrigin)
    , m_filter(filter)
    , m_angle(angle)
    , m_shapesCorrection(shapesCorrection)
    , m_selectionHelper(0, KisSelectionBasedProcessingHelper::Functor())
{
}

void KisTransformProcessingVisitor::setSelection(KisSelectionSP selection)
{
    m_selectionHelper.setSelection(selection);
}

KUndo2Command *KisTransformProcessingVisitor::createInitCommand()
{
    return m_selectionHelper.createInitCommand(
        std::bind(&KisTransformProcessingVisitor::transformOneDevice,
                  this,
                  std::placeholders::_1,
                  (KoUpdater*)0));
}

void KisTransformProcessingVisitor::visit(KisNode *node, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(node);
    Q_UNUSED(undoAdapter);
}

void KisTransformProcessingVisitor::visit(KisPaintLayer *layer, KisUndoAdapter *undoAdapter)
{
    transformPaintDevice(layer->paintDevice(), undoAdapter, ProgressHelper(layer));
    transformClones(layer, undoAdapter);
}

void KisTransformProcessingVisitor::visit(KisGroupLayer *layer, KisUndoAdapter *undoAdapter)
{
    using namespace KisDoSomethingCommandOps;
    undoAdapter->addCommand(new KisDoSomethingCommand<ResetOp, KisGroupLayer*>(layer, false));
    undoAdapter->addCommand(new KisDoSomethingCommand<ResetOp, KisGroupLayer*>(layer, true));
    transformClones(layer, undoAdapter);

}

void KisTransformProcessingVisitor::visit(KisAdjustmentLayer *layer, KisUndoAdapter *undoAdapter)
{
    using namespace KisDoSomethingCommandOps;
    undoAdapter->addCommand(new KisDoSomethingCommand<ResetOp, KisAdjustmentLayer*>(layer, false));
    transformSelection(layer->internalSelection(), undoAdapter, ProgressHelper(layer));
    undoAdapter->addCommand(new KisDoSomethingCommand<ResetOp, KisAdjustmentLayer*>(layer, true));

    transformClones(layer, undoAdapter);
}

void KisTransformProcessingVisitor::visit(KisExternalLayer *layer, KisUndoAdapter *undoAdapter)
{
    KisTransformWorker tw(layer->projection(), m_sx, m_sy, m_shearx, m_sheary,
                          m_shearOrigin.x(), m_shearOrigin.y(),
                          m_angle, m_tx, m_ty, 0,
                          m_filter);

    KUndo2Command* command = layer->transform(tw.transform() * m_shapesCorrection);
    if(command) {
        undoAdapter->addCommand(command);
    }

    transformClones(layer, undoAdapter);
}

void KisTransformProcessingVisitor::visit(KisGeneratorLayer *layer, KisUndoAdapter *undoAdapter)
{
    using namespace KisDoSomethingCommandOps;
    undoAdapter->addCommand(new KisDoSomethingCommand<UpdateOp, KisGeneratorLayer*>(layer, false));
    transformSelection(layer->internalSelection(), undoAdapter, ProgressHelper(layer));
    undoAdapter->addCommand(new KisDoSomethingCommand<UpdateOp, KisGeneratorLayer*>(layer, true));

    transformClones(layer, undoAdapter);
}

void KisTransformProcessingVisitor::visit(KisCloneLayer *layer, KisUndoAdapter *undoAdapter)
{
    transformClones(layer, undoAdapter);
}

void KisTransformProcessingVisitor::visit(KisFilterMask *mask, KisUndoAdapter *undoAdapter)
{
    transformSelection(mask->selection(), undoAdapter, ProgressHelper(mask));
}

void KisTransformProcessingVisitor::visit(KisTransformMask *mask, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(mask);
    Q_UNUSED(undoAdapter);

    KisTransformWorker tw(0, m_sx, m_sy, m_shearx, m_sheary,
                          m_shearOrigin.x(), m_shearOrigin.y(),
                          m_angle, m_tx, m_ty, 0,
                          m_filter);

    KisTransformMaskParamsInterfaceSP params = mask->transformParams()->clone();
    params->transformSrcAndDst(tw.transform());

    struct UndoCommand : public KUndo2Command
    {
        UndoCommand(KisTransformMaskSP mask,
                    KisTransformMaskParamsInterfaceSP oldParams,
                    KisTransformMaskParamsInterfaceSP newParams)
            : m_mask(mask),
              m_oldParams(oldParams),
              m_newParams(newParams)
        {
        }

        void undo() override {
            m_mask->setTransformParams(m_oldParams);
            m_mask->threadSafeForceStaticImageUpdate();
        }

        void redo() override {
            m_mask->setTransformParams(m_newParams);
            m_mask->threadSafeForceStaticImageUpdate();
        }

    private:
        KisTransformMaskSP m_mask;
        KisTransformMaskParamsInterfaceSP m_oldParams;
        KisTransformMaskParamsInterfaceSP m_newParams;
    };

    undoAdapter->addCommand(new UndoCommand(mask, mask->transformParams(), params));
}

void KisTransformProcessingVisitor::visit(KisTransparencyMask *mask, KisUndoAdapter *undoAdapter)
{
    transformSelection(mask->selection(), undoAdapter, ProgressHelper(mask));
}

void KisTransformProcessingVisitor::visit(KisSelectionMask *mask, KisUndoAdapter *undoAdapter)
{
    transformSelection(mask->selection(), undoAdapter, ProgressHelper(mask));
}

void KisTransformProcessingVisitor::visit(KisColorizeMask *mask, KisUndoAdapter *undoAdapter)
{
    QVector<KisPaintDeviceSP> devices = mask->allPaintDevices();

    Q_FOREACH (KisPaintDeviceSP device, devices) {
        transformPaintDevice(device, undoAdapter, ProgressHelper(mask));
    }
}

void KisTransformProcessingVisitor::transformClones(KisLayer *layer, KisUndoAdapter *undoAdapter)
{
    QList<KisCloneLayerWSP> clones = layer->registeredClones();

    Q_FOREACH (KisCloneLayerSP clone, clones) {
        // we have just casted an object from a weak pointer,
        // so check validity first
        if(!clone) continue;

        KisTransformWorker tw(clone->projection(), m_sx, m_sy, m_shearx, m_sheary,
                              m_shearOrigin.x(), m_shearOrigin.y(),
                              m_angle, m_tx, m_ty, 0,
                              m_filter);

        QTransform trans  = tw.transform();
        QTransform offsetTrans = QTransform::fromTranslate(clone->x(), clone->y());

        QTransform newTrans = trans.inverted() * offsetTrans * trans;

        QPoint oldPos(clone->x(), clone->y());
        QPoint newPos(newTrans.dx(), newTrans.dy());
        KUndo2Command *command = new KisNodeMoveCommand2(clone, oldPos, newPos);
        undoAdapter->addCommand(command);
    }
}

void KisTransformProcessingVisitor::transformPaintDevice(KisPaintDeviceSP device, KisUndoAdapter *adapter, const ProgressHelper &helper)
{
    m_selectionHelper.transformPaintDevice(device,
                                           adapter,
                                           std::bind(&KisTransformProcessingVisitor::transformOneDevice,
                                                     this,
                                                     std::placeholders::_1,
                                                     helper.updater()));
}

void KisTransformProcessingVisitor::transformOneDevice(KisPaintDeviceSP device,
                                                       KoUpdater *updater)
{
    KisTransformWorker tw(device, m_sx, m_sy, m_shearx, m_sheary,
                          m_shearOrigin.x(), m_shearOrigin.y(),
                          m_angle, m_tx, m_ty, updater,
                          m_filter);
    tw.run();
}

void KisTransformProcessingVisitor::transformSelection(KisSelectionSP selection, KisUndoAdapter *adapter, const ProgressHelper &helper)
{
    if (selection->hasShapeSelection()) {
        KisTransformWorker tw(selection->projection(), m_sx, m_sy, m_shearx, m_sheary,
                              m_shearOrigin.x(), m_shearOrigin.y(),
                              m_angle, m_tx, m_ty, 0,
                              m_filter);

        KUndo2Command* command =
            selection->shapeSelection()->transform(tw.transform() * m_shapesCorrection);
        if (command) {
            adapter->addCommand(command);
        }
    } else {
        transformPaintDevice(selection->pixelSelection(), adapter, helper);
    }

    selection->updateProjection();
}

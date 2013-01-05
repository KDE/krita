/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_transform_processing_visitor.h"

#include "klocale.h"

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
#include "kis_selection_mask.h"

#include "kis_external_layer_iface.h"

#include "kis_paint_device.h"
#include "kis_transaction.h"
#include "kis_undo_adapter.h"
#include "kis_transform_worker.h"
#include "commands_new/kis_node_move_command2.h"


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
{
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
    Q_UNUSED(undoAdapter);
    layer->resetCache();
    transformClones(layer, undoAdapter);
}

void KisTransformProcessingVisitor::visit(KisAdjustmentLayer *layer, KisUndoAdapter *undoAdapter)
{
    transformSelection(layer->selection(), undoAdapter, ProgressHelper(layer));
    layer->resetCache();
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
    ProgressHelper helper(layer);
    transformSelection(layer->selection(), undoAdapter, ProgressHelper(layer));
    layer->resetCache();
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

void KisTransformProcessingVisitor::visit(KisTransparencyMask *mask, KisUndoAdapter *undoAdapter)
{
    transformSelection(mask->selection(), undoAdapter, ProgressHelper(mask));
}

void KisTransformProcessingVisitor::visit(KisSelectionMask *mask, KisUndoAdapter *undoAdapter)
{
    transformSelection(mask->selection(), undoAdapter, ProgressHelper(mask));
}

void KisTransformProcessingVisitor::transformClones(KisLayer *layer, KisUndoAdapter *undoAdapter)
{
    QList<KisCloneLayerWSP> clones = layer->registeredClones();

    foreach(KisCloneLayerSP clone, clones) {
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
        KUndo2Command *command = new KisNodeMoveCommand2(clone, oldPos, newPos, undoAdapter);
        undoAdapter->addCommand(command);
    }
}

void KisTransformProcessingVisitor::transformPaintDevice(KisPaintDeviceSP device, KisUndoAdapter *adapter, const ProgressHelper &helper)
{
    KisTransaction transaction(i18n("Transform Node"), device);

    KisTransformWorker tw(device, m_sx, m_sy, m_shearx, m_sheary,
                          m_shearOrigin.x(), m_shearOrigin.y(),
                          m_angle, m_tx, m_ty, helper.updater(),
                          m_filter);
    tw.run();
    transaction.commit(adapter);
}

void KisTransformProcessingVisitor::transformSelection(KisSelectionSP selection, KisUndoAdapter *adapter, const ProgressHelper &helper)
{
    if(selection->hasPixelSelection()) {
        transformPaintDevice(selection->pixelSelection(), adapter, helper);
    }

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
    }

    selection->updateProjection();
}


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

#include "move_selection_stroke_strategy.h"

#include <klocalizedstring.h>
#include <KoColorSpace.h>
#include <KoCompositeOpRegistry.h>
#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_painter.h"
#include "kis_transaction.h"
#include <commands_new/kis_selection_move_command2.h>


MoveSelectionStrokeStrategy::MoveSelectionStrokeStrategy(KisPaintLayerSP paintLayer,
                                                         KisSelectionSP selection,
                                                         KisUpdatesFacade *updatesFacade,
                                                         KisStrokeUndoFacade *undoFacade)
    : KisStrokeStrategyUndoCommandBased(kundo2_i18n("Move Selection"), false, undoFacade),
      m_paintLayer(paintLayer),
      m_selection(selection),
      m_updatesFacade(updatesFacade)
{
    enableJob(KisSimpleStrokeStrategy::JOB_INIT);
    enableJob(KisSimpleStrokeStrategy::JOB_FINISH);
    enableJob(KisSimpleStrokeStrategy::JOB_CANCEL);
}

MoveSelectionStrokeStrategy::MoveSelectionStrokeStrategy(const MoveSelectionStrokeStrategy &rhs)
    : KisStrokeStrategyUndoCommandBased(rhs),
      m_paintLayer(rhs.m_paintLayer),
      m_selection(rhs.m_selection),
      m_updatesFacade(rhs.m_updatesFacade)
{
}

void MoveSelectionStrokeStrategy::initStrokeCallback()
{
    KisStrokeStrategyUndoCommandBased::initStrokeCallback();

    KisPaintDeviceSP paintDevice = m_paintLayer->paintDevice();

    KisPaintDeviceSP movedDevice = new KisPaintDevice(m_paintLayer.data(), paintDevice->colorSpace());


    QRect copyRect = m_selection->selectedRect();
    KisPainter gc(movedDevice);
    gc.setSelection(m_selection);
    gc.bitBlt(copyRect.topLeft(), paintDevice, copyRect);
    gc.end();

    KisTransaction cutTransaction(name(), paintDevice);
    paintDevice->clearSelection(m_selection);
    runAndSaveCommand(KUndo2CommandSP(cutTransaction.endAndTake()),
                      KisStrokeJobData::SEQUENTIAL,
                      KisStrokeJobData::NORMAL);

    KisIndirectPaintingSupport *indirect =
        static_cast<KisIndirectPaintingSupport*>(m_paintLayer.data());
    indirect->setTemporaryTarget(movedDevice);
    indirect->setTemporaryCompositeOp(COMPOSITE_OVER);
    indirect->setTemporaryOpacity(OPACITY_OPAQUE_U8);

    m_initialDeviceOffset = QPoint(movedDevice->x(), movedDevice->y());

    m_selection->setVisible(false);
}

void MoveSelectionStrokeStrategy::finishStrokeCallback()
{
    KisIndirectPaintingSupport *indirect =
        static_cast<KisIndirectPaintingSupport*>(m_paintLayer.data());

    KisTransaction transaction(name(), m_paintLayer->paintDevice());
    indirect->mergeToLayer(m_paintLayer, (KisPostExecutionUndoAdapter*)0, KUndo2MagicString());

    runAndSaveCommand(KUndo2CommandSP(transaction.endAndTake()),
                      KisStrokeJobData::SEQUENTIAL,
                      KisStrokeJobData::NORMAL);

    indirect->setTemporaryTarget(0);

    QPoint selectionOffset(m_selection->x(), m_selection->y());

    m_updatesFacade->blockUpdates();

    KUndo2CommandSP moveSelectionCommand(
        new KisSelectionMoveCommand2(m_selection, selectionOffset, selectionOffset + m_finalOffset));

    runAndSaveCommand(
        moveSelectionCommand,
            KisStrokeJobData::SEQUENTIAL,
            KisStrokeJobData::EXCLUSIVE);

    m_updatesFacade->unblockUpdates();

    m_selection->setVisible(true);

    KisStrokeStrategyUndoCommandBased::finishStrokeCallback();
}

void MoveSelectionStrokeStrategy::cancelStrokeCallback()
{
    KisIndirectPaintingSupport *indirect =
        static_cast<KisIndirectPaintingSupport*>(m_paintLayer.data());

    if (indirect) {
        KisPaintDeviceSP t = indirect->temporaryTarget();
        if (t) {
            QRegion dirtyRegion = t->region();

            indirect->setTemporaryTarget(0);

            m_selection->setVisible(true);

            m_paintLayer->setDirty(dirtyRegion);
        }
    }
    KisStrokeStrategyUndoCommandBased::cancelStrokeCallback();
}

#include "tool/strokes/move_stroke_strategy.h"

void MoveSelectionStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    MoveStrokeStrategy::Data *d = dynamic_cast<MoveStrokeStrategy::Data*>(data);

    if (d) {
        KisIndirectPaintingSupport *indirect =
            static_cast<KisIndirectPaintingSupport*>(m_paintLayer.data());

        KisPaintDeviceSP movedDevice = indirect->temporaryTarget();

        QRegion dirtyRegion = movedDevice->region();

        QPoint currentDeviceOffset(movedDevice->x(), movedDevice->y());
        QPoint newDeviceOffset(m_initialDeviceOffset + d->offset);

        dirtyRegion |= dirtyRegion.translated(newDeviceOffset - currentDeviceOffset);

        movedDevice->setX(newDeviceOffset.x());
        movedDevice->setY(newDeviceOffset.y());
        m_finalOffset = d->offset;

        m_paintLayer->setDirty(dirtyRegion);
    } else {
        KisStrokeStrategyUndoCommandBased::doStrokeCallback(data);
    }
}

KisStrokeStrategy* MoveSelectionStrokeStrategy::createLodClone(int levelOfDetail)
{
    Q_UNUSED(levelOfDetail);

    MoveSelectionStrokeStrategy *clone = new MoveSelectionStrokeStrategy(*this);
    return clone;
}

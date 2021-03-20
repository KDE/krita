/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_painter_based_stroke_strategy.h"

#include <KoColorSpace.h>
#include <KoColor.h>
#include <KoCompositeOp.h>
#include "kis_painter.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "kis_transaction.h"
#include "kis_image.h"
#include <kis_distance_information.h>
#include "kis_undo_stores.h"
#include "KisFreehandStrokeInfo.h"
#include "KisMaskedFreehandStrokePainter.h"
#include "KisMaskingBrushRenderer.h"
#include "KisRunnableStrokeJobData.h"

#include "kis_paintop_preset.h"
#include "kis_paintop_settings.h"

KisPainterBasedStrokeStrategy::KisPainterBasedStrokeStrategy(const QLatin1String &id,
                                                             const KUndo2MagicString &name,
                                                             KisResourcesSnapshotSP resources,
                                                             QVector<KisFreehandStrokeInfo*> strokeInfos,bool useMergeID)
    : KisRunnableBasedStrokeStrategy(id, name),
      m_resources(resources),
      m_strokeInfos(strokeInfos),
      m_transaction(0),
      m_useMergeID(useMergeID),
      m_supportsMaskingBrush(false),
      m_supportsIndirectPainting(false)
{
    init();
}

KisPainterBasedStrokeStrategy::KisPainterBasedStrokeStrategy(const QLatin1String &id,
                                                             const KUndo2MagicString &name,
                                                             KisResourcesSnapshotSP resources,
                                                             KisFreehandStrokeInfo *strokeInfo,bool useMergeID)
    : KisRunnableBasedStrokeStrategy(id, name),
      m_resources(resources),
      m_strokeInfos(QVector<KisFreehandStrokeInfo*>() <<  strokeInfo),
      m_transaction(0),
      m_useMergeID(useMergeID),
      m_supportsMaskingBrush(false),
      m_supportsIndirectPainting(false)
{
    init();
}

KisPainterBasedStrokeStrategy::~KisPainterBasedStrokeStrategy()
{
}

void KisPainterBasedStrokeStrategy::init()
{
    enableJob(KisSimpleStrokeStrategy::JOB_INIT);
    enableJob(KisSimpleStrokeStrategy::JOB_FINISH);
    enableJob(KisSimpleStrokeStrategy::JOB_CANCEL, true, KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);

    enableJob(KisSimpleStrokeStrategy::JOB_SUSPEND);
    enableJob(KisSimpleStrokeStrategy::JOB_RESUME);
}

KisPainterBasedStrokeStrategy::KisPainterBasedStrokeStrategy(const KisPainterBasedStrokeStrategy &rhs, int levelOfDetail)
    : KisRunnableBasedStrokeStrategy(rhs),
      m_resources(rhs.m_resources),
      m_transaction(rhs.m_transaction),
      m_useMergeID(rhs.m_useMergeID),
      m_supportsMaskingBrush(rhs.m_supportsMaskingBrush),
      m_supportsIndirectPainting(rhs.m_supportsIndirectPainting)
{
    Q_FOREACH (KisFreehandStrokeInfo *info, rhs.m_strokeInfos) {
        m_strokeInfos.append(new KisFreehandStrokeInfo(info, levelOfDetail));
    }

    KIS_ASSERT_RECOVER_NOOP(
        rhs.m_maskStrokeInfos.isEmpty() &&
        !rhs.m_transaction &&
        !rhs.m_targetDevice &&
        !rhs.m_activeSelection &&
        "After the stroke has been started, no copying must happen");
}

KisPaintDeviceSP KisPainterBasedStrokeStrategy::targetDevice() const
{
    return m_targetDevice;
}

KisSelectionSP KisPainterBasedStrokeStrategy::activeSelection() const
{
    return m_activeSelection;
}

KisMaskedFreehandStrokePainter *KisPainterBasedStrokeStrategy::maskedPainter(int strokeInfoId)
{
    return m_maskedPainters[strokeInfoId];
}

int KisPainterBasedStrokeStrategy::numMaskedPainters() const
{
    return m_maskedPainters.size();
}

bool KisPainterBasedStrokeStrategy::needsMaskingUpdates() const
{
    return m_maskingBrushRenderer;
}

QVector<KisRunnableStrokeJobData *> KisPainterBasedStrokeStrategy::doMaskingBrushUpdates(const QVector<QRect> &rects)
{
    QVector<KisRunnableStrokeJobData *> jobs;
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_maskingBrushRenderer, jobs);

    Q_FOREACH (const QRect &rc, rects) {
        jobs.append(new KisRunnableStrokeJobData(
            [this, rc] () {
                this->m_maskingBrushRenderer->updateProjection(rc);
            },
            KisStrokeJobData::CONCURRENT));
    }

    return jobs;
}

void KisPainterBasedStrokeStrategy::setSupportsMaskingBrush(bool value)
{
    m_supportsMaskingBrush = value;
}

bool KisPainterBasedStrokeStrategy::supportsMaskingBrush() const
{
    return m_supportsMaskingBrush;
}

void KisPainterBasedStrokeStrategy::setSupportsIndirectPainting(bool value)
{
    m_supportsIndirectPainting = value;
}

bool KisPainterBasedStrokeStrategy::supportsIndirectPainting() const
{
    return m_supportsIndirectPainting;
}

void KisPainterBasedStrokeStrategy::initPainters(KisPaintDeviceSP targetDevice,
                                                 KisPaintDeviceSP maskingDevice,
                                                 KisSelectionSP selection,
                                                 bool hasIndirectPainting,
                                                 const QString &indirectPaintingCompositeOp)
{
    Q_FOREACH (KisFreehandStrokeInfo *info, m_strokeInfos) {
        KisPainter *painter = info->painter;

        painter->begin(targetDevice, !hasIndirectPainting ? selection : 0);
        painter->setRunnableStrokeJobsInterface(runnableJobsInterface());
        m_resources->setupPainter(painter);

        if(hasIndirectPainting) {
            painter->setCompositeOp(targetDevice->colorSpace()->compositeOp(indirectPaintingCompositeOp));
            painter->setOpacity(OPACITY_OPAQUE_U8);
            painter->setChannelFlags(QBitArray());
        }
    }

    if (maskingDevice) {
        for (int i = 0; i < m_strokeInfos.size(); i++) {
            KisFreehandStrokeInfo *maskingInfo =
                new KisFreehandStrokeInfo(*m_strokeInfos[i]->dragDistance);

            KisPainter *painter = maskingInfo->painter;

            painter->begin(maskingDevice, 0);
            m_resources->setupMaskingBrushPainter(painter);

            KIS_SAFE_ASSERT_RECOVER_NOOP(hasIndirectPainting);
            m_maskStrokeInfos.append(maskingInfo);
        }
    }

    for (int i = 0; i < m_strokeInfos.size(); i++) {
        m_maskedPainters.append(
            new KisMaskedFreehandStrokePainter(m_strokeInfos[i],
                                               !m_maskStrokeInfos.isEmpty() ?
                                                   m_maskStrokeInfos[i] : 0));
    }
}

void KisPainterBasedStrokeStrategy::deletePainters()
{
    Q_FOREACH (KisFreehandStrokeInfo *info, m_strokeInfos) {
        delete info;
    }

    m_strokeInfos.clear();

    Q_FOREACH (KisFreehandStrokeInfo *info, m_maskStrokeInfos) {
        delete info;
    }

    m_maskStrokeInfos.clear();

    Q_FOREACH (KisMaskedFreehandStrokePainter *info, m_maskedPainters) {
        delete info;
    }

    m_maskedPainters.clear();
}

void KisPainterBasedStrokeStrategy::initStrokeCallback()
{
    KisNodeSP node = m_resources->currentNode();
    KisPaintDeviceSP paintDevice = node->paintDevice();
    KisPaintDeviceSP targetDevice = paintDevice;
    bool hasIndirectPainting = supportsIndirectPainting() && m_resources->needsIndirectPainting();
    const QString indirectCompositeOp = m_resources->indirectPaintingCompositeOp();

    KisSelectionSP selection =  m_resources->activeSelection();

    if (hasIndirectPainting) {
        KisIndirectPaintingSupport *indirect =
            dynamic_cast<KisIndirectPaintingSupport*>(node.data());

        if (indirect) {
            targetDevice = paintDevice->createCompositionSourceDevice();
            targetDevice->setParentNode(node);
            indirect->setCurrentColor(m_resources->currentFgColor());
            indirect->setTemporaryTarget(targetDevice);

            indirect->setTemporaryCompositeOp(m_resources->compositeOpId());
            indirect->setTemporaryOpacity(m_resources->opacity());
            indirect->setTemporarySelection(selection);

            QBitArray channelLockFlags = m_resources->channelLockFlags();
            indirect->setTemporaryChannelFlags(channelLockFlags);
        }
        else {
            hasIndirectPainting = false;
        }
    }
    if (m_useMergeID) {
        m_transaction = new KisTransaction(name(), targetDevice, 0, timedID(this->id()));
    }
    else {
        m_transaction = new KisTransaction(name(), targetDevice);
    }

    // WARNING: masked brush cannot work without indirect painting mode!
    KIS_SAFE_ASSERT_RECOVER_NOOP(!(supportsMaskingBrush() &&
                                   m_resources->needsMaskingBrushRendering()) || hasIndirectPainting);

    if (hasIndirectPainting &&
        supportsMaskingBrush() &&
        m_resources->needsMaskingBrushRendering()) {

        const QString compositeOpId =
            m_resources->currentPaintOpPreset()->settings()->maskingBrushCompositeOp();

        m_maskingBrushRenderer.reset(new KisMaskingBrushRenderer(targetDevice, compositeOpId));

        initPainters(m_maskingBrushRenderer->strokeDevice(),
                     m_maskingBrushRenderer->maskDevice(),
                     selection,
                     hasIndirectPainting,
                     indirectCompositeOp);

    } else {
        initPainters(targetDevice, 0, selection, hasIndirectPainting, indirectCompositeOp);
    }

    m_targetDevice = targetDevice;
    m_activeSelection = selection;

    // sanity check: selection should be applied only once
    if (selection && !m_strokeInfos.isEmpty()) {
        KisIndirectPaintingSupport *indirect =
            dynamic_cast<KisIndirectPaintingSupport*>(node.data());
        KIS_ASSERT_RECOVER_RETURN(hasIndirectPainting || m_strokeInfos.first()->painter->selection());
        KIS_ASSERT_RECOVER_RETURN(!hasIndirectPainting || !indirect->temporarySelection() || !m_strokeInfos.first()->painter->selection());
    }
}

void KisPainterBasedStrokeStrategy::finishStrokeCallback()
{
    KisNodeSP node = m_resources->currentNode();
    KisIndirectPaintingSupport *indirect =
        dynamic_cast<KisIndirectPaintingSupport*>(node.data());

    KisPostExecutionUndoAdapter *undoAdapter =
        m_resources->postExecutionUndoAdapter();

    QScopedPointer<KisPostExecutionUndoAdapter> dumbUndoAdapter;
    QScopedPointer<KisUndoStore> dumbUndoStore;

    if (!undoAdapter) {
        dumbUndoStore.reset(new KisDumbUndoStore());
        dumbUndoAdapter.reset(new KisPostExecutionUndoAdapter(dumbUndoStore.data(), 0));

        undoAdapter = dumbUndoAdapter.data();
    }


    if (indirect && indirect->hasTemporaryTarget()) {
        KUndo2MagicString transactionText = m_transaction->text();
        m_transaction->end();
        if(m_useMergeID){
            indirect->mergeToLayer(node,
                                   undoAdapter,
                                   transactionText,timedID(this->id()));
        }
        else{
            indirect->mergeToLayer(node,
                                   undoAdapter,
                                   transactionText);
        }
    }
    else {
        m_transaction->commit(undoAdapter);
    }
    delete m_transaction;
    deletePainters();
}

void KisPainterBasedStrokeStrategy::cancelStrokeCallback()
{
    KisNodeSP node = m_resources->currentNode();
    KisIndirectPaintingSupport *indirect =
        dynamic_cast<KisIndirectPaintingSupport*>(node.data());

    bool revert = true;
    if (indirect) {
        KisPaintDeviceSP t = indirect->temporaryTarget();
        if (t) {
            delete m_transaction;
            deletePainters();

            KisRegion region = t->region();
            indirect->setTemporaryTarget(0);
            node->setDirty(region);
            revert = false;
        }
    }

    if (revert) {
        m_transaction->revert();
        delete m_transaction;
        deletePainters();
    }
}

void KisPainterBasedStrokeStrategy::suspendStrokeCallback()
{
    KisNodeSP node = m_resources->currentNode();
    KisIndirectPaintingSupport *indirect =
        dynamic_cast<KisIndirectPaintingSupport*>(node.data());

    if(indirect && indirect->hasTemporaryTarget()) {
        indirect->setTemporaryTarget(0);
    }
}

void KisPainterBasedStrokeStrategy::resumeStrokeCallback()
{
    KisNodeSP node = m_resources->currentNode();
    KisIndirectPaintingSupport *indirect =
        dynamic_cast<KisIndirectPaintingSupport*>(node.data());

    if(indirect) {
        // todo: don't ask about paint device
        // todo:change to an assert
        if (node->paintDevice() != m_targetDevice) {
            indirect->setTemporaryTarget(m_targetDevice);
            indirect->setTemporaryCompositeOp(m_resources->compositeOpId());
            indirect->setTemporaryOpacity(m_resources->opacity());
            indirect->setTemporarySelection(m_activeSelection);
        }
    }
}

KisNodeSP KisPainterBasedStrokeStrategy::targetNode() const
{
    return m_resources->currentNode();
}

/*
 *  SPDX-FileCopyrightText: 2004 Bart Coppens <kde@bartcoppens.be>
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <compositeops/KoVcMultiArchBuildSupport.h> //MSVC requires that Vc come first
#include "kis_indirect_painting_support.h"

#include <QMutex>
#include <QMutexLocker>
#include <QReadWriteLock>

#include <KoCompositeOp.h>
#include "kis_layer.h"
#include "kis_paint_layer.h"
#include "kis_paint_device.h"
#include "kis_selection.h"
#include "kis_painter.h"
#include <KisFakeRunnableStrokeJobsExecutor.h>
#include "KisRunnableStrokeJobData.h"
#include "KisRunnableStrokeJobUtils.h"
#include "kis_transaction.h"
#include "kis_pointer_utils.h"


struct Q_DECL_HIDDEN KisIndirectPaintingSupport::Private {
    // To simulate the indirect painting
    KisPaintDeviceSP temporaryTarget;
    QString compositeOp;
    quint8 compositeOpacity;
    QBitArray channelFlags;
    KisSelectionSP selection;

    QReadWriteLock lock;
    bool finalMergeInProgress = true;
};


KisIndirectPaintingSupport::KisIndirectPaintingSupport()
    : d(new Private)
{
}

KisIndirectPaintingSupport::~KisIndirectPaintingSupport()
{
    delete d;
}

void KisIndirectPaintingSupport::setCurrentColor(const KoColor &color)
{
    Q_UNUSED(color);
}

void KisIndirectPaintingSupport::setTemporaryTarget(KisPaintDeviceSP t)
{
    d->temporaryTarget = t;
}

void KisIndirectPaintingSupport::setTemporaryCompositeOp(const QString &id)
{
    d->compositeOp = id;
}

void KisIndirectPaintingSupport::setTemporaryOpacity(quint8 o)
{
    d->compositeOpacity = o;
}

void KisIndirectPaintingSupport::setTemporaryChannelFlags(const QBitArray& channelFlags)
{
    d->channelFlags = channelFlags;
}

void KisIndirectPaintingSupport::setTemporarySelection(KisSelectionSP selection)
{
    d->selection = selection;
}

void KisIndirectPaintingSupport::lockTemporaryTarget() const
{
    d->lock.lockForRead();
}

void KisIndirectPaintingSupport::lockTemporaryTargetForWrite() const
{
    d->lock.lockForWrite();
    d->finalMergeInProgress = true;
}

void KisIndirectPaintingSupport::unlockTemporaryTarget() const
{
    d->lock.unlock();
    d->finalMergeInProgress = false;
}

KisPaintDeviceSP KisIndirectPaintingSupport::temporaryTarget() const
{
    return d->temporaryTarget;
}

bool KisIndirectPaintingSupport::supportsNonIndirectPainting() const
{
    return true;
}

KisIndirectPaintingSupport::FinalMergeSuspenderSP KisIndirectPaintingSupport::trySuspendFinalMerge()
{
    return toQShared(d->finalMergeInProgress ? new FinalMergeSuspender(this) : nullptr);
}

QString KisIndirectPaintingSupport::temporaryCompositeOp() const
{
    return d->compositeOp;
}

KisSelectionSP KisIndirectPaintingSupport::temporarySelection() const
{
    return d->selection;
}

bool KisIndirectPaintingSupport::hasTemporaryTarget() const
{
    return d->temporaryTarget;
}

void KisIndirectPaintingSupport::setupTemporaryPainter(KisPainter *painter) const
{
     painter->setOpacity(d->compositeOpacity);
     painter->setCompositeOp(d->compositeOp);
     painter->setChannelFlags(d->channelFlags);
     painter->setSelection(d->selection);
}

void KisIndirectPaintingSupport::mergeToLayer(KisNodeSP layer, KisPostExecutionUndoAdapter *undoAdapter, const KUndo2MagicString &transactionText, int timedID)
{
    QVector<KisRunnableStrokeJobData*> jobs;
    mergeToLayerThreaded(layer, undoAdapter, transactionText, timedID, &jobs);

    /**
     * When merging, we use barrier jobs only for ensuring that the merge jobs
     * are not split by the update jobs. Merge jobs hold the shared lock, so
     * forcinf them out of CPU will basically cause a deadlock. When running in
     * the fake executor, the jobs cannot be split anyway, so there is no danger
     * in that.
     */
    KisFakeRunnableStrokeJobsExecutor executor(KisFakeRunnableStrokeJobsExecutor::AllowBarrierJobs);
    executor.addRunnableJobs(implicitCastList<KisRunnableStrokeJobDataBase*>(jobs));
}

void KisIndirectPaintingSupport::mergeToLayerThreaded(KisNodeSP layer, KisPostExecutionUndoAdapter *undoAdapter, const KUndo2MagicString &transactionText,int timedID, QVector<KisRunnableStrokeJobData*> *jobs)
{
    /**
     * We create the lock in an unlocked state to avoid a deadlock, when
     * layer-stack updating jobs push out the stroke jobs from the CPU and
     * start sleeping on lockTemporaryTarget().
     */

    WriteLockerSP sharedWriteLock(new WriteLocker(this, std::defer_lock));

    /**
     * Now wait for all update jobs to finish and lock the indirect target
     */
    KritaUtils::addJobBarrier(*jobs,
        [sharedWriteLock] () {
            sharedWriteLock->relock();
        });

    mergeToLayerImpl(layer->paintDevice(), undoAdapter, transactionText,
                     timedID, true, sharedWriteLock,
                     jobs);
}

void KisIndirectPaintingSupport::mergeToLayerImpl(KisPaintDeviceSP dst, KisPostExecutionUndoAdapter *undoAdapter, const KUndo2MagicString &transactionText, int timedID, bool cleanResources,
                                                  WriteLockerSP sharedWriteLock, QVector<KisRunnableStrokeJobData*> *jobs)
{
    struct SharedState {
        QScopedPointer<KisTransaction> transaction;
    };

    QSharedPointer<SharedState> sharedState(new SharedState());

    KritaUtils::addJobSequential(*jobs,
        [sharedState, sharedWriteLock, dst, undoAdapter, transactionText, timedID] () {
            Q_UNUSED(sharedWriteLock); // just a RAII holder object for the lock

            /**
             * Move tool may not have an undo adapter
             */
             if (undoAdapter) {
                 sharedState->transaction.reset(
                     new KisTransaction(transactionText, dst, nullptr, timedID));
             }
        }
    );

    KisPaintDeviceSP src = d->temporaryTarget;
    Q_FOREACH (const QRect &rc, src->region().rects()) {
        KritaUtils::addJobConcurrent(*jobs,
            [this, rc, src, dst, sharedState, sharedWriteLock] () {
                Q_UNUSED(sharedWriteLock); // just a RAII holder object for the lock

                /**
                 * Brushes don't apply the selection, we apply that during the indirect
                 * painting merge operation. It is cheaper calculation-wise.
                 */

                KisPainter gc(dst);
                setupTemporaryPainter(&gc);
                this->writeMergeData(&gc, src, rc);
            }
        );
    }

    KritaUtils::addJobSequential(*jobs,
        [this, sharedState, sharedWriteLock, undoAdapter, cleanResources] () {
            Q_UNUSED(sharedWriteLock); // just a RAII holder object for the lock

            if (cleanResources) {
                releaseResources();
            }

            if (sharedState->transaction) {
                sharedState->transaction->commit(undoAdapter);
            }
        }
    );
}

void KisIndirectPaintingSupport::writeMergeData(KisPainter *painter, KisPaintDeviceSP src, const QRect &rc)
{
    painter->bitBlt(rc.topLeft(), src, rc);
}

void KisIndirectPaintingSupport::releaseResources()
{
    d->temporaryTarget = 0;
    d->selection = 0;
    d->compositeOp = COMPOSITE_OVER;
    d->compositeOpacity = OPACITY_OPAQUE_U8;
    d->channelFlags.clear();
}

KisIndirectPaintingSupport::FinalMergeSuspender::FinalMergeSuspender(KisIndirectPaintingSupport *indirect)
    : m_lock(indirect)
{
    m_lock->unlockTemporaryTarget();
}

KisIndirectPaintingSupport::FinalMergeSuspender::~FinalMergeSuspender()
{
    m_lock->lockTemporaryTargetForWrite();
}

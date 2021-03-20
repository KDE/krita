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


struct Q_DECL_HIDDEN KisIndirectPaintingSupport::Private {
    // To simulate the indirect painting
    KisPaintDeviceSP temporaryTarget;
    QString compositeOp;
    quint8 compositeOpacity;
    QBitArray channelFlags;
    KisSelectionSP selection;

    QReadWriteLock lock;
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
}

void KisIndirectPaintingSupport::unlockTemporaryTarget() const
{
    d->lock.unlock();
}

KisPaintDeviceSP KisIndirectPaintingSupport::temporaryTarget() const
{
    return d->temporaryTarget;
}

bool KisIndirectPaintingSupport::supportsNonIndirectPainting() const
{
    return true;
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

void KisIndirectPaintingSupport::mergeToLayer(KisNodeSP layer, KisPostExecutionUndoAdapter *undoAdapter, const KUndo2MagicString &transactionText,int timedID)
{
    QWriteLocker l(&d->lock);
    mergeToLayerImpl(layer->paintDevice(), undoAdapter, transactionText, timedID);
}

void KisIndirectPaintingSupport::mergeToLayerImpl(KisPaintDeviceSP dst, KisPostExecutionUndoAdapter *undoAdapter, const KUndo2MagicString &transactionText, int timedID, bool cleanResources)
{
    /**
     * Brushes don't apply the selection, we apply that during the indirect
     * painting merge operation. It is cheaper calculation-wise.
     */
    KisPainter gc(dst);
    setupTemporaryPainter(&gc);

    /**
     * Scratchpad may not have an undo adapter
     */
    if(undoAdapter) {
        gc.beginTransaction(transactionText,timedID);
    }

    writeMergeData(&gc, d->temporaryTarget);

    if (cleanResources) {
        releaseResources();
    }

    if(undoAdapter) {
        gc.endTransaction(undoAdapter);
    }
}

void KisIndirectPaintingSupport::writeMergeData(KisPainter *painter, KisPaintDeviceSP src)
{
    Q_FOREACH (const QRect &rc, src->region().rects()) {
        painter->bitBlt(rc.topLeft(), src, rc);
    }
}

void KisIndirectPaintingSupport::releaseResources()
{
    d->temporaryTarget = 0;
    d->selection = 0;
    d->compositeOp = COMPOSITE_OVER;
    d->compositeOpacity = OPACITY_OPAQUE_U8;
    d->channelFlags.clear();
}

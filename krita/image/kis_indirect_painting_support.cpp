/*
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_indirect_painting_support.h"

#include <QMutex>
#include <QMutexLocker>
#include <QReadWriteLock>

#include <KoCompositeOp.h>
#include "kis_layer.h"
#include "kis_paint_layer.h"
#include "kis_paint_device.h"
#include "kis_painter.h"


struct KisIndirectPaintingSupport::Private {
    // To simulate the indirect painting
    KisPaintDeviceSP temporaryTarget;
    const KoCompositeOp* compositeOp;
    quint8 compositeOpacity;
    QBitArray channelFlags;
    QReadWriteLock lock;
};


KisIndirectPaintingSupport::KisIndirectPaintingSupport()
    : d(new Private)
{
    d->compositeOp = 0;
}

KisIndirectPaintingSupport::~KisIndirectPaintingSupport()
{
    delete d;
}

void KisIndirectPaintingSupport::setTemporaryTarget(KisPaintDeviceSP t)
{
    d->temporaryTarget = t;
}

void KisIndirectPaintingSupport::setTemporaryCompositeOp(const KoCompositeOp* c)
{
    d->compositeOp = c;
}

void KisIndirectPaintingSupport::setTemporaryOpacity(quint8 o)
{
    d->compositeOpacity = o;
}

void KisIndirectPaintingSupport::setTemporaryChannelFlags(const QBitArray& channelFlags)
{
    d->channelFlags = channelFlags;
}

void KisIndirectPaintingSupport::lockTemporaryTarget() const
{
    d->lock.lockForRead();
}

void KisIndirectPaintingSupport::unlockTemporaryTarget() const
{
    d->lock.unlock();
}

KisPaintDeviceSP KisIndirectPaintingSupport::temporaryTarget()
{
    return d->temporaryTarget;
}

const KisPaintDeviceSP KisIndirectPaintingSupport::temporaryTarget() const
{
    return d->temporaryTarget;
}

const KoCompositeOp* KisIndirectPaintingSupport::temporaryCompositeOp() const
{
    return d->compositeOp;
}

quint8 KisIndirectPaintingSupport::temporaryOpacity() const
{
    return d->compositeOpacity;
}

const QBitArray& KisIndirectPaintingSupport::temporaryChannelFlags() const
{
    return d->channelFlags;
}

bool KisIndirectPaintingSupport::hasTemporaryTarget() const
{
    return d->temporaryTarget;
}

void KisIndirectPaintingSupport::mergeToLayer(KisLayerSP layer, KisUndoAdapter *undoAdapter, const QString &transactionText)
{
    mergeToLayerImpl(layer, undoAdapter, transactionText);
}

void KisIndirectPaintingSupport::mergeToLayer(KisLayerSP layer, KisPostExecutionUndoAdapter *undoAdapter, const QString &transactionText)
{
    mergeToLayerImpl(layer, undoAdapter, transactionText);
}

template<class UndoAdapter>
void KisIndirectPaintingSupport::mergeToLayerImpl(KisLayerSP layer,
                                                  UndoAdapter *undoAdapter,
                                                  const QString &transactionText)
{
    /**
     * We do not apply selection here, because it has already
     * been taken into account in a tool code
     */
    KisPainter gc(layer->paintDevice());
    gc.setCompositeOp(d->compositeOp);
    gc.setOpacity(d->compositeOpacity);
    gc.setChannelFlags(d->channelFlags);

    d->lock.lockForWrite();

    /**
     * Scratchpad may not have an undo adapter
     */
    if(undoAdapter) {
        gc.beginTransaction(transactionText);
    }
    foreach (const QRect &rc, d->temporaryTarget->region().rects()) {
        gc.bitBlt(rc.topLeft(), d->temporaryTarget, rc);
    }
    d->temporaryTarget = 0;

    if(undoAdapter) {
        gc.endTransaction(undoAdapter);
    }

    d->lock.unlock();
}

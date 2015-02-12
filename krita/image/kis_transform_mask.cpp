/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *
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

#include <KoIcon.h>
#include <KoCompositeOpRegistry.h>

#include "kis_layer.h"
#include "kis_transform_mask.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "kis_selection.h"
#include "kis_processing_information.h"
#include "kis_node.h"
#include "kis_node_visitor.h"
#include "kis_processing_visitor.h"
#include "kis_node_progress_proxy.h"
#include "kis_transaction.h"
#include "kis_painter.h"

#include <KoUpdater.h>
#include "kis_perspectivetransform_worker.h"
#include "kis_transform_mask_params_interface.h"
#include "kis_recalculate_transform_mask_job.h"
#include "kis_signal_compressor.h"
#include "kis_algebra_2d.h"
#include "kis_safe_transform.h"



#define UPDATE_DELAY 3000 /*ms */

struct KisTransformMask::Private
{
    Private()
        : worker(0, QTransform(), 0),
          staticCacheValid(false),
          recalculatingStaticImage(false),
          updateSignalCompressor(UPDATE_DELAY, KisSignalCompressor::POSTPONE)
    {
    }

    Private(const Private &rhs)
        : worker(rhs.worker),
          params(rhs.params),
          staticCacheValid(false),
          recalculatingStaticImage(rhs.recalculatingStaticImage),
          updateSignalCompressor(UPDATE_DELAY, KisSignalCompressor::POSTPONE)
    {
    }

    KisPerspectiveTransformWorker worker;
    KisTransformMaskParamsInterfaceSP params;

    bool staticCacheValid;
    bool recalculatingStaticImage;
    KisPaintDeviceSP staticCacheDevice;

    KisSignalCompressor updateSignalCompressor;
};


KisTransformMask::KisTransformMask()
    : KisEffectMask(),
      m_d(new Private)
{
    setTransformParams(
        KisTransformMaskParamsInterfaceSP(
            new KisDumbTransformMaskParams()));

    connect(this, SIGNAL(initiateDelayedStaticUpdate()), &m_d->updateSignalCompressor, SLOT(start()));
    connect(&m_d->updateSignalCompressor, SIGNAL(timeout()), SLOT(slotDelayedStaticUpdate()));
}

KisTransformMask::~KisTransformMask()
{
}

KisTransformMask::KisTransformMask(const KisTransformMask& rhs)
    : KisEffectMask(rhs),
      m_d(new Private(*rhs.m_d))
{
    connect(&m_d->updateSignalCompressor, SIGNAL(timeout()), SLOT(slotDelayedStaticUpdate()));
}

KisPaintDeviceSP KisTransformMask::paintDevice() const
{
    return 0;
}

QIcon KisTransformMask::icon() const
{
    return koIcon("edit-cut");
}

void KisTransformMask::setTransformParams(KisTransformMaskParamsInterfaceSP params)
{
    KIS_ASSERT_RECOVER(params) {
        params = KisTransformMaskParamsInterfaceSP(
            new KisDumbTransformMaskParams());
    }

    QTransform affineTransform;
    if (params->isAffine()) {
        affineTransform = params->finalAffineTransform();
    }
    m_d->worker.setForwardTransform(affineTransform);

    m_d->params = params;
    m_d->updateSignalCompressor.stop();
}

KisTransformMaskParamsInterfaceSP KisTransformMask::transformParams() const
{
    return m_d->params;
}

void KisTransformMask::slotDelayedStaticUpdate()
{
    /**
     * The mask might have been deleted from the layers stack in the
     * meanwhile. Just ignore the updates in the case.
     */

    KisLayerSP parentLayer = dynamic_cast<KisLayer*>(parent().data());
    if (!parentLayer) return;

    KisImageSP image = parentLayer->image();
    if (image) {
        image->addSpontaneousJob(new KisRecalculateTransformMaskJob(this));
    }
}

KisPaintDeviceSP KisTransformMask::buildPreviewDevice()
{
    /**
     * Note: this function must be called from within the scheduler's
     * context. We are accessing parent's updateProjection(), which
     * is not entirely safe.
     */

    KisLayerSP parentLayer = dynamic_cast<KisLayer*>(parent().data());
    KIS_ASSERT_RECOVER(parentLayer) { return new KisPaintDevice(colorSpace()); }

    KisPaintDeviceSP device =
        new KisPaintDevice(parentLayer->original()->colorSpace());

    QRect requestedRect = parentLayer->original()->exactBounds();
    parentLayer->buildProjectionUpToNode(device, this, requestedRect);

    return device;
}

void KisTransformMask::recaclulateStaticImage()
{
    /**
     * Note: this function must be called from within the scheduler's
     * context. We are accessing parent's updateProjection(), which
     * is not entirely safe.
     */

    KisLayerSP parentLayer = dynamic_cast<KisLayer*>(parent().data());
    KIS_ASSERT_RECOVER_RETURN(parentLayer);

    if (!m_d->staticCacheDevice) {
        m_d->staticCacheDevice =
            new KisPaintDevice(parentLayer->original()->colorSpace());
    }

    m_d->recalculatingStaticImage = true;
    /**
     * updateProjection() is assuming that the requestedRect takes
     * into account all the change rects of all the masks. Usually,
     * this work is done by the walkers.
     */
    QRect requestedRect = parentLayer->changeRect(parentLayer->original()->exactBounds());

    /**
     * Here we use updateProjection() to regenerate the projection of
     * the layer and after that a special update call (no-filthy) will
     * be issued to pass the changed further through the stack.
     */
    parentLayer->updateProjection(requestedRect, this);
    m_d->recalculatingStaticImage = false;

    m_d->staticCacheValid = true;
}

QRect KisTransformMask::decorateRect(KisPaintDeviceSP &src,
                                     KisPaintDeviceSP &dst,
                                     const QRect & rc,
                                     PositionToFilthy maskPos) const
{
    Q_ASSERT(nodeProgressProxy());
    Q_ASSERT_X(src != dst, "KisTransformMask::decorateRect",
               "src must be != dst, because we cant create transactions "
               "during merge, as it breaks reentrancy");

    KIS_ASSERT_RECOVER(m_d->params) { return rc; }

    if (m_d->params->isHidden()) return rc;
    KIS_ASSERT_RECOVER_NOOP(maskPos == N_FILTHY ||
                            maskPos == N_ABOVE_FILTHY ||
                            maskPos == N_BELOW_FILTHY);

    if (!m_d->recalculatingStaticImage &&
        (maskPos == N_FILTHY || maskPos == N_ABOVE_FILTHY)) {

        m_d->staticCacheValid = false;
        emit initiateDelayedStaticUpdate();
    }

    if (m_d->recalculatingStaticImage) {
        m_d->staticCacheDevice->clear();
        m_d->params->transformDevice(const_cast<KisTransformMask*>(this), src, m_d->staticCacheDevice);
        dst->makeCloneFrom(m_d->staticCacheDevice, m_d->staticCacheDevice->extent());
    } else if (!m_d->staticCacheValid && m_d->params->isAffine()) {
        m_d->worker.runPartialDst(src, dst, rc);
    } else {
        KisPainter gc(dst);
        gc.setCompositeOp(COMPOSITE_COPY);
        gc.bitBlt(rc.topLeft(), m_d->staticCacheDevice, rc);
    }

    return rc;
}

bool KisTransformMask::accept(KisNodeVisitor &v)
{
    return v.visit(this);
}

void KisTransformMask::accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter)
{
    return visitor.visit(this, undoAdapter);
}

QRect KisTransformMask::changeRect(const QRect &rect, PositionToFilthy pos) const
{
    Q_UNUSED(pos);

    /**
     * FIXME: This check of the emptiness should be done
     * on the higher/lower level
     */
    if (rect.isEmpty()) return rect;
    if (!m_d->params->isAffine()) return rect;

    QRect bounds;
    QRect interestRect;
    KisNodeSP parentNode = parent();

    if (parentNode) {
        bounds = parentNode->original()->defaultBounds()->bounds();
        interestRect = parentNode->original()->extent();
    } else {
        bounds = QRect(0,0,777,777);
        interestRect = QRect(0,0,888,888);
        qWarning() << "WARNING: transform mask has no parent (change rect)."
                   << "Cannot run safe transformations."
                   << "Will limit bounds to" << ppVar(bounds);
    }

    const QRect limitingRect = KisAlgebra2D::blowRect(bounds, 0.5);

    KisSafeTransform transform(m_d->worker.forwardTransform(), limitingRect, interestRect);
    QRect changeRect = transform.mapRectForward(rect);

    return changeRect;
}

QRect KisTransformMask::needRect(const QRect& rect, PositionToFilthy pos) const
{
    Q_UNUSED(pos);

    /**
     * FIXME: This check of the emptiness should be done
     * on the higher/lower level
     */
    if (rect.isEmpty()) return rect;
    if (!m_d->params->isAffine()) return rect;

    QRect bounds;
    QRect interestRect;
    KisNodeSP parentNode = parent();

    if (parentNode) {
        bounds = parentNode->original()->defaultBounds()->bounds();
        interestRect = parentNode->original()->extent();
    } else {
        bounds = QRect(0,0,777,777);
        interestRect = QRect(0,0,888,888);
        qWarning() << "WARNING: transform mask has no parent (need rect)."
                   << "Cannot run safe transformations."
                   << "Will limit bounds to" << ppVar(bounds);
    }

    const QRect limitingRect = KisAlgebra2D::blowRect(bounds, 0.5);

    KisSafeTransform transform(m_d->worker.forwardTransform(), limitingRect, interestRect);
    QRect needRect = transform.mapRectBackward(rect);

    return needRect;
}

QRect KisTransformMask::extent() const
{
    QRect rc = KisMask::extent();

    QRect partialChangeRect;
    QRect existentProjection;
    KisLayerSP parentLayer = dynamic_cast<KisLayer*>(parent().data());
    if (parentLayer) {
        partialChangeRect = parentLayer->partialChangeRect(const_cast<KisTransformMask*>(this), rc);
        existentProjection = parentLayer->projection()->extent();
    }
    return changeRect(partialChangeRect) | existentProjection;
}

QRect KisTransformMask::exactBounds() const
{
    QRect rc = KisMask::exactBounds();

    QRect partialChangeRect;
    QRect existentProjection;
    KisLayerSP parentLayer = dynamic_cast<KisLayer*>(parent().data());
    if (parentLayer) {
        partialChangeRect = parentLayer->partialChangeRect(const_cast<KisTransformMask*>(this), rc);
        existentProjection = parentLayer->projection()->exactBounds();
    }

    return changeRect(partialChangeRect) | existentProjection;
}

#include "kis_transform_mask.moc"

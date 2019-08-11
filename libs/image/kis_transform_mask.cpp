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
#include <kis_icon.h>
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

#include "kis_busy_progress_indicator.h"
#include "kis_perspectivetransform_worker.h"
#include "kis_transform_mask_params_interface.h"
#include "kis_transform_mask_params_factory_registry.h"
#include "kis_recalculate_transform_mask_job.h"
#include "kis_thread_safe_signal_compressor.h"
#include "kis_algebra_2d.h"
#include "kis_safe_transform.h"
#include "kis_keyframe_channel.h"

#include "kis_image_config.h"

//#include "kis_paint_device_debug_utils.h"
//#define DEBUG_RENDERING
//#define DUMP_RECT QRect(0,0,512,512)

#define UPDATE_DELAY 3000 /*ms */

struct Q_DECL_HIDDEN KisTransformMask::Private
{
    Private()
        : worker(0, QTransform(), 0),
          staticCacheValid(false),
          recalculatingStaticImage(false),
          updateSignalCompressor(UPDATE_DELAY, KisSignalCompressor::POSTPONE),
          offBoundsReadArea(0.5)
    {
    }

    Private(const Private &rhs)
        : worker(rhs.worker),
          params(rhs.params),
          staticCacheValid(rhs.staticCacheValid),
          recalculatingStaticImage(rhs.recalculatingStaticImage),
          updateSignalCompressor(UPDATE_DELAY, KisSignalCompressor::POSTPONE),
          offBoundsReadArea(rhs.offBoundsReadArea)
    {
    }

    void reloadParameters()
    {
        QTransform affineTransform;
        if (params->isAffine()) {
            affineTransform = params->finalAffineTransform();
        }
        worker.setForwardTransform(affineTransform);

        params->clearChangedFlag();
        staticCacheValid = false;
    }

    KisPerspectiveTransformWorker worker;
    KisTransformMaskParamsInterfaceSP params;

    bool staticCacheValid;
    bool recalculatingStaticImage;
    KisPaintDeviceSP staticCacheDevice;

    KisThreadSafeSignalCompressor updateSignalCompressor;
    qreal offBoundsReadArea;
};


KisTransformMask::KisTransformMask()
    : KisEffectMask(),
      m_d(new Private())
{
    setTransformParams(
        KisTransformMaskParamsInterfaceSP(
            new KisDumbTransformMaskParams()));

    connect(&m_d->updateSignalCompressor, SIGNAL(timeout()), SLOT(slotDelayedStaticUpdate()));
    connect(this, SIGNAL(sigInternalForceStaticImageUpdate()), SLOT(slotInternalForceStaticImageUpdate()));
    m_d->offBoundsReadArea = KisImageConfig(true).transformMaskOffBoundsReadArea();
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
    return KisIconUtils::loadIcon("transformMask");
}

void KisTransformMask::setTransformParams(KisTransformMaskParamsInterfaceSP params)
{
    KIS_ASSERT_RECOVER(params) {
        params = KisTransformMaskParamsInterfaceSP(
            new KisDumbTransformMaskParams());
    }

    m_d->params = params;
    m_d->reloadParameters();

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

    KisLayerSP parentLayer = qobject_cast<KisLayer*>(parent().data());
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

    KisLayerSP parentLayer = qobject_cast<KisLayer*>(parent().data());
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

    KisLayerSP parentLayer = qobject_cast<KisLayer*>(parent().data());
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
    Q_ASSERT_X(src != dst, "KisTransformMask::decorateRect",
               "src must be != dst, because we can't create transactions "
               "during merge, as it breaks reentrancy");

    KIS_ASSERT_RECOVER(m_d->params) { return rc; }

    if (m_d->params->isHidden()) return rc;
    KIS_ASSERT_RECOVER_NOOP(maskPos == N_FILTHY ||
                            maskPos == N_ABOVE_FILTHY ||
                            maskPos == N_BELOW_FILTHY);

    if (m_d->params->hasChanged()) m_d->reloadParameters();

    if (!m_d->recalculatingStaticImage &&
        (maskPos == N_FILTHY || maskPos == N_ABOVE_FILTHY)) {

        m_d->staticCacheValid = false;
        m_d->updateSignalCompressor.start();
    }

    if (m_d->recalculatingStaticImage) {
        m_d->staticCacheDevice->clear();
        m_d->params->transformDevice(const_cast<KisTransformMask*>(this), src, m_d->staticCacheDevice);
        QRect updatedRect = m_d->staticCacheDevice->extent();
        KisPainter::copyAreaOptimized(updatedRect.topLeft(), m_d->staticCacheDevice, dst, updatedRect);

#ifdef DEBUG_RENDERING
        qDebug() << "Recalculate" << name() << ppVar(src->exactBounds()) << ppVar(dst->exactBounds()) << ppVar(rc);
        KIS_DUMP_DEVICE_2(src, DUMP_RECT, "recalc_src", "dd");
        KIS_DUMP_DEVICE_2(dst, DUMP_RECT, "recalc_dst", "dd");
#endif /* DEBUG_RENDERING */

    } else if (!m_d->staticCacheValid && m_d->params->isAffine()) {
        m_d->worker.runPartialDst(src, dst, rc);

#ifdef DEBUG_RENDERING
        qDebug() << "Partial" << name() << ppVar(src->exactBounds()) << ppVar(src->extent()) << ppVar(dst->exactBounds()) << ppVar(dst->extent()) << ppVar(rc);
        KIS_DUMP_DEVICE_2(src, DUMP_RECT, "partial_src", "dd");
        KIS_DUMP_DEVICE_2(dst, DUMP_RECT, "partial_dst", "dd");
#endif /* DEBUG_RENDERING */

    } else if (m_d->staticCacheDevice && m_d->staticCacheValid) {
        KisPainter::copyAreaOptimized(rc.topLeft(), m_d->staticCacheDevice, dst, rc);

#ifdef DEBUG_RENDERING
        qDebug() << "Fetch" << name() << ppVar(src->exactBounds()) << ppVar(dst->exactBounds()) << ppVar(rc);
        KIS_DUMP_DEVICE_2(src, DUMP_RECT, "fetch_src", "dd");
        KIS_DUMP_DEVICE_2(dst, DUMP_RECT, "fetch_dst", "dd");
#endif /* DEBUG_RENDERING */

    }

    KIS_ASSERT_RECOVER_NOOP(this->busyProgressIndicator());
    this->busyProgressIndicator()->update();

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

    QRect changeRect = rect;

    if (m_d->params->isAffine()) {
        QRect bounds;
        QRect interestRect;
        KisNodeSP parentNode = parent();

        if (parentNode) {
            bounds = parentNode->original()->defaultBounds()->bounds();
            interestRect = parentNode->original()->extent();
        } else {
            bounds = QRect(0,0,777,777);
            interestRect = QRect(0,0,888,888);
            warnKrita << "WARNING: transform mask has no parent (change rect)."
                      << "Cannot run safe transformations."
                      << "Will limit bounds to" << ppVar(bounds);
        }

        const QRect limitingRect = KisAlgebra2D::blowRect(bounds, m_d->offBoundsReadArea);

        if (m_d->params->hasChanged()) m_d->reloadParameters();
        KisSafeTransform transform(m_d->worker.forwardTransform(), limitingRect, interestRect);
        changeRect = transform.mapRectForward(rect);
    } else {
        QRect interestRect;
        interestRect = parent() ? parent()->original()->extent() : QRect();

        changeRect = m_d->params->nonAffineChangeRect(rect);
    }

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
        warnKrita << "WARNING: transform mask has no parent (need rect)."
                   << "Cannot run safe transformations."
                   << "Will limit bounds to" << ppVar(bounds);
    }

    QRect needRect = rect;

    if (m_d->params->isAffine()) {
        const QRect limitingRect = KisAlgebra2D::blowRect(bounds, m_d->offBoundsReadArea);

        if (m_d->params->hasChanged()) m_d->reloadParameters();
        KisSafeTransform transform(m_d->worker.forwardTransform(), limitingRect, interestRect);
        needRect = transform.mapRectBackward(rect);

    } else {
        needRect = m_d->params->nonAffineNeedRect(rect, interestRect);
    }

    return needRect;
}

QRect KisTransformMask::extent() const
{
    QRect rc = KisMask::extent();

    QRect partialChangeRect;
    QRect existentProjection;
    KisLayerSP parentLayer = qobject_cast<KisLayer*>(parent().data());
    if (parentLayer) {
        partialChangeRect = parentLayer->partialChangeRect(const_cast<KisTransformMask*>(this), rc);
        existentProjection = parentLayer->projection()->extent();
    }

    return changeRect(partialChangeRect) | existentProjection;
}

QRect KisTransformMask::exactBounds() const
{
    QRect existentProjection;
    KisLayerSP parentLayer = qobject_cast<KisLayer*>(parent().data());
    if (parentLayer) {
        existentProjection = parentLayer->projection()->exactBounds();
    }

    return changeRect(sourceDataBounds()) | existentProjection;
}

QRect KisTransformMask::sourceDataBounds() const
{
    QRect rc = KisMask::exactBounds();

    QRect partialChangeRect = rc;
    KisLayerSP parentLayer = qobject_cast<KisLayer*>(parent().data());
    if (parentLayer) {
        partialChangeRect = parentLayer->partialChangeRect(const_cast<KisTransformMask*>(this), rc);
    }

    return partialChangeRect;
}

void KisTransformMask::setX(qint32 x)
{
    m_d->params->translate(QPointF(x - this->x(), 0));
    setTransformParams(m_d->params);
    KisEffectMask::setX(x);
}

void KisTransformMask::setY(qint32 y)
{
    m_d->params->translate(QPointF(0, y - this->y()));
    setTransformParams(m_d->params);
    KisEffectMask::setY(y);
}

void KisTransformMask::forceUpdateTimedNode()
{
    if (hasPendingTimedUpdates()) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(!m_d->staticCacheValid);

        m_d->updateSignalCompressor.stop();
        slotDelayedStaticUpdate();
    }
}

bool KisTransformMask::hasPendingTimedUpdates() const
{
    return m_d->updateSignalCompressor.isActive();
}

void KisTransformMask::threadSafeForceStaticImageUpdate()
{
    emit sigInternalForceStaticImageUpdate();
}

void KisTransformMask::slotInternalForceStaticImageUpdate()
{
    m_d->updateSignalCompressor.stop();
    slotDelayedStaticUpdate();
}

KisKeyframeChannel *KisTransformMask::requestKeyframeChannel(const QString &id)
{
    if (id == KisKeyframeChannel::TransformArguments.id() ||
        id == KisKeyframeChannel::TransformPositionX.id() ||
        id == KisKeyframeChannel::TransformPositionY.id() ||
        id == KisKeyframeChannel::TransformScaleX.id() ||
        id == KisKeyframeChannel::TransformScaleY.id() ||
        id == KisKeyframeChannel::TransformShearX.id() ||
        id == KisKeyframeChannel::TransformShearY.id() ||
        id == KisKeyframeChannel::TransformRotationX.id() ||
        id == KisKeyframeChannel::TransformRotationY.id() ||
        id == KisKeyframeChannel::TransformRotationZ.id()) {

        KisAnimatedTransformParamsInterface *animatedParams = dynamic_cast<KisAnimatedTransformParamsInterface*>(m_d->params.data());

        if (!animatedParams) {
            auto converted = KisTransformMaskParamsFactoryRegistry::instance()->animateParams(m_d->params);
            if (converted.isNull()) return KisEffectMask::requestKeyframeChannel(id);
            m_d->params = converted;
            animatedParams = dynamic_cast<KisAnimatedTransformParamsInterface*>(converted.data());
        }

        KisKeyframeChannel *channel = animatedParams->getKeyframeChannel(id, parent()->original()->defaultBounds());
        if (channel) return channel;
    }

    return KisEffectMask::requestKeyframeChannel(id);
}


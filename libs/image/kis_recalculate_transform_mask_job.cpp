/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_recalculate_transform_mask_job.h"

#include "kis_transform_mask.h"
#include "kis_debug.h"
#include "kis_layer.h"
#include "kis_image.h"
#include "kis_image_animation_interface.h"
#include "kis_abstract_projection_plane.h"
#include "kis_transform_mask_params_interface.h"

KisRecalculateTransformMaskJob::KisRecalculateTransformMaskJob(KisTransformMaskSP mask)
    : m_mask(mask)
{
    setExclusive(true);
}

bool KisRecalculateTransformMaskJob::overrides(const KisSpontaneousJob *_otherJob)
{
    const KisRecalculateTransformMaskJob *otherJob =
        dynamic_cast<const KisRecalculateTransformMaskJob*>(_otherJob);

    return otherJob && otherJob->m_mask == m_mask;
}

void KisRecalculateTransformMaskJob::run()
{
    /**
     * The mask might have been deleted from the layers stack. In
     * such a case, don't try do update it.
     */
    if (!m_mask->parent()) return;
    if (!m_mask->visible()) return;

    const QRect oldMaskExtent = m_mask->extent();
    m_mask->recaclulateStaticImage();

    KisLayerSP layer = qobject_cast<KisLayer*>(m_mask->parent().data());

    if (!layer) {
        warnKrita << "WARNING: KisRecalculateTransformMaskJob::run() Mask has no parent layer! Skipping projection update!";
        return;
    }

    KisImageSP image = layer->image();
    Q_ASSERT(image);

    /**
     * Depending on whether the mask is hidden we should either
     * update it entirely via the setDirty() call, or we can use a
     * lightweight approach by directly regenerating the
     * precalculated static image using
     * KisRecalculateTransformMaskJob.
     */
    if (m_mask->transformParams()->isHidden()) {
        QRect updateRect = m_mask->extent() | oldMaskExtent;

        if (layer->original()) {
            updateRect |= layer->original()->defaultBounds()->bounds();
        }

        if (layer->isAnimated()) {
            m_mask->setDirty(updateRect);
        } else {
            m_mask->setDirtyDontResetAnimationCache(updateRect);
        }
    } else {
        /**
         * When we call requestProjectionUpdateNoFilthy() on a layer,
         * its masks' change rect is not counted, because it is considered
         * to be N_ABOVE_FILTHY. Therefore, we should expand the dirty
         * rect manually to get the correct update
         */
        QRect updateRect = oldMaskExtent |
            layer->projectionPlane()->changeRect(layer->extent(), KisLayer::N_FILTHY);

        if (!m_mask->isAnimated()) {
            image->requestProjectionUpdateNoFilthy(layer, updateRect, image->bounds(), false); // Should there be a case where this is flushed?
        }
    }
}

int KisRecalculateTransformMaskJob::levelOfDetail() const
{
    return 0;
}

QString KisRecalculateTransformMaskJob::debugName() const
{
    QString result;
    QDebug dbg(&result);
    dbg << "KisRecalculateTransformMaskJob" << m_mask;
    return result;
}

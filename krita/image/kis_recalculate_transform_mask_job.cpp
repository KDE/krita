/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_recalculate_transform_mask_job.h"

#include "kis_transform_mask.h"
#include "kis_debug.h"
#include "kis_layer.h"
#include "kis_image.h"


KisRecalculateTransformMaskJob::KisRecalculateTransformMaskJob(KisTransformMaskSP mask)
    : m_mask(mask)
{
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

    m_mask->recaclulateStaticImage();

    KisLayerSP layer = dynamic_cast<KisLayer*>(m_mask->parent().data());

    if (!layer) {
        warnKrita << "WARNING: KisRecalculateTransformMaskJob::run() Mask has no parent layer! Skipping projection update!";
        return;
    }

    KisImageSP image = layer->image();
    Q_ASSERT(image);

    image->requestProjectionUpdateNoFilthy(layer, layer->extent(), image->bounds());
}

int KisRecalculateTransformMaskJob::levelOfDetail() const
{
    return 0;
}

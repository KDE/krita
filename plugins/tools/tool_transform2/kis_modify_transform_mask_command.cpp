/*
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

#include "kis_modify_transform_mask_command.h"
#include "kundo2command.h"
#include "kis_types.h"
#include "kis_recalculate_transform_mask_job.h"
#include "kis_transform_mask.h"
#include "kis_transform_mask_params_interface.h"

KisModifyTransformMaskCommand::KisModifyTransformMaskCommand(KisTransformMaskSP mask, KisTransformMaskParamsInterfaceSP params)
: m_mask(mask),
  m_params(params),
  m_oldParams(m_mask->transformParams())
{
}

void KisModifyTransformMaskCommand::redo() {
    m_mask->setTransformParams(m_params);

    updateMask(m_params->isHidden());
}

void KisModifyTransformMaskCommand::undo() {
    m_mask->setTransformParams(m_oldParams);

    updateMask(m_oldParams->isHidden());
}

void KisModifyTransformMaskCommand::updateMask(bool isHidden) {
    /**
     * Depending on whether the mask is hidden we should either
     * update it entirely via the setDirty() call, or we can use a
     * lightweight approach by directly regenerating the
     * precalculated static image using
     * KisRecalculateTransformMaskJob.
     */

    if (!isHidden) {
        KisRecalculateTransformMaskJob updateJob(m_mask);
        updateJob.run();
    } else {
        m_mask->recaclulateStaticImage();

        QRect updateRect = m_mask->extent();

        KisNodeSP parent = m_mask->parent();
        if (parent && parent->original()) {
            updateRect |= parent->original()->defaultBounds()->bounds();
        }

        m_mask->setDirty(updateRect);
    }
}

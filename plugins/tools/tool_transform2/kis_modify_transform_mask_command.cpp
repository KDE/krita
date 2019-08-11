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
#include "tool_transform_args.h"
#include "kis_scalar_keyframe_channel.h"
#include "kis_transform_args_keyframe_channel.h"
#include "kis_animated_transform_parameters.h"

KisModifyTransformMaskCommand::KisModifyTransformMaskCommand(KisTransformMaskSP mask, KisTransformMaskParamsInterfaceSP params)
: m_mask(mask),
  m_params(params),
  m_oldParams(m_mask->transformParams())
{
    m_wasHidden = m_oldParams->isHidden();

    auto *animatedParameters = dynamic_cast<KisAnimatedTransformMaskParameters*>(m_oldParams.data());
    if (animatedParameters) {
        int time = m_mask->parent()->original()->defaultBounds()->currentTime();
        KisAnimatedTransformMaskParameters::addKeyframes(m_mask, time, params, this);
    }
}

void KisModifyTransformMaskCommand::redo() {
    KisTransformMaskParamsInterfaceSP params;

    auto *animatedParameters = dynamic_cast<KisAnimatedTransformMaskParameters*>(m_oldParams.data());
    if (animatedParameters) {
        params = m_oldParams;
        animatedParameters->setHidden(m_params->isHidden());
        KUndo2Command::redo();
    } else {
        params = m_params;
    }

    m_mask->setTransformParams(params);
    m_mask->threadSafeForceStaticImageUpdate();
}

void KisModifyTransformMaskCommand::undo() {
    auto *animatedParameters = dynamic_cast<KisAnimatedTransformMaskParameters*>(m_oldParams.data());

    if (animatedParameters) {
        animatedParameters->setHidden(m_wasHidden);
        KUndo2Command::undo();
    }

    m_mask->setTransformParams(m_oldParams);
    m_mask->threadSafeForceStaticImageUpdate();
}

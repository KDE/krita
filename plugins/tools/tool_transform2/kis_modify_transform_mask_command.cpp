/*
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_modify_transform_mask_command.h"
#include "kundo2command.h"
#include "kis_types.h"
#include "kis_recalculate_transform_mask_job.h"
#include "kis_transform_mask.h"
#include "kis_transform_mask_params_interface.h"
#include "tool_transform_args.h"
#include "kis_scalar_keyframe_channel.h"
#include "kis_animated_transform_parameters.h"

KisModifyTransformMaskCommand::KisModifyTransformMaskCommand(KisTransformMaskSP mask, KisTransformMaskParamsInterfaceSP params, bool skipUpdate)
: m_mask(mask),
  m_params(params),
  m_oldParams(m_mask->transformParams()),
  m_skipUpdate(skipUpdate)
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
    if (!m_skipUpdate) {
        m_mask->threadSafeForceStaticImageUpdate();
    }
}

void KisModifyTransformMaskCommand::undo() {
    auto *animatedParameters = dynamic_cast<KisAnimatedTransformMaskParameters*>(m_oldParams.data());

    if (animatedParameters) {
        animatedParameters->setHidden(m_wasHidden);
        KUndo2Command::undo();
    }

    m_mask->setTransformParams(m_oldParams);
    if (!m_skipUpdate) {
        m_mask->threadSafeForceStaticImageUpdate();
    }
}

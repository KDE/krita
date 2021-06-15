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

KisModifyTransformMaskCommand::KisModifyTransformMaskCommand(KisTransformMaskSP mask, KisTransformMaskParamsInterfaceSP params, QWeakPointer<boost::none_t> updatesBlockerCookie)
: m_mask(mask),
  m_params(params),
  m_oldParams(m_mask->transformParams()),
  m_updatesBlockerCookie(updatesBlockerCookie)
{
    m_wasHidden = m_oldParams->isHidden();

    auto *animatedParameters = dynamic_cast<KisAnimatedTransformMaskParameters*>(mask->transformParams().data());
    if (animatedParameters) {
        const int time = mask->parent()->original()->defaultBounds()->currentTime();
        KisAnimatedTransformMaskParameters::setKeyframes(mask, time, params, this);
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
    if (!m_updatesBlockerCookie) {
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
    if (!m_updatesBlockerCookie) {
        m_mask->threadSafeForceStaticImageUpdate();
    }
}

KisInitializeTransformMaskKeyframesCommand::KisInitializeTransformMaskKeyframesCommand(KisTransformMaskSP mask) : KUndo2Command() {
    auto *animatedParameters = dynamic_cast<KisAnimatedTransformMaskParameters*>(mask->transformParams().data());
    if (animatedParameters) {
        int time = mask->parent()->original()->defaultBounds()->currentTime();
        KisAnimatedTransformMaskParameters::addKeyframes(mask, time, mask->transformParams(), this);
    }
}

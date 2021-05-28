/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisSimpleModifyTransformMaskCommand.h"

#include <kis_command_ids.h>
#include <kis_transform_mask.h>


KisSimpleModifyTransformMaskCommand::KisSimpleModifyTransformMaskCommand(KisTransformMaskSP mask, KisTransformMaskParamsInterfaceSP oldParams, KisTransformMaskParamsInterfaceSP newParams)
    : m_mask(mask),
      m_oldParams(oldParams),
      m_newParams(newParams)
{
}

int KisSimpleModifyTransformMaskCommand::id() const
{
    return KisCommandUtils::ChangeTransformMaskCommand;
}

bool KisSimpleModifyTransformMaskCommand::mergeWith(const KUndo2Command *other)
{
    const KisSimpleModifyTransformMaskCommand *otherCommand =
            dynamic_cast<const KisSimpleModifyTransformMaskCommand*>(other);

    bool retval = false;

    if (otherCommand &&
            otherCommand->m_mask == m_mask &&
            otherCommand->m_oldParams == m_newParams) {


        m_newParams = otherCommand->m_newParams;

        retval = true;
    }

    return retval;
}

void KisSimpleModifyTransformMaskCommand::undo()
{
    m_mask->setTransformParams(m_oldParams);
    m_mask->threadSafeForceStaticImageUpdate();
}

void KisSimpleModifyTransformMaskCommand::redo()
{
    m_mask->setTransformParams(m_newParams);
    m_mask->threadSafeForceStaticImageUpdate();
}

/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisSimpleModifyTransformMaskCommand.h"

#include <kis_command_ids.h>
#include <kis_transform_mask.h>


KisSimpleModifyTransformMaskCommand::KisSimpleModifyTransformMaskCommand(KisTransformMaskSP mask,
                                                                         KisTransformMaskParamsInterfaceSP newParams,
                                                                         QWeakPointer<boost::none_t> updatesBlockerCookie,
                                                                         KUndo2Command *parent)
    : KUndo2Command(parent),
      m_mask(mask),
      m_oldParams(m_mask->transformParams()),
      m_newParams(newParams),
      m_updatesBlockerCookie(updatesBlockerCookie)
{
}

int KisSimpleModifyTransformMaskCommand::id() const
{
    return KisCommandUtils::ChangeTransformMaskCommand;
}

bool KisSimpleModifyTransformMaskCommand::mergeWith(const KUndo2Command *other)
{
    const KisSimpleModifyTransformMaskCommand *otherCommandConst =
            dynamic_cast<const KisSimpleModifyTransformMaskCommand*>(other);

    bool retval = false;

    if (otherCommandConst &&
            otherCommandConst->m_mask == m_mask &&
            otherCommandConst->m_oldParams == m_newParams) {

        KisSimpleModifyTransformMaskCommand *otherCommand =
                const_cast<KisSimpleModifyTransformMaskCommand*>(otherCommandConst);

        m_newParams = otherCommand->m_newParams;
        std::move(otherCommand->m_undoCommands.begin(), otherCommand->m_undoCommands.end(),
                  std::back_inserter(m_undoCommands));

        retval = true;
    }

    return retval;
}

void KisSimpleModifyTransformMaskCommand::redo()
{
    if (!m_isInitialized) {
        std::unique_ptr<KUndo2Command> parent(new KUndo2Command);
        m_mask->setTransformParamsWithUndo(m_newParams, parent.get());
        m_undoCommands.emplace_back(parent.release());
        m_isInitialized = true;
    }

    KUndo2Command::redo();
    std::for_each(m_undoCommands.begin(), m_undoCommands.end(), std::mem_fn(&KUndo2Command::redo));

    if (!m_updatesBlockerCookie) {
        m_mask->threadSafeForceStaticImageUpdate();
    }
}

void KisSimpleModifyTransformMaskCommand::undo()
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_isInitialized);

    std::for_each(m_undoCommands.begin(), m_undoCommands.end(), std::mem_fn(&KUndo2Command::undo));
    KUndo2Command::undo();

    if (!m_updatesBlockerCookie) {
        m_mask->threadSafeForceStaticImageUpdate();
    }
}

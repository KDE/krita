/*
 * SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "SvgTextRemoveCommand.h"

#include <klocalizedstring.h>
#include "kis_command_ids.h"

#include "KoSvgTextShape.h"
#include "KoSvgTextShapeMarkupConverter.h"
#include <KoShapeBulkActionLock.h>

SvgTextRemoveCommand::SvgTextRemoveCommand(KoSvgTextShape *shape,
                                           int endIndex, int pos,
                                           int anchor,
                                           int length,
                                           bool allowCleanUp,
                                           KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_shape(shape)
    , m_index(endIndex)
    , m_originalPos(pos)
    , m_anchor(anchor)
    , m_length(length)
    , m_allowCleanUp(allowCleanUp)
{
    Q_ASSERT(shape);
    setText(kundo2_i18n("Remove Text"));
}

void SvgTextRemoveCommand::redo()
{
    KoShapeBulkActionLock lock(m_shape);

    m_textData = m_shape->getMemento();

    int idx = m_index - m_length;
    m_shape->removeText(idx, m_length);
    m_index = idx + m_length;

    if (m_allowCleanUp) {
        m_shape->cleanUp();
    }

    KoShapeBulkActionLock::bulkShapesUpdate(lock.unlock());

    int pos = qMax(0, m_shape->posForIndex(idx, false, false));
    m_shape->notifyCursorPosChanged(pos, pos);
}

void SvgTextRemoveCommand::undo()
{
    KoShapeBulkActionLock lock(m_shape);
    m_shape->setMemento(m_textData, m_originalPos, m_anchor);
    KoShapeBulkActionLock::bulkShapesUpdate(lock.unlock());
}

int SvgTextRemoveCommand::id() const
{
    return KisCommandUtils::SvgRemoveTextCommand;
}

bool SvgTextRemoveCommand::mergeWith(const KUndo2Command *other)
{
    const SvgTextRemoveCommand *command = dynamic_cast<const SvgTextRemoveCommand*>(other);


    if (!command || command->m_shape != m_shape) {
        return false;
    }
    int oldIndex = m_index;
    int otherOldIndex = command->m_index;

    if (oldIndex - m_length == otherOldIndex) {
        m_length += command->m_length;
        return true;
    } else if (otherOldIndex - command->m_length == oldIndex - m_length) {
        return true;
    } else {
        return false;
    }
    return true;
}

/*
 * SPDX-FileCopyrightText: 2023 Alvin <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "SvgMoveTextCommand.h"

#include <klocalizedstring.h>

#include "KoSvgTextShape.h"
#include "kis_command_ids.h"
#include <KoShapeBulkActionLock.h>

SvgMoveTextCommand::SvgMoveTextCommand(KoSvgTextShape *shape,
                                       const QPointF &newPosition,
                                       const QPointF &oldPosition,
                                       KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_shape(shape)
    , m_newPosition(newPosition)
    , m_oldPosition(oldPosition)
{
    setText(kundo2_i18n("Move text"));
}

static void moveShape(KoSvgTextShape *shape, const QPointF &position)
{
    KoShapeBulkActionLock lock(shape);
    shape->setAbsolutePosition(position);
    KoShapeBulkActionLock::bulkShapesUpdate(lock.unlock());
}

void SvgMoveTextCommand::redo()
{
    moveShape(m_shape, m_newPosition);
}

void SvgMoveTextCommand::undo()
{
    moveShape(m_shape, m_oldPosition);
}

int SvgMoveTextCommand::id() const
{
    return KisCommandUtils::SvgMoveTextCommand;
}

bool SvgMoveTextCommand::mergeWith(const KUndo2Command *otherCommand)
{
    const SvgMoveTextCommand *other = dynamic_cast<const SvgMoveTextCommand *>(otherCommand);

    if (!other || other->m_shape != m_shape) {
        return false;
    }

    m_newPosition = other->m_newPosition;

    return true;
}

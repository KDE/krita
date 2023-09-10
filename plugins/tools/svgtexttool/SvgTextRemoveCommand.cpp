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
SvgTextRemoveCommand::SvgTextRemoveCommand(KoSvgTextShape *shape,
                                           int pos, int originalPos,
                                           int anchor,
                                           int length,
                                           KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_shape(shape)
    , m_pos(pos)
    , m_originalPos(originalPos)
    , m_anchor(anchor)
    , m_length(length)
{
    Q_ASSERT(shape);
    setText(kundo2_i18n("Remove Text"));
}

void SvgTextRemoveCommand::redo()
{
    QRectF updateRect = m_shape->boundingRect();
    int oldIndex = m_shape->indexForPos(m_pos);

    KoSvgTextShapeMarkupConverter converter(m_shape);
    converter.convertToSvg(&m_oldSvg, &m_oldDefs);
    m_shape->removeText(m_pos, m_length);
    m_shape->updateAbsolute( updateRect| m_shape->boundingRect());

    int pos = qMax(0, m_shape->posForIndex(oldIndex, false, false));
    m_shape->notifyCursorPosChanged(pos, pos);
}

void SvgTextRemoveCommand::undo()
{
    QRectF updateRect = m_shape->boundingRect();
    KoSvgTextShapeMarkupConverter converter(m_shape);
    // Hardcoded resolution?
    converter.convertFromSvg(m_oldSvg, m_oldDefs, m_shape->boundingRect(), 72.0);
    m_shape->updateAbsolute( updateRect| m_shape->boundingRect());

    m_shape->notifyCursorPosChanged(m_originalPos, m_anchor);
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
    int oldIndex = m_shape->indexForPos(m_pos);
    int otherOldIndex = m_shape->indexForPos(command->m_pos);
    if (oldIndex - m_length != otherOldIndex) {
        return false;
    }

    m_length += command->m_length;
    return true;
}

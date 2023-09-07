/*
 * SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "SvgTextInsertCommand.h"
#include "KoSvgTextShape.h"
#include "KoSvgTextShapeMarkupConverter.h"
#include "SvgTextCursor.h"

#include "kis_command_ids.h"
SvgTextInsertCommand::SvgTextInsertCommand(KoSvgTextShape *shape, int pos, int anchor, QString text, SvgTextCursor *cursor, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_shape(shape)
    , m_pos(pos)
    , m_anchor(anchor)
    , m_text(text)
    , m_cursor(cursor)
{
    QRegExp exp;
    // This replaces...
    // - carriage return
    // - linefeed-carriage return
    // - carriage return-linefeed
    // - line seperator
    // - paragraph seperator
    // - vertical tab/line tab
    // with a single linefeed to avoid them from being added to the textShape.
    exp.setPattern("[\\r|\\r\\n|\\x2029|\\x2028\\x000b]");
    text.replace(exp, "\n");
    m_text = text;
}

void SvgTextInsertCommand::redo()
{
    QRectF updateRect = m_shape->boundingRect();
    int oldIndex = m_shape->indexForPos(m_pos);

    KoSvgTextShapeMarkupConverter converter(m_shape);
    converter.convertToSvg(&m_oldSvg, &m_oldDefs);
    m_shape->insertText(m_pos, m_text);
    m_shape->updateAbsolute( updateRect| m_shape->boundingRect());

    if (m_cursor) {
        int pos = m_shape->posForIndex(oldIndex + m_text.size(), false, false);
        m_cursor->setPos(pos, pos);
    }
}

void SvgTextInsertCommand::undo()
{
    QRectF updateRect = m_shape->boundingRect();
    KoSvgTextShapeMarkupConverter converter(m_shape);
    // Hardcoded resolution?
    converter.convertFromSvg(m_oldSvg, m_oldDefs, m_shape->boundingRect(), 72.0);
    m_shape->updateAbsolute( updateRect| m_shape->boundingRect());

    if (m_cursor) {
        m_cursor->setPos(m_pos, m_anchor);
    }
}

int SvgTextInsertCommand::id() const
{
    return KisCommandUtils::SvgInsertTextCommand;
}

bool SvgTextInsertCommand::mergeWith(const KUndo2Command *other)
{
    const SvgTextInsertCommand *command = dynamic_cast<const SvgTextInsertCommand*>(other);


    if (!command || command->m_shape != m_shape || command->m_cursor != m_cursor) {
        return false;
    }
    int oldIndex = m_shape->indexForPos(m_pos);
    int otherOldIndex = m_shape->indexForPos(command->m_pos);
    if (oldIndex + m_text.size() != otherOldIndex) {
        return false;
    }

    m_text += command->m_text;
    return true;
}

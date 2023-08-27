/*
 * SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "SvgTextInsertCommand.h"
#include "KoSvgTextShape.h"

SvgTextInsertCommand::SvgTextInsertCommand(KoSvgTextShape *shape, int pos, int anchor, QString text, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_shape(shape)
    , m_pos(pos)
    , m_anchor(anchor)
    , m_text(text)
{
    setText(kundo2_i18n("Insert Text"));
}

void SvgTextInsertCommand::redo()
{
    QRectF updateRect = m_shape->boundingRect();
    m_shape->insertText(m_pos, m_text);
    m_shape->updateAbsolute( updateRect| m_shape->boundingRect());
}

void SvgTextInsertCommand::undo()
{
    QRectF updateRect = m_shape->boundingRect();
    m_shape->removeText(m_pos, m_text.size());
    m_shape->updateAbsolute( updateRect| m_shape->boundingRect());
}

/*
 * SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "SvgTextInsertRichCommand.h"
#include "KoSvgTextShape.h"
#include "KoSvgTextShapeMarkupConverter.h"

SvgTextInsertRichCommand::SvgTextInsertRichCommand(KoSvgTextShape *shape, KoSvgTextShape *insert, int pos, int anchor, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_shape(shape)
    , m_insert(insert)
    , m_pos(pos)
    , m_anchor(anchor)
{
    setText(kundo2_i18n("Insert Rich Text"));
}

void SvgTextInsertRichCommand::redo()
{
    QRectF updateRect = m_shape->boundingRect();
    // Index defaults to -1 when there's no text in the shape.
    int oldIndex = qMax(0, m_shape->indexForPos(m_pos));

    KoSvgTextShapeMarkupConverter converter(m_shape);
    converter.convertToSvg(&m_oldSvg, &m_oldDefs);
    m_shape->insertRichText(m_pos, m_insert);
    m_shape->cleanUp();
    m_shape->updateAbsolute( updateRect| m_shape->boundingRect());

    int pos = m_shape->posForIndex(oldIndex + m_insert->plainText().size(), false, false);
    m_shape->notifyCursorPosChanged(pos, pos);
}

void SvgTextInsertRichCommand::undo()
{
    QRectF updateRect = m_shape->boundingRect();
    KoSvgTextShapeMarkupConverter converter(m_shape);
    // Hardcoded resolution?
    converter.convertFromSvg(m_oldSvg, m_oldDefs, m_shape->boundingRect(), 72.0);
    m_shape->updateAbsolute( updateRect| m_shape->boundingRect());

    m_shape->notifyCursorPosChanged(m_pos, m_anchor);
}

/*
 * SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "SvgTextRemoveCommand.h"

#include <klocalizedstring.h>

#include "KoSvgTextShape.h"
#include "KoSvgTextShapeMarkupConverter.h"
SvgTextRemoveCommand::SvgTextRemoveCommand(KoSvgTextShape *shape,
                                           int pos,
                                           int length,
                                           KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_shape(shape)
    , m_pos(pos)
    , m_length(length)
{
    Q_ASSERT(shape);
    setText(kundo2_i18n("Remove Text"));
}

void SvgTextRemoveCommand::redo()
{
    QRectF updateRect = m_shape->boundingRect();
    KoSvgTextShapeMarkupConverter converter(m_shape);
    converter.convertToSvg(&m_oldSvg, &m_oldDefs);
    m_shape->removeText(m_pos, m_length);
    m_shape->updateAbsolute( updateRect| m_shape->boundingRect());
}

void SvgTextRemoveCommand::undo()
{
    QRectF updateRect = m_shape->boundingRect();
    KoSvgTextShapeMarkupConverter converter(m_shape);
    // Hardcoded resolution?
    converter.convertFromSvg(m_oldSvg, m_oldDefs, m_shape->boundingRect(), 72.0);
    m_shape->updateAbsolute( updateRect| m_shape->boundingRect());
}

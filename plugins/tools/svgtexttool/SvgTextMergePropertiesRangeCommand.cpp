/*
 * SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "SvgTextMergePropertiesRangeCommand.h"

#include "KoSvgTextShape.h"
#include "KoSvgTextShapeMarkupConverter.h"

SvgTextMergePropertiesRangeCommand::SvgTextMergePropertiesRangeCommand(KoSvgTextShape *shape,
                                                                       KoSvgTextProperties props,
                                                                       int pos,
                                                                       int anchor,
                                                                       KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_shape(shape)
    , m_props(props)
    , m_pos(pos)
    , m_anchor(anchor)
{
    setText(kundo2_i18n("Change Text Properties"));
    KoSvgTextShapeMarkupConverter converter(m_shape);
    converter.convertToSvg(&m_oldSvg, &m_oldDefs);
}

void SvgTextMergePropertiesRangeCommand::redo()
{
    QRectF updateRect = m_shape->boundingRect();
    m_shape->mergePropertiesIntoRange(qMin(m_pos, m_anchor), qMax(m_pos, m_anchor), m_props);
    m_shape->updateAbsolute( updateRect| m_shape->boundingRect());
}

void SvgTextMergePropertiesRangeCommand::undo()
{
    QRectF updateRect = m_shape->boundingRect();
    KoSvgTextShapeMarkupConverter converter(m_shape);
    // Hardcoded resolution?
    converter.convertFromSvg(m_oldSvg, m_oldDefs, m_shape->boundingRect(), 72.0);
    m_shape->updateAbsolute( updateRect| m_shape->boundingRect());

    m_shape->notifyCursorPosChanged(m_pos, m_anchor);
}

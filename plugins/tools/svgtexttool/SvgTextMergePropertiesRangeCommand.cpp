/*
 * SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "SvgTextMergePropertiesRangeCommand.h"

#include "KoSvgTextShape.h"
#include "kis_command_ids.h"

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
    , m_textData(m_shape->getMemento())
{
    setText(kundo2_i18n("Change Text Properties"));

    // Some properties may change cursor pos count, so we need the indices.
    m_startIndex = m_shape->indexForPos(qMin(pos, anchor));
    m_endIndex = m_shape->indexForPos(qMax(pos, anchor));
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
    m_shape->setMemento(m_textData);
    m_shape->updateAbsolute( updateRect| m_shape->boundingRect());

    m_shape->notifyCursorPosChanged(m_pos, m_anchor);
}

int SvgTextMergePropertiesRangeCommand::id() const
{
    return KisCommandUtils::SvgTextMergePropertiesRangeCommand;
}

bool SvgTextMergePropertiesRangeCommand::mergeWith(const KUndo2Command *other)
{
    const SvgTextMergePropertiesRangeCommand *command = dynamic_cast<const SvgTextMergePropertiesRangeCommand*>(other);

    if (!command || command->m_shape != m_shape || m_startIndex != command->m_startIndex || m_endIndex != command->m_endIndex) {
        return false;
    }

    Q_FOREACH(KoSvgTextProperties::PropertyId p, command->m_props.properties()) {
        m_props.setProperty(p, command->m_props.property(p));
    }

    return true;
}

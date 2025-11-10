/*
 * SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "SvgTextMergePropertiesRangeCommand.h"

#include "KoSvgTextShape.h"
#include "kis_command_ids.h"
#include <KoShapeBulkActionLock.h>

SvgTextMergePropertiesRangeCommand::SvgTextMergePropertiesRangeCommand(KoSvgTextShape *shape,
                                                                       const KoSvgTextProperties props,
                                                                       const int pos,
                                                                       const int anchor,
                                                                       const QSet<KoSvgTextProperties::PropertyId> removeProperties,
                                                                       KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_shape(shape)
    , m_props(props)
    , m_removeProperties(removeProperties)
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
    KoShapeBulkActionLock lock(m_shape);
    m_shape->mergePropertiesIntoRange(qMin(m_pos, m_anchor), qMax(m_pos, m_anchor), m_props, m_removeProperties);
    KoShapeBulkActionLock::bulkShapesUpdate(lock.unlock());

    m_shape->notifyMarkupChanged();
}

void SvgTextMergePropertiesRangeCommand::undo()
{
    KoShapeBulkActionLock lock(m_shape);
    m_shape->setMemento(m_textData, m_pos, m_anchor);
    KoShapeBulkActionLock::bulkShapesUpdate(lock.unlock());
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

    /**
     * The merging algorithm should follow the ordering of
     * KoSvgTextShape::mergePropertiesIntoRange, that is, firstly,
     * the properties in @p removeProperties list are removed,
     * then properties in @p properties are applied. If the property is
     * present in both lists, then the value from @p properties is used.
     */

    Q_FOREACH(KoSvgTextProperties::PropertyId p, command->m_removeProperties) {
        m_props.removeProperty(p);
        m_removeProperties.insert(p);
    }

    Q_FOREACH(KoSvgTextProperties::PropertyId p, command->m_props.properties()) {
        m_props.setProperty(p, command->m_props.property(p));
        m_removeProperties.remove(p);
    }

    return true;
}

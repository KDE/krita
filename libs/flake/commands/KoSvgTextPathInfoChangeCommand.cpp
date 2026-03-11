/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "KoSvgTextPathInfoChangeCommand.h"
#include "kis_command_ids.h"
#include <KoShapeBulkActionLock.h>

KoSvgTextPathInfoChangeCommand::KoSvgTextPathInfoChangeCommand(KoSvgTextShape *shape, int pos, KoSvgText::TextOnPathInfo textPathInfo, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_shape(shape)
    , m_textData(shape->getMemento())
    , m_pos(pos)
    , m_newInfo(textPathInfo)
{
    setText(kundo2_i18n("Change Text On Path Properties"));
}

void KoSvgTextPathInfoChangeCommand::redo()
{
    KoShapeBulkActionLock lock(m_shape);
    KoSvgTextNodeIndex index = m_shape->topLevelNodeForPos(m_pos);


    if (m_newInfo.side != index.textPathInfo()->side) {
        KoSvgText::TextAnchor anchor = KoSvgText::TextAnchor(index.properties()->propertyOrDefault(KoSvgTextProperties::TextAnchorId).toInt());
        anchor = anchor == KoSvgText::AnchorStart? KoSvgText::AnchorEnd:
                                                   anchor == KoSvgText::AnchorEnd? KoSvgText::AnchorStart: KoSvgText::AnchorMiddle;
        index.properties()->setProperty(KoSvgTextProperties::TextAnchorId, anchor);
    }

    index.textPathInfo()->startOffset = m_newInfo.startOffset;
    index.textPathInfo()->side = m_newInfo.side;
    index.textPathInfo()->method = m_newInfo.method;
    index.textPathInfo()->startOffsetIsPercentage = m_newInfo.startOffsetIsPercentage;
    index.textPathInfo()->spacing = m_newInfo.spacing;

    m_shape->relayout();
    m_shape->notifyChanged();
    KoShapeBulkActionLock::bulkShapesUpdate(lock.unlock());
    m_shape->notifyCursorPosChanged(m_pos, m_pos);
}

void KoSvgTextPathInfoChangeCommand::undo()
{
    KoShapeBulkActionLock lock(m_shape);
    m_shape->setMemento(m_textData, m_pos, m_pos);
    KoShapeBulkActionLock::bulkShapesUpdate(lock.unlock());
}

int KoSvgTextPathInfoChangeCommand::id() const
{
    return KisCommandUtils::SvgTextPathInfoChangeCommandId;
}

bool KoSvgTextPathInfoChangeCommand::mergeWith(const KUndo2Command *other)
{
    const KoSvgTextPathInfoChangeCommand *command = dynamic_cast<const KoSvgTextPathInfoChangeCommand*>(other);

    if (!command || command->m_shape != m_shape || m_pos != command->m_pos) {
        return false;
    }

    m_newInfo = command->m_newInfo;
    return true;

}

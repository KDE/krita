/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "KoSvgConvertTextTypeCommand.h"
#include <KoShapeBulkActionLock.h>

KoSvgConvertTextTypeCommand::KoSvgConvertTextTypeCommand(KoSvgTextShape *shape, KoSvgTextShape::TextType type, int pos, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_shape(shape)
    , m_conversionType(type)
    , m_pos(pos)
{
    setText(kundo2_i18n("Convert Text Type"));
}

void KoSvgConvertTextTypeCommand::redo()
{
    KoShapeBulkActionLock lock(m_shape);

    m_textData = m_shape->getMemento();
    const int oldIndex = qMax(0, m_shape->indexForPos(m_pos));

    if (m_conversionType == KoSvgTextShape::PreformattedText) {
        m_shape->convertCharTransformsToPreformatted(false);
    } else if (m_conversionType == KoSvgTextShape::InlineWrap) {
        m_shape->convertCharTransformsToPreformatted(true);
    } else if (m_conversionType == KoSvgTextShape::PrePositionedText) {
        m_shape->setCharacterTransformsFromLayout();
    }

    KoShapeBulkActionLock::bulkShapesUpdate(lock.unlock());
    const int pos = m_shape->posForIndex(oldIndex, false, false);
    m_shape->notifyCursorPosChanged(pos, pos);
}

void KoSvgConvertTextTypeCommand::undo()
{
    KoShapeBulkActionLock lock(m_shape);
    m_shape->setMemento(m_textData, m_pos, m_pos);
    KoShapeBulkActionLock::bulkShapesUpdate(lock.unlock());
}

/*
 * SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "SvgTextInsertRichCommand.h"
#include "KoSvgTextShapeMarkupConverter.h"
#include <KoShapeBulkActionLock.h>

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
    KoShapeBulkActionLock lock(m_shape);
    // Index defaults to -1 when there's no text in the shape.
    int oldIndex = qMax(0, m_shape->indexForPos(m_pos));

    m_textData = m_shape->getMemento();
    m_shape->insertRichText(m_pos, m_insert);
    m_shape->cleanUp();
    KoShapeBulkActionLock::bulkShapesUpdate(lock.unlock());

    int pos = m_shape->posForIndex(oldIndex + m_insert->plainText().size(), false, false);
    m_shape->notifyCursorPosChanged(pos, pos);
    m_shape->notifyMarkupChanged();
}

void SvgTextInsertRichCommand::undo()
{
    KoShapeBulkActionLock lock(m_shape);
    m_shape->setMemento(m_textData, m_pos, m_anchor);
    KoShapeBulkActionLock::bulkShapesUpdate(lock.unlock());
}

/*
 * SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "SvgTextInsertCommand.h"
#include "KoSvgTextShape.h"
#include "KoSvgTextShapeMarkupConverter.h"
#include <KoShapeBulkActionLock.h>

#include <QRegExp>

#include "kis_command_ids.h"
SvgTextInsertCommand::SvgTextInsertCommand(KoSvgTextShape *shape, int pos, int anchor, QString text, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_shape(shape)
    , m_pos(pos)
    , m_anchor(anchor)
    , m_text(text)
{
    setText(kundo2_i18n("Insert Text"));

    QRegExp exp;
    // This replaces...
    // - carriage return
    // - linefeed-carriage return
    // - carriage return-linefeed
    // - line separator
    // - paragraph separator
    // - vertical tab/line tab
    // with a single linefeed to avoid them from being added to the textShape.
    exp.setPattern("[\\r|\\r\\n|\\x2029|\\x2028\\x000b]");

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    text.replace(exp, "\n");
#else
    exp.replaceIn(text, "\n");
#endif
    m_text = text;
}

void SvgTextInsertCommand::redo()
{
    KoShapeBulkActionLock lock(m_shape);
    // Index defaults to -1 when there's no text in the shape.
    int oldIndex = qMax(0, m_shape->indexForPos(m_pos));

    m_textData = m_shape->getMemento();
    m_shape->insertText(m_pos, m_text);
    m_shape->cleanUp();
    KoShapeBulkActionLock::bulkShapesUpdate(lock.unlock());

    int pos = m_shape->posForIndex(oldIndex + m_text.size(), false, false);
    m_shape->notifyCursorPosChanged(pos, pos);

}

void SvgTextInsertCommand::undo()
{
    KoShapeBulkActionLock lock(m_shape);
    m_shape->setMemento(m_textData, m_pos, m_anchor);
    KoShapeBulkActionLock::bulkShapesUpdate(lock.unlock());
}

int SvgTextInsertCommand::id() const
{
    return KisCommandUtils::SvgInsertTextCommand;
}

bool SvgTextInsertCommand::mergeWith(const KUndo2Command *other)
{
    const SvgTextInsertCommand *command = dynamic_cast<const SvgTextInsertCommand*>(other);


    if (!command || command->m_shape != m_shape) {
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

/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_scheduled_undo_command.h"
#include "kis_stroke_strategy_undo_command_based.h"
#include "kis_image.h"
#include "kis_transaction_data.h"


KisScheduledUndoCommand::KisScheduledUndoCommand(KUndo2CommandSP command,
                                                 KisImageWSP image,
                                                 bool isExclusive)
    : KUndo2Command(command->text()),
      m_realCommand(command),
      m_image(image),
      m_exclusive(isExclusive)
{
    /**
     * All the commands are first executed in the stroke and then
     * added to the undo stack. It means that the first redo should be
     * skipped
     */
    m_skipOneRedoStroke = true;
}

KisScheduledUndoCommand::~KisScheduledUndoCommand()
{
}

void KisScheduledUndoCommand::undo()
{
    KisStrokeStrategyUndoCommandBased *strategy =
        new KisStrokeStrategyUndoCommandBased(m_realCommand->text(), true,
                                              0, m_realCommand);
    strategy->setExclusive(m_exclusive);

    KisStrokeId id = m_image->startStroke(strategy);
    m_image->endStroke(id);
}

void KisScheduledUndoCommand::redo()
{
    if(m_skipOneRedoStroke) {
        m_skipOneRedoStroke = false;
        return;
    }

    KisStrokeStrategyUndoCommandBased *strategy =
        new KisStrokeStrategyUndoCommandBased(m_realCommand->text(), false,
                                              0, m_realCommand);
    strategy->setExclusive(m_exclusive);

    KisStrokeId id = m_image->startStroke(strategy);
    m_image->endStroke(id);
}

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

#include "kis_scheduled_undo_adapter.h"

#include "kis_image.h"
#include "kis_stroke_strategy_undo_command_based.h"


KisScheduledUndoAdapter::KisScheduledUndoAdapter()
    : m_macroCounter(0)
{
}

const KUndo2Command* KisScheduledUndoAdapter::presentCommand()
{
    return 0;
}

void KisScheduledUndoAdapter::undoLastCommand()
{
    // sorry, we can't do this
}

void KisScheduledUndoAdapter::addCommand(KUndo2CommandSP command)
{
    if(!command) return;

    /**
     * NOTE: This is a kind of hack until all the tool are ported
     * to the strokes framework. We wrap all the commands
     * into a special command that executes the internal
     * command onto the scheduler
     */

    if(m_macroCounter) {
        image()->addJob(m_macroStrokeId,
                        new KisStrokeStrategyUndoCommandBased::Data(command));
    }
    else {
        KisStrokeStrategyUndoCommandBased *strategy =
            new KisStrokeStrategyUndoCommandBased(command->text(),
                                                  false, image()->realUndoAdapter(),
                                                  command);
        KisStrokeId id = image()->startStroke(strategy);
        image()->endStroke(id);
    }
}

void KisScheduledUndoAdapter::beginMacro(const QString& macroName)
{
    if(!m_macroCounter) {
        KisStrokeStrategy *strategy =
            new KisStrokeStrategyUndoCommandBased(macroName,
                                                  false, image()->realUndoAdapter());
        m_macroStrokeId = image()->startStroke(strategy);
    }

    m_macroCounter++;
}

void KisScheduledUndoAdapter::endMacro()
{
    m_macroCounter--;

    if(!m_macroCounter) {
        image()->endStroke(m_macroStrokeId);
    }
}


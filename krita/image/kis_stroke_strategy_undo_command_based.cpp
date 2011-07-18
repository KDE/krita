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

#include "kis_stroke_strategy_undo_command_based.h"

#include <QMutexLocker>
#include "kis_undo_adapter.h"

KisStrokeStrategyUndoCommandBased::
KisStrokeStrategyUndoCommandBased(const QString &name,
                                  bool undo,
                                  KisUndoAdapter *undoAdapter,
                                  KUndo2CommandSP initCommand,
                                  KUndo2CommandSP finishCommand)
  : KisSimpleStrokeStrategy("STROKE_UNDO_COMMAND_BASED", name),
    m_undo(undo),
    m_initCommand(initCommand),
    m_finishCommand(finishCommand),
    m_undoAdapter(undoAdapter)
{
    if(m_initCommand) {
        enableJob(KisSimpleStrokeStrategy::JOB_INIT);
    }

    enableJob(KisSimpleStrokeStrategy::JOB_FINISH);
    enableJob(KisSimpleStrokeStrategy::JOB_CANCEL);
    enableJob(KisSimpleStrokeStrategy::JOB_DOSTROKE);
}

void KisStrokeStrategyUndoCommandBased::executeCommand(KUndo2CommandSP command)
{
    if(!command) return;

    if(m_undo) {
        command->undo();
    } else {
        command->redo();
    }

    notifyCommandDone(command);
}

void KisStrokeStrategyUndoCommandBased::initStrokeCallback()
{
    executeCommand(m_initCommand);
}

void KisStrokeStrategyUndoCommandBased::finishStrokeCallback()
{
    executeCommand(m_finishCommand);

    if(m_undoAdapter) {
        m_undoAdapter->beginMacroWorkaround(name());

        QVector<KUndo2CommandSP> commands = takeFinishedCommands();
        foreach(KUndo2CommandSP cmd, commands) {
            m_undoAdapter->addCommandWorkaroundSP(cmd);
        }

        m_undoAdapter->endMacroWorkaround();
    }
}

void KisStrokeStrategyUndoCommandBased::cancelStrokeCallback()
{
    QVector<KUndo2CommandSP> commands = takeFinishedCommands();
    QVectorIterator<KUndo2CommandSP> it(commands);

    it.toBack();
    while(it.hasPrevious()) {
        KUndo2CommandSP cmd = it.previous();

        if(!m_undo) {
            cmd->undo();
        } else {
            cmd->redo();
        }
    }
}

void KisStrokeStrategyUndoCommandBased::doStrokeCallback(KisStrokeJobStrategy::StrokeJobData *data)
{
    Data *d = dynamic_cast<Data*>(data);
    executeCommand(d->command);
}

void KisStrokeStrategyUndoCommandBased::notifyCommandDone(KUndo2CommandSP command)
{
    QMutexLocker locker(&m_mutex);
    m_doneCommands.append(command);
}

QVector<KUndo2CommandSP> KisStrokeStrategyUndoCommandBased::takeFinishedCommands()
{
    QMutexLocker locker(&m_mutex);

    QVector<KUndo2CommandSP> commands = m_doneCommands;
    m_doneCommands.clear();

    return commands;
}

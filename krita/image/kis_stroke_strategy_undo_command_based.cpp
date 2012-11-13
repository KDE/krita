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
#include "kis_post_execution_undo_adapter.h"
#include "commands_new/kis_saved_commands.h"


KisStrokeStrategyUndoCommandBased::
KisStrokeStrategyUndoCommandBased(const QString &name,
                                  bool undo,
                                  KisPostExecutionUndoAdapter *undoAdapter,
                                  KUndo2CommandSP initCommand,
                                  KUndo2CommandSP finishCommand)
  : KisSimpleStrokeStrategy("STROKE_UNDO_COMMAND_BASED", name),
    m_undo(undo),
    m_initCommand(initCommand),
    m_finishCommand(finishCommand),
    m_undoAdapter(undoAdapter),
    m_macroCommand(0)
{
    enableJob(KisSimpleStrokeStrategy::JOB_INIT);
    enableJob(KisSimpleStrokeStrategy::JOB_FINISH);
    enableJob(KisSimpleStrokeStrategy::JOB_CANCEL);
    enableJob(KisSimpleStrokeStrategy::JOB_DOSTROKE);
}

void KisStrokeStrategyUndoCommandBased::executeCommand(KUndo2CommandSP command, bool undo)
{
    if(!command) return;

    if(undo) {
        command->undo();
    } else {
        command->redo();
    }
}

void KisStrokeStrategyUndoCommandBased::initStrokeCallback()
{
    if(m_undoAdapter) {
        m_macroCommand = m_undoAdapter->createMacro(name());
    }

    executeCommand(m_initCommand, m_undo);
    notifyCommandDone(m_initCommand,
                      KisStrokeJobData::SEQUENTIAL,
                      KisStrokeJobData::NORMAL);
}

void KisStrokeStrategyUndoCommandBased::finishStrokeCallback()
{
    executeCommand(m_finishCommand, m_undo);
    notifyCommandDone(m_finishCommand,
                      KisStrokeJobData::SEQUENTIAL,
                      KisStrokeJobData::NORMAL);

    QMutexLocker locker(&m_mutex);
    if(m_macroCommand) {
        Q_ASSERT(m_undoAdapter);
        m_undoAdapter->addMacro(m_macroCommand);
        m_macroCommand = 0;
    }
}

void KisStrokeStrategyUndoCommandBased::cancelStrokeCallback()
{
    QMutexLocker locker(&m_mutex);
    if(m_macroCommand) {
        m_macroCommand->performCancel(cancelStrokeId(), m_undo);
        delete m_macroCommand;
        m_macroCommand = 0;
    }
}

void KisStrokeStrategyUndoCommandBased::doStrokeCallback(KisStrokeJobData *data)
{
    Data *d = dynamic_cast<Data*>(data);
    executeCommand(d->command, d->undo);
    notifyCommandDone(d->command, d->sequentiality(), d->exclusivity());
}

void KisStrokeStrategyUndoCommandBased::runAndSaveCommand(KUndo2CommandSP command,
                                                          KisStrokeJobData::Sequentiality sequentiality,
                                                          KisStrokeJobData::Exclusivity exclusivity)
{
    command->redo();
    notifyCommandDone(command, sequentiality, exclusivity);
}

void KisStrokeStrategyUndoCommandBased::notifyCommandDone(KUndo2CommandSP command,
                                                          KisStrokeJobData::Sequentiality sequentiality,
                                                          KisStrokeJobData::Exclusivity exclusivity)
{
    if(!command) return;

    QMutexLocker locker(&m_mutex);
    if(m_macroCommand) {
        m_macroCommand->addCommand(command, sequentiality, exclusivity);
    }
}

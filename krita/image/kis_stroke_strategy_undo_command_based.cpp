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
  : KisStrokeStrategy("STROKE_UNDO_COMMAND_BASED", name),
    m_undo(undo),
    m_initCommand(initCommand),
    m_finishCommand(finishCommand),
    m_sequential(true),
    m_undoAdapter(undoAdapter)
{
}

void KisStrokeStrategyUndoCommandBased::setSequential(bool value)
{
    m_sequential = value;
}

KisStrokeJobStrategy* KisStrokeStrategyUndoCommandBased::createInitStrategy()
{
    return m_initCommand ? new KisStrokeJobStrategyUndoCommandBased(KisStrokeJobStrategy::SEQUENTIAL, this, 0) : 0;
}

KisStrokeJobStrategy* KisStrokeStrategyUndoCommandBased::createFinishStrategy()
{
    return m_finishCommand ? new KisStrokeJobStrategyUndoCommandBased(KisStrokeJobStrategy::SEQUENTIAL, this, m_undoAdapter) : 0;
}

KisStrokeJobStrategy* KisStrokeStrategyUndoCommandBased::createCancelStrategy()
{
    return new KisStrokeJobStrategyCancelUndoCommandBased(this);
}

KisStrokeJobStrategy* KisStrokeStrategyUndoCommandBased::createDabStrategy()
{
    return new KisStrokeJobStrategyUndoCommandBased(m_sequential ? KisStrokeJobStrategy::SEQUENTIAL : KisStrokeJobStrategy::CONCURRENT, this, 0);
}


KisStrokeJobStrategy::StrokeJobData* KisStrokeStrategyUndoCommandBased::createInitData()
{
    return m_initCommand ? new KisStrokeJobStrategyUndoCommandBased::Data(m_initCommand, false) : 0;
}

KisStrokeJobStrategy::StrokeJobData* KisStrokeStrategyUndoCommandBased::createFinishData()
{
    return m_finishCommand ? new KisStrokeJobStrategyUndoCommandBased::Data(m_finishCommand, true) : 0;
}

KisStrokeJobStrategy::StrokeJobData* KisStrokeStrategyUndoCommandBased::createCancelData()
{
    return 0;
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


KisStrokeJobStrategyUndoCommandBased::KisStrokeJobStrategyUndoCommandBased(Sequentiality sequentiality, KisStrokeStrategyUndoCommandBased *parentStroke, KisUndoAdapter *undoAdapter)
    : KisStrokeJobStrategy(sequentiality, NORMAL),
      m_parentStroke(parentStroke),
      m_undoAdapter(undoAdapter)
{
}

void KisStrokeJobStrategyUndoCommandBased::run(StrokeJobData *data)
{
    Data *d = dynamic_cast<Data*>(data);

    if(m_parentStroke->undo()) {
        d->command->undo();
    } else {
        d->command->redo();
    }

    m_parentStroke->notifyCommandDone(d->command);

    if(d->doFinish && m_undoAdapter) {
        m_undoAdapter->beginMacroWorkaround(m_parentStroke->name());

        QVector<KUndo2CommandSP> commands = m_parentStroke->takeFinishedCommands();
        foreach(KUndo2CommandSP cmd, commands) {
            m_undoAdapter->addCommandWorkaroundSP(cmd);
        }

        m_undoAdapter->endMacroWorkaround();
    }
}

KisStrokeJobStrategyCancelUndoCommandBased::KisStrokeJobStrategyCancelUndoCommandBased(KisStrokeStrategyUndoCommandBased *parentStroke)
    : KisStrokeJobStrategy(SEQUENTIAL, NORMAL),
      m_parentStroke(parentStroke)
{
}

void KisStrokeJobStrategyCancelUndoCommandBased::run(StrokeJobData *data)
{
    Q_UNUSED(data);

    QVector<KUndo2CommandSP> commands = m_parentStroke->takeFinishedCommands();
    QVectorIterator<KUndo2CommandSP> it(commands);

    it.toBack();
    while(it.hasPrevious()) {
        KUndo2CommandSP cmd = it.previous();

        if(!m_parentStroke->undo()) {
            cmd->undo();
        } else {
            cmd->redo();
        }
    }
}

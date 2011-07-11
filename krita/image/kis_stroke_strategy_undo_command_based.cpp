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


KisStrokeStrategyUndoCommandBased::
KisStrokeStrategyUndoCommandBased(const QString &name,
                                  bool undo,
                                  QUndoCommandSP initCommand,
                                  QUndoCommandSP finishCommand)
  : KisStrokeStrategy("STROKE_UNDO_COMMAND_BASED", name),
    m_undo(undo),
    m_initCommand(initCommand),
    m_finishCommand(finishCommand),
    m_sequential(true)
{
}

void KisStrokeStrategyUndoCommandBased::setSequential(bool value)
{
    m_sequential = value;
}

KisStrokeJobStrategy* KisStrokeStrategyUndoCommandBased::createInitStrategy()
{
    return new KisStrokeJobStrategyUndoCommandBased(true, this);
}

KisStrokeJobStrategy* KisStrokeStrategyUndoCommandBased::createFinishStrategy()
{
    return new KisStrokeJobStrategyUndoCommandBased(true, this);
}

KisStrokeJobStrategy* KisStrokeStrategyUndoCommandBased::createCancelStrategy()
{
    return new KisStrokeJobStrategyCancelUndoCommandBased(!m_undo, this);
}

KisStrokeJobStrategy* KisStrokeStrategyUndoCommandBased::createDabStrategy()
{
    return new KisStrokeJobStrategyUndoCommandBased(m_sequential, this);
}


KisStrokeJobStrategy::StrokeJobData* KisStrokeStrategyUndoCommandBased::createInitData()
{
    return new KisStrokeJobStrategyUndoCommandBased::Data(m_undo, m_initCommand);
}

KisStrokeJobStrategy::StrokeJobData* KisStrokeStrategyUndoCommandBased::createFinishData()
{
    return new KisStrokeJobStrategyUndoCommandBased::Data(m_undo, m_finishCommand);
}

KisStrokeJobStrategy::StrokeJobData* KisStrokeStrategyUndoCommandBased::createCancelData()
{
    return 0;
}

void KisStrokeStrategyUndoCommandBased::notifyCommandDone(QUndoCommandSP command)
{
    QMutexLocker locker(&m_mutex);
    m_doneCommands.prepend(command);
}

QVector<QUndoCommandSP> KisStrokeStrategyUndoCommandBased::takeUndoCommands()
{
    QMutexLocker locker(&m_mutex);

    QVector<QUndoCommandSP> commands = m_doneCommands;
    m_doneCommands.clear();

    return commands;
}


KisStrokeJobStrategyUndoCommandBased::KisStrokeJobStrategyUndoCommandBased(bool isSequential, KisStrokeStrategyUndoCommandBased *parentStroke)
    : KisStrokeJobStrategy(isSequential, false),
      m_parentStroke(parentStroke)
{
}

void KisStrokeJobStrategyUndoCommandBased::run(StrokeJobData *data)
{
    Data *d = dynamic_cast<Data*>(data);

    if(d->undo) {
        d->command->undo();
    } else {
        d->command->redo();
    }

    if(m_parentStroke) {
        m_parentStroke->notifyCommandDone(d->command);
    }
}

KisStrokeJobStrategyCancelUndoCommandBased::KisStrokeJobStrategyCancelUndoCommandBased(bool undo, KisStrokeStrategyUndoCommandBased *parentStroke)
    : KisStrokeJobStrategy(true, false),
      m_undo(undo),
      m_parentStroke(parentStroke)
{
}

void KisStrokeJobStrategyCancelUndoCommandBased::run(StrokeJobData *data)
{
    Q_UNUSED(data);

    QVector<QUndoCommandSP> commands = m_parentStroke->takeUndoCommands();
    foreach(QUndoCommandSP cmd, commands) {
        if(m_undo) {
            cmd->undo();
        } else {
            cmd->redo();
        }
    }
}

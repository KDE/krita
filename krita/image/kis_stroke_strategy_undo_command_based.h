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

#ifndef __KIS_STROKE_STRATEGY_UNDO_COMMAND_BASED_H
#define __KIS_STROKE_STRATEGY_UNDO_COMMAND_BASED_H

#include <kundo2command.h>
#include <QSharedPointer>
#include <QVector>
#include <QMutex>

#include "kis_types.h"
#include "kis_stroke_strategy.h"


class QString;

class KisStrokeJob;
class KisUndoAdapter;

class KRITAIMAGE_EXPORT KisStrokeStrategyUndoCommandBased : public KisStrokeStrategy
{
public:
    KisStrokeStrategyUndoCommandBased(const QString &name,
                                      bool undo,
                                      KisUndoAdapter *undoAdapter,
                                      KUndo2CommandSP initCommand = KUndo2CommandSP(0),
                                      KUndo2CommandSP finishCommand = KUndo2CommandSP(0));

    /**
     * WARNING: This method is not considered to be called after
     * the KisStroke object has been created
     */
    using KisStrokeStrategy::setExclusive;

    /**
     * Defines whether the commands added with addJob() are sequential
     * or not. All the jobs are sequential by default.
     *
     * WARNING: This method is not considered to be called after
     * the KisStroke object has been created
     */
    void setSequential(bool value);


    KisStrokeJobStrategy* createInitStrategy();
    KisStrokeJobStrategy* createFinishStrategy();
    KisStrokeJobStrategy* createCancelStrategy();
    KisStrokeJobStrategy* createDabStrategy();

    KisStrokeJobStrategy::StrokeJobData* createInitData();
    KisStrokeJobStrategy::StrokeJobData* createFinishData();
    KisStrokeJobStrategy::StrokeJobData* createCancelData();

    void notifyCommandDone(KUndo2CommandSP command);
    QVector<KUndo2CommandSP> takeFinishedCommands();
    bool undo() {
        return m_undo;
    }

private:
    bool m_undo;
    KUndo2CommandSP m_initCommand;
    KUndo2CommandSP m_finishCommand;
    bool m_sequential;
    KisUndoAdapter *m_undoAdapter;

    // protects done commands only
    QMutex m_mutex;
    QVector<KUndo2CommandSP> m_doneCommands;
};


class KRITAIMAGE_EXPORT KisStrokeJobStrategyUndoCommandBased : public KisStrokeJobStrategy
{
public:
    class Data : public StrokeJobData {
    public:
        Data(KUndo2CommandSP _command, bool _doFinish)
        : command(_command),
          doFinish(_doFinish)
        {
        }

        KUndo2CommandSP command;
        bool doFinish;
    };

public:
    KisStrokeJobStrategyUndoCommandBased(bool isSequential, KisStrokeStrategyUndoCommandBased *parentStroke, KisUndoAdapter *undoAdapter);
    void run(StrokeJobData *data);

private:
    // testing suite
    friend QString getCommandName(KisStrokeJob *job);

    KisStrokeStrategyUndoCommandBased *m_parentStroke;
    KisUndoAdapter *m_undoAdapter;
};


class KRITAIMAGE_EXPORT KisStrokeJobStrategyCancelUndoCommandBased : public KisStrokeJobStrategy
{
public:
    KisStrokeJobStrategyCancelUndoCommandBased(KisStrokeStrategyUndoCommandBased *parentStroke);
    void run(StrokeJobData *data);

private:
    // testing suite
    friend QString getCommandName(KisStrokeJob *job);

    KisStrokeStrategyUndoCommandBased *m_parentStroke;
};

#endif /* __KIS_STROKE_STRATEGY_UNDO_COMMAND_BASED_H */

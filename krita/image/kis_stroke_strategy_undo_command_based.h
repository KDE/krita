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
#include "kis_simple_stroke_strategy.h"


class QString;

class KisStrokeJob;
class KisUndoAdapter;

class KRITAIMAGE_EXPORT KisStrokeStrategyUndoCommandBased : public KisSimpleStrokeStrategy
{
public:
    class Data : public KisStrokeJobData {
    public:
        Data(KUndo2CommandSP _command,
             Sequentiality _sequentiality = SEQUENTIAL,
             Exclusivity _exclusivity = NORMAL)
            : KisStrokeJobData(_sequentiality, _exclusivity),
              command(_command)
        {
        }
        KUndo2CommandSP command;
    };

public:
    KisStrokeStrategyUndoCommandBased(const QString &name,
                                      bool undo,
                                      KisUndoAdapter *undoAdapter,
                                      KUndo2CommandSP initCommand = KUndo2CommandSP(0),
                                      KUndo2CommandSP finishCommand = KUndo2CommandSP(0));

    using KisSimpleStrokeStrategy::setExclusive;

    void initStrokeCallback();
    void finishStrokeCallback();
    void cancelStrokeCallback();
    void doStrokeCallback(KisStrokeJobData *data);

private:
    void executeCommand(KUndo2CommandSP command);
    void notifyCommandDone(KUndo2CommandSP command);
    QVector<KUndo2CommandSP> takeFinishedCommands();

private:
    bool m_undo;
    KUndo2CommandSP m_initCommand;
    KUndo2CommandSP m_finishCommand;
    KisUndoAdapter *m_undoAdapter;

    // protects done commands only
    QMutex m_mutex;
    QVector<KUndo2CommandSP> m_doneCommands;
};

#endif /* __KIS_STROKE_STRATEGY_UNDO_COMMAND_BASED_H */

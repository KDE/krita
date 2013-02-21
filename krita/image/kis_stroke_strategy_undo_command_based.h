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
#include <QVector>
#include <QMutex>

#include "kis_types.h"
#include "kis_simple_stroke_strategy.h"
#include "commands_new/kis_saved_commands.h"


class QString;

class KisStrokeJob;
class KisPostExecutionUndoAdapter;


class KRITAIMAGE_EXPORT KisStrokeStrategyUndoCommandBased : public KisSimpleStrokeStrategy
{
public:
    class Data : public KisStrokeJobData {
    public:
        Data(KUndo2CommandSP _command,
             bool _undo = false,
             Sequentiality _sequentiality = SEQUENTIAL,
             Exclusivity _exclusivity = NORMAL)
            : KisStrokeJobData(_sequentiality, _exclusivity),
              command(_command),
              undo(_undo)
        {
        }

        Data(KUndo2Command *_command,
             bool _undo = false,
             Sequentiality _sequentiality = SEQUENTIAL,
             Exclusivity _exclusivity = NORMAL)
            : KisStrokeJobData(_sequentiality, _exclusivity),
              command(_command),
              undo(_undo)
        {
        }

        KUndo2CommandSP command;
        bool undo;
    };

public:
    KisStrokeStrategyUndoCommandBased(const QString &name,
                                      bool undo,
                                      KisPostExecutionUndoAdapter *undoAdapter,
                                      KUndo2CommandSP initCommand = KUndo2CommandSP(0),
                                      KUndo2CommandSP finishCommand = KUndo2CommandSP(0));

    using KisSimpleStrokeStrategy::setExclusive;

    void initStrokeCallback();
    void finishStrokeCallback();
    void cancelStrokeCallback();
    void doStrokeCallback(KisStrokeJobData *data);

protected:
    void runAndSaveCommand(KUndo2CommandSP command,
                           KisStrokeJobData::Sequentiality sequentiality,
                           KisStrokeJobData::Exclusivity exclusivity);
    void notifyCommandDone(KUndo2CommandSP command,
                           KisStrokeJobData::Sequentiality sequentiality,
                           KisStrokeJobData::Exclusivity exclusivity);

private:
    void executeCommand(KUndo2CommandSP command, bool undo);

private:
    bool m_undo;
    KUndo2CommandSP m_initCommand;
    KUndo2CommandSP m_finishCommand;
    KisPostExecutionUndoAdapter *m_undoAdapter;

    // protects done commands only
    QMutex m_mutex;
    KisSavedMacroCommand *m_macroCommand;
};

#endif /* __KIS_STROKE_STRATEGY_UNDO_COMMAND_BASED_H */

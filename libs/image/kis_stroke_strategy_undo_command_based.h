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
#include "KisRunnableBasedStrokeStrategy.h"


class KisStrokeJob;
class KisSavedMacroCommand;
class KisStrokeUndoFacade;
class KisStrokesQueueMutatedJobInterface;


class KRITAIMAGE_EXPORT KisStrokeStrategyUndoCommandBased : public KisRunnableBasedStrokeStrategy
{
public:
    struct MutatedCommandInterface
    {
        virtual ~MutatedCommandInterface() {}

        void setRunnableJobsInterface(KisRunnableStrokeJobsInterface *interface) {
            m_mutatedJobsInterface = interface;
        }

        KisRunnableStrokeJobsInterface* runnableJobsInterface() const {
            return m_mutatedJobsInterface;
        }

    private:
        KisRunnableStrokeJobsInterface *m_mutatedJobsInterface;
    };


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
    KisStrokeStrategyUndoCommandBased(const KUndo2MagicString &name,
                                      bool undo,
                                      KisStrokeUndoFacade *undoFacade,
                                      KUndo2CommandSP initCommand = KUndo2CommandSP(0),
                                      KUndo2CommandSP finishCommand = KUndo2CommandSP(0));

    using KisSimpleStrokeStrategy::setExclusive;

    void initStrokeCallback() override;
    void finishStrokeCallback() override;
    void cancelStrokeCallback() override;
    void doStrokeCallback(KisStrokeJobData *data) override;

    /**
     * Set extra data that will be assigned to the command
     * representing this action. Using extra data has the following
     * restrictions:
     *
     * 1) The \p data must be set *before* the stroke has been started.
     *    Setting the \p data after the stroke has been started with
     *    image->startStroke(strokeId) leads to an undefined behaviour.
     *
     * 2) \p data becomes owned by the strategy/command right after
     *    setting it. Don't try to change it afterwards.
     */
    void setCommandExtraData(KUndo2CommandExtraData *data);

    /**
     * Sets the id of this action. Will be used for merging the undo commands
     *
     * The \p value must be set *before* the stroke has been started.
     * Setting the \p value after the stroke has been started with
     * image->startStroke(strokeId) leads to an undefined behaviour.
     */
    void setMacroId(int value);

    /**
     * The undo-command-based is a low-level strategy, so it allows
     * changing its wraparound mode status.
     *
     * WARNING: the switch must be called *before* the stroke has been
     * started! Otherwise the mode will not be activated.
     */
    using KisStrokeStrategy::setSupportsWrapAroundMode;

    void setUsedWhileUndoRedo(bool value);

protected:
    void runAndSaveCommand(KUndo2CommandSP command,
                           KisStrokeJobData::Sequentiality sequentiality,
                           KisStrokeJobData::Exclusivity exclusivity);
    void notifyCommandDone(KUndo2CommandSP command,
                           KisStrokeJobData::Sequentiality sequentiality,
                           KisStrokeJobData::Exclusivity exclusivity);

    KisStrokeStrategyUndoCommandBased(const KisStrokeStrategyUndoCommandBased &rhs);

    virtual void postProcessToplevelCommand(KUndo2Command *command);

    KisStrokeUndoFacade* undoFacade() const;

private:
    void executeCommand(KUndo2CommandSP command, bool undo);

private:
    bool m_undo;
    KUndo2CommandSP m_initCommand;
    KUndo2CommandSP m_finishCommand;
    KisStrokeUndoFacade *m_undoFacade;

    QScopedPointer<KUndo2CommandExtraData> m_commandExtraData;
    int m_macroId;

    // protects done commands only
    QMutex m_mutex;
    KisSavedMacroCommand *m_macroCommand;
};

#endif /* __KIS_STROKE_STRATEGY_UNDO_COMMAND_BASED_H */

/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_stroke_strategy_undo_command_based.h"

#include <QMutexLocker>
#include "kis_image_interfaces.h"
#include "kis_post_execution_undo_adapter.h"
#include "commands_new/kis_saved_commands.h"


KisStrokeStrategyUndoCommandBased::
KisStrokeStrategyUndoCommandBased(const KUndo2MagicString &name,
                                  bool undo,
                                  KisStrokeUndoFacade *undoFacade,
                                  KUndo2CommandSP initCommand,
                                  KUndo2CommandSP finishCommand)
  : KisRunnableBasedStrokeStrategy(QLatin1String("STROKE_UNDO_COMMAND_BASED"), name),
    m_undo(undo),
    m_initCommand(initCommand),
    m_finishCommand(finishCommand),
    m_undoFacade(undoFacade),
    m_macroId(-1),
    m_macroCommand(0)
{
    enableJob(KisSimpleStrokeStrategy::JOB_INIT);
    enableJob(KisSimpleStrokeStrategy::JOB_FINISH);
    enableJob(KisSimpleStrokeStrategy::JOB_CANCEL);
    enableJob(KisSimpleStrokeStrategy::JOB_DOSTROKE);
}

KisStrokeStrategyUndoCommandBased::
KisStrokeStrategyUndoCommandBased(const KisStrokeStrategyUndoCommandBased &rhs)
  : KisRunnableBasedStrokeStrategy(rhs),
    m_undo(false),
    m_initCommand(rhs.m_initCommand),
    m_finishCommand(rhs.m_finishCommand),
    m_undoFacade(rhs.m_undoFacade),
    m_macroCommand(0)
{
    KIS_ASSERT_RECOVER_NOOP(!rhs.m_macroCommand &&
                            !rhs.m_undo &&
                            "After the stroke has been started, no copying must happen");
}

void KisStrokeStrategyUndoCommandBased::setUsedWhileUndoRedo(bool value)
{
    setClearsRedoOnStart(!value);
}

void KisStrokeStrategyUndoCommandBased::executeCommand(KUndo2CommandSP command, bool undo)
{
    if(!command) return;

    if (MutatedCommandInterface *mutatedCommand = dynamic_cast<MutatedCommandInterface*>(command.data())) {
        mutatedCommand->setRunnableJobsInterface(this->runnableJobsInterface());
    }

    if(undo) {
        command->undo();
    } else {
        command->redo();
    }
}

void KisStrokeStrategyUndoCommandBased::initStrokeCallback()
{
    if(m_undoFacade) {
        m_macroCommand = m_undoFacade->postExecutionUndoAdapter()->createMacro(name());
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
        Q_ASSERT(m_undoFacade);
        postProcessToplevelCommand(m_macroCommand);
        m_undoFacade->postExecutionUndoAdapter()->addMacro(m_macroCommand);
        m_macroCommand = 0;
    }
}

void KisStrokeStrategyUndoCommandBased::cancelStrokeCallback()
{
    QMutexLocker locker(&m_mutex);
    if(m_macroCommand) {
        QVector<KisStrokeJobData *> jobs;
        m_macroCommand->getCommandExecutionJobs(&jobs, !m_undo);
        addMutatedJobs(jobs);

        delete m_macroCommand;
        m_macroCommand = 0;
    }
}

void KisStrokeStrategyUndoCommandBased::doStrokeCallback(KisStrokeJobData *data)
{
    Data *d = dynamic_cast<Data*>(data);

    if (d) {
        executeCommand(d->command, d->undo);
        if (d->shouldGoToHistory) {
            notifyCommandDone(d->command,
                              d->sequentiality(),
                              d->exclusivity());
        }
    } else {
        KisRunnableBasedStrokeStrategy::doStrokeCallback(data);
    }

}

void KisStrokeStrategyUndoCommandBased::runAndSaveCommand(KUndo2CommandSP command,
                                                          KisStrokeJobData::Sequentiality sequentiality,
                                                          KisStrokeJobData::Exclusivity exclusivity)
{
    if (!command) return;

    executeCommand(command, false);
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

void KisStrokeStrategyUndoCommandBased::setCommandExtraData(KUndo2CommandExtraData *data)
{
    if (m_undoFacade && m_macroCommand) {
        warnKrita << "WARNING: KisStrokeStrategyUndoCommandBased::setCommandExtraData():"
                   << "the extra data is set while the stroke has already been started!"
                   << "The result is undefined, continued actions may not work!";
    }

    m_commandExtraData.reset(data);
}

void KisStrokeStrategyUndoCommandBased::setMacroId(int value)
{
    m_macroId = value;
}

void KisStrokeStrategyUndoCommandBased::postProcessToplevelCommand(KUndo2Command *command)
{
    if (m_commandExtraData) {
        command->setExtraData(m_commandExtraData.take());
    }

    KisSavedMacroCommand *savedCommand = dynamic_cast<KisSavedMacroCommand*>(command);
    if (savedCommand) {
        savedCommand->setMacroId(m_macroId);
    }
}

 KisStrokeUndoFacade* KisStrokeStrategyUndoCommandBased::undoFacade() const
 {
     return m_undoFacade;
 }

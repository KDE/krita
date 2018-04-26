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

#include "kis_saved_commands.h"

#include <QVector>

#include "kis_image_interfaces.h"
#include "kis_stroke_strategy_undo_command_based.h"


KisSavedCommandBase::KisSavedCommandBase(const KUndo2MagicString &name,
                                         KisStrokesFacade *strokesFacade)
    : KUndo2Command(name),
      m_strokesFacade(strokesFacade),
      m_skipOneRedo(true)
{
}

KisSavedCommandBase::~KisSavedCommandBase()
{
}

KisStrokesFacade* KisSavedCommandBase::strokesFacade()
{
    return m_strokesFacade;
}

void KisSavedCommandBase::runStroke(bool undo)
{
    KisStrokeStrategyUndoCommandBased *strategy =
        new KisStrokeStrategyUndoCommandBased(text(), undo, 0);
    strategy->setUsedWhileUndoRedo(true);

    KisStrokeId id = m_strokesFacade->startStroke(strategy);
    addCommands(id, undo);
    m_strokesFacade->endStroke(id);
}

void KisSavedCommandBase::undo()
{

    runStroke(true);
}



void KisSavedCommandBase::redo()
{
    /**
     * All the commands are first executed in the stroke and then
     * added to the undo stack. It means that the first redo should be
     * skipped
     */

    if(m_skipOneRedo) {
        m_skipOneRedo = false;
        return;
    }

    runStroke(false);
}


KisSavedCommand::KisSavedCommand(KUndo2CommandSP command,
                                 KisStrokesFacade *strokesFacade)
    : KisSavedCommandBase(command->text(), strokesFacade),
      m_command(command)
{
}

int KisSavedCommand::id() const
{
    return m_command->id();
}

bool KisSavedCommand::mergeWith(const KUndo2Command* command)
{
    const KisSavedCommand *other =
        dynamic_cast<const KisSavedCommand*>(command);

    if (other) {
        command = other->m_command.data();
    }

    return m_command->mergeWith(command);
}

void KisSavedCommand::addCommands(KisStrokeId id, bool undo)
{
    strokesFacade()->
        addJob(id, new KisStrokeStrategyUndoCommandBased::Data(m_command, undo));
}
int KisSavedCommand::timedId()
{
    return m_command->timedId();
}
void KisSavedCommand::setTimedID(int timedID)
{
    m_command->setTimedID(timedID);
}

bool KisSavedCommand::timedMergeWith(KUndo2Command *other)
{
    return m_command->timedMergeWith(other);
}
QVector<KUndo2Command*> KisSavedCommand::mergeCommandsVector()
{
    return m_command->mergeCommandsVector();
}
void KisSavedCommand::setTime()
{
    m_command->setTime();
}

QTime KisSavedCommand::time()
{
    return m_command->time();
}
void KisSavedCommand::setEndTime()
{
    m_command->setEndTime();
}

QTime KisSavedCommand::endTime()
{
    return m_command->endTime();
}
bool KisSavedCommand::isMerged()
{
    return m_command->isMerged();
}



struct KisSavedMacroCommand::Private
{
    struct SavedCommand {
        KUndo2CommandSP command;
        KisStrokeJobData::Sequentiality sequentiality;
        KisStrokeJobData::Exclusivity exclusivity;
    };

    QVector<SavedCommand> commands;
    int macroId = -1;
};

KisSavedMacroCommand::KisSavedMacroCommand(const KUndo2MagicString &name,
                                           KisStrokesFacade *strokesFacade)
    : KisSavedCommandBase(name, strokesFacade),
      m_d(new Private())
{
}

KisSavedMacroCommand::~KisSavedMacroCommand()
{
    delete m_d;
}

void KisSavedMacroCommand::setMacroId(int value)
{
    m_d->macroId = value;
}

int KisSavedMacroCommand::id() const
{
    return m_d->macroId;
}

bool KisSavedMacroCommand::mergeWith(const KUndo2Command* command)
{
    const KisSavedMacroCommand *other =
        dynamic_cast<const KisSavedMacroCommand*>(command);

    if (!other || other->id() != id()) return false;

    QVector<Private::SavedCommand> &otherCommands = other->m_d->commands;

    if (m_d->commands.size() != otherCommands.size()) return false;

    auto it = m_d->commands.constBegin();
    auto end = m_d->commands.constEnd();

    auto otherIt = otherCommands.constBegin();
    auto otherEnd = otherCommands.constEnd();

    bool sameCommands = true;
    while (it != end && otherIt != otherEnd) {
        if (it->command->id() != otherIt->command->id() ||
            it->sequentiality != otherIt->sequentiality ||
            it->exclusivity != otherIt->exclusivity) {

            sameCommands = false;
            break;
        }
        ++it;
        ++otherIt;
    }

    if (!sameCommands) return false;

    it = m_d->commands.constBegin();
    otherIt = otherCommands.constBegin();

    while (it != end && otherIt != otherEnd) {
        if (it->command->id() != -1) {
            bool result = it->command->mergeWith(otherIt->command.data());
            KIS_ASSERT_RECOVER(result) { return false; }
        }
        ++it;
        ++otherIt;
    }

    return true;
}

void KisSavedMacroCommand::addCommand(KUndo2CommandSP command,
                                      KisStrokeJobData::Sequentiality sequentiality,
                                      KisStrokeJobData::Exclusivity exclusivity)
{
    Private::SavedCommand item;
    item.command = command;
    item.sequentiality = sequentiality;
    item.exclusivity = exclusivity;

    m_d->commands.append(item);
}

void KisSavedMacroCommand::performCancel(KisStrokeId id, bool strokeUndo)
{
    addCommands(id, !strokeUndo);
}

void KisSavedMacroCommand::addCommands(KisStrokeId id, bool undo)
{
    QVector<Private::SavedCommand>::iterator it;

    if(!undo) {
        for(it = m_d->commands.begin(); it != m_d->commands.end(); it++) {
            strokesFacade()->
                addJob(id, new KisStrokeStrategyUndoCommandBased::
                       Data(it->command,
                            undo,
                            it->sequentiality,
                            it->exclusivity));
        }
    }
    else {
        for(it = m_d->commands.end(); it != m_d->commands.begin();) {
            --it;

            strokesFacade()->
                addJob(id, new KisStrokeStrategyUndoCommandBased::
                       Data(it->command,
                            undo,
                            it->sequentiality,
                            it->exclusivity));
        }
    }
}

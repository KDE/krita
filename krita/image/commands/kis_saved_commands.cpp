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

#include "kis_stroke_strategy_undo_command_based.h"
#include "kis_image.h"


KisSavedCommandBase::KisSavedCommandBase(const QString &name,
                                         KisImageWSP image)
    : m_name(name),
      m_image(image),
      m_skipOneRedo(true)
{
}

KisSavedCommandBase::~KisSavedCommandBase()
{
}

KisImageWSP KisSavedCommandBase::image()
{
    return m_image;
}

void KisSavedCommandBase::runStroke(bool undo)
{
    KisStrokeStrategyUndoCommandBased *strategy =
        new KisStrokeStrategyUndoCommandBased(m_name, undo, 0);

    KisStrokeId id = m_image->startStroke(strategy);
    addCommands(id, undo);
    m_image->endStroke(id);
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
                                 KisImageWSP image)
    : KisSavedCommandBase(command->text(), image),
      m_command(command)
{
}

void KisSavedCommand::addCommands(KisStrokeId id, bool undo)
{
    image()->
        addJob(id, new KisStrokeStrategyUndoCommandBased::Data(m_command, undo));
}


struct KisSavedMacroCommand::Private
{
    struct SavedCommand {
        KUndo2CommandSP command;
        KisStrokeJobData::Sequentiality sequentiality;
        KisStrokeJobData::Exclusivity exclusivity;
    };

    QVector<SavedCommand> commands;
};

KisSavedMacroCommand::KisSavedMacroCommand(const QString &name,
                                           KisImageWSP image)
    : KisSavedCommandBase(name, image),
      m_d(new Private())
{
}

KisSavedMacroCommand::~KisSavedMacroCommand()
{
    delete m_d;
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
            image()->
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

            image()->
                addJob(id, new KisStrokeStrategyUndoCommandBased::
                       Data(it->command,
                            undo,
                            it->sequentiality,
                            it->exclusivity));
        }
    }
}

/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_gui_context_command.h"
#include "kis_gui_context_command_p.h"


KisGuiContextCommand::KisGuiContextCommand(KUndo2Command *command, QObject *guiObject)
    : m_command(command),
      m_delegate(new KisGuiContextCommandDelegate(0))
{
    /**
     * We owe the delegate ourselves, so don't assign a parent to it,
     * but just move it to the GUI thread
     */
    m_delegate->moveToThread(guiObject->thread());

    connect(this, SIGNAL(sigExecuteCommand(KUndo2Command*,bool)),
            m_delegate.data(), SLOT(executeCommand(KUndo2Command*,bool)),
            Qt::BlockingQueuedConnection);
}

KisGuiContextCommand::~KisGuiContextCommand()
{
}

void KisGuiContextCommand::undo()
{
    emit sigExecuteCommand(m_command.data(), true);
}

void KisGuiContextCommand::redo()
{
    emit sigExecuteCommand(m_command.data(), false);
}

/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

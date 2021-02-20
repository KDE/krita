/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_gui_context_command_p.h"
#include "kundo2command.h"

KisGuiContextCommandDelegate::KisGuiContextCommandDelegate(QObject *parent)
    : QObject(parent)
{
}

void KisGuiContextCommandDelegate::executeCommand(KUndo2Command *command, bool undo)
{
    if (!undo) {
        command->redo();
    } else {
        command->undo();
    }
}

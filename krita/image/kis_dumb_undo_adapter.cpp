/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_dumb_undo_adapter.h"

#include <kundo2command.h>


KisDumbUndoAdapter::KisDumbUndoAdapter()
    : KisUndoAdapter(0)
{
}

KisDumbUndoAdapter::~KisDumbUndoAdapter()
{
}

const KUndo2Command * KisDumbUndoAdapter::presentCommand() {
    return 0;
}

void KisDumbUndoAdapter::addCommand(KUndo2Command *command) {
    command->redo();
    notifyCommandAdded(command);
    delete command;
}

void KisDumbUndoAdapter::undoLastCommand() {
    /**
     * Ermm.. Do we actually have one? We are dumb! ;)
     */
}

void KisDumbUndoAdapter::beginMacro(const QString& macroName) {
    /**
     * Yes, sir! >:)
     */
    Q_UNUSED(macroName);
}

void KisDumbUndoAdapter::endMacro() {
    /**
     * Roger that! :)
     */
}

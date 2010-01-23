/*
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_undo_adapter.h"

#include "kis_debug.h"
#include "KoDocument.h"
#include <KUndoStack>

KisUndoAdapter::KisUndoAdapter(KoDocument* doc): m_doc(doc)
{
    m_undo = true;
}

KisUndoAdapter::~KisUndoAdapter()
{
}

void KisUndoAdapter::setCommandHistoryListener(KisCommandHistoryListener * l)
{
    if (!m_undoListeners.contains(l))
        m_undoListeners.append(l);
}
void KisUndoAdapter::removeCommandHistoryListener(KisCommandHistoryListener * l)
{
    int index = m_undoListeners.indexOf(l);
    if (index != -1)
        m_undoListeners.remove(index);
}

void KisUndoAdapter::notifyCommandExecuted(const QUndoCommand *command)
{
    if (!command) {
        kWarning() << "Empty command!";
        return;
    }
    foreach(KisCommandHistoryListener*  l, m_undoListeners) {
        l->notifyCommandExecuted(command);
    }

}

const QUndoCommand * KisUndoAdapter::presentCommand()
{
    return m_doc->undoStack()->command(m_doc->undoStack()->index() - 1);
}

void KisUndoAdapter::addCommand(QUndoCommand *command)
{
    m_doc->addCommand(command);
    foreach(KisCommandHistoryListener*  l, m_undoListeners) {
        l->notifyCommandAdded(command);
    }
}

void KisUndoAdapter::undoLastCommand()
{
    /**
     * FIXME: Added as a workaround for being able to cancel creation
     * of the new adjustment mask (or any other mask whose
     * creation can be cancelled).
     *
     * Ideally, we should use "addToIndex-commit" technique like git does.
     * When a user presses Create button, we call command->redo()
     * and save this command in a cache. When the user confirms creation
     * of the layer with "OK" button, we "commit" the command to the undoStack.
     * If the user changes his mind and presses Cancel, we just call
     * command->undo() and remove the cache without committing it
     * to the undoStack
     */
    m_doc->undoStack()->undo();
}

void KisUndoAdapter::setUndo(bool undo)
{
    m_undo = undo;
}

bool KisUndoAdapter::undo() const
{
    return m_undo;
}

void KisUndoAdapter::beginMacro(const QString& macroName)
{
    m_doc->beginMacro(macroName);
}

void KisUndoAdapter::endMacro()
{
    m_doc->endMacro();
}

void KisUndoAdapter::emitSelectionChanged()
{
    emit selectionChanged();
}

#include "kis_undo_adapter.moc"



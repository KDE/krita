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
#include "kis_image.h"


KisUndoAdapter::KisUndoAdapter()
{
}

KisUndoAdapter::~KisUndoAdapter()
{
}

void KisUndoAdapter::setCommandHistoryListener(KisCommandHistoryListener *listener)
{
    if (!m_undoListeners.contains(listener)) {
        m_undoListeners.append(listener);
    }
}
void KisUndoAdapter::removeCommandHistoryListener(KisCommandHistoryListener *listener)
{
    int index = m_undoListeners.indexOf(listener);
    if (index != -1) {
        m_undoListeners.remove(index);
    }
}

void KisUndoAdapter::notifyCommandAdded(const KUndo2Command *command)
{
    if (!command) {
        kWarning() << "Empty command!";
        return;
    }
    foreach(KisCommandHistoryListener*  l, m_undoListeners) {
        l->notifyCommandAdded(command);
    }
}

void KisUndoAdapter::notifyCommandExecuted(const KUndo2Command *command)
{
    if (!command) {
        kWarning() << "Empty command!";
        return;
    }
    foreach(KisCommandHistoryListener*  l, m_undoListeners) {
        l->notifyCommandExecuted(command);
    }
}

void KisUndoAdapter::emitSelectionChanged()
{
    emit selectionChanged();
}

void KisUndoAdapter::setImage(KisImageWSP image)
{
    m_image = image;
}

KisImageWSP KisUndoAdapter::image()
{
    return m_image;
}

void KisUndoAdapter::addCommand(KUndo2Command *cmd)
{
    addCommand(KUndo2CommandSP(cmd));
}

#include "kis_undo_adapter.moc"



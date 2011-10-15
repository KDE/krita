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

#include "kis_undo_store.h"

#include "kis_debug.h"


KisUndoStore::KisUndoStore()
{
}

KisUndoStore::~KisUndoStore()
{
}

void KisUndoStore::setCommandHistoryListener(KisCommandHistoryListener *listener)
{
    if (!m_undoListeners.contains(listener)) {
        m_undoListeners.append(listener);
    }
}
void KisUndoStore::removeCommandHistoryListener(KisCommandHistoryListener *listener)
{
    int index = m_undoListeners.indexOf(listener);
    if (index != -1) {
        m_undoListeners.remove(index);
    }
}

void KisUndoStore::notifyCommandAdded(const KUndo2Command *command)
{
    if (!command) {
        kWarning() << "KisUndoStore::notifyCommandAdded(): empty command!";
        return;
    }
    foreach(KisCommandHistoryListener*  l, m_undoListeners) {
        l->notifyCommandAdded(command);
    }
}

void KisUndoStore::notifyCommandExecuted(const KUndo2Command *command)
{
    if (!command) {
        kWarning() << "KisUndoStore::notifyCommandExecuted(): empty command!";
        return;
    }
    foreach(KisCommandHistoryListener*  l, m_undoListeners) {
        l->notifyCommandExecuted(command);
    }
}

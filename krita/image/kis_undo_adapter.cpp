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


KisUndoAdapter::KisUndoAdapter(KisUndoStore *undoStore)
    : m_undoStore(undoStore)
{
}

KisUndoAdapter::~KisUndoAdapter()
{
}

void KisUndoAdapter::setCommandHistoryListener(KisCommandHistoryListener *listener)
{
    m_undoStore->setCommandHistoryListener(listener);
}
void KisUndoAdapter::removeCommandHistoryListener(KisCommandHistoryListener *listener)
{
    m_undoStore->removeCommandHistoryListener(listener);
}

void KisUndoAdapter::emitSelectionChanged()
{
    emit selectionChanged();
}

#include "kis_undo_adapter.moc"



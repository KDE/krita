/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2008 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_selection_transaction_data.h"

#include "kis_selection.h"
#include "kis_pixel_selection.h"
#include "kis_undo_adapter.h"

KisSelectionTransactionData::KisSelectionTransactionData(const QString& name, KisUndoAdapter *undoAdapter, KisSelectionSP selection, KUndo2Command* parent) :
        KisTransactionData(name, selection->getOrCreatePixelSelection().data(), parent)
        , m_undoAdapter(undoAdapter)
        , m_selection(selection)
{
}

KisSelectionTransactionData::~KisSelectionTransactionData()
{
}

void KisSelectionTransactionData::redo()
{
    KisTransactionData::redo();
    m_selection->updateProjection();

    m_undoAdapter->emitSelectionChanged();
}

void KisSelectionTransactionData::undo()
{
    KisTransactionData::undo();
    m_selection->updateProjection();

    m_undoAdapter->emitSelectionChanged();
}

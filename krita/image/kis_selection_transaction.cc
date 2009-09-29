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

#include "kis_selection_transaction.h"

#include "kis_types.h"
#include "kis_selection.h"
#include "kis_pixel_selection.h"
#include "kis_image.h"
#include "kis_undo_adapter.h"

KisSelectionTransaction::KisSelectionTransaction(const QString& name, KisImageWSP image, KisSelectionSP selection, QUndoCommand* parent) :
        KisTransaction(name, selection->getOrCreatePixelSelection().data(), parent)
        , m_image(image)
        , m_selection(selection)
        , m_wasDeselected(selection->isDeselected())
{
    if (m_selection->isDeselected()) {
        m_selection->getOrCreatePixelSelection()->clear();
        m_selection->setDeselected(false);
    }
}

KisSelectionTransaction::~KisSelectionTransaction()
{
}

void KisSelectionTransaction::redo()
{
    KisTransaction::redo();
    m_selection->setDirty(m_image->bounds());
    m_image->undoAdapter()->emitSelectionChanged();
}

void KisSelectionTransaction::undo()
{
    KisTransaction::undo();
    m_selection->setDirty(m_image->bounds());
    m_selection->setDeselected(m_wasDeselected);
    m_image->undoAdapter()->emitSelectionChanged();
}

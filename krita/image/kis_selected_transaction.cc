/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#include "kis_selected_transaction.h"
#include "kis_types.h"
#include "kis_global.h"

#include "kis_selection.h"
#include "kis_pixel_selection.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_undo_adapter.h"

KisSelectedTransaction::KisSelectedTransaction(const QString& name, KisNodeSP node, QUndoCommand* parent)
        : KisTransaction(name, node->paintDevice(), parent)
        , m_selTransaction(0)
        , m_hadSelection(false /*device->hasSelection()*/)
{
    m_layer = dynamic_cast<KisLayer*>(node.data());
    while (!m_layer && node->parent()) {
        m_layer = dynamic_cast<KisLayer*>(node->parent().data());
        node = node->parent();
    }

    if (m_layer->selection())
        m_selTransaction = new KisTransaction(name, KisPaintDeviceSP(m_layer->selection()->getOrCreatePixelSelection().data()));
//     if(! m_hadSelection) {
//         m_device->deselect(); // let us not be the cause of select
//     }
}

KisSelectedTransaction::~KisSelectedTransaction()
{
    delete m_selTransaction;
}

void KisSelectedTransaction::redo()
{
    // Both transactions will block on the first redo
    KisTransaction::redo();

    if (m_selTransaction)
        m_selTransaction->redo();
//     if(m_redoHasSelection)
//         m_device->selection();
//     else
//         m_device->deselect();

    m_layer->setDirty(m_layer->image()->bounds());
    m_layer->image()->undoAdapter()->emitSelectionChanged();
}

void KisSelectedTransaction::undo()
{
//     m_redoHasSelection = m_device->hasSelection();

    KisTransaction::undo();

    if (m_selTransaction)
        m_selTransaction->undo();
//     if(m_hadSelection)
//         m_device->selection();
//     else
//         m_device->deselect();
//
    m_layer->setDirty(m_layer->image()->bounds());
    m_layer->image()->undoAdapter()->emitSelectionChanged();
}

void KisSelectedTransaction::undoNoUpdate()
{
//     m_redoHasSelection = m_device->hasSelection();
//
//     KisTransaction::undoNoUpdate();
//     m_selTransaction->undoNoUpdate();
//     if(m_hadSelection)
//         m_device->selection();
//     else
//         m_device->deselect();
}

KisLayerSP KisSelectedTransaction::layer()
{
    return m_layer;
}

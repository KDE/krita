/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
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
#include "kis_mask_manager.h"

#include <klocale.h>
#include <kstdaction.h>
#include <kaction.h>
#include <ktoggleaction.h>
#include <kcommand.h>

#include <kis_undo_adapter.h>
#include <kis_paint_layer.h>

#include "kis_view2.h"

KisMaskManager::KisMaskManager( KisView2 * view)
    : m_view( view )
    , m_createMask( 0 )
    , m_maskFromSelection( 0 )
    , m_maskToSelection( 0 )
    , m_applyMask( 0 )
    , m_removeMask( 0 )
    , m_editMask( 0 )
    , m_showMask( 0 )
{
}

void KisMaskManager::setup( KActionCollection * actionCollection )
{
    m_createMask = new KAction(i18n("Create Mask"), KShortcut(), this,
                               SLOT(slotCreateMask()), actionCollection, "create_mask");
    m_maskFromSelection = new KAction(i18n("Mask From Selection"), KShortcut(), this,
                                      SLOT(slotMaskFromSelection()), actionCollection,
                                      "mask_fromsel");
    m_maskToSelection = new KAction(i18n("Mask To Selection"), KShortcut(), this,
                                    SLOT(slotMaskToSelection()), actionCollection, "mask_tosel");
    m_applyMask = new KAction(i18n("Apply Mask"), KShortcut(), this, SLOT(slotApplyMask()),
                              actionCollection, "apply_mask");
    m_removeMask = new KAction(i18n("Remove Mask"), KShortcut(), this,
                               SLOT(slotRemoveMask()), actionCollection, "remove_mask");
    m_editMask = new KToggleAction(i18n( "Edit Mask" ), KShortcut(), this,
                                   SLOT(slotEditMask()), actionCollection, "edit_mask");
    m_showMask = new KToggleAction(i18n( "Show Mask" ), KShortcut(), this,
                                   SLOT(slotShowMask()), actionCollection, "show_mask");


}

void KisMaskManager::updateGUI()
{

}



void KisMaskManager::slotCreateMask() {
    KisPaintLayer* layer = dynamic_cast<KisPaintLayer*>(m_view->image()->activeLayer().data());
    if (!layer)
        return;
    KNamedCommand *cmd = layer->createMaskCommand();
    cmd->execute();
    if (m_view->undoAdapter() && m_view->undoAdapter()->undo()) {
        m_view->undoAdapter()->addCommand(cmd);
    }
}
void KisMaskManager::slotMaskFromSelection() {
    KisPaintLayer* layer = dynamic_cast<KisPaintLayer*>(m_view->image()->activeLayer().data());
    if (!layer)
        return;

    KNamedCommand *cmd = layer->maskFromSelectionCommand();
    cmd->execute();
    if (m_view->undoAdapter() && m_view->undoAdapter()->undo()) {
        m_view->undoAdapter()->addCommand(cmd);
    }
}


void KisMaskManager::slotMaskToSelection() {
    KisPaintLayer* layer = dynamic_cast<KisPaintLayer*>(m_view->image()->activeLayer().data());
    if (!layer)
        return;

    KNamedCommand *cmd = layer->maskToSelectionCommand();
    cmd->execute();
    if (m_view->undoAdapter() && m_view->undoAdapter()->undo()) {
        m_view->undoAdapter()->addCommand(cmd);
    }
}

void KisMaskManager::slotApplyMask() {
    KisPaintLayer* layer = dynamic_cast<KisPaintLayer*>(m_view->image()->activeLayer().data());
    if (!layer)
        return;

    KNamedCommand *cmd = layer->applyMaskCommand();
    cmd->execute();
    if (m_view->undoAdapter() && m_view->undoAdapter()->undo()) {
        m_view->undoAdapter()->addCommand(cmd);
    }
}

void KisMaskManager::slotRemoveMask() {
    KisPaintLayer* layer = dynamic_cast<KisPaintLayer*>(m_view->image()->activeLayer().data());
    if (!layer)
        return;

    KNamedCommand *cmd = layer->removeMaskCommand();
    cmd->execute();
    if (m_view->undoAdapter() && m_view->undoAdapter()->undo()) {
        m_view->undoAdapter()->addCommand(cmd);
    }
}

void KisMaskManager::slotEditMask() {
    KisPaintLayer* layer = dynamic_cast<KisPaintLayer*>(m_view->image()->activeLayer().data());
    if (!layer)
        return;

    layer->setEditMask(m_editMask->isChecked());
}

void KisMaskManager::slotShowMask() {
    KisPaintLayer* layer = dynamic_cast<KisPaintLayer*>(m_view->image()->activeLayer().data());
    if (!layer)
        return;

    layer->setRenderMask(m_showMask->isChecked());
}

void KisMaskManager::maskUpdated() {
    KisPaintLayer* layer = dynamic_cast<KisPaintLayer*>(m_view->image()->activeLayer().data());
    if (!layer) {
        m_createMask->setEnabled(false);
        m_applyMask->setEnabled(false);
        m_removeMask->setEnabled(false);
        m_editMask->setEnabled(false);
        m_showMask->setEnabled(false);
        return;
    }
    m_createMask->setEnabled(!layer->hasMask());
    m_maskFromSelection->setEnabled(true); // Perhaps also update this to false when no selection?
    m_maskToSelection->setEnabled(layer->hasMask());
    m_applyMask->setEnabled(layer->hasMask());
    m_removeMask->setEnabled(layer->hasMask());

    m_editMask->setEnabled(layer->hasMask());
    m_editMask->setChecked(layer->editMask());
    m_showMask->setEnabled(layer->hasMask());
    m_showMask->setChecked(layer->renderMask());
}

#include "kis_mask_manager.moc"

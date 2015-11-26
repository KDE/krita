/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include <klocalizedstring.h>
#include "kis_node.h"
#include "kis_layer.h"
#include "kis_image.h"
#include "kis_selection_mask.h"
#include "kis_paint_layer.h"
#include "commands/kis_node_property_list_command.h"
#include "kis_undo_adapter.h"



KisNodePropertyListCommand::KisNodePropertyListCommand(KisNodeSP node, KisNodeModel::PropertyList newPropertyList)
    : KisNodeCommand(kundo2_i18n("Property Changes"), node),
      m_newPropertyList(newPropertyList),
      m_oldPropertyList(node->sectionModelProperties())
/**
 * TODO instead of "Property Changes" check which property
 * has been changed and display either lock/unlock, visible/hidden
 * or "Property Changes" (this require new strings)
 */
{
}

void KisNodePropertyListCommand::redo()
{
    m_node->setSectionModelProperties(m_newPropertyList);
    doUpdate(m_oldPropertyList, m_newPropertyList);
}

void KisNodePropertyListCommand::undo()
{
    m_node->setSectionModelProperties(m_oldPropertyList);
    doUpdate(m_newPropertyList, m_oldPropertyList);
}

void KisNodePropertyListCommand::doUpdate(const KisNodeModel::PropertyList &oldPropertyList,
                                          const KisNodeModel::PropertyList &newPropertyList)
{
    bool oldPassThroughValue = false;
    bool newPassThroughValue = false;

    Q_FOREACH (const KisNodeModel::Property &prop, oldPropertyList) {
        if (prop.name == i18n("Pass Through")) {
            oldPassThroughValue = prop.state.toBool();
        }
    }

    Q_FOREACH (const KisNodeModel::Property &prop, newPropertyList) {
        if (prop.name == i18n("Pass Through")) {
            newPassThroughValue = prop.state.toBool();
        }
    }

    if (oldPassThroughValue && !newPassThroughValue) {
        KisLayer *layer = qobject_cast<KisLayer*>(m_node.data());
        layer->image()->refreshGraphAsync(layer);
    } else if (m_node->parent() && !oldPassThroughValue && newPassThroughValue) {
        KisLayer *layer = qobject_cast<KisLayer*>(m_node->parent().data());
        layer->image()->refreshGraphAsync(layer);
    } else {
        m_node->setDirty(); // TODO check if visibility was changed or not
    }
}

void KisNodePropertyListCommand::setNodePropertiesNoUndo(KisNodeSP node, KisImageSP image, PropertyList proplist)
{
    bool undo = true;
    Q_FOREACH (const KisNodeModel::Property &prop, proplist) {
        if (prop.name == i18n("Visible") && node->visible() != prop.state.toBool()) undo = false;
        if (prop.name == i18n("Locked") && node->userLocked() != prop.state.toBool()) undo = false;
        if (prop.name == i18n("Active")) {
            if (KisSelectionMask *m = dynamic_cast<KisSelectionMask*>(node.data())) {
                if (m->active() != prop.state.toBool()) {
                    undo = false;
                }
            }
        }
        if (prop.name == i18n("Alpha Locked")) {
            if (KisPaintLayer* l = dynamic_cast<KisPaintLayer*>(node.data())) {
                if (l->alphaLocked() != prop.state.toBool()) {
                    undo = false;
                }
            }
        }
    }

    QScopedPointer<KUndo2Command> cmd(new KisNodePropertyListCommand(node, proplist));

    if (undo) {
        image->undoAdapter()->addCommand(cmd.take());
    }
    else {
        image->setModified();
        cmd->redo();
    }
}

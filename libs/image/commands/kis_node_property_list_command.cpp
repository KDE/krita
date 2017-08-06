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
#include "kis_layer_properties_icons.h"

// HACK! please refactor out!
#include "kis_simple_stroke_strategy.h"


KisNodePropertyListCommand::KisNodePropertyListCommand(KisNodeSP node, KisBaseNode::PropertyList newPropertyList)
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

bool checkOnionSkinChanged(const KisBaseNode::PropertyList &oldPropertyList,
                           const KisBaseNode::PropertyList &newPropertyList)
{
    if (oldPropertyList.size() != newPropertyList.size()) return false;

    bool oldOnionSkinsValue = false;
    bool newOnionSkinsValue = false;

    Q_FOREACH (const KisBaseNode::Property &prop, oldPropertyList) {
        if (prop.id == KisLayerPropertiesIcons::onionSkins.id()) {
            oldOnionSkinsValue = prop.state.toBool();
        }
    }

    Q_FOREACH (const KisBaseNode::Property &prop, newPropertyList) {
        if (prop.id == KisLayerPropertiesIcons::onionSkins.id()) {
            newOnionSkinsValue = prop.state.toBool();
        }
    }

    return oldOnionSkinsValue != newOnionSkinsValue;
}


void KisNodePropertyListCommand::doUpdate(const KisBaseNode::PropertyList &oldPropertyList,
                                          const KisBaseNode::PropertyList &newPropertyList)
{
    bool oldPassThroughValue = false;
    bool newPassThroughValue = false;

    Q_FOREACH (const KisBaseNode::Property &prop, oldPropertyList) {
        if (prop.name == i18n("Pass Through")) {
            oldPassThroughValue = prop.state.toBool();
        }
    }

    Q_FOREACH (const KisBaseNode::Property &prop, newPropertyList) {
        if (prop.name == i18n("Pass Through")) {
            newPassThroughValue = prop.state.toBool();
        }
    }

    if (oldPassThroughValue && !newPassThroughValue) {
        KisLayerSP layer(qobject_cast<KisLayer*>(m_node.data()));
        KisImageSP image = layer->image().toStrongRef();
        if (image) {
            image->refreshGraphAsync(layer);
        }
    } else if (m_node->parent() && !oldPassThroughValue && newPassThroughValue) {
        KisLayerSP layer(qobject_cast<KisLayer*>(m_node->parent().data()));
        KisImageSP image = layer->image().toStrongRef();
        if (image) {
            image->refreshGraphAsync(layer);
        }
    } else if (checkOnionSkinChanged(oldPropertyList, newPropertyList)) {
        m_node->setDirtyDontResetAnimationCache();
    } else {
        m_node->setDirty(); // TODO check if visibility was changed or not
    }
}

void KisNodePropertyListCommand::setNodePropertiesNoUndo(KisNodeSP node, KisImageSP image, PropertyList proplist)
{
    bool undo = true;
    Q_FOREACH (const KisBaseNode::Property &prop, proplist) {
        if (prop.isInStasis) undo = false;
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

        /**
         * HACK ALERT!
         *
         * Here we start a fake legacy stroke, so that all the LoD planes would
         * be invalidated. Ideally, we should refactor this method and avoid
         * resetting LoD planes when node visibility changes, Instead there should
         * be two commands executes: LoD agnostic one (which sets the properties
         * themselves), and two LoD-specific update commands: one for lodN and
         * another one for lod0.
         */
        KisStrokeId strokeId = image->startStroke(new KisSimpleStrokeStrategy());
        image->endStroke(strokeId);
    }
}

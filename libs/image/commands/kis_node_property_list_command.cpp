/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include "kis_command_ids.h"

// HACK! please refactor out!
#include "kis_simple_stroke_strategy.h"
#include "kis_abstract_projection_plane.h"

namespace {

QSet<QString> changedProperties(const KisBaseNode::PropertyList &before,
                                const KisBaseNode::PropertyList &after)
{
    QSet<QString> changedIds;

    auto valueForId = [] (const QString &id, const KisBaseNode::PropertyList &list) {
        QVariant value;
        Q_FOREACH (const KisBaseNode::Property &prop, list) {
            if (prop.id == id) {
                value = prop.state;
                break;
            }

        }
        return value;
    };

    /// we expect that neither of the lists has duplicated values,
    /// therefore we can just iterate over teh bigger list
    const KisBaseNode::PropertyList &list1 = before.size() >= after.size() ? before : after;
    const KisBaseNode::PropertyList &list2 = before.size() >= after.size() ? after : before;

    Q_FOREACH (const KisBaseNode::Property &prop, list1) {
        if (prop.state != valueForId(prop.id, list2)) {
            changedIds.insert(prop.id);
        }
    }

    return changedIds;
}

}


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
    const KisBaseNode::PropertyList propsBefore = m_node->sectionModelProperties();
    const QRect oldExtent = m_node->projectionPlane()->tightUserVisibleBounds();
    m_node->setSectionModelProperties(m_newPropertyList);
    doUpdate(propsBefore, m_node->sectionModelProperties(), oldExtent | m_node->projectionPlane()->tightUserVisibleBounds());
}

void KisNodePropertyListCommand::undo()
{
    const KisBaseNode::PropertyList propsBefore = m_node->sectionModelProperties();
    const QRect oldExtent = m_node->projectionPlane()->tightUserVisibleBounds();
    m_node->setSectionModelProperties(m_oldPropertyList);
    doUpdate(propsBefore, m_node->sectionModelProperties(), oldExtent | m_node->projectionPlane()->tightUserVisibleBounds());
}

int KisNodePropertyListCommand::id() const
{
    return KisCommandUtils::NodePropertyListCommandId;
}

bool KisNodePropertyListCommand::mergeWith(const KUndo2Command *command)
{
    const KisNodePropertyListCommand *other =
        dynamic_cast<const KisNodePropertyListCommand*>(command);

    if (other && other->m_node == m_node &&
        (changedProperties(m_oldPropertyList, m_newPropertyList).isEmpty() ||
         changedProperties(m_oldPropertyList, m_newPropertyList) ==
             changedProperties(other->m_oldPropertyList, other->m_newPropertyList))) {

        KIS_SAFE_ASSERT_RECOVER_NOOP(m_newPropertyList == other->m_oldPropertyList);
        m_newPropertyList = other->m_newPropertyList;
        return true;
    }

    return false;
}

bool KisNodePropertyListCommand::canMergeWith(const KUndo2Command *command) const
{
    const KisNodePropertyListCommand *other =
        dynamic_cast<const KisNodePropertyListCommand*>(command);

    return other && other->m_node == m_node &&
        (changedProperties(m_oldPropertyList, m_newPropertyList).isEmpty() ||
         changedProperties(m_oldPropertyList, m_newPropertyList) ==
             changedProperties(other->m_oldPropertyList, other->m_newPropertyList));
}

bool checkOnionSkinChanged(const KisBaseNode::PropertyList &oldPropertyList,
                           const KisBaseNode::PropertyList &newPropertyList)
{
    return changedProperties(oldPropertyList, newPropertyList).contains(KisLayerPropertiesIcons::onionSkins.id());
}


void KisNodePropertyListCommand::doUpdate(const KisBaseNode::PropertyList &oldPropertyList,
                                          const KisBaseNode::PropertyList &newPropertyList,
                                          const QRect &totalUpdateExtent)
{
    /**
     * Sometimes the node might refuse to change the property, e.g. needs-update for colorize
     * mask. In this case we should avoid issuing the update and set-modified call.
     */
    if (oldPropertyList == newPropertyList) {
        return;
    }

    bool oldPassThroughValue = false;
    bool newPassThroughValue = false;

    bool oldVisibilityValue = false;
    bool newVisibilityValue = false;

    Q_FOREACH (const KisBaseNode::Property &prop, oldPropertyList) {
        if (prop.id == KisLayerPropertiesIcons::passThrough.id()) {
            oldPassThroughValue = prop.state.toBool();
        }
        if (prop.id == KisLayerPropertiesIcons::visible.id()) {
            oldVisibilityValue = prop.state.toBool();
        }
    }

    Q_FOREACH (const KisBaseNode::Property &prop, newPropertyList) {
        if (prop.id == KisLayerPropertiesIcons::passThrough.id()) {
            newPassThroughValue = prop.state.toBool();
        }
        if (prop.id == KisLayerPropertiesIcons::visible.id()) {
            newVisibilityValue = prop.state.toBool();
        }
    }

    if (oldPassThroughValue && !newPassThroughValue) {
        KisLayerSP layer(qobject_cast<KisLayer*>(m_node.data()));
        KisImageSP image = layer->image().toStrongRef();
        if (image) {
            image->refreshGraphAsync(layer);
        }
    } else if ((m_node->parent() && !oldPassThroughValue && newPassThroughValue) ||
               (oldPassThroughValue && newPassThroughValue &&
                !oldVisibilityValue && newVisibilityValue)) {

        KisLayerSP layer(qobject_cast<KisLayer*>(m_node->parent().data()));
        KisImageSP image = layer->image().toStrongRef();
        if (image) {
            image->refreshGraphAsync(layer);
        }
    } else if (checkOnionSkinChanged(oldPropertyList, newPropertyList)) {
        m_node->setDirtyDontResetAnimationCache(totalUpdateExtent);
    } else {
        m_node->setDirty(totalUpdateExtent); // TODO check if visibility was actually changed or not
    }
}

void KisNodePropertyListCommand::setNodePropertiesAutoUndo(KisNodeSP node, KisImageSP image, PropertyList proplist)
{
    const bool undo = true;

    QScopedPointer<KUndo2Command> cmd(new KisNodePropertyListCommand(node, proplist));

    image->setModified();

    if (undo) {
        image->undoAdapter()->addCommand(cmd.take());
    }
    else {
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

        struct SimpleLodResettingStroke : public KisSimpleStrokeStrategy {
            SimpleLodResettingStroke(KUndo2Command *cmd, KisImageSP image)
                : KisSimpleStrokeStrategy(QLatin1String("SimpleLodResettingStroke")),
                  m_cmd(cmd),
                  m_image(image)
            {
                setClearsRedoOnStart(false);
                this->enableJob(JOB_INIT, true);
            }

            void initStrokeCallback() override {
                m_cmd->redo();
                m_image->setModifiedWithoutUndo();
            }

        private:
            QScopedPointer<KUndo2Command> m_cmd;
            KisImageSP m_image;
        };

        KisStrokeId strokeId = image->startStroke(new SimpleLodResettingStroke(cmd.take(), image));
        image->endStroke(strokeId);
    }

}

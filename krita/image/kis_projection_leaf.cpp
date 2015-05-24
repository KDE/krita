/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_projection_leaf.h"

#include <KoColorSpace.h>

#include "kis_layer.h"
#include "kis_mask.h"
#include "kis_group_layer.h"
#include "kis_adjustment_layer.h"

#include "krita_utils.h"


struct KisProjectionLeaf::Private
{
    Private(KisNode *_node) : node(_node) {}

    KisNode* node;

    static bool checkPassThrough(const KisNode *node) {
        const KisGroupLayer *group = qobject_cast<const KisGroupLayer*>(node);
        return group && group->passThroughMode();
    }

    bool checkParentPassThrough() {
        return node->parent() && checkPassThrough(node->parent());
    }

    bool checkThisPassThrough() {
        return checkPassThrough(node);
    }
};

KisProjectionLeaf::KisProjectionLeaf(KisNode *node)
    : m_d(new Private(node))
{
}

KisProjectionLeaf::~KisProjectionLeaf()
{
}

KisProjectionLeafSP KisProjectionLeaf::parent() const
{
    KisNodeSP node = m_d->node->parent();

    if (node && Private::checkPassThrough(node)) {
        node = node->parent();
    }

    return node ? node->projectionLeaf() : KisProjectionLeafSP();
}


KisProjectionLeafSP KisProjectionLeaf::firstChild() const
{
    KisNodeSP node;

    if (!m_d->checkThisPassThrough()) {
        node = m_d->node->firstChild();
    }

    return node ? node->projectionLeaf() : KisProjectionLeafSP();
}

KisProjectionLeafSP KisProjectionLeaf::lastChild() const
{
    KisNodeSP node;

    if (!m_d->checkThisPassThrough()) {
        node = m_d->node->lastChild();
    }

    return node ? node->projectionLeaf() : KisProjectionLeafSP();
}

KisProjectionLeafSP KisProjectionLeaf::prevSibling() const
{
    KisNodeSP node;

    if (m_d->checkThisPassThrough()) {
        node = m_d->node->lastChild();
    }

    if (!node) {
        node = m_d->node->prevSibling();
    }

    if (!node && m_d->checkParentPassThrough()) {
        node = m_d->node->parent()->prevSibling();
    }

    return node ? node->projectionLeaf() : KisProjectionLeafSP();
}

KisProjectionLeafSP KisProjectionLeaf::nextSibling() const
{
    KisNodeSP node = m_d->node->nextSibling();

    if (node && Private::checkPassThrough(node) && node->firstChild()) {
        node = node->firstChild();
    }

    if (!node && m_d->checkParentPassThrough()) {
        node = m_d->node->parent();
    }

    return node ? node->projectionLeaf() : KisProjectionLeafSP();
}

bool KisProjectionLeaf::hasChildren() const
{
    return m_d->node->firstChild();
}

KisNodeSP KisProjectionLeaf::node() const
{
    return m_d->node;
}

KisAbstractProjectionPlaneSP KisProjectionLeaf::projectionPlane() const
{
    return m_d->node->projectionPlane();
}

bool KisProjectionLeaf::accept(KisNodeVisitor &visitor)
{
    return m_d->node->accept(visitor);
}

KisPaintDeviceSP KisProjectionLeaf::original()
{
    return m_d->node->original();
}

KisPaintDeviceSP KisProjectionLeaf::projection()
{
    return m_d->node->projection();
}

bool KisProjectionLeaf::isRoot() const
{
    return (bool)!m_d->node->parent();
}

bool KisProjectionLeaf::isLayer() const
{
    return (bool)qobject_cast<const KisLayer*>(m_d->node);
}

bool KisProjectionLeaf::isMask() const
{
    return (bool)qobject_cast<const KisMask*>(m_d->node);
}

bool KisProjectionLeaf::canHaveChildLayers() const
{
    return (bool)qobject_cast<const KisGroupLayer*>(m_d->node);
}

bool KisProjectionLeaf::dependsOnLowerNodes() const
{
    return (bool)qobject_cast<const KisAdjustmentLayer*>(m_d->node);
}

bool KisProjectionLeaf::visible() const
{
    // TODO: check opacity as well!

    bool hiddenByParentPassThrough =
        m_d->checkParentPassThrough() && !m_d->node->parent()->visible();

    return m_d->node->visible(false) &&
        !m_d->checkThisPassThrough() &&
        !hiddenByParentPassThrough;
}

quint8 KisProjectionLeaf::opacity() const
{
    quint8 resultOpacity = m_d->node->opacity();

    if (m_d->checkParentPassThrough()) {
        quint8 parentOpacity = m_d->node->parent()->projectionLeaf()->opacity();

        resultOpacity = KritaUtils::mergeOpacity(resultOpacity, parentOpacity);
    }

    return resultOpacity;
}

QBitArray KisProjectionLeaf::channelFlags() const
{
    QBitArray channelFlags;

    KisLayer *layer = qobject_cast<KisLayer*>(m_d->node);
    if (!layer) return channelFlags;

    channelFlags = layer->channelFlags();

    if (m_d->checkParentPassThrough()) {
        QBitArray parentChannelFlags;

        if (*m_d->node->colorSpace() ==
            *m_d->node->parent()->colorSpace()) {

            KisLayer *parentLayer = qobject_cast<KisLayer*>(m_d->node->parent().data());
            parentChannelFlags = parentLayer->channelFlags();
        }

        channelFlags = KritaUtils::mergeChannelFlags(channelFlags, parentChannelFlags);
    }

    return channelFlags;
}

bool KisProjectionLeaf::isStillInGraph() const
{
    return (bool)m_d->node->graphListener();
}

bool KisProjectionLeaf::isDroppedMask() const
{
    return qobject_cast<KisMask*>(m_d->node) &&
        m_d->checkParentPassThrough();
}

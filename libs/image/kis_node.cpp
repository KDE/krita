/*
 * Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_node.h"

#include <QList>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>
#include <QPainterPath>
#include <QRect>

#include <KoProperties.h>

#include "kis_global.h"
#include "kis_node_graph_listener.h"
#include "kis_node_visitor.h"
#include "kis_processing_visitor.h"
#include "kis_node_progress_proxy.h"
#include "kis_busy_progress_indicator.h"

#include "kis_clone_layer.h"

#include "kis_safe_read_list.h"
typedef KisSafeReadList<KisNodeSP> KisSafeReadNodeList;

#include "kis_abstract_projection_plane.h"
#include "kis_projection_leaf.h"
#include "kis_undo_adapter.h"

#include "kis_keyframe_channel.h"
#include "kis_time_range.h"

/**
 *The link between KisProjection ans KisImageUpdater
 *uses queued signals with an argument of KisNodeSP type,
 *so we should register it beforehand
 */
struct KisNodeSPStaticRegistrar {
    KisNodeSPStaticRegistrar() {
        qRegisterMetaType<KisNodeSP>("KisNodeSP");
    }
};
static KisNodeSPStaticRegistrar __registrar1;

struct KisNodeListStaticRegistrar {
    KisNodeListStaticRegistrar() {
        qRegisterMetaType<KisNodeList>("KisNodeList");
    }
};
static KisNodeListStaticRegistrar __registrar2;


/**
 * Note about "thread safety" of KisNode
 *
 * 1) One can *read* any information about node and node graph in any
 *    number of threads concurrently. This operation is safe because
 *    of the usage of KisSafeReadNodeList and will run concurrently
 *    (lock-free).
 *
 * 2) One can *write* any information into the node or node graph in a
 *    single thread only! Changing the graph concurrently is *not*
 *    sane and therefore not supported.
 *
 * 3) One can *read and write* information about the node graph
 *    concurrently, given that there is only *one* writer thread and
 *    any number of reader threads. Please note that in this case the
 *    node's code is just guaranteed *not to crash*, which is ensured
 *    by nodeSubgraphLock. You need to ensure the sanity of the data
 *    read by the reader threads yourself!
 */

struct Q_DECL_HIDDEN KisNode::Private
{
public:
    Private(KisNode *node)
            : graphListener(0)
            , nodeProgressProxy(0)
            , busyProgressIndicator(0)
            , animated(false)
            , useInTimeline(false)
            , projectionLeaf(new KisProjectionLeaf(node))
    {
    }

    KisNodeWSP parent;
    KisNodeGraphListener *graphListener;
    KisSafeReadNodeList nodes;
    KisNodeProgressProxy *nodeProgressProxy;
    KisBusyProgressIndicator *busyProgressIndicator;
    QReadWriteLock nodeSubgraphLock;
    QMap<QString, KisKeyframeChannel*> keyframeChannels;
    bool animated;
    bool useInTimeline;

    KisProjectionLeafSP projectionLeaf;

    const KisNode* findSymmetricClone(const KisNode *srcRoot,
                                      const KisNode *dstRoot,
                                      const KisNode *srcTarget);
    void processDuplicatedClones(const KisNode *srcDuplicationRoot,
                                 const KisNode *dstDuplicationRoot,
                                 KisNode *node);
};

/**
 * Finds the layer in \p dstRoot subtree, which has the same path as
 * \p srcTarget has in \p srcRoot
 */
const KisNode* KisNode::Private::findSymmetricClone(const KisNode *srcRoot,
                                                    const KisNode *dstRoot,
                                                    const KisNode *srcTarget)
{
    if (srcRoot == srcTarget) return dstRoot;

    KisSafeReadNodeList::const_iterator srcIter = srcRoot->m_d->nodes.constBegin();
    KisSafeReadNodeList::const_iterator dstIter = dstRoot->m_d->nodes.constBegin();

    for (; srcIter != srcRoot->m_d->nodes.constEnd(); srcIter++, dstIter++) {

        KIS_ASSERT_RECOVER_RETURN_VALUE((srcIter != srcRoot->m_d->nodes.constEnd()) ==
                                        (dstIter != dstRoot->m_d->nodes.constEnd()), 0);

        const KisNode *node = findSymmetricClone(srcIter->data(), dstIter->data(), srcTarget);
        if (node) return node;

    }

    return 0;
}

/**
 * This function walks through a subtrees of old and new layers and
 * searches for clone layers. For each clone layer it checks whether
 * its copyFrom() lays inside the old subtree, and if it is so resets
 * it to the corresponding layer in the new subtree.
 *
 * That is needed when the user duplicates a group layer with all its
 * layer subtree. In such a case all the "internal" clones must stay
 * "internal" and not point to the layers of the older group.
 */
void KisNode::Private::processDuplicatedClones(const KisNode *srcDuplicationRoot,
                                               const KisNode *dstDuplicationRoot,
                                               KisNode *node)
{
    if (KisCloneLayer *clone = dynamic_cast<KisCloneLayer*>(node)) {
        KIS_ASSERT_RECOVER_RETURN(clone->copyFrom());
        const KisNode *newCopyFrom = findSymmetricClone(srcDuplicationRoot,
                                                        dstDuplicationRoot,
                                                        clone->copyFrom());

        if (newCopyFrom) {
            KisLayer *newCopyFromLayer = dynamic_cast<KisLayer*>(const_cast<KisNode*>(newCopyFrom));
            KIS_ASSERT_RECOVER_RETURN(newCopyFromLayer);

            clone->setCopyFrom(newCopyFromLayer);
        }
    }

    KisSafeReadNodeList::const_iterator iter;
    FOREACH_SAFE(iter, node->m_d->nodes) {
        KisNode *child = const_cast<KisNode*>((*iter).data());
        processDuplicatedClones(srcDuplicationRoot, dstDuplicationRoot, child);
    }
}

KisNode::KisNode()
        : m_d(new Private(this))
{
    m_d->parent = 0;
    m_d->graphListener = 0;
}

KisNode::KisNode(const KisNode & rhs)
        : KisBaseNode(rhs)
        , m_d(new Private(this))
{
    m_d->parent = 0;
    m_d->graphListener = 0;

    // NOTE: the nodes are not supposed to be added/removed while
    // creation of another node, so we do *no* locking here!

    KisSafeReadNodeList::const_iterator iter;
    FOREACH_SAFE(iter, rhs.m_d->nodes) {
        KisNodeSP child = (*iter)->clone();
        child->createNodeProgressProxy();
        m_d->nodes.append(child);
        child->setParent(this);
    }

    m_d->processDuplicatedClones(&rhs, this, this);
}

KisNode::~KisNode()
{
    if (m_d->busyProgressIndicator) {
        m_d->busyProgressIndicator->endUpdatesBeforeDestroying();
        m_d->busyProgressIndicator->deleteLater();
    }

    if (m_d->nodeProgressProxy) {
        m_d->nodeProgressProxy->deleteLater();
    }

    {
        QWriteLocker l(&m_d->nodeSubgraphLock);
        m_d->nodes.clear();
    }

    delete m_d;
}

QRect KisNode::needRect(const QRect &rect, PositionToFilthy pos) const
{
    Q_UNUSED(pos);
    return rect;
}

QRect KisNode::changeRect(const QRect &rect, PositionToFilthy pos) const
{
    Q_UNUSED(pos);
    return rect;
}

QRect KisNode::accessRect(const QRect &rect, PositionToFilthy pos) const
{
    Q_UNUSED(pos);
    return rect;
}

KisAbstractProjectionPlaneSP KisNode::projectionPlane() const
{
    KIS_ASSERT_RECOVER_NOOP(0 && "KisNode::projectionPlane() is not defined!");
    static KisAbstractProjectionPlaneSP plane =
        toQShared(new KisDumbProjectionPlane());

    return plane;
}

QList<KisKeyframeChannel*> KisNode::keyframeChannels() const
{
    return m_d->keyframeChannels.values();
}

KisKeyframeChannel * KisNode::getKeyframeChannel(const QString &id) const
{
    QMap<QString, KisKeyframeChannel*>::iterator i = m_d->keyframeChannels.find(id);
    if (i == m_d->keyframeChannels.end()) return 0;
    return i.value();
}

bool KisNode::isAnimated() const
{
    return m_d->animated;
}

void KisNode::enableAnimation()
{
    m_d->animated = true;
    baseNodeChangedCallback();
}

bool KisNode::useInTimeline() const
{
    return m_d->useInTimeline;
}

void KisNode::setUseInTimeline(bool value)
{
    if (value == m_d->useInTimeline) return;

    m_d->useInTimeline = value;
    baseNodeChangedCallback();
}

void KisNode::addKeyframeChannel(KisKeyframeChannel *channel)
{
    m_d->keyframeChannels.insert(channel->id(), channel);
}

KisProjectionLeafSP KisNode::projectionLeaf() const
{
    return m_d->projectionLeaf;
}

bool KisNode::accept(KisNodeVisitor &v)
{
    return v.visit(this);
}

void KisNode::accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter)
{
    return visitor.visit(this, undoAdapter);
}

int KisNode::graphSequenceNumber() const
{
    return m_d->graphListener ? m_d->graphListener->graphSequenceNumber() : -1;
}

KisNodeGraphListener *KisNode::graphListener() const
{
    return m_d->graphListener;
}

void KisNode::setGraphListener(KisNodeGraphListener *graphListener)
{
    m_d->graphListener = graphListener;

    QReadLocker l(&m_d->nodeSubgraphLock);
    KisSafeReadNodeList::const_iterator iter;
    FOREACH_SAFE(iter, m_d->nodes) {
        KisNodeSP child = (*iter);
        child->setGraphListener(graphListener);
    }
}

void KisNode::setParent(KisNodeWSP parent)
{
    QWriteLocker l(&m_d->nodeSubgraphLock);
    m_d->parent = parent;
}

KisNodeSP KisNode::parent() const
{
    QReadLocker l(&m_d->nodeSubgraphLock);
    return m_d->parent.isValid() ? KisNodeSP(m_d->parent) : 0;
}

KisBaseNodeSP KisNode::parentCallback() const
{
    return parent();
}

void KisNode::notifyParentVisibilityChanged(bool value)
{
    QReadLocker l(&m_d->nodeSubgraphLock);

    KisSafeReadNodeList::const_iterator iter;
    FOREACH_SAFE(iter, m_d->nodes) {
        KisNodeSP child = (*iter);
        child->notifyParentVisibilityChanged(value);
    }
}

void KisNode::baseNodeChangedCallback()
{
    if(m_d->graphListener) {
        m_d->graphListener->nodeChanged(this);
    }
}

void KisNode::baseNodeInvalidateAllFramesCallback()
{
    if(m_d->graphListener) {
        m_d->graphListener->invalidateAllFrames();
    }
}

KisNodeSP KisNode::firstChild() const
{
    QReadLocker l(&m_d->nodeSubgraphLock);
    return !m_d->nodes.isEmpty() ? m_d->nodes.first() : 0;
}

KisNodeSP KisNode::lastChild() const
{
    QReadLocker l(&m_d->nodeSubgraphLock);
    return !m_d->nodes.isEmpty() ? m_d->nodes.last() : 0;
}

KisNodeSP KisNode::prevChildImpl(KisNodeSP child)
{
    /**
     * Warning: mind locking policy!
     *
     * The graph locks must be *always* taken in descending
     * order. That is if you want to (or it implicitly happens that
     * you) take a lock of a parent and a chil, you must first take
     * the lock of a parent, and only after that ask a child to do the
     * same.  Otherwise you'll get a deadlock.
     */

    QReadLocker l(&m_d->nodeSubgraphLock);

    int i = m_d->nodes.indexOf(child) - 1;
    return i >= 0 ? m_d->nodes.at(i) : 0;
}

KisNodeSP KisNode::nextChildImpl(KisNodeSP child)
{
    /**
     * See a comment in KisNode::prevChildImpl()
     */
    QReadLocker l(&m_d->nodeSubgraphLock);

    int i = m_d->nodes.indexOf(child) + 1;
    return i > 0 && i < m_d->nodes.size() ? m_d->nodes.at(i) : 0;
}

KisNodeSP KisNode::prevSibling() const
{
    KisNodeSP parentNode = parent();
    return parentNode ? parentNode->prevChildImpl(const_cast<KisNode*>(this)) : 0;
}

KisNodeSP KisNode::nextSibling() const
{
    KisNodeSP parentNode = parent();
    return parentNode ? parentNode->nextChildImpl(const_cast<KisNode*>(this)) : 0;
}

quint32 KisNode::childCount() const
{
    QReadLocker l(&m_d->nodeSubgraphLock);
    return m_d->nodes.size();
}


KisNodeSP KisNode::at(quint32 index) const
{
    QReadLocker l(&m_d->nodeSubgraphLock);

    if (!m_d->nodes.isEmpty() && index < (quint32)m_d->nodes.size()) {
        return m_d->nodes.at(index);
    }

    return 0;
}

int KisNode::index(const KisNodeSP node) const
{
    QReadLocker l(&m_d->nodeSubgraphLock);

    return m_d->nodes.indexOf(node);
}

QList<KisNodeSP> KisNode::childNodes(const QStringList & nodeTypes, const KoProperties & properties) const
{
    QReadLocker l(&m_d->nodeSubgraphLock);

    QList<KisNodeSP> nodes;

    KisSafeReadNodeList::const_iterator iter;
    FOREACH_SAFE(iter, m_d->nodes) {
        if (*iter) {
            if (properties.isEmpty() || (*iter)->check(properties)) {
                bool rightType = true;

                if(!nodeTypes.isEmpty()) {
                    rightType = false;
                    Q_FOREACH (const QString &nodeType,  nodeTypes) {
                        if ((*iter)->inherits(nodeType.toLatin1())) {
                            rightType = true;
                            break;
                        }
                    }
                }
                if (rightType) {
                    nodes.append(*iter);
                }
            }
        }
    }
    return nodes;
}

bool KisNode::add(KisNodeSP newNode, KisNodeSP aboveThis)
{
    Q_ASSERT(newNode);

    if (!newNode) return false;
    if (aboveThis && aboveThis->parent().data() != this) return false;
    if (!allowAsChild(newNode)) return false;
    if (newNode->parent()) return false;
    if (index(newNode) >= 0) return false;

    int idx = aboveThis ? this->index(aboveThis) + 1 : 0;

    // threoretical race condition may happen here ('idx' may become
    // deprecated until the write lock will be held). But we ignore
    // it, because it is not supported to add/remove nodes from two
    // concurrent threads simultaneously

    if (m_d->graphListener) {
        m_d->graphListener->aboutToAddANode(this, idx);
    }

    {
        QWriteLocker l(&m_d->nodeSubgraphLock);

        newNode->createNodeProgressProxy();

        m_d->nodes.insert(idx, newNode);

        newNode->setParent(this);
        newNode->setGraphListener(m_d->graphListener);
    }

    if (m_d->graphListener) {
        m_d->graphListener->nodeHasBeenAdded(this, idx);
    }


    return true;
}

bool KisNode::remove(quint32 index)
{
    if (index < childCount()) {
        KisNodeSP removedNode = at(index);

        if (m_d->graphListener) {
            m_d->graphListener->aboutToRemoveANode(this, index);
        }

        {
            QWriteLocker l(&m_d->nodeSubgraphLock);

            removedNode->setGraphListener(0);

            removedNode->setParent(0);   // after calling aboutToRemoveANode or then the model get broken according to TT's modeltest

            m_d->nodes.removeAt(index);
        }

        if (m_d->graphListener) {
            m_d->graphListener->nodeHasBeenRemoved(this, index);
        }

        return true;
    }
    return false;
}

bool KisNode::remove(KisNodeSP node)
{
    return node->parent().data() == this ? remove(index(node)) : false;
}

KisNodeProgressProxy* KisNode::nodeProgressProxy() const
{
    if (m_d->nodeProgressProxy) {
        return m_d->nodeProgressProxy;
    } else if (parent()) {
        return parent()->nodeProgressProxy();
    }
    return 0;
}

KisBusyProgressIndicator* KisNode::busyProgressIndicator() const
{
    if (m_d->busyProgressIndicator) {
        return m_d->busyProgressIndicator;
    } else if (parent()) {
        return parent()->busyProgressIndicator();
    }
    return 0;
}

void KisNode::createNodeProgressProxy()
{
    if (!m_d->nodeProgressProxy) {
        m_d->nodeProgressProxy = new KisNodeProgressProxy(this);
        m_d->busyProgressIndicator = new KisBusyProgressIndicator(m_d->nodeProgressProxy);
    }
}

void KisNode::setDirty()
{
    setDirty(extent());
}

void KisNode::setDirty(const QVector<QRect> &rects)
{
    Q_FOREACH (const QRect &rc, rects) {
        setDirty(rc);
    }
}

void KisNode::setDirty(const QRegion &region)
{
    setDirty(region.rects());
}

void KisNode::setDirty(const QRect & rect)
{
    if(m_d->graphListener) {
        m_d->graphListener->requestProjectionUpdate(this, rect);
    }
}

void KisNode::invalidateFrames(const KisTimeRange &range, const QRect &rect)
{
    if(m_d->graphListener) {
        m_d->graphListener->invalidateFrames(range, rect);
    }
}

void KisNode::requestTimeSwitch(int time)
{
    if(m_d->graphListener) {
        m_d->graphListener->requestTimeSwitch(time);
    }
}

void KisNode::syncLodCache()
{
    KisPaintDeviceSP device = paintDevice();
    if (device) {
        QRegion dirtyRegion = device->syncLodCache(device->defaultBounds()->currentLevelOfDetail());
        Q_UNUSED(dirtyRegion);
    }

    KisPaintDeviceSP originalDevice = original();
    if (originalDevice && originalDevice != device) {
        QRegion dirtyRegion = originalDevice->syncLodCache(originalDevice->defaultBounds()->currentLevelOfDetail());
        Q_UNUSED(dirtyRegion);
    }

    projectionPlane()->syncLodCache();
}

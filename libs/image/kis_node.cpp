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
#include <QCoreApplication>

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
#include "kis_image.h"
#include "kis_layer_utils.h"

/**
 *The link between KisProjection and KisImageUpdater
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
            , projectionLeaf(new KisProjectionLeaf(node))
    {
    }

    KisNodeWSP parent;
    KisNodeGraphListener *graphListener;
    KisSafeReadNodeList nodes;
    KisNodeProgressProxy *nodeProgressProxy;
    KisBusyProgressIndicator *busyProgressIndicator;
    QReadWriteLock nodeSubgraphLock;

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
            KisLayer *newCopyFromLayer = qobject_cast<KisLayer*>(const_cast<KisNode*>(newCopyFrom));
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

KisNode::KisNode(KisImageWSP image)
        : KisBaseNode(image),
          m_d(new Private(this))
{
    m_d->parent = 0;
    m_d->graphListener = 0;
    moveToThread(qApp->thread());
}

KisNode::KisNode(const KisNode & rhs)
        : KisBaseNode(rhs)
        , m_d(new Private(this))
{
    m_d->parent = 0;
    m_d->graphListener = 0;
    moveToThread(qApp->thread());

    // HACK ALERT: we create opacity channel in KisBaseNode, but we cannot
    //             initialize its node from there! So workaround it here!
    QMap<QString, KisKeyframeChannel*> channels = keyframeChannels();
    for (auto it = channels.begin(); it != channels.end(); ++it) {
        it.value()->setNode(this);
    }

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
        m_d->busyProgressIndicator->prepareDestroying();
        m_d->busyProgressIndicator->deleteLater();
    }

    if (m_d->nodeProgressProxy) {
        m_d->nodeProgressProxy->prepareDestroying();
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

void KisNode::childNodeChanged(KisNodeSP /*changedChildNode*/)
{
}

KisAbstractProjectionPlaneSP KisNode::projectionPlane() const
{
    KIS_ASSERT_RECOVER_NOOP(0 && "KisNode::projectionPlane() is not defined!");
    static KisAbstractProjectionPlaneSP plane =
        toQShared(new KisDumbProjectionPlane());

    return plane;
}

KisProjectionLeafSP KisNode::projectionLeaf() const
{
    return m_d->projectionLeaf;
}

void KisNode::setImage(KisImageWSP image)
{
    KisBaseNode::setImage(image);

    KisNodeSP node = firstChild();
    while (node) {
        KisLayerUtils::recursiveApplyNodes(node,
                                           [image] (KisNodeSP node) {
                                               node->setImage(image);
                                           });

        node = node->nextSibling();
    }
}

bool KisNode::accept(KisNodeVisitor &v)
{
    return v.visit(this);
}

void KisNode::accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter)
{
    visitor.visit(this, undoAdapter);
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
    return m_d->parent.isValid() ? KisNodeSP(m_d->parent) : KisNodeSP();
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
        emit sigNodeChangedInternal();
    }
}

void KisNode::baseNodeInvalidateAllFramesCallback()
{
    if(m_d->graphListener) {
        m_d->graphListener->invalidateAllFrames();
    }
}

void KisNode::baseNodeCollapsedChangedCallback()
{
    if(m_d->graphListener) {
        m_d->graphListener->nodeCollapsedChanged(this);
    }
}

void KisNode::addKeyframeChannel(KisKeyframeChannel *channel)
{
    channel->setNode(this);
    KisBaseNode::addKeyframeChannel(channel);
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

KisNodeSP KisNode::findChildByName(const QString &name)
{
    KisNodeSP child = firstChild();
    while (child) {
        if (child->name() == name) {
            return child;
        }
        if (child->childCount() > 0) {
            KisNodeSP grandChild = child->findChildByName(name);
            if (grandChild) {
                return grandChild;
            }
        }
        child = child->nextSibling();
    }
    return 0;
}

bool KisNode::add(KisNodeSP newNode, KisNodeSP aboveThis)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(newNode, false);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!aboveThis || aboveThis->parent().data() == this, false);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(allowAsChild(newNode), false);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!newNode->parent(), false);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(index(newNode) < 0, false);

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

    childNodeChanged(newNode);

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

        childNodeChanged(removedNode);

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

        m_d->nodeProgressProxy->moveToThread(this->thread());
        m_d->busyProgressIndicator->moveToThread(this->thread());
    }
}

void KisNode::setDirty()
{
    setDirty(extent());
}

void KisNode::setDirty(const QVector<QRect> &rects)
{
    if(m_d->graphListener) {
        m_d->graphListener->requestProjectionUpdate(this, rects, true);
    }
}

void KisNode::setDirty(const QRegion &region)
{
    setDirty(region.rects());
}

void KisNode::setDirty(const QRect & rect)
{
    setDirty(QVector<QRect>({rect}));
}

void KisNode::setDirtyDontResetAnimationCache()
{
    setDirtyDontResetAnimationCache(QVector<QRect>({extent()}));
}

void KisNode::setDirtyDontResetAnimationCache(const QRect &rect)
{
    setDirtyDontResetAnimationCache(QVector<QRect>({rect}));
}

void KisNode::setDirtyDontResetAnimationCache(const QVector<QRect> &rects)
{
    if(m_d->graphListener) {
        m_d->graphListener->requestProjectionUpdate(this, rects, false);
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
    // noop. everything is done by getLodCapableDevices()
}

KisPaintDeviceList KisNode::getLodCapableDevices() const
{
    KisPaintDeviceList list;

    KisPaintDeviceSP device = paintDevice();
    if (device) {
        list << device;
    }

    KisPaintDeviceSP originalDevice = original();
    if (originalDevice && originalDevice != device) {
        list << originalDevice;
    }

    list << projectionPlane()->getLodCapableDevices();

    return list;
}

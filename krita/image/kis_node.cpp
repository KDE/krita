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
#include <QMutex>
#include <QMutexLocker>
#include <QPainterPath>
#include <QRect>

#include <ksharedconfig.h>

#include <KoProperties.h>

#include "kis_global.h"
#include "kis_node_graph_listener.h"
#include "kis_node_visitor.h"
#include "kis_processing_visitor.h"
#include "kis_node_progress_proxy.h"

#include "kis_safe_read_list.h"
typedef KisSafeReadList<KisNodeSP> KisSafeReadNodeList;


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
static KisNodeSPStaticRegistrar __registrar;


struct KisNode::Private
{
public:
    Private()
            : graphListener(0)
            , nodeProgressProxy(0) {
    }

    KisNodeWSP parent;
    KisNodeGraphListener *graphListener;
    KisSafeReadNodeList nodes;
    KisNodeProgressProxy *nodeProgressProxy;
};

KisNode::KisNode()
        : m_d(new Private())
{
    m_d->parent = 0;
    m_d->graphListener = 0;
}


KisNode::KisNode(const KisNode & rhs)
        : KisBaseNode(rhs)
        , m_d(new Private())
{
    m_d->parent = 0;
    m_d->graphListener = 0;

    KisSafeReadNodeList::const_iterator iter;
    FOREACH_SAFE(iter, rhs.m_d->nodes) {
        KisNodeSP child = (*iter)->clone();
        child->createNodeProgressProxy();
        m_d->nodes.append(child);
        child->setParent(this);
    }
}

KisNode::~KisNode()
{
    delete m_d->nodeProgressProxy;
    m_d->nodes.clear();
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

    KisSafeReadNodeList::const_iterator iter;
    FOREACH_SAFE(iter, m_d->nodes) {
        KisNodeSP child = (*iter);
        child->setGraphListener(graphListener);
    }
}

KisNodeSP KisNode::parent() const
{
    if (m_d->parent.isValid()) {
        return m_d->parent;
    }
    else {
        return 0;
    }
}

KisBaseNodeSP KisNode::parentCallback() const
{
    return parent();
}

void KisNode::baseNodeChangedCallback()
{
    if(m_d->graphListener) {
        m_d->graphListener->nodeChanged(this);
    }
}

void KisNode::setParent(KisNodeWSP parent)
{
    m_d->parent = parent;
}

KisNodeSP KisNode::firstChild() const
{
    if (!m_d->nodes.isEmpty())
        return m_d->nodes.first();
    else
        return 0;
}

KisNodeSP KisNode::lastChild() const
{
    if (!m_d->nodes.isEmpty())
        return m_d->nodes.last();
    else
        return 0;
}

KisNodeSP KisNode::prevSibling() const
{
    if (!parent()) return 0;
    int i = parent()->index(const_cast<KisNode*>(this));
    return parent()->at(i - 1);

}

KisNodeSP KisNode::nextSibling() const
{
    if (!parent()) return 0;

    return parent()->at(parent()->index(const_cast<KisNode*>(this)) + 1);
}

quint32 KisNode::childCount() const
{
    return m_d->nodes.size();
}


KisNodeSP KisNode::at(quint32 index) const
{
    if (!m_d->nodes.isEmpty() && index < (quint32)m_d->nodes.size()) {
        return m_d->nodes.at(index);
    }

    return 0;
}

int KisNode::index(const KisNodeSP node) const
{
    if (m_d->nodes.contains(node)) {
        return m_d->nodes.indexOf(node);
    }

    return -1;
}

QList<KisNodeSP> KisNode::childNodes(const QStringList & nodeTypes, const KoProperties & properties) const
{
    QList<KisNodeSP> nodes;

    KisSafeReadNodeList::const_iterator iter;
    FOREACH_SAFE(iter, m_d->nodes) {
        if (properties.isEmpty() || (*iter)->check(properties)) {
            bool rightType = true;

            if(!nodeTypes.isEmpty()) {
                rightType = false;
                foreach(const QString &nodeType,  nodeTypes) {
                    if ((*iter)->inherits(nodeType.toLatin1())) {
                        rightType = true;
                        break;
                    }
                }
            }
            if(rightType) {
                nodes.append(*iter);
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
    if (m_d->nodes.contains(newNode)) return false;

    newNode->prepareForAddition();
    newNode->createNodeProgressProxy();

    int idx = 0;

    if (aboveThis != 0) {

        idx = this->index(aboveThis) + 1;

        if (m_d->graphListener)
            m_d->graphListener->aboutToAddANode(this, idx);

        m_d->nodes.insert(idx, newNode);
    } else {
        if (m_d->graphListener)
            m_d->graphListener->aboutToAddANode(this, idx);

        m_d->nodes.prepend(newNode);
    }

    newNode->setParent(this);
    newNode->setGraphListener(m_d->graphListener);
    newNode->initAfterAddition();

    if (m_d->graphListener)
        m_d->graphListener->nodeHasBeenAdded(this, idx);


    return true;
}

bool KisNode::remove(quint32 index)
{
    if (index < childCount()) {
        KisNodeSP removedNode = at(index);
        removedNode->prepareForRemoval();
        removedNode->setGraphListener(0);

        if (m_d->graphListener)
            m_d->graphListener->aboutToRemoveANode(this, index);

        removedNode->setParent(0);   // after calling aboutToRemoveANode or then the model get broken according to TT's modeltest

        m_d->nodes.removeAt(index);

        if (m_d->graphListener) m_d->graphListener->nodeHasBeenRemoved(this, index);

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

void KisNode::createNodeProgressProxy()
{
    if (!m_d->nodeProgressProxy) {
        m_d->nodeProgressProxy = new KisNodeProgressProxy(this);
    }
}

void KisNode::setDirty()
{
    setDirty(extent());
}

void KisNode::setDirty(const QVector<QRect> &rects)
{
    foreach(const QRect &rc, rects) {
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

#include "kis_node.moc"

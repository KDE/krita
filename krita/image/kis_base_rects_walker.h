/*
 *  Copyright (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_BASE_RECTS_WALKER_H
#define __KIS_BASE_RECTS_WALKER_H

#include <QStack>

#include "kis_layer.h"
#include "kis_mask.h"

class KisBaseRectsWalker;
typedef KisSharedPtr<KisBaseRectsWalker> KisBaseRectsWalkerSP;

class KRITAIMAGE_EXPORT KisBaseRectsWalker : public KisShared
{
public:
    enum UpdateType {
        UPDATE,
        FULL_REFRESH,
        UNSUPPORTED
    };


    typedef qint32 NodePosition;
    enum NodePositionValues {
        /**
         * There are two different sets of values.
         * The first describes the position of the node to the graph,
         * the second shows the position to the filthy node
         */

        N_NORMAL     = 0x00,
        N_TOPMOST    = 0x01,
        N_BOTTOMMOST = 0x02,
        N_EXTRA      = 0x04,

        N_ABOVE_FILTHY = 0x08,
        N_FILTHY_ORIGINAL   = 0x10, // not used actually
        N_FILTHY_PROJECTION = 0x20,
        N_FILTHY = 0x40,
        N_BELOW_FILTHY = 0x80
    };

    #define GRAPH_POSITION_MASK     0x07
    #define POSITION_TO_FILTHY_MASK 0xF8

    struct CloneNotification {
        CloneNotification() {}
        CloneNotification(KisNodeSP node, const QRect &dirtyRect)
            : m_layer(qobject_cast<KisLayer*>(node.data())),
              m_dirtyRect(dirtyRect) {}

        void notify() {
            Q_ASSERT(m_layer); // clones are possible for layers only
            m_layer->updateClones(m_dirtyRect);
        }

    private:
        friend class KisWalkersTest;

        KisLayerSP m_layer;
        QRect m_dirtyRect;
    };

    typedef QVector<CloneNotification> CloneNotificationsVector;

    struct JobItem {
        KisNodeSP m_node;
        NodePosition m_position;

        /**
         * The rect that should be prepared on this node.
         * E.g. area where the filter applies on filter layer
         * or an area of a paint layer that will be copied to
         * the projection.
         */
        QRect m_applyRect;
    };

    typedef QStack<JobItem> NodeStack;

public:
    virtual ~KisBaseRectsWalker() {
    }

    void collectRects(KisNodeSP node, const QRect& requestedRect) {
        clear();
        m_nodeChecksum = calculateChecksum(node, requestedRect);
        m_graphChecksum = node->graphSequenceNumber();
        m_resultChangeRect = requestedRect;
        m_resultUncroppedChangeRect = requestedRect;
        m_requestedRect = requestedRect;
        m_startNode = node;
        startTrip(node);
    }

    inline void recalculate(const QRect& requestedRect) {
        Q_ASSERT(m_startNode);

        if(isStillInGraph(m_startNode)) {
            collectRects(m_startNode, requestedRect);
        }
        else {
            clear();
            m_nodeChecksum = calculateChecksum(m_startNode, requestedRect);
            m_graphChecksum = m_startNode->graphSequenceNumber();
            m_resultChangeRect = QRect();
            m_resultUncroppedChangeRect = QRect();
        }
    }

    bool checksumValid() {
        Q_ASSERT(m_startNode);
        return
            m_nodeChecksum == calculateChecksum(m_startNode, m_requestedRect) &&
            m_graphChecksum == m_startNode->graphSequenceNumber();
    }

    inline void setCropRect(QRect cropRect) {
        m_cropRect = cropRect;
    }

    inline QRect cropRect() const{
        return m_cropRect;
    }

    // return a reference for efficiency reasons
    inline NodeStack& nodeStack() {
        return m_mergeTask;
    }

    // return a reference for efficiency reasons
    inline CloneNotificationsVector& cloneNotifications() {
        return m_cloneNotifications;
    }

    inline const QRect& accessRect() const {
        return m_resultAccessRect;
    }

    inline const QRect& changeRect() const {
        return m_resultChangeRect;
    }

    inline const QRect& uncroppedChangeRect() const {
        return m_resultUncroppedChangeRect;
    }

    inline bool needRectVaries() const {
        return m_needRectVaries;
    }

    inline bool changeRectVaries() const {
        return m_changeRectVaries;
    }

    inline KisNodeSP startNode() const {
        return m_startNode;
    }

    inline const QRect& requestedRect() const {
        return m_requestedRect;
    }

    virtual UpdateType type() const = 0;

protected:

    /**
     * Initiates collecting of rects.
     * Should be implemented in derived classes
     */
    virtual void startTrip(KisNodeSP startWith) = 0;

protected:
    static inline KisNode::PositionToFilthy getPositionToFilthy(qint32 position) {
        qint32 positionToFilthy = position & POSITION_TO_FILTHY_MASK;
        // We do not use N_FILTHY_ORIGINAL yet, so...
        Q_ASSERT(!(positionToFilthy & N_FILTHY_ORIGINAL));

        return static_cast<KisNode::PositionToFilthy>(positionToFilthy);
    }

    static inline qint32 getGraphPosition(qint32 position) {
        return position & GRAPH_POSITION_MASK;
    }

    static inline bool isStillInGraph(KisNodeSP node) {
        return node->graphListener();
    }

    static inline bool isLayer(KisNodeSP node) {
        return qobject_cast<KisLayer*>(node.data());
    }

    static inline bool isMask(KisNodeSP node) {
        return qobject_cast<KisMask*>(node.data());
    }

    static inline bool hasClones(KisNodeSP node) {
        KisLayer *layer = qobject_cast<KisLayer*>(node.data());
        return layer && layer->hasClones();
    }

    static inline NodePosition calculateNodePosition(KisNodeSP node) {
        KisNodeSP nextNode = node->nextSibling();
        while(nextNode && !isLayer(nextNode)) nextNode = nextNode->nextSibling();
        if (!nextNode) return N_TOPMOST;

        KisNodeSP prevNode = node->prevSibling();
        while(prevNode && !isLayer(prevNode)) prevNode = prevNode->prevSibling();
        if (!prevNode) return N_BOTTOMMOST;

        return N_NORMAL;
    }

    inline void clear() {
        m_resultAccessRect = m_resultNeedRect = /*m_resultChangeRect =*/
            m_childNeedRect = m_lastNeedRect = QRect();

        m_needRectVaries = m_changeRectVaries = false;
        m_mergeTask.clear();
        m_cloneNotifications.clear();

        // Not needed really. Think over removing.
        //m_startNode = 0;
        //m_requestedRect = QRect();
    }

    inline void pushJob(KisNodeSP node, NodePosition position, QRect applyRect) {
        JobItem item = {node, position, applyRect};
        m_mergeTask.push(item);
    }

    inline QRect cropThisRect(const QRect& rect) {
        return m_cropRect.isValid() ? rect & m_cropRect : rect;
    }

    /**
     * Used by KisFullRefreshWalker as it has a special changeRect strategy
     */
    inline void setExplicitChangeRect(KisNodeSP node, const QRect &changeRect, bool changeRectVaries) {
        m_resultChangeRect = changeRect;
        m_resultUncroppedChangeRect = changeRect;
        m_changeRectVaries = changeRectVaries;
        registerCloneNotification(node, N_FILTHY);
    }

    /**
     * Called for every node we meet on a forward way of the trip.
     */
    virtual void registerChangeRect(KisNodeSP node, NodePosition position) {
        // We do not work with masks here. It is KisLayer's job.
        if(!isLayer(node)) return;

        QRect currentChangeRect = node->changeRect(m_resultChangeRect,
                                                   getPositionToFilthy(position));
        currentChangeRect = cropThisRect(currentChangeRect);

        if(!m_changeRectVaries)
            m_changeRectVaries = currentChangeRect != m_resultChangeRect;

        m_resultChangeRect = currentChangeRect;

        m_resultUncroppedChangeRect = node->changeRect(m_resultUncroppedChangeRect,
                                                       getPositionToFilthy(position));
        registerCloneNotification(node, position);
    }

    void registerCloneNotification(KisNodeSP node, NodePosition position) {
        /**
         * Note, we do not check for (N_ABOVE_FILTHY &&
         * dependOnLowerNodes(node)) because it may lead to an
         * infinite loop with filter layer. Activate it when it is
         * guaranteed that it is not possible to create a filter layer
         * avobe its own clone
         */

        if(hasClones(node) && position & (N_FILTHY | N_FILTHY_PROJECTION)) {
            m_cloneNotifications.append(
                CloneNotification(node, m_resultUncroppedChangeRect));
        }
    }

    /**
     * Called for every node we meet on a backward way of the trip.
     */
    virtual void registerNeedRect(KisNodeSP node, NodePosition position) {
        // We do not work with masks here. It is KisLayer's job.
        if(!isLayer(node)) return;

        if(m_mergeTask.isEmpty())
            m_resultAccessRect = m_resultNeedRect = m_childNeedRect =
                m_lastNeedRect = m_resultChangeRect;

        QRect currentNeedRect;

        if(position & N_TOPMOST)
            m_lastNeedRect = m_childNeedRect;

        if(position & (N_FILTHY | N_ABOVE_FILTHY | N_EXTRA)) {
            if(!m_lastNeedRect.isEmpty())
                pushJob(node, position, m_lastNeedRect);
            //else /* Why push empty rect? */;

            m_resultAccessRect |= node->accessRect(m_lastNeedRect,
                                                   getPositionToFilthy(position));

            m_lastNeedRect = node->needRect(m_lastNeedRect,
                                            getPositionToFilthy(position));
            m_lastNeedRect = cropThisRect(m_lastNeedRect);
            m_childNeedRect = m_lastNeedRect;
        }
        else if(position & (N_BELOW_FILTHY | N_FILTHY_PROJECTION)) {
            if(!m_lastNeedRect.isEmpty()) {
                pushJob(node, position, m_lastNeedRect);

                m_resultAccessRect |= node->accessRect(m_lastNeedRect,
                                                       getPositionToFilthy(position));

                m_lastNeedRect = node->needRect(m_lastNeedRect,
                                                getPositionToFilthy(position));
                m_lastNeedRect = cropThisRect(m_lastNeedRect);
            }
        }
        else {
            // N_FILTHY_ORIGINAL is not used so it goes there
            qFatal("KisBaseRectsWalker: node position(%d) is out of range", position);
        }

        if(!m_needRectVaries)
            m_needRectVaries = m_resultNeedRect != m_lastNeedRect;
        m_resultNeedRect |= m_lastNeedRect;
    }

    virtual void adjustMasksChangeRect(KisNodeSP firstMask) {
        KisNodeSP currentNode = firstMask;

        while (currentNode) {
            /**
             * ATTENTION: we miss the first mask
             */

            do {
                currentNode = currentNode->nextSibling();
            } while (currentNode &&
                     (!isMask(currentNode) || !currentNode->visible()));

            if(currentNode) {
                QRect changeRect = currentNode->changeRect(m_resultChangeRect);
                m_changeRectVaries |= changeRect != m_resultChangeRect;
                m_resultChangeRect = changeRect;
            }
        }
    }

    static qint32 calculateChecksum(KisNodeSP node, const QRect &requestedRect) {
        qint32 checksum = 0;
        qint32 x, y, w, h;
        QRect tempRect;

        tempRect = node->changeRect(requestedRect);
        tempRect.getRect(&x, &y, &w, &h);
        checksum += -x - y + w + h;

        tempRect = node->needRect(requestedRect);
        tempRect.getRect(&x, &y, &w, &h);
        checksum += -x - y + w + h;

//        qCritical() << node << requestedRect << "-->" << checksum;

        return checksum;
    }

private:
    /**
     * The result variables.
     * By the end of a recursion they will store a complete
     * data for a successful merge operation.
     */
    QRect m_resultAccessRect;
    QRect m_resultNeedRect;
    QRect m_resultChangeRect;
    QRect m_resultUncroppedChangeRect;
    bool m_needRectVaries;
    bool m_changeRectVaries;
    NodeStack m_mergeTask;
    CloneNotificationsVector m_cloneNotifications;

    /**
     * Used by update optimization framework
     */
    KisNodeSP m_startNode;
    QRect m_requestedRect;

    /**
     * Used for getting know whether the start node
     * properties have changed since the walker was
     * calculated
     */
    qint32 m_nodeChecksum;

    /**
     * Used for getting know whether the structure of
     * the graph has changed since the walker was
     * calculated
     */
    qint32 m_graphChecksum;

    /**
     * Temporary variables
     */
    QRect m_cropRect;

    QRect m_childNeedRect;
    QRect m_lastNeedRect;
};

#endif /* __KIS_BASE_RECTS_WALKER_H */


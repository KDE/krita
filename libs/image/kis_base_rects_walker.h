/*
 *  SPDX-FileCopyrightText: 2009 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_BASE_RECTS_WALKER_H
#define __KIS_BASE_RECTS_WALKER_H

#include <QStack>

#include "kis_layer.h"

#include "kis_abstract_projection_plane.h"
#include "kis_projection_leaf.h"


class KisBaseRectsWalker;
typedef KisSharedPtr<KisBaseRectsWalker> KisBaseRectsWalkerSP;

class KRITAIMAGE_EXPORT KisBaseRectsWalker : public KisShared
{
public:
    enum UpdateType {
        UPDATE,
        UPDATE_NO_FILTHY,
        FULL_REFRESH,
        FULL_REFRESH_NO_FILTHY,
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

    static inline KisNode::PositionToFilthy convertPositionToFilthy(NodePosition position) {
        static const int positionToFilthyMask =
            N_ABOVE_FILTHY |
            N_FILTHY_PROJECTION |
            N_FILTHY |
            N_BELOW_FILTHY;

        qint32 positionToFilthy = position & N_EXTRA ? N_FILTHY : position & positionToFilthyMask;
        // We do not use N_FILTHY_ORIGINAL yet, so...
        Q_ASSERT(positionToFilthy);

        return static_cast<KisNode::PositionToFilthy>(positionToFilthy);
    }

    struct CloneNotification {
        CloneNotification() {}
        CloneNotification(KisNodeSP node, const QRect &dirtyRect,
                          bool dontInvalidateFrames)
            : m_layer(qobject_cast<KisLayer*>(node.data())),
              m_dirtyRect(dirtyRect),
            m_dontInvalidateFrames(dontInvalidateFrames) {}

        void notify() {
            Q_ASSERT(m_layer); // clones are possible for layers only
            m_layer->updateClones(m_dirtyRect, m_dontInvalidateFrames);
        }

    private:
        friend class KisWalkersTest;

        KisLayerSP m_layer;
        QRect m_dirtyRect;
        bool m_dontInvalidateFrames {false};
    };

    typedef QVector<CloneNotification> CloneNotificationsVector;

    struct JobItem {
        KisProjectionLeafSP m_leaf;
        NodePosition m_position;

        /**
         * The rect that should be prepared on this node.
         * E.g. area where the filter applies on filter layer
         * or an area of a paint layer that will be copied to
         * the projection.
         */
        QRect m_applyRect;

        KisRenderPassFlags m_renderFlags = KisRenderPassFlag::None;
    };

    typedef QStack<JobItem> LeafStack;

    enum SubtreeVisitFlag {
        None = 0x0,
        SkipNonRenderableNodes = 0x1,
        NoFilthyMode = 0x2,
        DontNotifyClones = 0x4
    };

    Q_DECLARE_FLAGS(SubtreeVisitFlags, SubtreeVisitFlag);

public:
    KisBaseRectsWalker()
        : m_levelOfDetail(0)
    {
    }

    virtual ~KisBaseRectsWalker() {
    }

    void collectRects(KisNodeSP node, const QRect& requestedRect) {
        clear();

        KisProjectionLeafSP startLeaf = node->projectionLeaf();

        m_nodeChecksum = calculateChecksum(startLeaf, requestedRect);
        m_graphChecksum = node->graphSequenceNumber();
        m_resultChangeRect = requestedRect;
        m_resultUncroppedChangeRect = requestedRect;
        m_requestedRect = requestedRect;
        m_startNode = node;
        m_levelOfDetail = getNodeLevelOfDetail(startLeaf);
        startTrip(startLeaf);
        addCloneSourceRegenerationJobs();
    }

    inline void recalculate(const QRect& requestedRect) {
        KIS_SAFE_ASSERT_RECOVER_RETURN(m_startNode);

        KisProjectionLeafSP startLeaf = m_startNode->projectionLeaf();

        int calculatedLevelOfDetail = getNodeLevelOfDetail(startLeaf);

        if (m_levelOfDetail != calculatedLevelOfDetail) {
            qWarning() << "WARNING: KisBaseRectsWalker::recalculate()"
                       << "The levelOfDetail has changes with time,"
                       << "which couldn't have happened!"
                       << ppVar(m_levelOfDetail)
                       << ppVar(calculatedLevelOfDetail);

            m_levelOfDetail = calculatedLevelOfDetail;
        }

        if(startLeaf->isStillInGraph()) {
            collectRects(m_startNode, requestedRect);
        }
        else {
            clear();
            m_nodeChecksum = calculateChecksum(startLeaf, requestedRect);
            m_graphChecksum = m_startNode->graphSequenceNumber();
            m_resultChangeRect = QRect();
            m_resultUncroppedChangeRect = QRect();
        }
    }

    bool checksumValid() {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_startNode, false);
        return
            m_nodeChecksum == calculateChecksum(m_startNode->projectionLeaf(), m_requestedRect) &&
            m_graphChecksum == m_startNode->graphSequenceNumber();
    }

    inline void setClonesDontInvalidateFrames(bool value) {
        m_clonesDontInvalidateFrames = value;
    }

    inline bool clonesDontInvalidateFrames() const {
        return m_clonesDontInvalidateFrames;
    }

    inline void setCropRect(QRect cropRect) {
        m_cropRect = cropRect;
    }

    inline QRect cropRect() const{
        return m_cropRect;
    }

    // return a reference for efficiency reasons
    inline LeafStack& leafStack() {
        return m_mergeTask;
    }

    // return a reference for efficiency reasons
    inline CloneNotificationsVector& cloneNotifications() {
        return m_cloneNotifications;
    }

    inline QRect accessRect() const {
        return m_resultAccessRect;
    }

    inline QRect changeRect() const {
        return m_resultChangeRect;
    }

    inline QRect uncroppedChangeRect() const {
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

    inline QRect requestedRect() const {
        return m_requestedRect;
    }

    inline int levelOfDetail() const {
        return m_levelOfDetail;
    }

    virtual UpdateType type() const = 0;

protected:

    /**
     * Initiates collecting of rects.
     * Should be implemented in derived classes
     */
    virtual void startTrip(KisProjectionLeafSP startWith) = 0;

protected:

    static inline qint32 getGraphPosition(qint32 position) {
        return position & GRAPH_POSITION_MASK;
    }

    static inline bool hasClones(KisNodeSP node) {
        KisLayer *layer = qobject_cast<KisLayer*>(node.data());
        return layer && layer->hasClones();
    }

    static inline NodePosition calculateNodePosition(KisProjectionLeafSP leaf) {
        KisProjectionLeafSP nextLeaf = leaf->nextSibling();
        while(nextLeaf && !nextLeaf->isLayer()) nextLeaf = nextLeaf->nextSibling();
        if (!nextLeaf) return N_TOPMOST;

        KisProjectionLeafSP prevLeaf = leaf->prevSibling();
        while(prevLeaf && !prevLeaf->isLayer()) prevLeaf = prevLeaf->prevSibling();
        if (!prevLeaf) return N_BOTTOMMOST;

        return N_NORMAL;
    }

    inline bool isStartLeaf(KisProjectionLeafSP leaf) const {
        return leaf->node() == m_startNode;
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

    inline void pushJob(KisProjectionLeafSP leaf, NodePosition position, QRect applyRect, KisRenderPassFlags flags) {
        JobItem item = {leaf, position, applyRect, flags};
        m_mergeTask.push(item);
    }

    inline QRect cropThisRect(const QRect& rect, const QRect &cropRect) {
        return cropRect.isValid() ? rect & cropRect : rect;
    }

    /**
     * Used by KisFullRefreshWalker as it has a special changeRect strategy
     */
    inline void setExplicitChangeRect(const QRect &changeRect, bool changeRectVaries) {
        m_resultChangeRect = changeRect;
        m_resultUncroppedChangeRect = changeRect;
        m_changeRectVaries = changeRectVaries;
    }

    /**
     * Called for every node we meet on a forward way of the trip.
     */
    virtual void registerChangeRect(KisProjectionLeafSP leaf, NodePosition position) {
        // We do not work with masks here. It is KisLayer's job.
        if(!leaf->isLayer()) return;
        if(!(position & N_FILTHY) && !leaf->visible()) return;

        QRect currentChangeRect = leaf->projectionPlane()->changeRect(m_resultChangeRect,
                                                                      convertPositionToFilthy(position));
        currentChangeRect = cropThisRect(currentChangeRect, m_cropRect);

        if(!m_changeRectVaries)
            m_changeRectVaries = currentChangeRect != m_resultChangeRect;

        m_resultChangeRect = currentChangeRect;

        m_resultUncroppedChangeRect = leaf->projectionPlane()->changeRect(m_resultUncroppedChangeRect,
                                                                          convertPositionToFilthy(position));
        registerCloneNotification(leaf->node(), position);
    }

    void registerCloneNotification(KisNodeSP node, NodePosition position) {
        /**
         * Note, we do not check for (N_ABOVE_FILTHY &&
         * dependOnLowerNodes(node)) because it may lead to an
         * infinite loop with filter layer. Activate it when it is
         * guaranteed that it is not possible to create a filter layer
         * above its own clone
         */

        if(hasClones(node) && position & (N_FILTHY | N_FILTHY_PROJECTION | N_EXTRA)) {
            m_cloneNotifications.append(
                CloneNotification(node, m_resultUncroppedChangeRect, m_clonesDontInvalidateFrames));
        }
    }

    void registerNeedRect(KisProjectionLeafSP leaf, NodePosition position, KisRenderPassFlags flags) {
        registerNeedRect(leaf, position, flags, m_cropRect);
    }

    /**
     * Called for every node we meet on a backward way of the trip.
     */
    virtual void registerNeedRect(KisProjectionLeafSP leaf, NodePosition position,
                                  KisRenderPassFlags flags, const QRect &cropRect) {
        // We do not work with masks here. It is KisLayer's job.
        if(!leaf->isLayer()) return;

        if(m_mergeTask.isEmpty())
            m_resultAccessRect = m_resultNeedRect = m_childNeedRect =
                m_lastNeedRect = m_resultChangeRect;

        if (leaf->parent() && position & N_TOPMOST) {
            bool parentNeedRectFound = false;
            QRect parentNeedRect;

            for (auto it = std::make_reverse_iterator(m_mergeTask.end());
                 it != std::make_reverse_iterator(m_mergeTask.begin());
                 ++it) {

                if (it->m_leaf == leaf->parent()) {
                    parentNeedRect =
                        it->m_leaf->projectionPlane()->needRectForOriginal(it->m_applyRect);
                    parentNeedRectFound = true;
                    break;
                }
            }

            // TODO: check if we can put this requirement
            // KIS_SAFE_ASSERT_RECOVER_NOOP(parentNeedRectFound);

            if (parentNeedRectFound) {
                m_lastNeedRect = parentNeedRect;
            } else {
                // legacy way of fetching parent need rect, just
                // takes need rect of the last visited filthy node
                m_lastNeedRect = m_childNeedRect;
            }
        }

        if (!leaf->shouldBeRendered()) {
            if (!m_lastNeedRect.isEmpty()) {
                // push a dumb job to fit state machine requirements
                pushJob(leaf, position, m_lastNeedRect, flags);
            }
        } else if(position & (N_FILTHY | N_ABOVE_FILTHY | N_EXTRA)) {
            if(!m_lastNeedRect.isEmpty())
                pushJob(leaf, position, m_lastNeedRect, flags);
            //else /* Why push empty rect? */;

            m_resultAccessRect |= leaf->projectionPlane()->accessRect(m_lastNeedRect,
                                                                      convertPositionToFilthy(position));

            m_lastNeedRect = leaf->projectionPlane()->needRect(m_lastNeedRect,
                                                               convertPositionToFilthy(position));
            m_lastNeedRect = cropThisRect(m_lastNeedRect, cropRect);
            m_childNeedRect = m_lastNeedRect;
        }
        else if(position & (N_BELOW_FILTHY | N_FILTHY_PROJECTION)) {
            if(!m_lastNeedRect.isEmpty()) {
                pushJob(leaf, position, m_lastNeedRect, flags);

                m_resultAccessRect |= leaf->projectionPlane()->accessRect(m_lastNeedRect,
                                                                          convertPositionToFilthy(position));

                m_lastNeedRect = leaf->projectionPlane()->needRect(m_lastNeedRect,
                                                                   convertPositionToFilthy(position));
                m_lastNeedRect = cropThisRect(m_lastNeedRect, cropRect);
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

    virtual void adjustMasksChangeRect(KisProjectionLeafSP firstMask) {
        KisProjectionLeafSP currentLeaf = firstMask;

        while (currentLeaf) {
            /**
             * ATTENTION: we miss the first mask
             */

            do {
                currentLeaf = currentLeaf->nextSibling();
            } while (currentLeaf &&
                     (!currentLeaf->isMask() || !currentLeaf->visible()));

            if(currentLeaf) {
                QRect changeRect = currentLeaf->projectionPlane()->changeRect(m_resultChangeRect);
                m_changeRectVaries |= changeRect != m_resultChangeRect;
                m_resultChangeRect = changeRect;
                m_resultUncroppedChangeRect = changeRect;
            }
        }

        KisProjectionLeafSP parentLayer = firstMask->parent();
        KIS_SAFE_ASSERT_RECOVER_RETURN(parentLayer);

        registerCloneNotification(parentLayer->node(), N_FILTHY_PROJECTION);
    }

    static qint32 calculateChecksum(KisProjectionLeafSP leaf, const QRect &requestedRect) {
        qint32 checksum = 0;
        qint32 x, y, w, h;
        QRect tempRect;

        tempRect = leaf->projectionPlane()->changeRect(requestedRect);
        tempRect.getRect(&x, &y, &w, &h);
        checksum += -x - y + w + h;

        tempRect = leaf->projectionPlane()->needRect(requestedRect);
        tempRect.getRect(&x, &y, &w, &h);
        checksum += -x - y + w + h;

//        errKrita << leaf << requestedRect << "-->" << checksum;

        return checksum;
    }

    void addCloneSourceRegenerationJobs();
    void visitSubtreeTopToBottom(KisProjectionLeafSP startWith, SubtreeVisitFlags flags, KisRenderPassFlags renderFlags, const QRect &cropRect);

private:
    inline int getNodeLevelOfDetail(KisProjectionLeafSP leaf) {
        while (leaf && !leaf->projection()) {
            leaf = leaf->parent();
        }

        if (!leaf || !leaf->projection()) {
            /**
             * Such errors may happen during undo or too quick node removal,
             * they shouldn't cause any real problems in Krita work.
             */
            qWarning() << "WARNING: KisBaseRectsWalker::getNodeLevelOfDetail() "
                          "failed to fetch currentLevelOfDetail() from the node. "
                          "Perhaps the node was removed from the image in the meantime.";
            return 0;
        }

        return leaf->projection()->defaultBounds()->currentLevelOfDetail();
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
    bool m_needRectVaries {false};
    bool m_changeRectVaries {false};
    LeafStack m_mergeTask;
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
    qint32 m_nodeChecksum {0};

    /**
     * Used for getting know whether the structure of
     * the graph has changed since the walker was
     * calculated
     */
    qint32 m_graphChecksum {0};

    /**
     * Temporary variables
     */
    QRect m_cropRect;

    QRect m_childNeedRect;
    QRect m_lastNeedRect;

    int m_levelOfDetail {0};

    bool m_clonesDontInvalidateFrames {false};
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KisBaseRectsWalker::SubtreeVisitFlags);

#endif /* __KIS_BASE_RECTS_WALKER_H */


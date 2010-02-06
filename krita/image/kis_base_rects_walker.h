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

class KRITAIMAGE_EXPORT KisBaseRectsWalker
{
public:
    enum NodePosition {
        N_NORMAL,
        N_LOWER,
        N_TOPMOST,
        N_BOTTOMMOST
    };

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
    KisBaseRectsWalker(QRect cropRect) {
        setCropRect(cropRect);
    }

    virtual ~KisBaseRectsWalker() {
    }

    void collectRects(KisNodeSP node, const QRect& requestedRect) {
        clear();
        m_resultChangeRect = requestedRect;
        m_requestedRect = requestedRect;
        m_startNode = node;
        startTrip(node);
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

    inline const QRect& accessRect() const {
        return m_resultAccessRect;
    }

    inline const QRect& changeRect() const {
        return m_resultChangeRect;
    }

    inline bool needRectVaries() const {
        return m_needRectVaries;
    }

    inline bool changeRectVaries() const {
        return m_changeRectVaries;
    }

protected:

    /**
     * Initiates collecting of rects.
     * Should be implemented in derived classes
     */
    virtual void startTrip(KisNodeSP startWith) = 0;

protected:
    inline void clear() {
        m_resultAccessRect = /*m_resultChangeRect =*/
            m_childNeedRect = m_lastNeedRect = QRect();

        m_needRectVaries = m_changeRectVaries = false;
        m_mergeTask.clear();

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
     * Called for every node we meet on a forward way of the trip.
     */
    virtual void registerChangeRect(KisNodeSP node) {
        QRect currentChangeRect = node->changeRect(m_resultChangeRect);
        currentChangeRect = cropThisRect(currentChangeRect);

        if(!m_changeRectVaries)
            m_changeRectVaries = currentChangeRect != m_resultChangeRect;

        m_resultChangeRect = currentChangeRect;
    }

    /**
     * Called for every node we meet on a backward way of the trip.
     */
    virtual void registerNeedRect(KisNodeSP node, NodePosition position) {
        if(m_mergeTask.isEmpty())
            m_resultAccessRect = m_childNeedRect =
                m_lastNeedRect = m_resultChangeRect;

        QRect currentNeedRect;

        switch(position) {
        case N_TOPMOST:
            m_lastNeedRect = m_childNeedRect;
        case N_NORMAL:
            if(!m_lastNeedRect.isEmpty())
                pushJob(node, position, m_lastNeedRect);
            //else /* Why push empty rect? */;

            m_lastNeedRect = node->needRect(m_lastNeedRect,
                                            KisNode::NORMAL);
            m_lastNeedRect = cropThisRect(m_lastNeedRect);
            m_childNeedRect = m_lastNeedRect;
            break;
        case N_LOWER:
        case N_BOTTOMMOST:
            if(!m_lastNeedRect.isEmpty()) {
                pushJob(node, position, m_lastNeedRect);
                m_lastNeedRect = node->needRect(m_lastNeedRect,
                                                KisNode::BELOW_FILTHY);
                m_lastNeedRect = cropThisRect(m_lastNeedRect);
            }
            break;
        default:
            qFatal("Merge visitor: node position(%d) is out of range", position);
        }

        if(!m_needRectVaries)
            m_needRectVaries = m_resultAccessRect != m_lastNeedRect;
        m_resultAccessRect |= m_lastNeedRect;
    }

private:
    /**
     * The result variables.
     * By the end of a recursion they will store a complete
     * data for a successful merge operation.
     */
    QRect m_resultAccessRect;
    QRect m_resultChangeRect;
    bool m_needRectVaries;
    bool m_changeRectVaries;
    NodeStack m_mergeTask;

    /**
     * Temporary variables
     */
    KisNodeSP m_startNode;
    QRect m_requestedRect;
    QRect m_cropRect;

    QRect m_childNeedRect;
    QRect m_lastNeedRect;
};

#endif /* __KIS_BASE_RECTS_WALKER_H */


/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_NODE_GRAPH_LISTENER_H_
#define KIS_NODE_GRAPH_LISTENER_H_

#include "kritaimage_export.h"

#include <QScopedPointer>

class KisTimeSpan;
class KisNode;
class QRect;
class KisKeyframeChannel;

/**
 * Implementations of this class are called by nodes whenever the node
 * graph changes. These implementations can then emit the right
 * signals so Qt interview models can be updated before and after
 * changes.
 *
 * The reason for this go-between is that we don't want our nodes to
 * be QObjects, nor to have sig-slot connections between every node
 * and every mode.
 *
 * It also manages the sequence number of the graph. This is a number
 * which can be used as a checksum for whether the graph has changed
 * from some period of time or not. \see graphSequenceNumber()
 */
class KRITAIMAGE_EXPORT KisNodeGraphListener
{

public:
    KisNodeGraphListener();

    virtual ~KisNodeGraphListener();

    /**
     * Inform the model that we're going to add a node.
     */
    virtual void aboutToAddANode(KisNode *parent, int index);

    /**
     * Inform the model we're done adding a node.
     */
    virtual void nodeHasBeenAdded(KisNode *parent, int index);

    /**
     * Inform the model we're going to remove a node.
     */
    virtual void aboutToRemoveANode(KisNode *parent, int index);

    /**
     * Inform the model we're done removing a node.
     */
    virtual void nodeHasBeenRemoved(KisNode *parent, int index);

    /**
     * Inform the model we're about to start moving a node (which
     * includes removing and adding the same node)
     */
    virtual void aboutToMoveNode(KisNode * node, int oldIndex, int newIndex);

    /**
     * Inform the model we're done moving the node: it has been
     * removed and added successfully
     */
    virtual void nodeHasBeenMoved(KisNode * node, int oldIndex, int newIndex);

    virtual void nodeChanged(KisNode * node);

    virtual void nodeCollapsedChanged(KisNode * node);

    virtual void invalidateAllFrames();

    /**
     * Inform the model that one of the selections in the graph is
     * changed. The sender is not passed to the function (at least for
     * now) because the UI should decide itself whether it needs to
     * fetch new selection of not.
     */
    virtual void notifySelectionChanged();

    /**
     * Inform the model that a node has been changed (setDirty)
     */
    virtual void requestProjectionUpdate(KisNode * node, const QVector<QRect> &rects, bool resetAnimationCache);

    virtual void invalidateFrames(const KisTimeSpan &range, const QRect &rect);

    virtual void requestTimeSwitch(int time);

    virtual KisNode* graphOverlayNode() const;

    /**
     * Returns the sequence of the graph.
     *
     * Every time some operation performed, which might change the
     * hierarchy of the nodes, the sequence number grows by one. So
     * if you have any information about the graph which was acquired
     * when the sequence number was X and now it has become Y, it
     * means your information is outdated.
     *
     * It is used in the scheduler for checking whether queued walkers
     * should be regenerated.
     */
     int graphSequenceNumber() const;

     /**
      * Inform the model that a keyframe channel has been added or removed.
      */
     virtual void keyframeChannelHasBeenAdded(KisNode *node, KisKeyframeChannel *channel);
     virtual void keyframeChannelAboutToBeRemoved(KisNode *node, KisKeyframeChannel *channel);

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif

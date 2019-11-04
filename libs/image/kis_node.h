/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef _KIS_NODE_H
#define _KIS_NODE_H

#include "kis_types.h"

#include "kis_base_node.h"

#include "kritaimage_export.h"

#include <QVector>

class QRect;
class QStringList;

class KoProperties;

class KisNodeVisitor;
class KisNodeGraphListener;
class KisNodeProgressProxy;
class KisBusyProgressIndicator;
class KisAbstractProjectionPlane;
class KisProjectionLeaf;
class KisKeyframeChannel;
class KisTimeRange;
class KisUndoAdapter;


/**
 * A KisNode is a KisBaseNode that knows about its direct peers, parent
 * and children and whether it can have children.
 *
 * THREAD-SAFETY: All const methods of this class and setDirty calls
 *                are considered to be thread-safe(!). All the others
 *                especially add(), remove() and setParent() must be
 *                protected externally.
 *
 * NOTE: your subclasses must have the Q_OBJECT declaration, even if
 * you do not define new signals or slots.
 */
class KRITAIMAGE_EXPORT KisNode : public KisBaseNode
{
    friend class KisFilterMaskTest;
    Q_OBJECT

public:
    /**
     * The struct describing the position of the node
     * against the filthy node.
     * NOTE: please change KisBaseRectsWalker::getPositionToFilthy
     *       when changing this struct
     */
    enum PositionToFilthy {
        N_ABOVE_FILTHY = 0x08,
        N_FILTHY_PROJECTION = 0x20,
        N_FILTHY = 0x40,
        N_BELOW_FILTHY = 0x80
    };

    /**
     * Create an empty node without a parent.
     */
    KisNode(KisImageWSP image);

    /**
     * Create a copy of this node. The copy will not have a parent
     * node.
     */
    KisNode(const KisNode & rhs);

    /**
     * Delete this node
     */
    ~KisNode() override;

    virtual KisNodeSP clone() const = 0;

    bool accept(KisNodeVisitor &v) override;
    void accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter) override;

    /**
     * Re-implement this method to add constraints for the
     * subclasses that can be added as children to this node
     *
     * @return false if the given node is not allowed as a child to this node
     */
    virtual bool allowAsChild(KisNodeSP) const = 0;

    /**
     * Set the entire node extent dirty; this percolates up to parent
     * nodes all the way to the root node. By default this is the
     * empty rect (through KisBaseNode::extent())
     */
    virtual void setDirty();

    /**
     * Add the given rect to the set of dirty rects for this node;
     * this percolates up to parent nodes all the way to the root
     * node.
     */
    void setDirty(const QRect & rect);

    /**
     * Add the given rects to the set of dirty rects for this node;
     * this percolates up to parent nodes all the way to the root
     * node.
     */
    virtual void setDirty(const QVector<QRect> &rects);

    /**
     * Add the given region to the set of dirty rects for this node;
     * this percolates up to parent nodes all the way to the root
     * node, if propagate is true;
     */
    void setDirty(const QRegion &region);

    /**
     * Convenience override of multirect version of setDirtyDontResetAnimationCache()
     *
     * @see setDirtyDontResetAnimationCache(const QVector<QRect> &rects)
     */
    void setDirtyDontResetAnimationCache();

    /**
     * Convenience override of multirect version of setDirtyDontResetAnimationCache()
     *
     * @see setDirtyDontResetAnimationCache(const QVector<QRect> &rects)
     */
    void setDirtyDontResetAnimationCache(const QRect &rect);

    /**
     * @brief setDirtyDontResetAnimationCache does almost the same thing as usual
     * setDirty() call, but doesn't reset the animation cache (since onlion skins are
     * not used when rendering animation.
     */
    void setDirtyDontResetAnimationCache(const QVector<QRect> &rects);

    /**
     * Informs that the frames in the given range are no longer valid
     * and need to be recached.
     * @param range frames to invalidate
     */
    void invalidateFrames(const KisTimeRange &range, const QRect &rect);

    /**
     * Informs that the current world time should be changed.
     * Might be caused by e.g. undo operation
     */
    void requestTimeSwitch(int time);

    /**
     * \return a pointer to a KisAbstractProjectionPlane interface of
     *         the node. This interface is used by the image merging
     *         framework to get information and to blending for the
     *         layer.
     *
     * Please note the difference between need/change/accessRect and
     * the projectionPlane() interface. The former one gives
     * information about internal composition of the layer, and the
     * latter one about the total composition, including layer styles,
     * pass-through blending and etc.
     */
    virtual KisAbstractProjectionPlaneSP projectionPlane() const;

    /**
     * Synchronizes LoD caches of the node with the current state of it.
     * The current level of detail is fetched from the image pointed by
     * default bounds object
     */
    virtual void syncLodCache();
    virtual KisPaintDeviceList getLodCapableDevices() const;

    /**
     * The rendering of the image may not always happen in the order
     * of the main graph. Pass-through nodes make some subgraphs
     * linear, so it the order of rendering change. projectionLeaf()
     * is a special interface of KisNode that represents "a graph for
     * projection rendering". Therefore the nodes in projectionLeaf()
     * graph may have a different order the main one.
     */
    virtual KisProjectionLeafSP projectionLeaf() const;


    void setImage(KisImageWSP image) override;

protected:

    /**
     * \return internal changeRect() of the node. Do not mix with \see
     *         projectionPlane()
     *
     * Some filters will cause a change of pixels those are outside
     * a requested rect. E.g. we change a rect of 2x2, then we want to
     * apply a convolution filter with kernel 4x4 (changeRect is
     * (2+2*3)x(2+2*3)=8x8) to that area. The rect that should be updated
     * on the layer will be exactly 8x8. More than that the needRect for
     * that update will be 14x14. See \ref needeRect.
     */
    virtual QRect changeRect(const QRect &rect, PositionToFilthy pos = N_FILTHY) const;

    /**
     * \return internal needRect() of the node. Do not mix with \see
     *         projectionPlane()
     *
     * Some filters need pixels outside the current processing rect to
     * compute the new value (for instance, convolution filters)
     * See \ref changeRect
     * See \ref accessRect
     */
    virtual QRect needRect(const QRect &rect, PositionToFilthy pos = N_FILTHY) const;


    /**
     * \return internal accessRect() of the node. Do not mix with \see
     *         projectionPlane()
     *
     * Shows the area of image, that may be accessed during accessing
     * the node.
     *
     * Example. You have a layer that needs to prepare some rect on a
     * projection, say expectedRect. To perform this, the projection
     * of all the layers below of the size needRect(expectedRect)
     * should be calculated by the merger beforehand and the layer
     * will access some other area of image inside the rect
     * accessRect(expectedRect) during updateProjection call.
     *
     * This knowledge about real access rect of a node is used by the
     * scheduler to avoid collisions between two multithreaded updaters
     * and so avoid flickering of the image.
     *
     * Currently, this method has nondefault value for shifted clone
     * layers only.
     */
    virtual QRect accessRect(const QRect &rect, PositionToFilthy pos = N_FILTHY) const;

    /**
     * Called each time direct child nodes are added or removed under this
     * node as parent. This does not track changes inside the child nodes
     * or the child nodes' properties.
     */
    virtual void childNodeChanged(KisNodeSP changedChildNode);

public: // Graph methods

    /**
     * @return the graph sequence number calculated by the associated
     * graph listener. You can use it for checking for changes in the
     * graph.
     */
    int graphSequenceNumber() const;

    /**
     * @return the graph listener this node belongs to. 0 if the node
     * does not belong to a grap listener.
     */
    KisNodeGraphListener * graphListener() const;

    /**
     * Set the graph listener for this node. The graphlistener will be
     * informed before and after the list of child nodes has changed.
     */
    void setGraphListener(KisNodeGraphListener * graphListener);

    /**
     * Returns the parent node of this node. This is 0 only for a root
     * node; otherwise this will be an actual Node
     */
    KisNodeSP parent() const;

    /**
     * Returns the first child node of this node, or 0 if there are no
     * child nodes.
     */
    KisNodeSP firstChild() const;

    /**
     * Returns the last child node of this node, or 0 if there are no
     * child nodes.
     */
    KisNodeSP lastChild() const;

    /**
     * Returns the previous sibling of this node in the parent's list.
     * This is the node *above* this node in the composition stack. 0
     * is returned if this child has no more previous siblings (==
     * firstChild())
     */
    KisNodeSP prevSibling() const;

    /**
     * Returns the next sibling of this node in the parent's list.
     * This is the node *below* this node in the composition stack. 0
     * is returned if this child has no more next siblings (==
     * lastChild())
     */
    KisNodeSP nextSibling() const;

    /**
     * Returns how many direct child nodes this node has (not
     * recursive).
     */
    quint32 childCount() const;

    /**
     * Retrieve the child node at the specified index.
     *
     * @return 0 if there is no node at this index.
     */
    KisNodeSP at(quint32 index) const;

    /**
     * Retrieve the index of the specified child node.
     *
     * @return -1 if the specified node is not a child node of this
     * node.
     */
    int index(const KisNodeSP node) const;


    /**
     * Return a list of child nodes of the current node that conform
     * to the specified constraints. There are no guarantees about the
     * order of the nodes in the list. The function is not recursive.
     *
     * @param nodeTypes. if not empty, only nodes that inherit the
     * classnames in this stringlist will be returned.
     * @param properties. if not empty, only nodes for which
     * KisNodeBase::check(properties) returns true will be returned.
     */
    QList<KisNodeSP> childNodes(const QStringList & nodeTypes, const KoProperties & properties) const;

    /**
     * @brief findChildByName finds the first child that has the given name
     * @param name the name to look for
     * @return the first child with the given name
     */
    KisNodeSP findChildByName(const QString &name);

Q_SIGNALS:
    /**
     * Don't use this signal anywhere other than KisNodeShape. It's a hack.
     */
    void sigNodeChangedInternal();

public:

    /**
     * @return the node progress proxy used by this node, if this node has no progress
     *         proxy, it will return the proxy of its parent, if the parent has no progress proxy
     *         it will return 0
     */
    KisNodeProgressProxy* nodeProgressProxy() const;

    KisBusyProgressIndicator* busyProgressIndicator() const;

private:

    /**
     * Create a node progress proxy for this node. You need to create a progress proxy only
     * if the node is going to appear in the layerbox, and it needs to be created before
     * the layer box is made aware of the proxy.
     */
    void createNodeProgressProxy();

protected:
    KisBaseNodeSP parentCallback() const override;
    void notifyParentVisibilityChanged(bool value) override;
    void baseNodeChangedCallback() override;
    void baseNodeInvalidateAllFramesCallback() override;
    void baseNodeCollapsedChangedCallback() override;

protected:
    void addKeyframeChannel(KisKeyframeChannel* channel) override;
private:

    friend class KisNodeFacade;
    friend class KisNodeTest;
    friend class KisLayer; // Note: only for setting the preview mask!
    /**
     * Set the parent of this node.
     */
    void setParent(KisNodeWSP parent);

    /**
     * Add the specified node above the specified node. If aboveThis
     * is 0, the node is added at the bottom.
     */
    bool add(KisNodeSP newNode, KisNodeSP aboveThis);

    /**
     * Removes the node at the specified index from the child nodes.
     *
     * @return false if there is no node at this index
     */
    bool remove(quint32 index);

    /**
     * Removes the node from the child nodes.
     *
     * @return false if there's no such node in this node.
     */
    bool remove(KisNodeSP node);

    KisNodeSP prevChildImpl(KisNodeSP child);
    KisNodeSP nextChildImpl(KisNodeSP child);

private:

    struct Private;
    Private * const m_d;

};

Q_DECLARE_METATYPE(KisNodeSP)

Q_DECLARE_METATYPE(KisNodeWSP)

#endif

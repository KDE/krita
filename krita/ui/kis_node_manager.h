/*
 *  Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_NODE_MANAGER
#define KIS_NODE_MANAGER

#include <QObject>
#include <QList>

#include "kis_types.h"
#include <kritaui_export.h>

class KActionCollection;

class KoCompositeOp;
class KoColorSpace;
class KUndo2MagicString;

class KisFilterStrategy;
class KisViewManager;
class KisActionManager;
class KisView;
class KisNodeSelectionAdapter;

/**
 * The node manager passes requests for new layers or masks on to the mask and layer
 * managers.
 */
class KRITAUI_EXPORT KisNodeManager : public QObject
{

    Q_OBJECT

public:

    KisNodeManager(KisViewManager * view);
    ~KisNodeManager();
    
    void setView(QPointer<KisView>imageView);

Q_SIGNALS:

    /// emitted whenever a node is selected.
    void sigNodeActivated(KisNodeSP node);
    
    /// emitted whenever a different layer is selected.
    void sigLayerActivated(KisLayerSP layer);
    
    /// for the layer box: this sets the current node in the layerbox
    /// without telling the node manager that the node is activated,
    /// preventing loops (I think...)
    void sigUiNeedChangeActiveNode(KisNodeSP node);

    void sigUiNeedChangeSelectedNodes(const KisNodeList &nodes);
    
public:
    
    void setup(KActionCollection * collection, KisActionManager* actionManager);
    void updateGUI();

    /// Convenience function to get the active layer or mask
    KisNodeSP activeNode();

    /// convenience function to get the active layer. If a mask is
    /// active, it's parent layer is the active layer.
    KisLayerSP activeLayer();
    
    /// Get the paint device the user wants to paint on now
    KisPaintDeviceSP activePaintDevice();

    /**
     * @return the active color space used for composition, meaning the color space
     * of the active mask, or the color space of the parent of the active layer
     */
    const KoColorSpace* activeColorSpace();

    /**
     * Sets opacity for the node in a universal way (masks/layers)
     */
    void setNodeOpacity(KisNodeSP node, qint32 opacity, bool finalChange);

    /**
     * Sets compositeOp for the node in a universal way (masks/layers)
     */
    void setNodeCompositeOp(KisNodeSP node, const KoCompositeOp* compositeOp);

    KisNodeList selectedNodes();

    KisNodeSelectionAdapter* nodeSelectionAdapter() const;

public Q_SLOTS:

    /**
     * Explicitly activates \p node
     * The UI will be noticed that active node has been changed.
     * Both sigNodeActivated and sigUiNeedChangeActiveNode are emitted.
     *
     * WARNING: normally you needn't call this method manually. It is
     * automatically called when a node is added to the graph. If you
     * have some special cases when you need to activate a node, consider
     * adding them to KisDummiesFacadeBase instead. Calling this method
     * directly  should be the last resort.
     *
     * \see slotUiActivatedNode for comparison
     */
    void slotNonUiActivatedNode(KisNodeSP node);

    /**
     * Activates \p node.
     * All non-ui listeners are notified with sigNodeActivated,
     * sigUiNeedChangeActiveNode is *not* emitted.
     *
     * \see activateNode
     */
    void slotUiActivatedNode(KisNodeSP node);

    /**
     * Adds a node without searching appropriate position for it.
     * You *must* ensure that the node is allowed to be added to
     * the parent, otherwise you'll get an assert.
     */
    void addNodeDirect(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis);

    /**
     * Moves a node without searching appropriate position for it.
     * You *must* ensure that the node is allowed to be added to
     * the parent, otherwise you'll get an assert.
     */
    void moveNodeDirect(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis);


    void toggleIsolateActiveNode();
    void toggleIsolateMode(bool checked);
    void slotUpdateIsolateModeAction();
    void slotTryFinishIsolatedMode();

    void moveNodeAt(KisNodeSP node, KisNodeSP parent, int index);
    void createNode(const QString& nodeType, bool quiet = false, KisPaintDeviceSP copyFrom = 0);
    void convertNode(const QString &nodeType);
    void nodesUpdated();
    void nodeProperties(KisNodeSP node);
    void nodeOpacityChanged(qreal opacity, bool finalChange);
    void nodeCompositeOpChanged(const KoCompositeOp* op);
    void duplicateActiveNode();
    void removeNode();
    void mirrorNodeX();
    void mirrorNodeY();
    void mirrorNode(KisNodeSP node, const KUndo2MagicString& commandName, Qt::Orientation orientation);
    void activateNextNode();
    void activatePreviousNode();

    /**
     * move the active node up the nodestack.
     */
    void raiseNode();

    /**
     * move the active node down the nodestack
     */
    void lowerNode();

    void rotate(double radians);
    void rotate180();
    void rotateLeft90();
    void rotateRight90();

    void saveNodeAsImage();

    // merges the active layer with the layer below it.
    void mergeLayer();

    void slotSplitAlphaIntoMask();
    void slotSplitAlphaWrite();
    void slotSplitAlphaSaveMerged();

    /**
     * @brief slotSetSelectedNodes set the list of nodes selected in the layerbox. Selected nodes are not necessarily active nodes.
     * @param nodes the selected nodes
     */
    void slotSetSelectedNodes(const KisNodeList &nodes);

    void slotImageRequestNodeReselection(KisNodeSP activeNode, const KisNodeList &selectedNodes);

public:

    
    void shear(double angleX, double angleY);

    void scale(double sx, double sy, KisFilterStrategy *filterStrategy);

    void removeSingleNode(KisNodeSP node);
    KisLayerSP constructDefaultLayer();
    KisLayerSP createPaintLayer();

private:
    /**
     * Scales opacity from the range 0...1
     * to the integer range 0...255
     */
    qint32 convertOpacityToInt(qreal opacity);
    void removeSelectedNodes(KisNodeList selectedNodes);
    void slotSomethingActivatedNodeImpl(KisNodeSP node);

    struct Private;
    Private * const m_d;
};

#endif

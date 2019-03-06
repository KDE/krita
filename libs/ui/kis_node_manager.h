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
#include "kis_base_node.h"
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
class KisNodeInsertionAdapter;
class KisNodeDisplayModeAdapter;
class KisNodeJugglerCompressed;
class KoProperties;

/**
 * The node manager passes requests for new layers or masks on to the mask and layer
 * managers.
 */
class KRITAUI_EXPORT KisNodeManager : public QObject
{

    Q_OBJECT

public:

    KisNodeManager(KisViewManager * view);
    ~KisNodeManager() override;

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
    KisNodeInsertionAdapter* nodeInsertionAdapter() const;
    KisNodeDisplayModeAdapter* nodeDisplayModeAdapter() const;

    static bool isNodeHidden(KisNodeSP node, bool isGlobalSelectionHidden);

    bool trySetNodeProperties(KisNodeSP node, KisImageSP image, KisBaseNode::PropertyList properties) const;

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
     * Adds a list of nodes without searching appropriate position for
     * it.  You *must* ensure that the nodes are allowed to be added
     * to the parent, otherwise you'll get an assert.
     */
    void addNodesDirect(KisNodeList nodes, KisNodeSP parent, KisNodeSP aboveThis);

    /**
     * Moves a list of nodes without searching appropriate position
     * for it.  You *must* ensure that the nodes are allowed to be
     * added to the parent, otherwise you'll get an assert.
     */
    void moveNodesDirect(KisNodeList nodes, KisNodeSP parent, KisNodeSP aboveThis);

    /**
     * Copies a list of nodes without searching appropriate position
     * for it.  You *must* ensure that the nodes are allowed to be
     * added to the parent, otherwise you'll get an assert.
     */
    void copyNodesDirect(KisNodeList nodes, KisNodeSP parent, KisNodeSP aboveThis);

    /**
     * Create new layer from actually visible
     */
    void createFromVisible();

    void slotShowHideTimeline(bool value);

    void toggleIsolateActiveNode();
    void toggleIsolateMode(bool checked);
    void slotUpdateIsolateModeAction();
    void slotTryRestartIsolatedMode();

    void moveNodeAt(KisNodeSP node, KisNodeSP parent, int index);
    KisNodeSP createNode(const QString& nodeType, bool quiet = false, KisPaintDeviceSP copyFrom = 0);
    void convertNode(const QString &nodeType);
    void nodesUpdated();
    void nodeProperties(KisNodeSP node);
    void nodeOpacityChanged(qreal opacity, bool finalChange);
    void nodeCompositeOpChanged(const KoCompositeOp* op);
    void duplicateActiveNode();
    void removeNode();
    void mirrorNodeX();
    void mirrorNodeY();
    void mirrorAllNodesX();
    void mirrorAllNodesY();


    void mirrorNode(KisNodeSP node, const KUndo2MagicString& commandName, Qt::Orientation orientation, KisSelectionSP selection);


    void activateNextNode();
    void activatePreviousNode();
    void switchToPreviouslyActiveNode();

    /**
     * move the active node up the nodestack.
     */
    void raiseNode();

    /**
     * move the active node down the nodestack
     */
    void lowerNode();

    void saveNodeAsImage();
    void saveVectorLayerAsImage();

    void slotSplitAlphaIntoMask();
    void slotSplitAlphaWrite();
    void slotSplitAlphaSaveMerged();

    void toggleLock();
    void toggleVisibility();
    void toggleAlphaLock();
    void toggleInheritAlpha();

    /**
     * @brief slotSetSelectedNodes set the list of nodes selected in the layerbox. Selected nodes are not necessarily active nodes.
     * @param nodes the selected nodes
     */
    void slotSetSelectedNodes(const KisNodeList &nodes);

    void slotImageRequestNodeReselection(KisNodeSP activeNode, const KisNodeList &selectedNodes);

    void cutLayersToClipboard();
    void copyLayersToClipboard();
    void pasteLayersFromClipboard();

    void createQuickGroup();
    void createQuickClippingGroup();
    void quickUngroup();

    void selectAllNodes();
    void selectVisibleNodes();
    void selectLockedNodes();
    void selectInvisibleNodes();
    void selectUnlockedNodes();

public:
    void removeSingleNode(KisNodeSP node);
    KisLayerSP createPaintLayer();

private:
    /**
     * Scales opacity from the range 0...1
     * to the integer range 0...255
     */
    qint32 convertOpacityToInt(qreal opacity);
    void removeSelectedNodes(KisNodeList selectedNodes);
    void slotSomethingActivatedNodeImpl(KisNodeSP node);
    void createQuickGroupImpl(KisNodeJugglerCompressed *juggler,
                              const QString &overrideGroupName,
                              KisNodeSP *newGroup,
                              KisNodeSP *newLastChild);
    void selectLayersImpl(const KoProperties &props, const KoProperties &invertedProps);

    struct Private;
    Private * const m_d;
};

#endif

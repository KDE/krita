/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_NODE_MANAGER
#define KIS_NODE_MANAGER

#include <QObject>
#include <QList>
#include <QAction>

#include "kis_types.h"
#include "kis_base_node.h"
#include "kis_image.h"
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
     * Sets the name for the node in a universal way (masks/layers)
     */
    void setNodeName(KisNodeSP node, const QString &name);

    /**
     * Sets opacity for the node in a universal way (masks/layers)
     */
    void setNodeOpacity(KisNodeSP node, qint32 opacity);

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


    bool canModifyLayers(KisNodeList nodes, bool showWarning = true);
    bool canModifyLayer(KisNodeSP node, bool showWarning = true);

    bool canMoveLayers(KisNodeList nodes, bool showWarning = true);
    bool canMoveLayer(KisNodeSP node, bool showWarning = true);

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

    void slotPinToTimeline(bool value);

    // Isolation Mode..

    void toggleIsolateActiveNode();
    void setIsolateActiveLayerMode(bool checked);
    void setIsolateActiveGroupMode(bool checked);

    void changeIsolationMode(bool isolateActiveLayer, bool isolateActiveGroup);
    void changeIsolationRoot(KisNodeSP isolationRoot);

    /**
     * Responds to external changes in isolation mode (i.e. from KisImage).
     */
    void handleExternalIsolationChange();
    void reinitializeIsolationActionGroup();

    // General Node Management..

    void moveNodeAt(KisNodeSP node, KisNodeSP parent, int index);
    KisNodeSP createNode(const QString& nodeType, bool quiet = false, KisPaintDeviceSP copyFrom = 0);
    void convertNode(const QString &nodeType);
    void nodesUpdated();
    void nodeProperties(KisNodeSP node);
    /// pop up a window for changing the source of the selected Clone Layers
    void changeCloneSource();
    void nodeOpacityChanged(qreal opacity);
    void nodeCompositeOpChanged(const KoCompositeOp* op);
    void duplicateActiveNode();
    void removeNode();
    void mirrorNodeX();
    void mirrorNodeY();
    void mirrorAllNodesX();
    void mirrorAllNodesY();

    void mirrorNode(KisNodeSP node, const KUndo2MagicString& commandName, Qt::Orientation orientation, KisSelectionSP selection);

    void activateNextNode(bool siblingsOnly = false);
    void activateNextSiblingNode();
    void activatePreviousNode(bool siblingsOnly = false);
    void activatePreviousSiblingNode();
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
    void pasteLayersFromClipboard(bool changeOffset = false, QPointF offset = QPointF());

    void createQuickGroup();
    void createQuickClippingGroup();
    void quickUngroup();

    void selectAllNodes();
    void selectVisibleNodes();
    void selectLockedNodes();
    void selectInvisibleNodes();
    void selectUnlockedNodes();

private Q_SLOTS:

    friend class KisNodeActivationActionCreatorVisitor;
    /**
     * @brief slotUiActivateNode inspects the sender to see which node needs to be activated.
     */
    void slotUiActivateNode();


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
    bool createQuickGroupImpl(KisNodeJugglerCompressed *juggler,
                              const QString &overrideGroupName,
                              KisNodeSP *newGroup,
                              KisNodeSP *newLastChild);
    void selectLayersImpl(const KoProperties &props, const KoProperties &invertedProps);

    struct Private;
    Private * const m_d;
};

#endif

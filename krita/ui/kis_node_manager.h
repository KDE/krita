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
#include <krita_export.h>

class KAction;
class QAction;
class KToggleAction;
class KActionCollection;

class KoCompositeOp;
class KoColorSpace;
class KoCompositeOp;

class KisDoc2;
class KisFilterStrategy;
class KisView2;
class KisFilterConfiguration;
/**
 * The node manager passes requests for new layers or masks on to the mask and layer
 * managers.
 */
class KRITAUI_EXPORT KisNodeManager : public QObject
{

    Q_OBJECT

public:

    KisNodeManager(KisView2 * view,  KisDoc2 * doc);
    ~KisNodeManager();

    
signals:

    /// emitted whenever a node is selected.
    void sigNodeActivated(KisNodeSP node);
    
    /// emitted whenever a different layer is selected.
    void sigLayerActivated(KisLayerSP layer);
    
    /// for the layer box: this sets the current node in the layerbox
    /// without telling the node manager that the node is activated,
    /// preventing loops (I think...)
    void sigUiNeedChangeActiveNode(KisNodeSP node);
    
public:
    
    void setup(KActionCollection * collection);
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

    /**
     * Explicitly activates \p node
     * The UI will be noticed that active node has been changed.
     *
     * Both sigNodeActivated and sigUiNeedChangeActiveNode are emitted.
     *
     * \see slotUiActivatedNode for comparison
     */
    void activateNode(KisNodeSP node);

public slots:

    /**
     * Activates \p node.
     * All non-ui listeners are notified with sigNodeActivated,
     * sigUiNeedChangeActiveNode is *not* emitted.
     *
     * \see activateNode
     */
    void slotUiActivatedNode(KisNodeSP node);

    void addNode(KisNodeSP node, KisNodeSP activeNode);
    void insertNode(KisNodeSP node, KisNodeSP parent, int index);
    void moveNode(KisNodeSP node, KisNodeSP activeNode);
    void moveNodeAt(KisNodeSP node, KisNodeSP parent, int index);
    void createNode(const QString & node);
    void nodesUpdated();
    void nodeProperties(KisNodeSP node);
    void nodeOpacityChanged(qreal opacity, bool finalChange);
    void nodeCompositeOpChanged(const KoCompositeOp* op);
    void duplicateActiveNode();
    void removeNode(KisNodeSP node);
    void mirrorNodeX();
    void mirrorNodeY();

    /**
     * move the active node up the nodestack.
     */
    void raiseNode();

    /**
     * move the active node down the nodestack
     */
    void lowerNode();

    /**
     * move the activenode to the top-most position of the nodestack
     * If the node is a mask, the stack is limited to the set of masks
     * belonging to the current layer.
     */
    void nodeToTop();

    /**
     * move the activenode to the bottom-most position of the nodestack
     * If the node is a mask, the stack is limited to the set of masks
     * belonging to the current layer.
     */
    void nodeToBottom();
    
    void rotate(double radians);
    void rotate180();
    void rotateLeft90();
    void rotateRight90();

    
public:

    // merges the active layer with the layer below it.
    void mergeLayerDown();
    
    void shear(double angleX, double angleY);

    void scale(double sx, double sy, KisFilterStrategy *filterStrategy);
    
private slots:
    
    // Those slots are used to ensure that the node that was selected remains selected after a move
    void aboutToMoveNode();
    void nodeHasBeenMoved();

private:
    
    void getNewNodeLocation(const QString & nodeType, KisNodeSP &parent, KisNodeSP &above, KisNodeSP active);

    /**
     * Scales opacity from the range 0...1
     * to the integer range 0...255
     */
    qint32 convertOpacityToInt(qreal opacity);

    struct Private;
    Private * const m_d;
    Q_PRIVATE_SLOT(m_d, void slotLayersChanged(KisGroupLayerSP))
};

#endif

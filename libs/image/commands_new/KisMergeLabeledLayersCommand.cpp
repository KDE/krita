/*
 *  SPDX-FileCopyrightText: 2020 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisMergeLabeledLayersCommand.h"

#include "KoCompositeOpRegistry.h"

#include "kis_layer_utils.h"
#include "kis_node.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_layer.h"
#include "kis_clone_layer.h"
#include "kis_paint_layer.h"
#include "kis_assert.h"
#include "KisDeleteLaterWrapper.h"
#include <kis_image_animation_interface.h>

KisMergeLabeledLayersCommand::KisMergeLabeledLayersCommand(KisImageSP image,
                                                           KisPaintDeviceSP newRefPaintDevice,
                                                           QList<int> selectedLabels,
                                                           GroupSelectionPolicy groupSelectionPolicy)
    : KUndo2Command(kundo2_noi18n("MERGE_LABELED_LAYERS"))
    , m_refImage(new KisImage(new KisSurrogateUndoStore(), image->width(), image->height(), image->colorSpace(), "Merge Labeled Layers Reference Image"))
    , m_prevRefNodeInfoList(nullptr)
    , m_newRefNodeInfoList(nullptr)
    , m_prevRefPaintDevice(nullptr)
    , m_newRefPaintDevice(newRefPaintDevice)
    , m_currentRoot(image->root())
    , m_selectedLabels(selectedLabels)
    , m_groupSelectionPolicy(groupSelectionPolicy)
    , m_forceRegeneration(true)
{
    KIS_ASSERT(newRefPaintDevice);
    if (image->animationInterface()->hasAnimation()) {
        m_refImage->animationInterface()->switchCurrentTimeAsync(image->animationInterface()->currentTime());
        m_refImage->waitForDone();
    }
}

KisMergeLabeledLayersCommand::KisMergeLabeledLayersCommand(KisImageSP image,
                                                           ReferenceNodeInfoListSP prevRefNodeInfoList,
                                                           ReferenceNodeInfoListSP newRefNodeInfoList,
                                                           KisPaintDeviceSP prevRefPaintDevice,
                                                           KisPaintDeviceSP newRefPaintDevice,
                                                           QList<int> selectedLabels,
                                                           GroupSelectionPolicy groupSelectionPolicy,
                                                           bool forceRegeneration)
    : KUndo2Command(kundo2_noi18n("MERGE_LABELED_LAYERS"))
    , m_refImage(new KisImage(new KisSurrogateUndoStore(), image->width(), image->height(), image->colorSpace(), "Merge Labeled Layers Reference Image"))
    , m_prevRefNodeInfoList(prevRefNodeInfoList)
    , m_newRefNodeInfoList(newRefNodeInfoList)
    , m_prevRefPaintDevice(prevRefPaintDevice)
    , m_newRefPaintDevice(newRefPaintDevice)
    , m_currentRoot(image->root())
    , m_selectedLabels(selectedLabels)
    , m_groupSelectionPolicy(groupSelectionPolicy)
    , m_forceRegeneration(forceRegeneration)
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(prevRefNodeInfoList);
    KIS_SAFE_ASSERT_RECOVER_NOOP(newRefNodeInfoList);
    KIS_SAFE_ASSERT_RECOVER_NOOP(prevRefPaintDevice);
    KIS_ASSERT(newRefPaintDevice);
    if (image->animationInterface()->hasAnimation()) {
        m_refImage->animationInterface()->switchCurrentTimeAsync(image->animationInterface()->currentTime());
        m_refImage->waitForDone();
    }
}

KisMergeLabeledLayersCommand::~KisMergeLabeledLayersCommand()
{}

void KisMergeLabeledLayersCommand::undo()
{
    KUndo2Command::undo();
}

void KisMergeLabeledLayersCommand::redo()
{
    if (m_refImage) {
        mergeLabeledLayers();
    }
    KUndo2Command::redo();
}

KisPaintDeviceSP KisMergeLabeledLayersCommand::createRefPaintDevice(KisImageSP originalImage, QString name)
{
    return KisPaintDeviceSP(new KisPaintDevice(originalImage->colorSpace(), name));
}

QPair<KisNodeSP, QPair<bool, bool>> KisMergeLabeledLayersCommand::collectNode(KisNodeSP node) const
{
    KisNodeSP transformedNode = nullptr;
    bool visitNextSibling, visitChildren;

    if (!node->visible()) {
        // Do not visit the children if the node is hidden but visit the next sibling
        visitNextSibling = true;
        visitChildren = false;

    } else if (!m_selectedLabels.contains(node->colorLabelIndex())) {
        // If the node is not labeled appropriately it's children are still
        // visited if it is a group, as well as the next sibling
        visitNextSibling = true;
        visitChildren = true;

    } else if (node->inherits("KisCloneLayer")) {
        // Make a copy of the clone layer as a paint layer. The source layer
        // may not be color labeled and therefore not added to the temporary
        // image. So this ensures that the real contents are represented
        // in any case
        KisCloneLayerSP cloneLayer = dynamic_cast<KisCloneLayer*>(node.data());
        transformedNode = cloneLayer->reincarnateAsPaintLayer();
        // The next sibling is visited. Although this is not a group
        // and does not have children, visitChildren is initialized to true
        visitNextSibling = true;
        visitChildren = true;

    } else if (node->inherits("KisAdjustmentLayer")) {
        // Similar to the clone layer case. The filter layer uses the
        // composition of the layers below as source to apply the effect
        // so we must use a paint layer representation of the result
        // because the next sibling nodes may not be color labeled and
        // therefore not added to the temporary image.
        KisPaintDeviceSP proj = new KisPaintDevice(*(node->projection()));
        KisPaintLayerSP newLayer = new KisPaintLayer(node->image(), node->name(), node->opacity(), proj);
        newLayer->setX(newLayer->x() + node->x());
        newLayer->setY(newLayer->y() + node->y());
        newLayer->mergeNodeProperties(node->nodeProperties());
        transformedNode = newLayer;
        // This new node already has the contents of the next layers on
        // the same level baked, so the next sibling nodes are not visited.
        // Although this is not a group and does not have children,
        // visitChildren is initialized to true
        visitNextSibling = false;
        visitChildren = true;

    } else if (node->inherits("KisGroupLayer")) {
        if (m_groupSelectionPolicy == GroupSelectionPolicy_NeverSelect ||
            (m_groupSelectionPolicy == GroupSelectionPolicy_SelectIfColorLabeled && node->colorLabelIndex() == 0)) {
            // If the node is a group and should not be selected, it's children
            // are still visited as well as the next sibling
            visitNextSibling = true;
            visitChildren = true;

        } else {
            // Just add the group node, which adds also all the children
            transformedNode = node;
            // This group node already has all the children, so the child nodes
            // are not visited. The next sibling is visited
            visitNextSibling = true;
            visitChildren = false;
        }
    } else {
        transformedNode = node;
        visitNextSibling = true;
        visitChildren = true;
    }
    
    return {transformedNode, {visitNextSibling, visitChildren}};
}

bool KisMergeLabeledLayersCommand::collectNodes(KisNodeSP node, QList<KisNodeSP> &nodeList, ReferenceNodeInfoList &nodeInfoList) const
{
    QPair<KisNodeSP, QPair<bool, bool>> result = collectNode(node);
    KisNodeSP collectedNode = result.first;
    const bool visitNextSibling = result.second.first;
    const bool visitChildren = result.second.second;

    if (collectedNode) {
        // If the node should be selected, it is added to the list
        nodeList << collectedNode;
        // Store additional info to check if the new list of reference nodes
        // is different. Use the original node to extract the info
        if (hastToCheckForChangesInNodes()) {
            const QUuid uuid = node->uuid();
            const int sequenceNumber = node->projection()->sequenceNumber();
            const int opacity = node->opacity();
            nodeInfoList.append({uuid, sequenceNumber, opacity});
        }
    }

    if (visitChildren) {
        node = node->lastChild();
        while (node) {
            const bool mustVisitNextSibling = collectNodes(node, nodeList, nodeInfoList);
            if (!mustVisitNextSibling) {
                break;
            }
            node = node->prevSibling();
        }
    }
    
    return visitNextSibling;
}

void KisMergeLabeledLayersCommand::mergeLabeledLayers()
{
    QList<KisNodeSP> currentNodesList;
    ReferenceNodeInfoList currentNodeInfoList;

    collectNodes(m_currentRoot, currentNodesList, currentNodeInfoList);

    if (hastToCheckForChangesInNodes()) {
        *m_newRefNodeInfoList = currentNodeInfoList;
    }
    
    if (hastToCheckForChangesInNodes() && !m_forceRegeneration && (currentNodeInfoList == *m_prevRefNodeInfoList)) {
        m_newRefPaintDevice->prepareClone(m_prevRefPaintDevice);
        m_newRefPaintDevice->makeCloneFromRough(m_prevRefPaintDevice, m_prevRefPaintDevice->extent());
    } else {
        QList<KisNodeSP> currentNodesListCopy;
        for (KisNodeSP node : currentNodesList) {
            KisNodeSP copy = node->clone();

            if (copy.isNull()) {
                continue;
            }

            if (copy->inherits("KisLayer")) {
                KisLayer* layerCopy = dynamic_cast<KisLayer*>(copy.data());
                KIS_ASSERT(layerCopy);
                layerCopy->setChannelFlags(QBitArray());
            }

            copy->setCompositeOpId(COMPOSITE_OVER);

            bool success = m_refImage->addNode(copy, m_refImage->root(), 0);

            if (!success) {
                continue;
            }

            currentNodesListCopy << copy;
        }

        currentNodesListCopy = KisLayerUtils::sortAndFilterAnyMergeableNodesSafe(currentNodesListCopy, m_refImage);
        m_refImage->initialRefreshGraph();
        KisLayerUtils::refreshHiddenAreaAsync(m_refImage, m_refImage->root(), m_refImage->bounds());
        m_refImage->waitForDone();

        if (m_refImage->root()->childCount() == 0) {
            return;
        }

        m_refImage->waitForDone();
        m_refImage->mergeMultipleLayers(currentNodesListCopy, 0);
        m_refImage->waitForDone();

        KisPainter::copyAreaOptimized(m_refImage->projection()->exactBounds().topLeft(), m_refImage->projection(), m_newRefPaintDevice, m_refImage->projection()->exactBounds());
    }

    // release resources: they are still owned by the caller
    // (or by some other object the caller passed them to)
    m_prevRefPaintDevice.clear();
    m_newRefPaintDevice.clear();
    m_currentRoot.clear();
    m_prevRefNodeInfoList.clear();
    m_newRefNodeInfoList.clear();

    // KisImage should be deleted only in the GUI thread (it has timers)
    makeKisDeleteLaterWrapper(m_refImage)->deleteLater();
    m_refImage.clear();
}

bool KisMergeLabeledLayersCommand::hastToCheckForChangesInNodes() const
{
    return m_prevRefNodeInfoList && m_newRefNodeInfoList && m_prevRefPaintDevice;
}

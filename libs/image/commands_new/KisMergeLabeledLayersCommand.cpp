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

void KisMergeLabeledLayersCommand::mergeLabeledLayers()
{
    QList<KisNodeSP> currentNodesList;
    ReferenceNodeInfoList currentNodeInfoList;
    KisImageSP refImage = m_refImage;

    KisLayerUtils::recursiveApplyNodes(
        m_currentRoot,
        [&currentNodesList, &currentNodeInfoList, this] (KisNodeSP node) mutable
        {
            if (!acceptNode(node)) {
                return;
            }

            currentNodesList << node;

            if (checkChangesInNodes()) {
                const QUuid uuid = node->uuid();
                const int sequenceNumber = node->projection()->sequenceNumber();
                const bool isVisible = node->visible();
                const int opacity = node->opacity();
                currentNodeInfoList.append({uuid, sequenceNumber, isVisible, opacity});
            }
        }
    );

    if (checkChangesInNodes()) {
        *m_newRefNodeInfoList = currentNodeInfoList;
    }
    
    if (checkChangesInNodes() && !m_forceRegeneration && (currentNodeInfoList == *m_prevRefNodeInfoList)) {
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
                layerCopy->setChannelFlags(QBitArray());
            }

            copy->setCompositeOpId(COMPOSITE_OVER);

            bool success = refImage->addNode(copy, refImage->root());

            if (!success) {
                continue;
            }

            currentNodesListCopy << copy;
        }

        currentNodesListCopy = KisLayerUtils::sortAndFilterAnyMergableNodesSafe(currentNodesListCopy, m_refImage);
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

bool KisMergeLabeledLayersCommand::acceptNode(KisNodeSP node) const
{
    if (node->inherits("KisGroupLayer") &&
        (m_groupSelectionPolicy == GroupSelectionPolicy_NeverSelect ||
          (m_groupSelectionPolicy == GroupSelectionPolicy_SelectIfColorLabeled &&
           node->colorLabelIndex() == 0))) {
        return false;
    }
    return m_selectedLabels.contains(node->colorLabelIndex());
}

bool KisMergeLabeledLayersCommand::checkChangesInNodes() const
{
    return m_prevRefNodeInfoList && m_newRefNodeInfoList && m_prevRefPaintDevice;
}

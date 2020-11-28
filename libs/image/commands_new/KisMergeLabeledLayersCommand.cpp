/*
 *  SPDX-FileCopyrightText: 2020 Agata Cacko <cacko.azh@gmail.com>
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
#include "KisDeleteLaterWrapper.h"



KisMergeLabeledLayersCommand::KisMergeLabeledLayersCommand(KisImageSP refImage, KisPaintDeviceSP refPaintDevice,
                                                           KisNodeSP currentRoot, QList<int> selectedLabels)
    : KUndo2Command(kundo2_noi18n("MERGE_LABELED_LAYERS"))
    , m_refImage(refImage)
    , m_refPaintDevice(refPaintDevice)
    , m_currentRoot(currentRoot)
    , m_selectedLabels(selectedLabels)
{
}

KisMergeLabeledLayersCommand::~KisMergeLabeledLayersCommand()
{
}

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

KisImageSP KisMergeLabeledLayersCommand::createRefImage(KisImageSP originalImage, QString name = "Reference Image")
{
    return KisImageSP(new KisImage(new KisSurrogateUndoStore(), originalImage->width(), originalImage->height(),
                                   originalImage->colorSpace(), name));
}

KisPaintDeviceSP KisMergeLabeledLayersCommand::createRefPaintDevice(KisImageSP originalImage, QString name  = "Reference Result Paint Device")
{
    return KisPaintDeviceSP(new KisPaintDevice(originalImage->colorSpace(), name));
}

void KisMergeLabeledLayersCommand::mergeLabeledLayers()
{
    QList<KisNodeSP> nodesList;

    KisImageSP refImage = m_refImage;
    KisLayerUtils::recursiveApplyNodes(m_currentRoot, [&nodesList, refImage, this] (KisNodeSP node) mutable {
        if (acceptNode(node)) {
            KisNodeSP copy = node->clone();

            if (copy.isNull()) {
                return;
            }

            if (node->inherits("KisLayer")) {
                KisLayer* layerCopy = dynamic_cast<KisLayer*>(copy.data());
                layerCopy->setChannelFlags(QBitArray());
            }

            copy->setCompositeOpId(COMPOSITE_OVER);

            bool success = refImage->addNode(copy, refImage->root());

            if (!success) {
                return;
            }
            nodesList << copy;
        }

    });

    nodesList = KisLayerUtils::sortAndFilterAnyMergableNodesSafe(nodesList, m_refImage);
    m_refImage->initialRefreshGraph();
    KisLayerUtils::refreshHiddenAreaAsync(m_refImage, m_refImage->root(), m_refImage->bounds());
    m_refImage->waitForDone();

    if (m_refImage->root()->childCount() == 0) {
        return;
    }

    m_refImage->waitForDone();
    m_refImage->mergeMultipleLayers(nodesList, 0);
    m_refImage->waitForDone();


    KisPainter::copyAreaOptimized(m_refImage->projection()->exactBounds().topLeft(), m_refImage->projection(), m_refPaintDevice, m_refImage->projection()->exactBounds());

    // release resources: they are still owned by the caller
    // (or by some other object the caller passed them to)
    m_refPaintDevice.clear();
    m_currentRoot.clear();

    // KisImage should be deleted only in the GUI thread (it has timers)
    makeKisDeleteLaterWrapper(m_refImage)->deleteLater();
    m_refImage.clear();

}

bool KisMergeLabeledLayersCommand::acceptNode(KisNodeSP node)
{
    return m_selectedLabels.contains(node->colorLabelIndex());
}

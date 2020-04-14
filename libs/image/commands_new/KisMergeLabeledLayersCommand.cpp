/*
 *  Copyright (c) 2020 Agata Cacko <cacko.azh@gmail.com>
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

#include "KisMergeLabeledLayersCommand.h"

#include "KoCompositeOpRegistry.h"

#include "kis_layer_utils.h"
#include "kis_node.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_layer.h"




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
    mergeLabeledLayers();
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

    if (m_refImage->root()->childCount() == 0) {
        return;
    }

    m_refImage->waitForDone();
    m_refImage->mergeMultipleLayers(nodesList, 0);
    m_refImage->waitForDone();

    KisPainter::copyAreaOptimized(QPoint(), m_refImage->projection(), m_refPaintDevice, m_refImage->bounds());
}

bool KisMergeLabeledLayersCommand::acceptNode(KisNodeSP node)
{
    return m_selectedLabels.contains(node->colorLabelIndex());
}

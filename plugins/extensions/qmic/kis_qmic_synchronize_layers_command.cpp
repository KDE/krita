/*
 * SPDX-FileCopyrightText: 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 * SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_qmic_synchronize_layers_command.h"

#include <tuple>
#include <utility>

#include <KisDocument.h>
#include <KisMainWindow.h>
#include <KisPart.h>
#include <KisViewManager.h>
#include <KoColorSpaceConstants.h>
#include <commands/kis_image_layer_add_command.h>
#include <kis_command_utils.h>
#include <kis_image.h>
#include <kis_layer_utils.h>
#include <kis_node_manager.h>
#include <kis_node_selection_adapter.h>
#include <kis_paint_layer.h>
#include <kis_selection.h>
#include <kis_surrogate_undo_adapter.h>
#include <kis_transaction.h>
#include <kis_types.h>

#include "kis_qmic_import_tools.h"

struct Q_DECL_HIDDEN KisQmicSynchronizeLayersCommand::Private {
    Private(KisNodeListSP nodes,
            QVector<KisQMicImageSP> images,
            KisImageWSP image,
            const QRect &dstRect,
            KisSelectionSP selection)
        : m_nodes(std::move(nodes))
        , m_newNodes(new KisNodeList())
        , m_images(std::move(images))
        , m_image(image)
        , m_dstRect(dstRect)
        , m_selection(selection)
    {
    }

    ~Private()
    {
        qDeleteAll(m_imageCommands);
        m_imageCommands.clear();
    }

    KisNodeListSP m_nodes;
    KisNodeListSP m_newNodes;
    QVector<KisQMicImageSP> m_images;
    KisImageWSP m_image;
    QRect m_dstRect;
    KisSelectionSP m_selection;
    QVector<KisImageCommand *> m_imageCommands;
    KisSurrogateUndoAdapter m_undoAdapter;
    bool m_firstRedo;
};

KisQmicSynchronizeLayersCommand::KisQmicSynchronizeLayersCommand(
    KisNodeListSP nodes,
    QVector<KisQMicImageSP> images,
    KisImageWSP image,
    const QRect &dstRect,
    KisSelectionSP selection)
    : KisCommandUtils::CompositeCommand()
    , d(new Private{std::move(nodes),
                    std::move(images),
                    image,
                    dstRect,
                    selection})
{
    dbgPlugins << "KisQmicSynchronizeLayersCommand";
}

KisQmicSynchronizeLayersCommand::~KisQmicSynchronizeLayersCommand()
{
    delete d;
}

void KisQmicSynchronizeLayersCommand::redo()
{
    dbgPlugins << "KisQmicSynchronizeLayersCommand::Redo";

    const auto pickAboveThis = [&]() {
        if (!d->m_newNodes->isEmpty()) {
            return d->m_newNodes->last()->prevSibling();
        } else {
            return d->m_nodes->last()->prevSibling();
        }
    };

    const auto getNode = [&](int i) {
        if (i >= d->m_nodes->size()) {
            return d->m_newNodes->at(i - d->m_nodes->size());
        } else {
            return d->m_nodes->at(i);
        }
    };

    KisNodeManager *nodeManager = [&]() -> KisNodeManager * {
        KisMainWindow *mainWin = KisPart::instance()->currentMainwindow();
        if (!mainWin)
            return nullptr;
        KisViewManager *viewManager = mainWin->viewManager();
        if (!viewManager)
            return nullptr;
        if (viewManager->document()->image() != d->m_image)
            return nullptr;
        return viewManager->nodeManager();
    }();

    if (!nodeManager)
        return;

    auto *selectOldLayer = new KisLayerUtils::KeepNodesSelectedCommand(
        nodeManager->selectedNodes(),
        nodeManager->selectedNodes(),
        nodeManager->activeNode(),
        nodeManager->activeNode(),
        d->m_image,
        false);

    selectOldLayer->redo();

    addCommand(new KisCommandUtils::SkipFirstRedoWrapper(selectOldLayer));

    // if gmic produces more layers
    if (d->m_nodes->size() < d->m_images.size()) {
        if (d->m_image) {
            const QPoint origin = [&]() -> QPoint {
                if (d->m_selection) {
                    return d->m_selection->selectedExactRect().topLeft();
                } else if (!d->m_nodes->isEmpty()) {
                    const auto root = d->m_nodes->at(0);
                    return {root->x(), root->y()};
                } else {
                    return {};
                }
            }();
            const int nodesCount = d->m_nodes->size();
            for (int i = nodesCount; i < d->m_images.size(); i++) {
                KisPaintDeviceSP device = new KisPaintDevice(d->m_image->colorSpace());
                KisLayerSP paintLayer = new KisPaintLayer(d->m_image, QString("New layer %1 from gmic filter").arg(i), OPACITY_OPAQUE_U8, device);
                paintLayer->setX(origin.x());
                paintLayer->setY(origin.y());

                KisQmicImportTools::gmicImageToPaintDevice(
                    *d->m_images[i].data(),
                    device,
                    d->m_selection,
                    d->m_dstRect);

                KisNodeSP aboveThis(nullptr);
                KisNodeSP parent(nullptr);

                if (nodesCount > 0) {
                    // This node is a copy made by GMic of an existing node;
                    // give it its name back (the existing node will be reused
                    // by KisImportQmicProcessingVisitor)
                    paintLayer->setName(getNode(i - nodesCount)->name());
                    aboveThis = pickAboveThis();
                    parent = d->m_nodes->at(0)->parent();

                    dbgPlugins << "Adding paint layer" << (i - nodesCount + 1) << paintLayer << "to parent" << parent->name() << "above" << aboveThis;
                }

                auto *layerNameCmd =
                    KisQmicImportTools::applyLayerNameChanges(*d->m_images[i],
                                                              paintLayer.data(),
                                                              d->m_selection);
                layerNameCmd->redo();

                addCommand(new KisCommandUtils::SkipFirstRedoWrapper(layerNameCmd));

                auto *addLayerCmd = new KisImageLayerAddCommand(d->m_image, paintLayer, parent, aboveThis, true, true);

                addLayerCmd->redo();
                addCommand(new KisCommandUtils::SkipFirstRedoWrapper(addLayerCmd));
                d->m_newNodes->append(paintLayer);
            }
        } else // small preview
        {
            Q_ASSERT(!d->m_nodes->empty());
            for (int i = d->m_nodes->size(); i < d->m_images.size(); i++) {
                KisPaintDeviceSP device = new KisPaintDevice(d->m_nodes->at(0)->colorSpace());
                KisLayerSP paintLayer = new KisPaintLayer(nullptr, "New layer from gmic filter", OPACITY_OPAQUE_U8, device);
                d->m_newNodes->append(paintLayer);
            }
        }
    } else if (d->m_nodes->size() > d->m_images.size()) {
        // if gmic produces less layers, we are going to drop some

        const KisNodeList extraNodes = [&]() {
            KisNodeList result;
            const auto minIndex = d->m_images.size();
            for (auto i = minIndex; i < d->m_nodes->size(); i++) {
                result.append(d->m_nodes->at(i));
            }
            return result;
        }();

        auto *removeLayerCmd = new KisLayerUtils::SimpleRemoveLayers(extraNodes, d->m_image);
        removeLayerCmd->redo();
        addCommand(new KisCommandUtils::SkipFirstRedoWrapper(removeLayerCmd));
    }

    const int boundary = std::min(d->m_nodes->size(), d->m_images.size());

    for (int index = 0; index < boundary; index++) {
        KisNodeSP node = d->m_nodes->at(index);

        const KisQMicImageSP &gimg = d->m_images.at(index);
        dbgPlugins << "Importing layer index" << index
                << "Size: " << gimg->m_width << "x" << gimg->m_height
                << "colorchannels: " << gimg->m_spectrum;

        KisPaintDeviceSP dst = node->paintDevice();

        const auto *layer = dynamic_cast<const KisLayer *>(node.data());
        const KisSelectionSP selection =
            layer ? layer->selection() : d->m_selection;

        KisTransaction transaction(dst);
        KisQmicImportTools::gmicImageToPaintDevice(*gimg,
                                                   dst,
                                                   selection,
                                                   d->m_dstRect);
        auto *layerNameCmd =
            KisQmicImportTools::applyLayerNameChanges(*gimg,
                                                      node.data(),
                                                      selection);
        layerNameCmd->redo();

        addCommand(new KisCommandUtils::SkipFirstRedoWrapper(layerNameCmd));

        transaction.commit(&d->m_undoAdapter);
        node->setDirty(d->m_dstRect);
    }

    KisNodeSP currentNode = [&]() -> KisNodeSP {
        if (!d->m_nodes->empty()) {
            return d->m_nodes->first();
        } else if (!d->m_newNodes->empty()) {
            return d->m_newNodes->first();
        }
        return {};
    }();

    if (!currentNode)
        return;

    auto *selectNewLayer = new KisLayerUtils::KeepNodesSelectedCommand(
        nodeManager->selectedNodes(),
        {currentNode},
        nodeManager->activeNode(),
        currentNode,
        d->m_image,
        true);

    selectNewLayer->redo();

    addCommand(new KisCommandUtils::SkipFirstRedoWrapper(selectNewLayer));

    dbgPlugins << "Set selected node to" << currentNode->name();
}

void KisQmicSynchronizeLayersCommand::undo()
{
    KisCommandUtils::CompositeCommand::undo();
    d->m_undoAdapter.undoAll();
    d->m_newNodes->clear();
}

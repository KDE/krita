/*
 * SPDX-FileCopyrightText: 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_qmic_synchronize_layers_command.h"

#include <QList>
#include <QSharedPointer>
#include <utility>

#include <KoColorSpaceConstants.h>
#include <commands/kis_image_layer_add_command.h>
#include <kis_command_utils.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_selection.h>
#include <kis_types.h>

#include "kis_qmic_import_tools.h"

struct Q_DECL_HIDDEN KisQmicSynchronizeLayersCommand::Private {
    Private(KisNodeListSP nodes, QVector<gmic_image<float> *> images, KisImageWSP image, const QRect &dstRect, KisSelectionSP selection)
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
    QVector<gmic_image<float> *> m_images;
    KisImageWSP m_image;
    QRect m_dstRect;
    KisSelectionSP m_selection;
    QVector<KisImageCommand *> m_imageCommands;
    bool m_firstRedo;
};

KisQmicSynchronizeLayersCommand::KisQmicSynchronizeLayersCommand(KisNodeListSP nodes,
                                                                 QVector<gmic_image<float> *> images,
                                                                 KisImageWSP image,
                                                                 const QRect &dstRect,
                                                                 KisSelectionSP selection)
    : KisCommandUtils::CompositeCommand()
    , d(new Private{std::move(nodes), std::move(images), image, dstRect, selection})
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

    // if gmic produces more layers
    if (d->m_nodes->size() < d->m_images.size()) {
        if (d->m_image) {
            const auto nodesCount = d->m_nodes->size();
            for (int i = nodesCount; i < d->m_images.size(); i++) {
                KisPaintDeviceSP device = new KisPaintDevice(d->m_image->colorSpace());
                KisLayerSP paintLayer = new KisPaintLayer(d->m_image, QString("New layer %1 from gmic filter").arg(i), OPACITY_OPAQUE_U8, device);

                KisQmicImportTools::gmicImageToPaintDevice(*d->m_images[i], device, d->m_selection, d->m_dstRect);

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

                auto *layerNameCmd = KisQmicImportTools::applyLayerNameChanges(*d->m_images[i], paintLayer.data());
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
    } else if (d->m_nodes->size() > int(d->m_images.size())) {
        // if gmic produces less layers, we are going to drop some
        errPlugins << "no support for removing layers from G'MIC yet!!";
    }
}

void KisQmicSynchronizeLayersCommand::undo()
{
    KisCommandUtils::CompositeCommand::undo();
    d->m_newNodes->clear();
}

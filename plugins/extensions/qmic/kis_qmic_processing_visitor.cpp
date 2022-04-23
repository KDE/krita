/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <utility>

#include <kis_debug.h>
#include <kis_node.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>
#include <kis_selection.h>
#include <kis_transaction.h>
#include <kis_types.h>

#include "gmic.h"
#include "kis_qmic_import_tools.h"
#include "kis_qmic_processing_visitor.h"

struct Q_DECL_HIDDEN KisQmicProcessingVisitor::Private {
    Private(const KisNodeListSP nodes, QVector<gmic_image<float> *> images, const QRect &dstRect, KisSelectionSP selection)
        : m_nodes(nodes)
        , m_images(std::move(images))
        , m_dstRect(dstRect)
        , m_selection(selection)
    {
    }

    ~Private() = default;

    const KisNodeListSP m_nodes;
    QVector<gmic_image<float> *> m_images;
    QRect m_dstRect;
    const KisSelectionSP m_selection;
};

KisQmicProcessingVisitor::KisQmicProcessingVisitor(const KisNodeListSP nodes,
                                                               QVector<gmic_image<float> *> images,
                                                               const QRect &dstRect,
                                                               KisSelectionSP selection)
    : d(new Private{nodes, std::move(images), dstRect, selection})
{
    dbgPlugins << "KisImportQmicProcessingVisitor";
}

KisQmicProcessingVisitor::~KisQmicProcessingVisitor()
{
    delete d;
}

void KisQmicProcessingVisitor::visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter)
{
    int index = d->m_nodes->indexOf(node);
    if (index >= 0 && index < d->m_images.size()) {
        const auto *gimg = d->m_images[index];
        dbgPlugins << "Importing layer index" << index << "Size: " << gimg->_width << "x" << gimg->_height << "colorchannels: " << gimg->_spectrum;

        auto dst = node->paintDevice();

        const auto *layer = dynamic_cast<KisLayer *>(node);
        const KisSelectionSP selection = layer ? layer->selection() : d->m_selection;

        KisTransaction transaction(dst);
        KisQmicImportTools::gmicImageToPaintDevice(*gimg, dst, selection, d->m_dstRect);
        if (undoAdapter) {
            undoAdapter->addCommand(KisQmicImportTools::applyLayerNameChanges(*gimg, node));
            transaction.commit(undoAdapter);
            node->setDirty(d->m_dstRect);
        }
    }
}

void KisQmicProcessingVisitor::visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(layer);
    Q_UNUSED(undoAdapter);
}

void KisQmicProcessingVisitor::visitColorizeMask(KisColorizeMask *mask, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(mask);
    Q_UNUSED(undoAdapter);
}

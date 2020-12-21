/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */



#include <kis_transaction.h>
#include <kis_paint_device.h>
#include <kis_debug.h>

#include "kis_qmic_simple_convertor.h"
#include <kis_node.h>
#include <kis_painter.h>
#include <commands/kis_image_layer_add_command.h>
#include <kis_paint_layer.h>
#include <KoCompositeOpRegistry.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <QtCore/QRegularExpression>

#include "kis_import_qmic_processing_visitor.h"
#include "gmic.h"

KisImportQmicProcessingVisitor::KisImportQmicProcessingVisitor(const KisNodeListSP nodes, QVector<gmic_image<float> *> images, const QRect &dstRect, KisSelectionSP selection)
    : m_nodes(nodes),
      m_images(images),
      m_dstRect(dstRect),
      m_selection(selection)
{
    dbgPlugins << "KisImportQmicProcessingVisitor";
}


void KisImportQmicProcessingVisitor::gmicImageToPaintDevice(gmic_image<float>& srcGmicImage,
                                                            KisPaintDeviceSP dst, KisSelectionSP selection, const QRect &dstRect)
{

    dbgPlugins << "KisImportQmicProcessingVisitor::gmicImageToPaintDevice();";
    if (selection) {
        KisPaintDeviceSP src = new KisPaintDevice(dst->colorSpace());
        KisQmicSimpleConvertor::convertFromGmicFast(srcGmicImage, src, 255.0f);

        KisPainter painter(dst, selection);
        painter.setCompositeOp(COMPOSITE_COPY);
        painter.bitBlt(dstRect.topLeft(), src, QRect(QPoint(0,0),dstRect.size()));
    }
    else {
        KisQmicSimpleConvertor::convertFromGmicFast(srcGmicImage, dst, 255.0f);
    }
}

void KisImportQmicProcessingVisitor::applyLayerNameChanges(const gmic_image<float> &srcGmicImage, KisNode *node, KisPaintDeviceSP dst)
{
    dbgPlugins << "Layer name: " << srcGmicImage.name;

    {
        const QRegExp modeRe("mode\\(\\s*([^)]*)\\s*\\)");
        if (modeRe.indexIn(srcGmicImage.name) != -1) {
            QString modeStr = modeRe.cap(1).trimmed();
            auto translatedMode = KisQmicSimpleConvertor::stringToBlendingMode(modeStr);
            dbgPlugins << "Detected mode: " << modeStr << " => " << translatedMode;
            if (!translatedMode.isNull())
                node->setCompositeOpId(translatedMode);
        }
    }

    {
        const QRegExp opacityRe("opacity\\(\\s*([^)]*)\\s*\\)");

        if (opacityRe.indexIn(srcGmicImage.name) != -1) {
            const auto opacity = opacityRe.cap(1).toUInt();
            dbgPlugins << "Detected opacity: " << opacity;
            node->setPercentOpacity(opacity);
        }
    }

    {
        const QRegExp nameRe("name\\(\\s*([^)]*)\\s*\\)");

        if (nameRe.indexIn(srcGmicImage.name) != -1) {
            const auto name = nameRe.cap(1);
            dbgPlugins << "Detected layer name: " << name;
            node->setName(name);
        }
    }

    // Some GMic filters encode layer position into the layer name.
    // E.g. from extract foreground: "name([unnamed] [foreground]),pos(55,35)"
    const QRegularExpression positionPattern(R"(\Wpos\((\d+),(\d+)\))");
    const QRegularExpressionMatch match = positionPattern.match(srcGmicImage.name);
    if (match.hasMatch()) {
        int x = match.captured(1).toInt();
        int y = match.captured(2).toInt();
        dst->moveTo(x, y);
    }
}

void KisImportQmicProcessingVisitor::visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter)
{
    int index = m_nodes->indexOf(node);
    if (index >= 0 && index < m_images.size()) {
        gmic_image<float> *gimg = m_images[index];
        dbgPlugins << "Importing layer index" << index << "Size: "<< gimg->_width << "x" << gimg->_height << "colorchannels: " << gimg->_spectrum;

        KisPaintDeviceSP dst = node->paintDevice();

        const KisLayer *layer = dynamic_cast<KisLayer*>(node);
        const KisSelectionSP selection = layer ? layer->selection() : m_selection;

        KisTransaction transaction(dst);
        KisImportQmicProcessingVisitor::gmicImageToPaintDevice(*gimg, dst, selection, m_dstRect);
        KisImportQmicProcessingVisitor::applyLayerNameChanges(*gimg, node, dst);
        if (undoAdapter) {
            transaction.commit(undoAdapter);
            node->setDirty(m_dstRect);
        }
    }
}

void KisImportQmicProcessingVisitor::visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(layer);
    Q_UNUSED(undoAdapter);
}

void KisImportQmicProcessingVisitor::visitColorizeMask(KisColorizeMask *mask, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(mask);
    Q_UNUSED(undoAdapter);
}

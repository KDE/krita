/*
 * SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 * SPDX-FileCopyrightText: 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QRegularExpression>

#include <KoCompositeOpRegistry.h>
#include <commands/KisNodeRenameCommand.h>
#include <commands/kis_node_compositeop_command.h>
#include <commands/kis_node_opacity_command.h>
#include <commands_new/kis_node_move_command2.h>
#include <kis_command_utils.h>
#include <kis_layer_utils.h>
#include <kis_node.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>
#include <kis_selection.h>
#include <kis_types.h>

#include "gmic.h"
#include "kis_qmic_import_tools.h"
#include "kis_qmic_interface.h"
#include "kis_qmic_simple_convertor.h"

namespace KisQmicImportTools
{
[[nodiscard]] KUndo2Command *
applyLayerNameChanges(const KisQMicImage &srcGmicImage,
                      KisNode *node,
                      KisSelectionSP selection)
{
    dbgPlugins << "KisQmicImportTools::applyLayerNameChanges";

    auto *cmd = new KisCommandUtils::CompositeCommand();

    dbgPlugins << "Layer name: " << srcGmicImage.m_layerName;

    {
        const QRegExp modeRe(R"(mode\(\s*([^)]*)\s*\))");
        if (modeRe.indexIn(srcGmicImage.m_layerName) != -1) {
            QString modeStr = modeRe.cap(1).trimmed();
            auto translatedMode =
                KisQmicSimpleConvertor::stringToBlendingMode(modeStr);
            dbgPlugins << "Detected mode: " << modeStr << " => "
                       << translatedMode;
            if (!translatedMode.isNull()) {
                cmd->addCommand(
                    new KisNodeCompositeOpCommand(node, translatedMode));
            }
        }
    }

    {
        const QRegExp opacityRe(R"(opacity\(\s*([^)]*)\s*\))");

        if (opacityRe.indexIn(srcGmicImage.m_layerName) != -1) {
            const auto opacity = opacityRe.cap(1).toFloat();
            dbgPlugins << "Detected opacity: " << opacity
                       << std::lround(opacity / 100.f * 255.f);
            cmd->addCommand(
                new KisNodeOpacityCommand(node,
                                          static_cast<quint8>(std::lround(
                                              float(opacity * 255) / 100.f))));
        }
    }

    {
        const QRegExp nameRe(R"(name\(\s*([^)]*)\s*\))");

        if (nameRe.indexIn(srcGmicImage.m_layerName) != -1) {
            const auto name = nameRe.cap(1);
            dbgPlugins << "Detected layer name: " << name;
            cmd->addCommand(new KisNodeRenameCommand(node, node->name(), name));
            // apply command
        }
    }

    if (!selection) {
        // Some GMic filters encode layer position into the layer name.
        // E.g. from extract foreground: "name([unnamed]
        // [foreground]),pos(55,35)"
        const QRegularExpression positionPattern(
            R"(pos\(\s*(-?\d*)[^)](-?\d*)\s*\))");
        const QRegularExpressionMatch match =
            positionPattern.match(srcGmicImage.m_layerName);
        if (match.hasMatch()) {
            const auto x = match.captured(1).toInt();
            const auto y = match.captured(2).toInt();
            const QPoint oldPos(node->x(), node->y());
            const QPoint newPos(x, y);
            dbgPlugins << "Detected layer position: " << oldPos << newPos
                       << node->paintDevice()->exactBounds();
            cmd->addCommand(new KisNodeMoveCommand2(node, oldPos, newPos));
        }
    }

    return cmd;
}

void gmicImageToPaintDevice(const KisQMicImage &srcGmicImage,
                            KisPaintDeviceSP dst,
                            KisSelectionSP selection,
                            const QRect &dstRect)
{
    dbgPlugins << "KisQmicImportTools::gmicImageToPaintDevice()" << dstRect;

    if (selection) {
        KisPaintDeviceSP src = new KisPaintDevice(dst->colorSpace());
        KisQmicSimpleConvertor::convertFromGmicFast(srcGmicImage, src, 255.0f);
        KisPainter painter(dst, selection);
        painter.setCompositeOpId(COMPOSITE_COPY);
        painter.bitBlt(dstRect.topLeft(),
                       src,
                       QRect(QPoint(0, 0), dstRect.size()));
    } else {
        KisQmicSimpleConvertor::convertFromGmicFast(srcGmicImage, dst, 255.0f);
    }
}

KisNodeListSP
inputNodes(KisImageSP image, InputLayerMode inputMode, KisNodeSP currentNode)
{
    /*
        ACTIVE_LAYER,
        ALL_LAYERS,
        ACTIVE_LAYER_BELOW_LAYER,
        ACTIVE_LAYER_ABOVE_LAYER,
        ALL_VISIBLE_LAYERS,
        ALL_INVISIBLE_LAYERS,
        ALL_VISIBLE_LAYERS_DECR,
        ALL_INVISIBLE_DECR,
        ALL_DECR
    */
    const auto isAvailable = [](KisNodeSP node) -> bool {
        auto *paintLayer = dynamic_cast<KisPaintLayer *>(node.data());
        return paintLayer && paintLayer->visible(false);
    };

    KisNodeListSP result(new QList<KisNodeSP>());
    switch (inputMode) {
    case InputLayerMode::NoInput: {
        break;
    }
    case InputLayerMode::Active: {
        if (isAvailable(currentNode)) {
            result->prepend(currentNode);
        }
        break; // drop down in case of one more layer modes
    }
    case InputLayerMode::All: {
        result = [&]() {
            KisNodeListSP r(new QList<KisNodeSP>());
            KisLayerUtils::recursiveApplyNodes(
                image->root(),
                [&](KisNodeSP item) {
                    auto *paintLayer =
                        dynamic_cast<KisPaintLayer *>(item.data());
                    if (paintLayer) {
                        r->prepend(item);
                    }
                });
            return r;
        }();
        break;
    }
    case InputLayerMode::ActiveAndBelow: {
        if (isAvailable(currentNode)) {
            result->prepend(currentNode);
            if (isAvailable(currentNode->prevSibling())) {
                result->prepend(currentNode->prevSibling());
            }
        }
        break;
    }
    case InputLayerMode::ActiveAndAbove: {
        if (isAvailable(currentNode)) {
            result->prepend(currentNode);
            if (isAvailable(currentNode->nextSibling())) {
                result->prepend(currentNode->nextSibling());
            }
        }
        break;
    }
    case InputLayerMode::AllVisible:
    case InputLayerMode::AllInvisible: {
        const bool visibility = (inputMode == InputLayerMode::AllInvisible);

        result = [&]() {
            KisNodeListSP r(new QList<KisNodeSP>());
            KisLayerUtils::recursiveApplyNodes(
                image->root(),
                [&](KisNodeSP item) {
                    auto *paintLayer =
                        dynamic_cast<KisPaintLayer *>(item.data());
                    if (paintLayer
                        && paintLayer->visible(false) == visibility) {
                        r->prepend(item);
                    }
                });
            return r;
        }();
        break;
    }
    case InputLayerMode::Unspecified:
    default: {
        qWarning()
            << "Inputmode" << static_cast<int>(inputMode)
            << "must be specified by GMic or is not implemented in Krita";
        break;
    }
    }
    return result;
}
} // namespace KisQmicImportTools

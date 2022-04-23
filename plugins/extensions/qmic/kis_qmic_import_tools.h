/*
 * SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 * SPDX-FileCopyrightText: 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_QMIC_IMPORT_TOOLS_H
#define KIS_QMIC_IMPORT_TOOLS_H

#include <QRegularExpression>

#include <KoCompositeOpRegistry.h>
#include <commands/KisNodeRenameCommand.h>
#include <commands/kis_node_compositeop_command.h>
#include <commands/kis_node_opacity_command.h>
#include <commands_new/kis_node_move_command2.h>
#include <kis_command_utils.h>
#include <kis_node.h>
#include <kis_painter.h>
#include <kis_types.h>

#include "gmic.h"
#include "kis_qmic_simple_convertor.h"

namespace KisQmicImportTools
{
[[nodiscard]] inline KUndo2Command *applyLayerNameChanges(const gmic_image<float> &srcGmicImage, KisNode *node)
{
    dbgPlugins << "KisQmicImportTools::applyLayerNameChanges";

    auto *cmd = new KisCommandUtils::CompositeCommand();

    dbgPlugins << "Layer name: " << srcGmicImage.name;

    {
        const QRegExp modeRe(R"(mode\(\s*([^)]*)\s*\))");
        if (modeRe.indexIn(srcGmicImage.name) != -1) {
            QString modeStr = modeRe.cap(1).trimmed();
            auto translatedMode = KisQmicSimpleConvertor::stringToBlendingMode(modeStr);
            dbgPlugins << "Detected mode: " << modeStr << " => " << translatedMode;
            if (!translatedMode.isNull()) {
                cmd->addCommand(new KisNodeCompositeOpCommand(node, translatedMode));
            }
        }
    }

    {
        const QRegExp opacityRe(R"(opacity\(\s*([^)]*)\s*\))");

        if (opacityRe.indexIn(srcGmicImage.name) != -1) {
            const auto opacity = opacityRe.cap(1).toFloat();
            dbgPlugins << "Detected opacity: " << opacity << std::lround(opacity / 100.f * 255.f);
            cmd->addCommand(new KisNodeOpacityCommand(node, static_cast<quint8>(std::lround(float(opacity * 255) / 100.f))));
        }
    }

    {
        const QRegExp nameRe(R"(name\(\s*([^)]*)\s*\))");

        if (nameRe.indexIn(srcGmicImage.name) != -1) {
            const auto name = nameRe.cap(1);
            dbgPlugins << "Detected layer name: " << name;
            cmd->addCommand(new KisNodeRenameCommand(node, node->name(), name));
            // apply command
        }
    }

    {
        // Some GMic filters encode layer position into the layer name.
        // E.g. from extract foreground: "name([unnamed] [foreground]),pos(55,35)"
        const QRegularExpression positionPattern(R"(\Wpos\((\d+),(\d+)\))");
        const QRegularExpressionMatch match = positionPattern.match(srcGmicImage.name);
        if (match.hasMatch()) {
            const auto x = match.captured(1).toInt();
            const auto y = match.captured(2).toInt();
            dbgPlugins << "Detected layer position: " << x << y;
            cmd->addCommand(new KisNodeMoveCommand2(node, QPoint(node->x(), node->y()), QPoint(x, y)));
        }
    }

    return cmd;
}

inline void gmicImageToPaintDevice(const gmic_image<float> &srcGmicImage, KisPaintDeviceSP dst, KisSelectionSP selection = nullptr, const QRect &dstRect = {})
{
    dbgPlugins << "KisQmicImportTools::gmicImageToPaintDevice();";

    if (selection) {
        KisPaintDeviceSP src = new KisPaintDevice(dst->colorSpace());
        KisQmicSimpleConvertor::convertFromGmicFast(srcGmicImage, src, 255.0f);

        KisPainter painter(dst, selection);
        painter.setCompositeOp(COMPOSITE_COPY);
        painter.bitBlt(dstRect.topLeft(), src, QRect(QPoint(0, 0), dstRect.size()));
    } else {
        KisQmicSimpleConvertor::convertFromGmicFast(srcGmicImage, dst, 255.0f);
    }
}
} // namespace KisQmicImportTools

#endif // KIS_QMIC_IMPORT_TOOLS_H

/*
 * SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 * SPDX-FileCopyrightText: 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_QMIC_IMPORT_TOOLS_H
#define KIS_QMIC_IMPORT_TOOLS_H

#include <kis_command_utils.h>
#include <kis_layer_utils.h>
#include <kis_node.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>
#include <kis_types.h>

#include "gmic.h"
#include "kis_qmic_interface.h"
#include "kis_qmic_simple_convertor.h"

namespace KisQmicImportTools
{
[[nodiscard]] KUndo2Command *
applyLayerNameChanges(const KisQMicImage &srcGmicImage,
                      KisNode *node,
                      KisSelectionSP selection);

void gmicImageToPaintDevice(const KisQMicImage &srcGmicImage,
                            KisPaintDeviceSP dst,
                            KisSelectionSP selection = nullptr,
                            const QRect &dstRect = {});

KisNodeListSP
inputNodes(KisImageSP image, InputLayerMode inputMode, KisNodeSP currentNode);
} // namespace KisQmicImportTools

#endif // KIS_QMIC_IMPORT_TOOLS_H

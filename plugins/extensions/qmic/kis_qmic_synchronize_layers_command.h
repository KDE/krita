/*
 * SPDX-FileCopyrightText: 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_GMIC_SYNCHRONIZE_LAYERS_COMMAND
#define _KIS_GMIC_SYNCHRONIZE_LAYERS_COMMAND

#include <kundo2command.h>

#include <kis_image.h>
#include <kis_selection.h>
#include <kis_node.h>
#include <kis_types.h>
#include <kis_command_utils.h>

#include "gmic.h"

class KisQmicSynchronizeLayersCommand : public KisCommandUtils::CompositeCommand
{
public:
    KisQmicSynchronizeLayersCommand(KisNodeListSP nodes,
                                    QVector<gmic_image<float> *> images,
                                    KisImageWSP image,
                                    const QRect &dstRect = QRect(),
                                    const KisSelectionSP selection = nullptr
    );

    ~KisQmicSynchronizeLayersCommand() override;

    void redo() override;
    void undo() override;

private:
    struct Private;
    Private* const d;

    Q_DISABLE_COPY(KisQmicSynchronizeLayersCommand);
};

#endif

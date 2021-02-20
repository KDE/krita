/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_IMAGE_CHANGE_LAYERS_COMMAND_H_
#define KIS_IMAGE_CHANGE_LAYERS_COMMAND_H_

#include <kritaimage_export.h>
#include "kis_types.h"
#include "kis_image_command.h"

class KisImageChangeLayersCommand : public KisImageCommand
{

public:
    KisImageChangeLayersCommand(KisImageWSP image, KisNodeSP oldRootLayer, KisNodeSP newRootLayer);

    void redo() override;
    void undo() override;

private:
    KisNodeSP m_oldRootLayer;
    KisNodeSP m_newRootLayer;
};

#endif


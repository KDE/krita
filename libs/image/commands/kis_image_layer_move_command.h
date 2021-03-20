/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_IMAGE_LAYER_MOVE_COMMAND_H_
#define KIS_IMAGE_LAYER_MOVE_COMMAND_H_

#include <kritaimage_export.h>

#include <QSize>
#include <QBitArray>
#include "kis_types.h"
#include "kis_image_command.h"

/// The command for layer moves inside the layer stack
class KRITAIMAGE_EXPORT KisImageLayerMoveCommand : public KisImageCommand
{


public:
    /**
     * Command for layer moves inside the layer stack
     *
     * @param image the image
     * @param node the moved node
     * @param newParent the next parent of the layer
     * @param newAbove the layer that will be below the layer after the move
     * @param doUpdates whether to do updates
     */
    KisImageLayerMoveCommand(KisImageWSP image, KisNodeSP node, KisNodeSP newParent, KisNodeSP newAbove, bool doUpdates = true);
    KisImageLayerMoveCommand(KisImageWSP image, KisNodeSP node, KisNodeSP newParent, quint32 index);

    void redo() override;
    void undo() override;

private:
    KisNodeSP m_layer;
    KisNodeSP m_prevParent;
    KisNodeSP m_prevAbove;
    KisNodeSP m_newParent;
    KisNodeSP m_newAbove;
    quint32 m_index;

    bool m_useIndex;
    bool m_doUpdates;
};

#endif

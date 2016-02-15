/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
     * @param layer the moved layer
     * @param newParent the next parent of the layer
     * @param newAbove the layer that will be below the layer after the move
     */
    KisImageLayerMoveCommand(KisImageWSP image, KisNodeSP node, KisNodeSP newParent, KisNodeSP newAbove, bool doUpdates = true);
    KisImageLayerMoveCommand(KisImageWSP image, KisNodeSP node, KisNodeSP newParent, quint32 index);

    virtual void redo();
    virtual void undo();

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

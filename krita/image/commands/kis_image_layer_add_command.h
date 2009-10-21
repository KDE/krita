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

#ifndef KIS_IMAGE_LAYER_ADD_COMMAND_H_
#define KIS_IMAGE_LAYER_ADD_COMMAND_H_

#include <krita_export.h>

#include <QUndoCommand>
#include <QSize>
#include <QBitArray>
#include "kis_types.h"
#include "kis_image_command.h"


class KoCompositeOp;
class KoColorSpace;
class KoColorProfile;

/// The command for adding a layer
class KRITAIMAGE_EXPORT KisImageLayerAddCommand : public KisImageCommand
{

public:
    /**
     * Constructor
     * @param image The image the command will be working on.
     * @param layer the layer to add
     */
    KisImageLayerAddCommand(KisImageWSP image, KisNodeSP layer, KisNodeSP parent, KisNodeSP aboveThis);
    KisImageLayerAddCommand(KisImageWSP image, KisNodeSP layer, KisNodeSP parent, quint32 index);

    virtual void redo();
    virtual void undo();

private:
    KisNodeSP m_layer;
    KisNodeSP m_parent;
    KisNodeSP m_aboveThis;
    quint32 m_index;
};
#endif

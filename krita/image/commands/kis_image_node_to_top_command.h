/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_IMAGE_NODE_TO_TOP_COMMAND_H_
#define KIS_IMAGE_NODE_TO_TOP_COMMAND_H_

#include <krita_export.h>

#include "kis_types.h"
#include "kis_image_command.h"

/// The command for adding a layer
class KRITAIMAGE_EXPORT KisImageNodeToTopCommand : public KisImageCommand
{

public:
    /**
     * Constructor
     * @param image The image the command will be working on.
     * @param layer the layer to add
     */
    KisImageNodeToTopCommand(KisImageWSP image, KisNodeSP node);

    virtual void redo();
    virtual void undo();

private:
    KisNodeSP m_node;
    KisNodeSP m_prevParent;
    KisNodeSP m_prevAbove;
};
#endif

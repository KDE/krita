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

#ifndef KIS_IMAGE_COMMAND_H_
#define KIS_IMAGE_COMMAND_H_

#include <kritaimage_export.h>

#include <kundo2command.h>

#include <QSize>
#include <QRect>
#include "kis_types.h"
#include "kis_paint_device.h"


/// the base command for commands altering a KisImage
class KRITAIMAGE_EXPORT KisImageCommand : public KUndo2Command
{

public:
    /**
     * Constructor
     * @param name The name that will be shown in the ui
     * @param image The image the command will be working on.
     * @param parent The parent command.
     */
    KisImageCommand(const KUndo2MagicString& name, KisImageWSP image, KUndo2Command *parent = 0);
    ~KisImageCommand() override;

protected:

    /**
     * Used for performing the smallest update
     * after a node has been removed from stack.
     * First tries to setDirty() node's siblings.
     * If it doesn't help, performs full refresh.
     */
    class UpdateTarget
    {
    public:
        UpdateTarget(KisImageWSP image, KisNodeSP removedNode, const QRect &updateRect);
        void update();

    private:
        KisImageWSP m_image;
        QRect m_updateRect;
        int m_removedNodeIndex;
        KisNodeSP m_removedNodeParent;
    };

protected:
    KisImageWSP m_image;
};

#endif // KIS_IMAGE_COMMAND_H_

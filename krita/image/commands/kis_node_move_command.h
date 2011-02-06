/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@kde.org>
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
#ifndef KIS_NODE_MOVE_COMMAND_H
#define KIS_NODE_MOVE_COMMAND_H

#include "kis_node_command.h"

#include "kis_paint_device.h"

class QPoint;

/// The command for moving of a node
class KRITAIMAGE_EXPORT KisNodeMoveCommand : public KisNodeCommand
{

public:
    /**
     * Constructor
     * @param node The node the command will be working on.
     * @param oldpos the old layer position
     * @param newpos the new layer position
     * @param image image, only needed if the moved node is a selection mask and the image should be notified
     */
    KisNodeMoveCommand(KisNodeSP node, const QPoint& oldpos, const QPoint& newpos, KisImageSP image);
    virtual ~KisNodeMoveCommand();

    virtual void redo();
    virtual void undo();

private:
    void moveTo(const QPoint& pos);

private:
    QRect m_updateRect;
    QPoint m_oldPos;
    QPoint m_newPos;
    KisImageSP m_image;
};


#endif

/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@kde.org>
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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
#ifndef KIS_NODE_MOVE_COMMAND2_H
#define KIS_NODE_MOVE_COMMAND2_H

#include "kis_move_command_common.h"

class KRITAIMAGE_EXPORT KisNodeMoveCommand2 : public KisMoveCommandCommon<KisNodeSP>
{
public:
    KisNodeMoveCommand2(KisNodeSP object, const QPoint& oldPos, const QPoint& newPos, KUndo2Command *parent = 0);

    void undo();
    void redo();

    static void tryNotifySelection(KisNodeSP node);
};

#endif

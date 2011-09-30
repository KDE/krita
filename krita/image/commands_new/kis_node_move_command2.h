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

#include "kundo2command.h"
#include "krita_export.h"
#include "kis_types.h"


class QPoint;
class KisUndoAdapter;


class KRITAIMAGE_EXPORT KisNodeMoveCommand2 : public KUndo2Command
{

public:
    KisNodeMoveCommand2(KisNodeSP node, const QPoint& oldPos, const QPoint& newPos, KisUndoAdapter *undoAdapter, KUndo2Command *parent = 0);
    virtual ~KisNodeMoveCommand2();

    virtual void redo();
    virtual void undo();

private:
    void moveTo(const QPoint& pos);

private:
    QPoint m_oldPos;
    QPoint m_newPos;
    KisNodeSP m_node;
    KisUndoAdapter *m_undoAdapter;
};


#endif

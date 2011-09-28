/*
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

#ifndef __MOVE_STROKE_STRATEGY_H
#define __MOVE_STROKE_STRATEGY_H

#include "kis_stroke_strategy_undo_command_based.h"
#include "kis_types.h"

class KisUpdatesFacade;
class KisUndoAdapter;
class KisPostExecutionUndoAdapter;


class KDE_EXPORT MoveStrokeStrategy : public KisStrokeStrategyUndoCommandBased
{
public:
    class KDE_EXPORT Data : public KisStrokeJobData {
    public:
        Data(QPoint _offset)
            : KisStrokeJobData(SEQUENTIAL, EXCLUSIVE),
              offset(_offset)
        {
        }
        QPoint offset;
    };

public:
    MoveStrokeStrategy(KisNodeSP node, KisUpdatesFacade *updatesFacade,
                       KisPostExecutionUndoAdapter *undoAdapter,
                       KisUndoAdapter *legacyUndoAdapter);

    /**
     * You can use deferred initialization of the node pointer
     * To use it you need to pass NULL to the constructor, and
     * set the node with setNode layer.
     * NOTE: once set, you cannot change the node anymore,
     *       you'll get an assert
     */

    void setNode(KisNodeSP node);

    void finishStrokeCallback();
    void cancelStrokeCallback();
    void doStrokeCallback(KisStrokeJobData *data);

private:
    void moveAndUpdate(QPoint offset);
    QRect moveNode(KisNodeSP node, QPoint offset);
    void addMoveCommands(KisNodeSP node, KUndo2Command *parent);

private:
    KisNodeSP m_node;
    KisUpdatesFacade *m_updatesFacade;
    KisUndoAdapter *m_legacyUndoAdapter;
    QPoint m_finalOffset;
    QRect m_dirtyRect;
};

#endif /* __MOVE_STROKE_STRATEGY_H */

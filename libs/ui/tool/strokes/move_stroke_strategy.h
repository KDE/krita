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

#include <QHash>

#include "kritaui_export.h"
#include "kis_stroke_strategy_undo_command_based.h"
#include "kis_types.h"
#include "kis_lod_transform.h"


class KisUpdatesFacade;
class KisPostExecutionUndoAdapter;


class KRITAUI_EXPORT MoveStrokeStrategy : public KisStrokeStrategyUndoCommandBased
{
public:
    class Data : public KisStrokeJobData {
    public:
        Data(QPoint _offset)
            : KisStrokeJobData(SEQUENTIAL, EXCLUSIVE),
              offset(_offset)
        {
        }

        KisStrokeJobData* createLodClone(int levelOfDetail) override {
            return new Data(*this, levelOfDetail);
        }

        QPoint offset;

    private:
        Data(const Data &rhs, int levelOfDetail)
            : KisStrokeJobData(rhs)
        {
            KisLodTransform t(levelOfDetail);
            offset = t.map(rhs.offset);
        }
    };

public:
    MoveStrokeStrategy(KisNodeList nodes, KisUpdatesFacade *updatesFacade,
                       KisStrokeUndoFacade *undoFacade);

    void initStrokeCallback() override;
    void finishStrokeCallback() override;
    void cancelStrokeCallback() override;
    void doStrokeCallback(KisStrokeJobData *data) override;

    KisStrokeStrategy* createLodClone(int levelOfDetail) override;

private:
    MoveStrokeStrategy(const MoveStrokeStrategy &rhs);
    void setUndoEnabled(bool value);
    void setUpdatesEnabled(bool value);
private:
    void moveAndUpdate(QPoint offset);
    QRect moveNode(KisNodeSP node, QPoint offset);
    void addMoveCommands(KisNodeSP node, KUndo2Command *parent);
    void saveInitialNodeOffsets(KisNodeSP node);

private:
    KisNodeList m_nodes;
    QSet<KisNodeSP> m_blacklistedNodes;
    KisUpdatesFacade *m_updatesFacade;
    QPoint m_finalOffset;
    QRect m_dirtyRect;
    QHash<KisNodeSP, QRect> m_dirtyRects;
    bool m_updatesEnabled;
    QHash<KisNodeSP, QPoint> m_initialNodeOffsets;
};

#endif /* __MOVE_STROKE_STRATEGY_H */

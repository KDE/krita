/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __MOVE_STROKE_STRATEGY_H
#define __MOVE_STROKE_STRATEGY_H

#include <QHash>
#include <QObject>

#include "kritaui_export.h"
#include "kis_stroke_strategy_undo_command_based.h"
#include "kis_types.h"
#include "kis_lod_transform.h"
#include <QElapsedTimer>
#include "KisAsyncronousStrokeUpdateHelper.h"
#include "KisNodeSelectionRecipe.h"

#include <memory>
#include <unordered_map>

class KisUpdatesFacade;
class KisPostExecutionUndoAdapter;


class KRITAUI_EXPORT MoveStrokeStrategy : public QObject, public KisStrokeStrategyUndoCommandBased
{
    Q_OBJECT
public:
    class KRITAUI_EXPORT Data : public KisStrokeJobData {
    public:
        Data(QPoint _offset);
        KisStrokeJobData* createLodClone(int levelOfDetail) override;

        QPoint offset;

    private:
        Data(const Data &rhs, int levelOfDetail);
    };

    class KRITAUI_EXPORT PickLayerData : public KisStrokeJobData {
    public:
        PickLayerData(QPoint _pos);

        KisStrokeJobData* createLodClone(int levelOfDetail) override;

        QPoint pos;

    private:
        PickLayerData(const PickLayerData &rhs, int levelOfDetail);
    };


    struct KRITAUI_EXPORT BarrierUpdateData : public KisAsyncronousStrokeUpdateHelper::UpdateData
    {
        BarrierUpdateData(bool forceUpdate);
        KisStrokeJobData* createLodClone(int levelOfDetail) override;
    protected:
        BarrierUpdateData(const BarrierUpdateData &rhs, int levelOfDetail);
    };

public:
    MoveStrokeStrategy(KisNodeSelectionRecipe nodeSelection,
                       KisUpdatesFacade *updatesFacade,
                       KisStrokeUndoFacade *undoFacade);

    MoveStrokeStrategy(KisNodeList nodes,
                       KisUpdatesFacade *updatesFacade,
                       KisStrokeUndoFacade *undoFacade);

    void initStrokeCallback() override;
    void finishStrokeCallback() override;
    void cancelStrokeCallback() override;
    void doStrokeCallback(KisStrokeJobData *data) override;

    KisStrokeStrategy* createLodClone(int levelOfDetail) override;

Q_SIGNALS:
    void sigHandlesRectCalculated(const QRect &handlesRect);
    void sigStrokeStartedEmpty();
    void sigLayersPicked(const KisNodeList &nodes);

private:
    MoveStrokeStrategy(const MoveStrokeStrategy &rhs, int lod);
    void setUndoEnabled(bool value);
    void setUpdatesEnabled(bool value);
private:
    QRect moveNode(KisNodeSP node, QPoint offset);
    void addMoveCommands(KisNodeSP node, KUndo2Command *parent);
    void saveInitialNodeOffsets(KisNodeSP node);
    void doCanvasUpdate(bool forceUpdate = false);
    void tryPostUpdateJob(bool forceUpdate);

private:
    KisNodeSelectionRecipe m_requestedNodeSelection;
    KisNodeList m_nodes;
    QSharedPointer<KisNodeList> m_sharedNodes;
    QSet<KisNodeSP> m_blacklistedNodes;
    KisUpdatesFacade *m_updatesFacade;
    QPoint m_finalOffset;
    QRect m_dirtyRect;
    QHash<KisNodeSP, QRect> m_dirtyRects;
    bool m_updatesEnabled;
    QHash<KisNodeSP, QPoint> m_initialNodeOffsets;

    struct TransformMaskData {
        QPointF currentOffset;
        std::unique_ptr<KUndo2Command> undoCommand;
    };

    std::unordered_map<KisNodeSP, TransformMaskData> m_transformMaskData;
    KUndo2Command* keyframeCommand;

    QElapsedTimer m_updateTimer;
    bool m_hasPostponedJob = false;
    const int m_updateInterval = 30;
};

#endif /* __MOVE_STROKE_STRATEGY_H */

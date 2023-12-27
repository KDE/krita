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
#include "KisAsynchronousStrokeUpdateHelper.h"
#include "KisNodeSelectionRecipe.h"
#include "kis_transaction.h"

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


    struct KRITAUI_EXPORT BarrierUpdateData : public KisAsynchronousStrokeUpdateHelper::UpdateData
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

    ~MoveStrokeStrategy() override;

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
    void doCanvasUpdate(bool forceUpdate = false);
    void tryPostUpdateJob(bool forceUpdate);

private:
    struct Private;
    QScopedPointer<Private> m_d;

    KisNodeSelectionRecipe m_requestedNodeSelection;
    KisNodeList m_nodes;
    QSharedPointer<std::pair<KisNodeList, QSet<KisNodeSP>>> m_sharedNodes;
    QSet<KisNodeSP> m_blacklistedNodes;
    KisUpdatesFacade *m_updatesFacade {nullptr};
    QPoint m_finalOffset;
    QHash<KisNodeSP, QRect> m_dirtyRects;
    bool m_updatesEnabled {true};

    QElapsedTimer m_updateTimer;
    bool m_hasPostponedJob {false};
    const int m_updateInterval {30};

    template <typename Functor>
    void recursiveApplyNodes(KisNodeList nodes, Functor &&func);
};

#endif /* __MOVE_STROKE_STRATEGY_H */

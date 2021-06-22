/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __INPLACE_TRANSFORM_STROKE_STRATEGY_H
#define __INPLACE_TRANSFORM_STROKE_STRATEGY_H

#include <QObject>
#include <QMutex>
#include <QElapsedTimer>
#include <KoUpdater.h>
#include <kis_stroke_strategy_undo_command_based.h>
#include <kis_types.h>
#include "tool_transform_args.h"
#include <kritatooltransform_export.h>

#include <transform_transaction_properties.h>

#include "KisAsyncronousStrokeUpdateHelper.h"
#include <commands_new/KisUpdateCommandEx.h>

class KisPostExecutionUndoAdapter;
class TransformTransactionProperties;
class KisUpdatesFacade;
class KisDecoratedNodeInterface;


class InplaceTransformStrokeStrategy : public QObject, public KisStrokeStrategyUndoCommandBased
{
    Q_OBJECT
public:
    class UpdateTransformData : public KisStrokeJobData {
    public:
        enum Destination {
            PAINT_DEVICE,
            SELECTION,
        };

    public:
        UpdateTransformData(ToolTransformArgs _args, Destination _dest)
            : KisStrokeJobData(SEQUENTIAL, NORMAL),
              args(_args),
              destination(_dest)
        {}

        KisStrokeJobData* createLodClone(int levelOfDetail) override {
            return new UpdateTransformData(*this, levelOfDetail);
        }

    private:
        UpdateTransformData(const UpdateTransformData &rhs, int levelOfDetail)
            : KisStrokeJobData(rhs),
              args(rhs.args),
              destination(rhs.destination)
        {
            Q_UNUSED(levelOfDetail);
        }

    public:
        ToolTransformArgs args;
        Destination destination;
    };

private:

    /**
     * A special barrier update data that triggers regeneration of
     * all the processed nodes.
     */
    struct BarrierUpdateData : public KisAsyncronousStrokeUpdateHelper::UpdateData
    {
        BarrierUpdateData(bool forceUpdate);
        KisStrokeJobData* createLodClone(int levelOfDetail) override;
    private:
        BarrierUpdateData(const BarrierUpdateData &rhs, int levelOfDetail);
    };

public:

    /**
     * The transformation pipeline usually looks like that:
     *
     * 1) Apply Clear commands for all the layers. Some clear commands might
     * be "temporary", that is, they do not go to the final history, e.g. when
     * clearing a shape layer's projection.
     *
     * 2) Apply TransoformLod commands to generate preview of the
     * transformation. Some commands may be declared as "temporary", that is,
     * they do not go to the final history, e.g. for the shape layer, for
     * which we just write to the projection device explicitly.
     *
     * 3) When transformation is changed we undo all TransformLod and
     * TransformTemporary commands to recover the old state. The temporary
     * command recovers the state of shape layers' projection device.
     *
     * 4) Repeat steps 2) and 3) until the user is satisfied.
     *
     * 5) When "Apply" button is pressed, all transform commands are undone
     * like in step 2).
     *
     * 6) All Transform commands are applied at Lod0-level. TransformTemporary
     * is not used atm.
     *
     * 7) All non-temporary commands go to the undo history.
     */

    enum CommandGroup {
        Clear = 0,
        ClearTemporary,
        Transform,
        TransformTemporary,
        TransformLod,
        TransformLodTemporary
    };
    Q_ENUM(CommandGroup);

public:
    InplaceTransformStrokeStrategy(ToolTransformArgs::TransformMode mode,
                                   const QString &filterId,
                                   bool forceReset,
                                   KisNodeSP rootNode,
                                   KisSelectionSP selection,
                                   KisPaintDeviceSP externalSource,
                                   KisStrokeUndoFacade *undoFacade,
                                   KisUpdatesFacade *updatesFacade, KisNodeSP imageRoot, bool forceLodMode);

    ~InplaceTransformStrokeStrategy() override;

    void initStrokeCallback() override;
    void finishStrokeCallback() override;
    void cancelStrokeCallback() override;
    void doStrokeCallback(KisStrokeJobData *data) override;

Q_SIGNALS:
    void sigTransactionGenerated(TransformTransactionProperties transaction, ToolTransformArgs args, void *cookie);

protected:
    void postProcessToplevelCommand(KUndo2Command *command) override;

private:
    friend class InitializeTransformModeStrokeStrategy;

    InplaceTransformStrokeStrategy(const InplaceTransformStrokeStrategy &rhs, int levelOfDetail);

    void tryPostUpdateJob(bool forceUpdate);
    void doCanvasUpdate(bool forceUpdate);

    int calculatePreferredLevelOfDetail(const QRect &srcRect);

    void executeAndAddCommand(KUndo2Command *cmd, CommandGroup group, KisStrokeJobData::Sequentiality seq);

    void notifyAllCommandsDone();
    void undoAllCommands();
    void undoTransformCommands(int levelOfDetail);

    void fetchAllUpdateRequests(int levelOfDetail, KisUpdateCommandEx::SharedDataSP updateData);

    void transformNode(KisNodeSP node, const ToolTransformArgs &config, int levelOfDetail);
    void createCacheAndClearNode(KisNodeSP node);
    void reapplyTransform(ToolTransformArgs args, QVector<KisStrokeJobData *> &mutatedJobs, int levelOfDetail, bool useHoldUI);
    void finalizeStrokeImpl(QVector<KisStrokeJobData *> &mutatedJobs, bool saveCommands);

    void finishAction(QVector<KisStrokeJobData *> &mutatedJobs);
    void cancelAction(QVector<KisStrokeJobData *> &mutatedJobs);
    void addDirtyRect(KisNodeSP node, const QRect &rect, int levelOfDetail);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __INPLACE_TRANSFORM_STROKE_STRATEGY_H */

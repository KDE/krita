/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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
        UpdateTransformData(ToolTransformArgs _args)
            : KisStrokeJobData(SEQUENTIAL, NORMAL),
              args(_args)
        {}

        KisStrokeJobData* createLodClone(int levelOfDetail) override {
            return new UpdateTransformData(*this, levelOfDetail);
        }

    private:
        UpdateTransformData(const UpdateTransformData &rhs, int levelOfDetail)
            : KisStrokeJobData(rhs),
              args(rhs.args)
        {
            Q_UNUSED(levelOfDetail);
        }

    public:
        ToolTransformArgs args;
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
                                   bool workRecursively,
                                   const QString &filterId,
                                   bool forceReset,
                                   KisNodeSP rootNode,
                                   KisSelectionSP selection,
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

    void executeAndAddCommand(KUndo2Command *cmd, CommandGroup group);

    void notifyAllCommandsDone();
    void undoAllCommands();
    void undoTransformCommands(int levelOfDetail);

    void postAllUpdates(int levelOfDetail);

    void transformNode(KisNodeSP node, const ToolTransformArgs &config, int levelOfDetail);
    void clearNode(KisNodeSP node);
    void reapplyTransform(ToolTransformArgs args, QVector<KisStrokeJobData *> &mutatedJobs, int levelOfDetail);
    void finalizeStrokeImpl(QVector<KisStrokeJobData *> &mutatedJobs, bool saveCommands);

    void finishAction(QVector<KisStrokeJobData *> &mutatedJobs);
    void cancelAction(QVector<KisStrokeJobData *> &mutatedJobs);
    void addDirtyRect(KisNodeSP node, const QRect &rect, int levelOfDetail);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __INPLACE_TRANSFORM_STROKE_STRATEGY_H */

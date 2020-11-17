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
#include <kis_processing_visitor.h>
#include <kritatooltransform_export.h>
#include <boost/optional.hpp>
#include <transform_transaction_properties.h>
#include "kis_selection_mask.h"
#include "KisAsyncronousStrokeUpdateHelper.h"
#include "kis_undo_stores.h"


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

    struct KRITAUI_EXPORT BarrierUpdateData : public KisAsyncronousStrokeUpdateHelper::UpdateData
    {
        BarrierUpdateData(bool forceUpdate);
    };

public:
    InplaceTransformStrokeStrategy(ToolTransformArgs::TransformMode mode,
                                   bool workRecursively,
                                   const QString &filterId,
                                   bool forceReset,
                                   KisNodeSP rootNode,
                                   KisSelectionSP selection,
                                   KisStrokeUndoFacade *undoFacade,
                                   KisUpdatesFacade *updatesFacade);

    ~InplaceTransformStrokeStrategy() override;

    static bool shouldRestartStrokeOnModeChange(ToolTransformArgs::TransformMode oldMode,
                                                ToolTransformArgs::TransformMode newMode,
                                                KisNodeList processedNodes);

    void initStrokeCallback() override;
    void finishStrokeCallback() override;
    void cancelStrokeCallback() override;
    void doStrokeCallback(KisStrokeJobData *data) override;

    static bool fetchArgsFromCommand(const KUndo2Command *command, ToolTransformArgs *args, KisNodeSP *rootNode, KisNodeList *transformedNodes);

Q_SIGNALS:
    void sigTransactionGenerated(TransformTransactionProperties transaction, ToolTransformArgs args, void *cookie);
    void sigPreviewDeviceReady(KisPaintDeviceSP device);

protected:
    void postProcessToplevelCommand(KUndo2Command *command) override;

private:
    KoUpdaterPtr fetchUpdater(KisNodeSP node);

    void transformAndMergeDevice(const ToolTransformArgs &config,
                                 KisPaintDeviceSP src,
                                 KisPaintDeviceSP dst,
                                 KisProcessingVisitor::ProgressHelper *helper);
    void transformDevice(const ToolTransformArgs &config,
                         KisPaintDeviceSP device,
                         KisProcessingVisitor::ProgressHelper *helper);

    void clearSelection(KisPaintDeviceSP device);

    KisPaintDeviceSP createDeviceCache(KisPaintDeviceSP src);

    bool haveDeviceInCache(KisPaintDeviceSP src);
    void putDeviceCache(KisPaintDeviceSP src, KisPaintDeviceSP cache);
    KisPaintDeviceSP getDeviceCache(KisPaintDeviceSP src);

    QList<KisNodeSP> fetchNodesList(ToolTransformArgs::TransformMode mode, KisNodeSP root, bool recursive);
    ToolTransformArgs resetArgsForMode(ToolTransformArgs::TransformMode mode,
                                       const QString &filterId,
                                       const TransformTransactionProperties &transaction);
    bool tryInitArgsFromNode(KisNodeSP node, ToolTransformArgs *args);
    bool tryFetchArgsFromCommandAndUndo(ToolTransformArgs *args,
                                        ToolTransformArgs::TransformMode mode,
                                        KisNodeSP currentNode,
                                        KisNodeList selectedNodes, QVector<KisStrokeJobData *> *undoJobs);

    void finishStrokeImpl(bool applyTransform,
                          const ToolTransformArgs &args);

    void finalizeStrokeImpl(QVector<KisStrokeJobData *> &mutatedJobs);

    void clearNode(KisNodeSP node);
    void transformNode(KisNodeSP node, const ToolTransformArgs &config);

    void tryPostUpdateJob(bool forceUpdate);
    void doCanvasUpdate(bool forceUpdate);


private:
    KisUpdatesFacade *m_updatesFacade;
    ToolTransformArgs::TransformMode m_mode;
    bool m_workRecursively;
    QString m_filterId;
    bool m_forceReset;

    KisSelectionSP m_selection;

    KisTransformMaskSP writeToTransformMask;

    ToolTransformArgs m_initialTransformArgs;
    ToolTransformArgs m_currentTransformArgs;
    KisNodeSP m_rootNode;
    KisNodeList m_processedNodes;
    QVector<KisDecoratedNodeInterface*> m_disabledDecoratedNodes;
    QList<KisSelectionSP> m_deactivatedSelections;
    KisSelectionMaskSP m_deactivatedOverlaySelectionMask;

    const KisSavedMacroCommand *m_overriddenCommand = 0;
    QVector<const KUndo2Command*> m_skippedWhileMergeCommands;

    bool m_finalizingActionsStarted = false;

    bool m_updatesDisabled = false;

    boost::optional<ToolTransformArgs> m_pendingUpdateArgs;
    QElapsedTimer m_updateTimer;
    const int m_updateInterval = 30;

    QMutex m_commandsMutex;
    QVector<KUndo2CommandSP> m_clearCommands;
    QVector<KUndo2CommandSP> m_transformCommands;

    QMutex m_devicesCacheMutex;
    QHash<KisPaintDevice*, KisPaintDeviceSP> m_devicesCacheHash;

    QMutex m_dirtyRectsMutex;
    QHash<KisNodeSP, QRect> m_dirtyRects;
    QHash<KisNodeSP, QRect> m_prevDirtyRects;


    void executeAndAddClearCommand(KUndo2Command *cmd);

    void executeAndAddTransformCommand(KUndo2Command *cmd);

    void addDirtyRect(KisNodeSP node, const QRect &rect) {
        QMutexLocker l(&m_dirtyRectsMutex);
        m_dirtyRects[node] |= rect;
    }
};

#endif /* __INPLACE_TRANSFORM_STROKE_STRATEGY_H */

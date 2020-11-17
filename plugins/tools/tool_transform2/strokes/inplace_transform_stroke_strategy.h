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
#include <KoUpdater.h>
#include <kis_stroke_strategy_undo_command_based.h>
#include <kis_types.h>
#include "tool_transform_args.h"
#include <kis_processing_visitor.h>
#include <kritatooltransform_export.h>
#include <boost/optional.hpp>
#include <transform_transaction_properties.h>
#include "kis_selection_mask.h"

class KisPostExecutionUndoAdapter;
class TransformTransactionProperties;
class KisUpdatesFacade;
class KisDecoratedNodeInterface;


class InplaceTransformStrokeStrategy : public QObject, public KisStrokeStrategyUndoCommandBased
{
    Q_OBJECT
public:
    struct SharedState {
        QMutex initializationMutex;
        bool processingStarted = false;
        ToolTransformArgs nextInitializationArgs;

        bool isInitialized = false;
        ToolTransformArgs args;
        ToolTransformArgs initialTransformArgs;
        TransformTransactionProperties props;
        KisSelectionSP selection;
        QVector<QRect> pendingUIUpdates;
        QList<KisSelectionSP> deactivatedSelections;
        KisSelectionMaskSP deactivatedOverlaySelectionMask;

        QMutex commandsMutex;
        QVector<KUndo2CommandSP> clearCommands;
        QVector<KUndo2CommandSP> transformCommands;

        QMutex devicesCacheMutex;
        QHash<KisPaintDevice*, KisPaintDeviceSP> devicesCacheHash;

        QAtomicInt skipCancellationMarker;


        QMutex dirtyRectsMutex;
        QHash<KisNodeSP, QRect> dirtyRects;


        void addDirtyRect(KisNodeSP node, const QRect &rect) {
            QMutexLocker l(&dirtyRectsMutex);
            dirtyRects[node] |= rect;
        }
    };
    typedef QSharedPointer<SharedState> SharedStateSP;


public:
    InplaceTransformStrokeStrategy(ToolTransformArgs::TransformMode mode,
                                   bool workRecursively,
                                   const QString &filterId,
                                   bool forceReset,
                                   KisNodeSP rootNode,
                                   KisSelectionSP selection,
                                   SharedStateSP sharedState,
                                   KisStrokeUndoFacade *undoFacade,
                                   KisUpdatesFacade *updatesFacade);

    InplaceTransformStrokeStrategy(const ToolTransformArgs &args,
                                   SharedStateSP sharedState,
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

private:
    KisUpdatesFacade *m_updatesFacade;
    ToolTransformArgs::TransformMode m_mode;
    bool m_workRecursively;
    QString m_filterId;
    bool m_forceReset;

    KisSelectionSP m_selection;

    KisTransformMaskSP writeToTransformMask;

    ToolTransformArgs m_initialTransformArgs;
    KisNodeSP m_rootNode;
    KisNodeList m_processedNodes;
    QVector<KisDecoratedNodeInterface*> m_disabledDecoratedNodes;

    const KisSavedMacroCommand *m_overriddenCommand = 0;
    QVector<const KUndo2Command*> m_skippedWhileMergeCommands;

    bool m_finalizingActionsStarted = false;

    SharedStateSP m_sharedState;
    bool m_updatesDisabled = false;
};

#endif /* __INPLACE_TRANSFORM_STROKE_STRATEGY_H */

/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __TRANSFORM_STROKE_STRATEGY_H
#define __TRANSFORM_STROKE_STRATEGY_H

#include <QObject>
#include <QMutex>
#include <KoUpdater.h>
#include <kis_stroke_strategy_undo_command_based.h>
#include <kis_types.h>
#include "tool_transform_args.h"
#include <kis_processing_visitor.h>
#include <kritatooltransform_export.h>
#include <boost/optional.hpp>
#include "commands_new/KisUpdateCommandEx.h"

class KisPostExecutionUndoAdapter;
class TransformTransactionProperties;
class KisUpdatesFacade;
class KisDecoratedNodeInterface;


class TransformStrokeStrategy : public QObject, public KisStrokeStrategyUndoCommandBased
{
    Q_OBJECT
public:
    struct TransformAllData : public KisStrokeJobData {
        TransformAllData(const ToolTransformArgs &_config)
            : KisStrokeJobData(SEQUENTIAL, NORMAL),
              config(_config) {}

        ToolTransformArgs config;
    };


    class TransformData : public KisStrokeJobData {
    public:
        enum Destination {
            PAINT_DEVICE,
            SELECTION,
        };

    public:
    TransformData(Destination _destination, const ToolTransformArgs &_config, KisNodeSP _node)
            : KisStrokeJobData(CONCURRENT, NORMAL),
            destination(_destination),
            config(_config),
            node(_node)
        {
        }

        Destination destination;
        ToolTransformArgs config;
        KisNodeSP node;
    };

    class ClearSelectionData : public KisStrokeJobData {
    public:
        ClearSelectionData(KisNodeSP _node)
            : KisStrokeJobData(SEQUENTIAL, EXCLUSIVE),
              node(_node)
        {
        }
        KisNodeSP node;
    };

    class PreparePreviewData : public KisStrokeJobData {
    public:
        PreparePreviewData()
            : KisStrokeJobData(BARRIER, NORMAL)
        {
        }
    };

public:
    TransformStrokeStrategy(ToolTransformArgs::TransformMode mode,
                            const QString &filterId,
                            bool forceReset,
                            KisNodeSP rootNode,
                            KisSelectionSP selection,
                            KisStrokeUndoFacade *undoFacade, KisUpdatesFacade *updatesFacade);

    ~TransformStrokeStrategy() override;

    void initStrokeCallback() override;
    void finishStrokeCallback() override;
    void cancelStrokeCallback() override;
    void doStrokeCallback(KisStrokeJobData *data) override;

Q_SIGNALS:
    void sigTransactionGenerated(TransformTransactionProperties transaction, ToolTransformArgs args, void *cookie);
    void sigPreviewDeviceReady(KisPaintDeviceSP device);

protected:
    void postProcessToplevelCommand(KUndo2Command *command) override;

private:
    KoUpdaterPtr fetchUpdater(KisNodeSP node);

    void clearSelection(KisPaintDeviceSP device);

    bool checkBelongsToSelection(KisPaintDeviceSP device) const;

    KisPaintDeviceSP createDeviceCache(KisPaintDeviceSP src);

    bool haveDeviceInCache(KisPaintDeviceSP src);
    void putDeviceCache(KisPaintDeviceSP src, KisPaintDeviceSP cache);
    KisPaintDeviceSP getDeviceCache(KisPaintDeviceSP src);

    void finishStrokeImpl(bool applyTransform,
                          const ToolTransformArgs &args);

private:
    KisUpdatesFacade *m_updatesFacade;
    KisUpdateCommandEx::SharedDataSP m_updateData;
    bool m_updatesDisabled = false;
    ToolTransformArgs::TransformMode m_mode;
    QString m_filterId;
    bool m_forceReset;

    KisSelectionSP m_selection;

    QMutex m_devicesCacheMutex;
    QHash<KisPaintDevice*, KisPaintDeviceSP> m_devicesCacheHash;

    KisTransformMaskSP writeToTransformMask;

    ToolTransformArgs m_initialTransformArgs;
    boost::optional<ToolTransformArgs> m_savedTransformArgs;
    KisNodeSP m_rootNode;
    KisNodeList m_processedNodes;
    QList<KisSelectionSP> m_deactivatedSelections;
    QList<KisNodeSP> m_hiddenProjectionLeaves;
    KisSelectionMaskSP m_deactivatedOverlaySelectionMask;
    QVector<KisDecoratedNodeInterface*> m_disabledDecoratedNodes;

    const KisSavedMacroCommand *m_overriddenCommand = 0;
    QVector<const KUndo2Command*> m_skippedWhileMergeCommands;

    bool m_finalizingActionsStarted = false;
};

#endif /* __TRANSFORM_STROKE_STRATEGY_H */

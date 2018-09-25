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



class KisPostExecutionUndoAdapter;


class TransformStrokeStrategy : public QObject, public KisStrokeStrategyUndoCommandBased
{
    Q_OBJECT
public:
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
            : KisStrokeJobData(SEQUENTIAL, NORMAL),
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
    TransformStrokeStrategy(KisNodeSP rootNode, KisNodeList processedNodes,
                            KisSelectionSP selection,
                            KisStrokeUndoFacade *undoFacade);

    ~TransformStrokeStrategy() override;

    void initStrokeCallback() override;
    void finishStrokeCallback() override;
    void cancelStrokeCallback() override;
    void doStrokeCallback(KisStrokeJobData *data) override;

    static bool fetchArgsFromCommand(const KUndo2Command *command, ToolTransformArgs *args, KisNodeSP *rootNode, KisNodeList *transformedNodes);

Q_SIGNALS:
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
    //void transformDevice(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisProcessingVisitor::ProgressHelper *helper);

    bool checkBelongsToSelection(KisPaintDeviceSP device) const;

    KisPaintDeviceSP createDeviceCache(KisPaintDeviceSP src);

    bool haveDeviceInCache(KisPaintDeviceSP src);
    void putDeviceCache(KisPaintDeviceSP src, KisPaintDeviceSP cache);
    KisPaintDeviceSP getDeviceCache(KisPaintDeviceSP src);

private:
    KisSelectionSP m_selection;

    QMutex m_devicesCacheMutex;
    QHash<KisPaintDevice*, KisPaintDeviceSP> m_devicesCacheHash;

    KisTransformMaskSP writeToTransformMask;

    ToolTransformArgs m_savedTransformArgs;
    KisNodeSP m_savedRootNode;
    KisNodeList m_savedProcessedNodes;
    QList<KisSelectionSP> m_deactivatedSelections;
    QList<KisNodeSP> m_hiddenProjectionLeaves;
};

#endif /* __TRANSFORM_STROKE_STRATEGY_H */

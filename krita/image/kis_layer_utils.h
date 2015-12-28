/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_LAYER_UTILS_H
#define __KIS_LAYER_UTILS_H

#include "kundo2command.h"
#include "kis_types.h"
#include "kritaimage_export.h"

namespace KisMetaData
{
    class MergeStrategy;
}

namespace KisLayerUtils
{
    KRITAIMAGE_EXPORT void sortMergableNodes(KisNodeSP root, QList<KisNodeSP> &inputNodes, QList<KisNodeSP> &outputNodes);
    KRITAIMAGE_EXPORT void filterMergableNodes(QList<KisNodeSP> &nodes);

    KRITAIMAGE_EXPORT void mergeDown(KisImageSP image, KisLayerSP layer, const KisMetaData::MergeStrategy* strategy);

    KRITAIMAGE_EXPORT QSet<int> fetchLayerFrames(KisNodeSP node);
    KRITAIMAGE_EXPORT QSet<int> fetchLayerFramesRecursive(KisNodeSP rootNode);

    KRITAIMAGE_EXPORT void mergeMultipleLayers(KisImageSP image, QList<KisNodeSP> mergedNodes, KisNodeSP putAfter);
    KRITAIMAGE_EXPORT bool tryMergeSelectionMasks(KisImageSP image, QList<KisNodeSP> mergedNodes, KisNodeSP putAfter);

    KRITAIMAGE_EXPORT void flattenLayer(KisImageSP image, KisLayerSP layer);
    KRITAIMAGE_EXPORT void flattenImage(KisImageSP image);

    typedef QMap<int, QSet<KisNodeSP> > FrameJobs;
    void updateFrameJobs(FrameJobs *jobs, KisNodeSP node);
    void updateFrameJobsRecursive(FrameJobs *jobs, KisNodeSP rootNode);

    struct SwitchFrameCommand : public KUndo2Command {
        struct SharedStorage {
            /**
             * For some reason the absence of a destructor in the SharedStorage
             * makes Krita crash on exit. Seems like some compiler weirdness... (DK)
             */
            ~SharedStorage();
            int value;
        };

        typedef QSharedPointer<SharedStorage> SharedStorageSP;

    public:
        SwitchFrameCommand(KisImageSP image, int time, bool finalize, SharedStorageSP storage);
        ~SwitchFrameCommand();
        void redo();
        void undo();

    private:
        void init();
        void end();

    private:
        KisImageWSP m_image;
        int m_newTime;
        bool m_finalize;
        SharedStorageSP m_storage;
    };
};

#endif /* __KIS_LAYER_UTILS_H */

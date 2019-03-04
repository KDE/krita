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

#include <functional>

#include "kundo2command.h"
#include "kis_types.h"
#include "kritaimage_export.h"
#include "kis_command_utils.h"

class KoProperties;
class KoColor;
class QUuid;

namespace KisMetaData
{
    class MergeStrategy;
}

namespace KisLayerUtils
{
    KRITAIMAGE_EXPORT void sortMergableNodes(KisNodeSP root, QList<KisNodeSP> &inputNodes, QList<KisNodeSP> &outputNodes);
    KRITAIMAGE_EXPORT KisNodeList sortMergableNodes(KisNodeSP root, KisNodeList nodes);
    KRITAIMAGE_EXPORT void filterMergableNodes(KisNodeList &nodes, bool allowMasks = false);
    KRITAIMAGE_EXPORT bool checkIsChildOf(KisNodeSP node, const KisNodeList &parents);

    /**
     * Returns true if:
     *     o \p node is a clone of some layer in \p nodes
     *     o \p node is a clone any child layer of any layer in \p nodes
     *     o \p node is a clone of a clone of a ..., that in the end points
     *       to any layer in \p nodes of their children.
     */
    KRITAIMAGE_EXPORT bool checkIsCloneOf(KisNodeSP node, const KisNodeList &nodes);
    KRITAIMAGE_EXPORT void forceAllDelayedNodesUpdate(KisNodeSP root);

    KRITAIMAGE_EXPORT KisNodeList sortAndFilterMergableInternalNodes(KisNodeList nodes, bool allowMasks = false);

    KRITAIMAGE_EXPORT void mergeDown(KisImageSP image, KisLayerSP layer, const KisMetaData::MergeStrategy* strategy);

    KRITAIMAGE_EXPORT QSet<int> fetchLayerFrames(KisNodeSP node);
    KRITAIMAGE_EXPORT QSet<int> fetchLayerFramesRecursive(KisNodeSP rootNode);

    KRITAIMAGE_EXPORT void mergeMultipleLayers(KisImageSP image, KisNodeList mergedNodes, KisNodeSP putAfter);
    KRITAIMAGE_EXPORT void newLayerFromVisible(KisImageSP image, KisNodeSP putAfter);
    
    KRITAIMAGE_EXPORT bool tryMergeSelectionMasks(KisImageSP image, KisNodeList mergedNodes, KisNodeSP putAfter);

    KRITAIMAGE_EXPORT void flattenLayer(KisImageSP image, KisLayerSP layer);
    KRITAIMAGE_EXPORT void flattenImage(KisImageSP image, KisNodeSP activeNode);

    KRITAIMAGE_EXPORT void addCopyOfNameTag(KisNodeSP node);
    KRITAIMAGE_EXPORT KisNodeList findNodesWithProps(KisNodeSP root, const KoProperties &props, bool excludeRoot);

    KRITAIMAGE_EXPORT void changeImageDefaultProjectionColor(KisImageSP image, const KoColor &color);

    typedef QMap<int, QSet<KisNodeSP> > FrameJobs;
    void updateFrameJobs(FrameJobs *jobs, KisNodeSP node);
    void updateFrameJobsRecursive(FrameJobs *jobs, KisNodeSP rootNode);

    struct SwitchFrameCommand : public KisCommandUtils::FlipFlopCommand {
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
        ~SwitchFrameCommand() override;

    private:
        void partA() override;
        void partB() override;

    private:
        KisImageWSP m_image;
        int m_newTime;
        SharedStorageSP m_storage;
    };

    /**
     * A command to keep correct set of selected/active nodes thoroughout
     * the action.
     */
    class KRITAIMAGE_EXPORT KeepNodesSelectedCommand : public KisCommandUtils::FlipFlopCommand
    {
    public:
    KeepNodesSelectedCommand(const KisNodeList &selectedBefore,
                             const KisNodeList &selectedAfter,
                             KisNodeSP activeBefore,
                             KisNodeSP activeAfter,
                             KisImageSP image,
                             bool finalize, KUndo2Command *parent = 0);
    void partB() override;

    private:
        KisNodeList m_selectedBefore;
        KisNodeList m_selectedAfter;
        KisNodeSP m_activeBefore;
        KisNodeSP m_activeAfter;
        KisImageWSP m_image;
    };

    struct KRITAIMAGE_EXPORT SelectGlobalSelectionMask : public KUndo2Command
    {
        SelectGlobalSelectionMask(KisImageSP image);
        void redo() override;

        KisImageSP m_image;
    };

    class KRITAIMAGE_EXPORT RemoveNodeHelper {
    public:
        virtual ~RemoveNodeHelper();
    protected:
        virtual void addCommandImpl(KUndo2Command *cmd) = 0;
        void safeRemoveMultipleNodes(KisNodeList nodes, KisImageSP image);
    private:
        bool checkIsSourceForClone(KisNodeSP src, const KisNodeList &nodes);
        static bool scanForLastLayer(KisImageWSP image, KisNodeList nodesToRemove);
    };

    struct SimpleRemoveLayers : private KisLayerUtils::RemoveNodeHelper, public KisCommandUtils::AggregateCommand {
        SimpleRemoveLayers(const KisNodeList &nodes,
                           KisImageSP image);

        void populateChildCommands() override;

    protected:
        void addCommandImpl(KUndo2Command *cmd) override;

    private:
        KisNodeList m_nodes;
        KisImageSP m_image;
        KisNodeList m_selectedNodes;
        KisNodeSP m_activeNode;
    };


    class KRITAIMAGE_EXPORT KisSimpleUpdateCommand : public KisCommandUtils::FlipFlopCommand
    {
    public:
        KisSimpleUpdateCommand(KisNodeList nodes, bool finalize, KUndo2Command *parent = 0);
        void partB() override;
        static void updateNodes(const KisNodeList &nodes);
    private:
        KisNodeList m_nodes;
    };

    template <typename T>
        bool checkNodesDiffer(KisNodeList nodes, std::function<T(KisNodeSP)> checkerFunc)
    {
        bool valueDiffers = false;
        bool initialized = false;
        T currentValue = T();
        Q_FOREACH (KisNodeSP node, nodes) {
            if (!initialized) {
                currentValue = checkerFunc(node);
                initialized = true;
            } else if (currentValue != checkerFunc(node)) {
                valueDiffers = true;
                break;
            }
        }
        return valueDiffers;
    }

    /**
     * Applies \p func to \p node and all its children recursively
     */
    template <typename NodePointer, typename Functor>
    void recursiveApplyNodes(NodePointer node, Functor func)
    {
        func(node);

        node = node->firstChild();
        while (node) {
            recursiveApplyNodes(node, func);
            node = node->nextSibling();
        }
    }


    /**
     * Walks through \p node and all its children recursively until
     * \p func returns true. When \p func returns true, the node is
     * considered to be found, the search is stopped and the found
     * node is returned to the caller.
     */
    KisNodeSP KRITAIMAGE_EXPORT recursiveFindNode(KisNodeSP node, std::function<bool(KisNodeSP)> func);

    /**
     * Recursively searches for a node with specified Uuid
     */
    KisNodeSP KRITAIMAGE_EXPORT findNodeByUuid(KisNodeSP root, const QUuid &uuid);

    KisImageSP KRITAIMAGE_EXPORT findImageByHierarchy(KisNodeSP node);
}

#endif /* __KIS_LAYER_UTILS_H */

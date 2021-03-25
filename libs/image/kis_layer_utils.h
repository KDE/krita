/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_LAYER_UTILS_H
#define __KIS_LAYER_UTILS_H

#include <functional>

#include "kundo2command.h"
#include "kis_types.h"
#include "kritaimage_export.h"
#include "kis_command_utils.h"
#include "kis_time_span.h"

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
    KRITAIMAGE_EXPORT KisNodeList sortAndFilterAnyMergableNodesSafe(const KisNodeList &nodes, KisImageSP image);
    KRITAIMAGE_EXPORT bool checkIsChildOf(KisNodeSP node, const KisNodeList &parents);
    KRITAIMAGE_EXPORT void filterUnlockedNodes(KisNodeList &nodes);
    KRITAIMAGE_EXPORT void refreshHiddenAreaAsync(KisImageSP image, KisNodeSP rootNode, const QRect &preparedArea);
    KRITAIMAGE_EXPORT QRect recursiveTightNodeVisibleBounds(KisNodeSP rootNode);

    /**
     * Returns true if:
     *     o \p node is a clone of some layer in \p nodes
     *     o \p node is a clone any child layer of any layer in \p nodes
     *     o \p node is a clone of a clone of a ..., that in the end points
     *       to any layer in \p nodes of their children.
     */
    KRITAIMAGE_EXPORT bool checkIsCloneOf(KisNodeSP node, const KisNodeList &nodes);
    KRITAIMAGE_EXPORT void forceAllDelayedNodesUpdate(KisNodeSP root);
    KRITAIMAGE_EXPORT bool hasDelayedNodeWithUpdates(KisNodeSP root);

    KRITAIMAGE_EXPORT void forceAllHiddenOriginalsUpdate(KisNodeSP root);

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

    KRITAIMAGE_EXPORT KisNodeSP  findRoot(KisNodeSP node);

    KRITAIMAGE_EXPORT bool canChangeImageProfileInvisibly(KisImageSP image);

    KRITAIMAGE_EXPORT void splitAlphaToMask(KisImageSP image, KisNodeSP node, const QString& maskName);

    KRITAIMAGE_EXPORT void convertToPaintLayer(KisImageSP image, KisNodeSP src);

    typedef QMap<int, QSet<KisNodeSP> > FrameJobs;
    void updateFrameJobs(FrameJobs *jobs, KisNodeSP node);
    void updateFrameJobsRecursive(FrameJobs *jobs, KisNodeSP rootNode);

    struct KRITAIMAGE_EXPORT SwitchFrameCommand : public KisCommandUtils::FlipFlopCommand {
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
    KRITAIMAGE_EXPORT KisNodeSP recursiveFindNode(KisNodeSP node, std::function<bool(KisNodeSP)> func);

    /**
     * Recursively searches for a node with specified Uuid
     */
    KRITAIMAGE_EXPORT KisNodeSP findNodeByUuid(KisNodeSP root, const QUuid &uuid);

    KRITAIMAGE_EXPORT KisImageSP findImageByHierarchy(KisNodeSP node);

    template <class T>
    T* findNodeByType(KisNodeSP root) {
        return dynamic_cast<T*>(recursiveFindNode(root, [] (KisNodeSP node) {
            return bool(dynamic_cast<T*>(node.data()));
        }).data());
    }

    // Methods used by filter manager, filter stroke strategy to get times associated with frameIDs.
    // Important for avoiding instanced frame data being processed twice!
    KRITAIMAGE_EXPORT int fetchLayerActiveRasterFrameID(KisNodeSP node);
    KRITAIMAGE_EXPORT int fetchLayerActiveRasterFrameTime(KisNodeSP node);
    KRITAIMAGE_EXPORT KisTimeSpan fetchLayerActiveRasterFrameSpan(KisNodeSP node, const int time);
    KRITAIMAGE_EXPORT QSet<int> fetchLayerIdenticalRasterFrameTimes(KisNodeSP node, const int& frameTime);

    /* Finds all frames matching a specific frame ID. useful to filter out duplicate frames. */
    KRITAIMAGE_EXPORT QSet<int> fetchLayerRasterFrameTimesMatchingID(KisNodeSP node, const int frameID);
    KRITAIMAGE_EXPORT QSet<int> fetchLayerRasterIDsAtTimes(KisNodeSP node, const QSet<int>& times);

    KRITAIMAGE_EXPORT QSet<int> filterTimesForOnlyRasterKeyedTimes(KisNodeSP node, const QSet<int> &times);
    /* Returns a set of times associated with every potential unique frame. */
    KRITAIMAGE_EXPORT QSet<int> fetchLayerUniqueRasterTimesMatchingIDs(KisNodeSP node, QSet<int> &frameIDs);
}

#endif /* __KIS_LAYER_UTILS_H */

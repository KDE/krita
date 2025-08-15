/*
 *  SPDX-FileCopyrightText: 2020 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_MERGE_LABELED_LAYERS_H
#define __KIS_MERGE_LABELED_LAYERS_H

#include <QList>
#include <QString>
#include <QSharedPointer>

#include "kundo2command.h"
#include "kritaimage_export.h"
#include "kis_types.h"
#include "kis_image.h"

class KisUpdatesFacade;

class KRITAIMAGE_EXPORT KisMergeLabeledLayersCommand : public KUndo2Command
{
public:
    /**
     * @brief Policies to stablish how the groups should be treated
     */
    enum GroupSelectionPolicy
    {
        /**
         * @brief Groups will be taken into account if their color label matches
         *        one of the selected color labels, even when the group has
         *        "no-color-label" as label
         */
        GroupSelectionPolicy_SelectAlways,
        /**
         * @brief Groups will be taken into account only if they have set
         *        an explicit color label. This ignores groups that have
         *        "no-color-label" as color label and the "no-color-label" label
         *        is included in the selected labels list
         */
        GroupSelectionPolicy_SelectIfColorLabeled,
        /**
         * @brief Groups will not be taken into account
         */
        GroupSelectionPolicy_NeverSelect
    };

    /**
     * @brief Basic info about a node. This is used to compare and see if the
     *        node changed
     */
    struct ReferenceNodeInfo
    {
        QUuid nodeId;
        int sequenceNumber;
        int opacity;

        bool operator==(const ReferenceNodeInfo &other) const
        {
            return nodeId == other.nodeId && sequenceNumber == other.sequenceNumber && opacity == other.opacity;
        }
    };

    using ReferenceNodeInfoList = QList<ReferenceNodeInfo>;
    using ReferenceNodeInfoListSP = QSharedPointer<ReferenceNodeInfoList>;

    /**
     * @brief Construct a new KisMergeLabeledLayersCommand that does not use
     *        a cache
     * @param image The image with the reference layers
     * @param newRefPaintDevice The resulting device that will contain the
     *                          merged result
     * @param selectedLabels The color labels of the reference layers
     * @param groupSelectionPolicy How to treat groups
     */
    KisMergeLabeledLayersCommand(KisImageSP image,
                                 KisPaintDeviceSP newRefPaintDevice,
                                 QList<int> selectedLabels,
                                 GroupSelectionPolicy groupSelectionPolicy = GroupSelectionPolicy_SelectAlways);

    /**
     * @brief Construct a new KisMergeLabeledLayersCommand that uses a cache
     * @param image The image with the reference layers
     * @param prevRefNodeInfoList The reference node info that is used to
     *                            compare with the current one to see if the
     *                            cache device can be used instead of generating
     *                            a new one
     * @param newRefNodeInfoList The resulting list of reference node info
     * @param prevRefPaintDevice The device that is used as a cache if possible
     * @param newRefPaintDevice The resulting device that will contain the
     *                          merged result
     * @param selectedLabels The color labels of the reference layers
     * @param groupSelectionPolicy How to treat groups
     * @param forceRegeneration If true, the cache is ignored and the merged
     *                          result is regenerated
     * @param activeNode The current node that is being edited.
     */
    KisMergeLabeledLayersCommand(KisImageSP image,
                                 ReferenceNodeInfoListSP prevRefNodeInfoList,
                                 ReferenceNodeInfoListSP newRefNodeInfoList,
                                 KisPaintDeviceSP prevRefPaintDevice,
                                 KisPaintDeviceSP newRefPaintDevice,
                                 QList<int> selectedLabels,
                                 GroupSelectionPolicy groupSelectionPolicy = GroupSelectionPolicy_SelectAlways,
                                 bool forceRegeneration = false,
                                 KisNodeSP activeNode = nullptr);
                                 
    ~KisMergeLabeledLayersCommand() override;

    void undo() override;
    void redo() override;

    static KisPaintDeviceSP createRefPaintDevice(KisImageSP originalImage, QString name = "Merge Labeled Layers Reference Paint Device");

private:
    void mergeLabeledLayers();
    bool hasToCheckForChangesInNodes() const;
    QPair<KisNodeSP, QPair<bool, bool>> collectNode(KisNodeSP node) const;
    bool collectNodes(KisNodeSP node, QList<KisNodeSP> &nodeList, ReferenceNodeInfoList &nodeInfoList) const;

private:
    KisImageSP m_refImage;
    ReferenceNodeInfoListSP m_prevRefNodeInfoList;
    ReferenceNodeInfoListSP m_newRefNodeInfoList;
    KisPaintDeviceSP m_prevRefPaintDevice;
    KisPaintDeviceSP m_newRefPaintDevice;
    KisNodeSP m_currentRoot;
    QList<int> m_selectedLabels;
    GroupSelectionPolicy m_groupSelectionPolicy;
    bool m_forceRegeneration;
    KisNodeSP m_activeNode;
};

#endif /* __KIS_MERGE_LABELED_LAYERS_H */

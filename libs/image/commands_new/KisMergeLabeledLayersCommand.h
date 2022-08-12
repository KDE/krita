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
    enum GroupSelectionPolicy
    {
        GroupSelectionPolicy_SelectAlways,
        GroupSelectionPolicy_SelectIfColorLabeled,
        GroupSelectionPolicy_NeverSelect
    };

    struct ReferenceNodeInfo
    {
        QUuid nodeId;
        int sequenceNumber;
        bool isVisible;
        int opacity;

        bool operator==(const ReferenceNodeInfo &other) const
        {
            return nodeId == other.nodeId && sequenceNumber == other.sequenceNumber &&
                   isVisible == other.isVisible && opacity == other.opacity;
        }
    };

    using ReferenceNodeInfoList = QList<ReferenceNodeInfo>;
    using ReferenceNodeInfoListSP = QSharedPointer<ReferenceNodeInfoList>;

    KisMergeLabeledLayersCommand(KisImageSP refImage,
                                 KisPaintDeviceSP newRefPaintDevice,
                                 KisNodeSP currentRoot,
                                 QList<int> selectedLabels,
                                 GroupSelectionPolicy groupSelectionPolicy = GroupSelectionPolicy_SelectAlways);

    KisMergeLabeledLayersCommand(KisImageSP refImage,
                                 ReferenceNodeInfoListSP prevRefNodeInfoList,
                                 ReferenceNodeInfoListSP newRefNodeInfoList,
                                 KisPaintDeviceSP prevRefPaintDevice,
                                 KisPaintDeviceSP newRefPaintDevice,
                                 KisNodeSP currentRoot,
                                 QList<int> selectedLabels,
                                 GroupSelectionPolicy groupSelectionPolicy = GroupSelectionPolicy_SelectAlways);
                                 
    ~KisMergeLabeledLayersCommand() override;

    void undo() override;
    void redo() override;

    static KisImageSP createRefImage(KisImageSP originalImage, QString name = "Reference Image");
    static KisPaintDeviceSP createRefPaintDevice(KisImageSP originalImage, QString name = "Reference Result Paint Device");

private:
    void mergeLabeledLayers();
    bool acceptNode(KisNodeSP node) const;
    bool checkChangesInNodes() const;

private:
    KisImageSP m_refImage;
    ReferenceNodeInfoListSP m_prevRefNodeInfoList;
    ReferenceNodeInfoListSP m_newRefNodeInfoList;
    KisPaintDeviceSP m_prevRefPaintDevice;
    KisPaintDeviceSP m_newRefPaintDevice;
    KisNodeSP m_currentRoot;
    QList<int> m_selectedLabels;
    GroupSelectionPolicy m_groupSelectionPolicy;
};

#endif /* __KIS_MERGE_LABELED_LAYERS_H */

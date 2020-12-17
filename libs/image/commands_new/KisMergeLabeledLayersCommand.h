/*
 *  SPDX-FileCopyrightText: 2020 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_MERGE_LABELED_LAYERS_H
#define __KIS_MERGE_LABELED_LAYERS_H

#include <QList>
#include <QString>

#include "kundo2command.h"
#include "kritaimage_export.h"
#include "kis_types.h"
#include "kis_image.h"



class KisUpdatesFacade;


class KRITAIMAGE_EXPORT KisMergeLabeledLayersCommand : public KUndo2Command
{
public:
    KisMergeLabeledLayersCommand(KisImageSP refImage, KisPaintDeviceSP refPaintDevice, KisNodeSP currentRoot, QList<int> selectedLabels);
    ~KisMergeLabeledLayersCommand() override;

    void undo() override;
    void redo() override;

    static KisImageSP createRefImage(KisImageSP originalImage, QString name);
    static KisPaintDeviceSP createRefPaintDevice(KisImageSP originalImage, QString name);

private:
    void mergeLabeledLayers();
    bool acceptNode(KisNodeSP node);

private:
    KisImageSP m_refImage;
    KisPaintDeviceSP m_refPaintDevice;
    KisNodeSP m_currentRoot;
    QList<int> m_selectedLabels;
};

#endif /* __KIS_MERGE_LABELED_LAYERS_H */

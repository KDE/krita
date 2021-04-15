/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KRITA_KISCOLORSMUDGEINTERSTROKEDATA_H
#define KRITA_KISCOLORSMUDGEINTERSTROKEDATA_H

#include "kis_types.h"
#include "KisInterstrokeData.h"
#include "KisOverlayPaintDeviceWrapper.h"

class KisTransaction;

struct KisColorSmudgeInterstrokeData : public KisInterstrokeData
{
    KisPaintDeviceSP colorBlendDevice;
    KisPaintDeviceSP heightmapDevice;
    KisPaintDeviceSP projectionDevice;
    KisOverlayPaintDeviceWrapper overlayDeviceWrapper;

    KisColorSmudgeInterstrokeData(KisPaintDeviceSP source);
    ~KisColorSmudgeInterstrokeData() override;

    void beginTransaction() override;

    KUndo2Command * endTransaction() override;

private:
    QScopedPointer<KUndo2Command> m_parentCommand;
    QScopedPointer<KisTransaction> m_colorBlendDeviceTransaction;
    QScopedPointer<KisTransaction> m_heightmapDeviceTransaction;
    QScopedPointer<KisTransaction> m_projectionDeviceTransaction;
};

#endif //KRITA_KISCOLORSMUDGEINTERSTROKEDATA_H

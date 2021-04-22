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

/**
 * Interstroke data needed for the lightness mode of colorsmudge brush.
 * It stores separate high-precision projections for the heightmap and
 * color layers. The layer itself stores only the low-precision final
 * projection, so as soon as the interstroke data is reset, the paint
 * is considered as "dried-out".
 */
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
    QScopedPointer<KisTransaction> m_heightmapDeviceTransaction;
};

#endif //KRITA_KISCOLORSMUDGEINTERSTROKEDATA_H

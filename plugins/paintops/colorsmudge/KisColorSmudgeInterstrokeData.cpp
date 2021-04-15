/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisColorSmudgeInterstrokeData.h"

#include <KoColorSpaceRegistry.h>

#include "kis_transaction.h"
#include "kis_paint_device.h"

KisColorSmudgeInterstrokeData::KisColorSmudgeInterstrokeData(KisPaintDeviceSP source)
        : KisInterstrokeData(source)
        , overlayDeviceWrapper(source, 2, KisOverlayPaintDeviceWrapper::PreciseMode)
{
    projectionDevice = overlayDeviceWrapper.overlay(0);
    colorBlendDevice = overlayDeviceWrapper.overlay(1);
    heightmapDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
}

KisColorSmudgeInterstrokeData::~KisColorSmudgeInterstrokeData()
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(!m_parentCommand);
}

void KisColorSmudgeInterstrokeData::beginTransaction()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(!m_parentCommand);

    m_parentCommand.reset(new KUndo2Command());
    m_colorBlendDeviceTransaction.reset(new KisTransaction(colorBlendDevice, m_parentCommand.data()));
    m_heightmapDeviceTransaction.reset(new KisTransaction(heightmapDevice, m_parentCommand.data()));
    m_projectionDeviceTransaction.reset(new KisTransaction(projectionDevice, m_parentCommand.data()));
}

KUndo2Command *KisColorSmudgeInterstrokeData::endTransaction()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_parentCommand, 0);

    // the internal undo commands are owned by m_parentCommand
    (void) m_colorBlendDeviceTransaction->endAndTake();
    (void) m_heightmapDeviceTransaction->endAndTake();
    (void) m_projectionDeviceTransaction->endAndTake();

    return m_parentCommand.take();
}

/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSmudgeOverlayModeOptionData.h"

#include <kis_paintop_settings.h>
#include <kis_properties_configuration.h>
#include <kis_paintop_lod_limitations.h>

bool KisSmudgeOverlayModeOptionData::read(const KisPropertiesConfiguration *setting)
{
    isChecked = setting->getBool("MergedPaint", false);
    return true;
}

void KisSmudgeOverlayModeOptionData::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty("MergedPaint", isChecked);
}

KisPaintopLodLimitations KisSmudgeOverlayModeOptionData::lodLimitations() const
{
    KisPaintopLodLimitations l;
    l.blockers << KoID("colorsmudge-overlay", i18nc("PaintOp instant preview limitation", "Overlay Option"));
    return l;
}

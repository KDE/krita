/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisPaintThicknessOptionData.h"

#include <kis_paintop_settings.h>


bool KisPaintThicknessOptionMixInImpl::read(const KisPropertiesConfiguration *setting)
{
    mode = (ThicknessMode)setting->getInt("PaintThicknessThicknessMode", OVERLAY);

    if (mode == RESERVED) {
        mode = OVERLAY;
    }

    return true;
}

void KisPaintThicknessOptionMixInImpl::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty("PaintThicknessThicknessMode", mode);
}

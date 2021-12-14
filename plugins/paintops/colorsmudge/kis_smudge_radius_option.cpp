/*
 *  SPDX-FileCopyrightText: 2014 Mohit Goyal <mohit.bits2011@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_smudge_radius_option.h"

#include <klocalizedstring.h>

#include <kis_painter.h>
#include <widgets/kis_curve_widget.h>


#include "kis_paint_device.h"


#include "KoPointerEvent.h"
#include "KoCanvasBase.h"
#include "kis_random_accessor_ng.h"
#include "KoColor.h"
#include <resources/KoColorSet.h>
#include <KoChannelInfo.h>
#include <KoMixColorsOp.h>
#include <kis_cross_device_color_sampler.h>



class KisRandomConstAccessorNG;

KisSmudgeRadiusOption::KisSmudgeRadiusOption()
    : KisRateOption(KoID("SmudgeRadius", i18n("Smudge Radius")), KisPaintOpOption::GENERAL, true)
{
    setValueRange(0.0,1.0);
}

void KisSmudgeRadiusOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisCurveOption::writeOptionSetting(setting);
    setting->setProperty(m_id.id() + "Version", 2);
}

void KisSmudgeRadiusOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisCurveOption::readOptionSetting(setting);

    const int smudgeRadiusVersion = setting->getInt(m_id.id() + "Version", 1);
    if (smudgeRadiusVersion < 2) {
        setValue(value() / 100.0);
    }
}

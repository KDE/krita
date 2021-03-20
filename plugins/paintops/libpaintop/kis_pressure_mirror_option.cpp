/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_pressure_mirror_option.h"

#include <klocalizedstring.h>

#include <kis_paint_device.h>
#include <widgets/kis_curve_widget.h>

#include <KoColor.h>

KisPressureMirrorOption::KisPressureMirrorOption()
    : KisCurveOption("Mirror", KisPaintOpOption::GENERAL, false)
{
    m_enableHorizontalMirror = false;
    m_enableVerticalMirror = false;
}

void KisPressureMirrorOption::enableHorizontalMirror(bool mirror)
{
    m_enableHorizontalMirror = mirror;
}

void KisPressureMirrorOption::enableVerticalMirror(bool mirror)
{
    m_enableVerticalMirror = mirror;
}

bool KisPressureMirrorOption::isHorizontalMirrorEnabled()
{
    return m_enableHorizontalMirror;
}

bool KisPressureMirrorOption::isVerticalMirrorEnabled()
{
    return m_enableVerticalMirror;
}

void KisPressureMirrorOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisCurveOption::writeOptionSetting(setting);
    setting->setProperty(MIRROR_HORIZONTAL_ENABLED, m_enableHorizontalMirror);
    setting->setProperty(MIRROR_VERTICAL_ENABLED, m_enableVerticalMirror);
}

void KisPressureMirrorOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisCurveOption::readOptionSetting(setting);
    m_enableHorizontalMirror = setting->getBool(MIRROR_HORIZONTAL_ENABLED, false);
    m_enableVerticalMirror = setting->getBool(MIRROR_VERTICAL_ENABLED, false);
}

MirrorProperties KisPressureMirrorOption::apply(const KisPaintInformation& info) const
{
    int mirrorXIncrement = info.canvasMirroredH();
    int mirrorYIncrement = info.canvasMirroredV();
    bool coordinateSystemFlipped = false;

    if (isChecked() && (m_enableHorizontalMirror || m_enableVerticalMirror)) {

        qreal sensorResult = computeSizeLikeValue(info);

        bool result = (sensorResult >= 0.5);

        mirrorXIncrement += result && m_enableHorizontalMirror;
        mirrorYIncrement += result && m_enableVerticalMirror;
        coordinateSystemFlipped = result &&
                                  (m_enableHorizontalMirror != m_enableVerticalMirror);
    }

    MirrorProperties mirrors;

    mirrors.verticalMirror = mirrorYIncrement % 2;
    mirrors.horizontalMirror = mirrorXIncrement % 2;
    mirrors.coordinateSystemFlipped = coordinateSystemFlipped;

    return mirrors;
}




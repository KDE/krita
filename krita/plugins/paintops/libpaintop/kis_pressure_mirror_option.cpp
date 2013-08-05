/* 
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_pressure_mirror_option.h"

#include <klocale.h>

#include <kis_paint_device.h>
#include <widgets/kis_curve_widget.h>

#include <KoColor.h>
#include <KoColorSpace.h>

KisPressureMirrorOption::KisPressureMirrorOption()
        : KisCurveOption(i18n("Mirror"), "Mirror", KisPaintOpOption::brushCategory(), false)
{
    m_enableHorizontalMirror = false;
    m_enableVerticalMirror = false;
    setMinimumLabel(i18n("Not mirrored"));
    setMaximumLabel(i18n("Mirrored"));
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

void KisPressureMirrorOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    KisCurveOption::writeOptionSetting(setting);
    setting->setProperty(MIRROR_HORIZONTAL_ENABLED, m_enableHorizontalMirror);
    setting->setProperty(MIRROR_VERTICAL_ENABLED, m_enableVerticalMirror);
}

void KisPressureMirrorOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    KisCurveOption::readOptionSetting(setting);
    m_enableHorizontalMirror = setting->getBool(MIRROR_HORIZONTAL_ENABLED,false);
    m_enableVerticalMirror = setting->getBool(MIRROR_VERTICAL_ENABLED, false);

    m_canvasAxisXMirrored = setting->getBool("runtimeCanvasMirroredX", false);
    m_canvasAxisYMirrored = setting->getBool("runtimeCanvasMirroredY", false);
}

MirrorProperties KisPressureMirrorOption::apply(const KisPaintInformation& info) const
{
    int mirrorXIncrement = m_canvasAxisXMirrored;
    int mirrorYIncrement = m_canvasAxisYMirrored;

    if (isChecked() && (m_enableHorizontalMirror || m_enableVerticalMirror)){
        qreal sensorResult = computeValue(info);
        bool result = (sensorResult >= 0.5);

        mirrorXIncrement += result && m_enableHorizontalMirror;
        mirrorYIncrement += result && m_enableVerticalMirror;
    }

    MirrorProperties mirrors;

    mirrors.verticalMirror = mirrorXIncrement % 2;
    mirrors.horizontalMirror = mirrorYIncrement % 2;

    return mirrors;
}




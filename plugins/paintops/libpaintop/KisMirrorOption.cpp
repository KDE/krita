/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisMirrorOption.h"


#include <kis_properties_configuration.h>
#include <kis_paint_information.h>
#include <KisMirrorOptionData.h>
#include <KisMirrorProperties.h>

KisMirrorOption::KisMirrorOption(const KisPropertiesConfiguration *setting)
    : KisCurveOption2(initializeFromData(setting))
{
}

MirrorProperties KisMirrorOption::apply(const KisPaintInformation &info) const
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

KisMirrorOptionData KisMirrorOption::initializeFromData(const KisPropertiesConfiguration *setting)
{
    KisMirrorOptionData data;
    data.read(setting);

    m_enableHorizontalMirror = data.enableHorizontalMirror;
    m_enableVerticalMirror = data.enableVerticalMirror;

    return data;
}

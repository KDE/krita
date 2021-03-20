/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2014 Mohit Goyal <mohit.bits2011@gmail.com>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include "kis_dynamic_sensor_fuzzy.h"

#include <QDomElement>
#include <brushengine/kis_paint_information.h>

#include <QCheckBox>
#include <QHBoxLayout>


KisDynamicSensorFuzzy::KisDynamicSensorFuzzy(bool fuzzyPerStroke, const QString &parentOptionName)
    : KisDynamicSensor(fuzzyPerStroke ? FUZZY_PER_STROKE : FUZZY_PER_DAB),
      m_fuzzyPerStroke(fuzzyPerStroke),
      m_perStrokeRandomSourceKey(parentOptionName + "FuzzyStroke")
{
}

void KisDynamicSensorFuzzy::reset()
{
}

bool KisDynamicSensorFuzzy::isAdditive() const
{
    return true;
}

qreal KisDynamicSensorFuzzy::value(const KisPaintInformation &info)
{
    qreal result = 0.0;

    if (!info.isHoveringMode()) {
        result = m_fuzzyPerStroke ?
            info.perStrokeRandomSource()->generateNormalized(m_perStrokeRandomSourceKey) :
            info.randomSource()->generateNormalized();
        result = 2.0 * result - 1.0;
    }

    return result;
}

bool KisDynamicSensorFuzzy::dependsOnCanvasRotation() const
{
    return false;
}



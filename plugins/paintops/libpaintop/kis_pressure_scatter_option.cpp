/*
 * SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_pressure_scatter_option.h"

#include <klocalizedstring.h>

#include <kis_paint_device.h>
#include <widgets/kis_curve_widget.h>

#include <KoColor.h>

#include <QtGlobal>


KisPressureScatterOption::KisPressureScatterOption()
    : KisCurveOption("Scatter", KisPaintOpOption::GENERAL, false, 1.0, 0.0, 5.0)
{
    m_axisX = true;
    m_axisY = true;
}

void KisPressureScatterOption::enableAxisX(bool enable)
{
    m_axisX = enable;
}

void KisPressureScatterOption::enableAxisY(bool enable)
{
    m_axisY = enable;
}

bool KisPressureScatterOption::isAxisXEnabled()
{
    return m_axisX;
}

bool KisPressureScatterOption::isAxisYEnabled()
{
    return m_axisY;
}

void KisPressureScatterOption::setScatterAmount(qreal amount)
{
    KisCurveOption::setValue(amount);
}

qreal KisPressureScatterOption::scatterAmount()
{
    return KisCurveOption::value();
}


void KisPressureScatterOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisCurveOption::writeOptionSetting(setting);
    setting->setProperty(SCATTER_X, m_axisX);
    setting->setProperty(SCATTER_Y, m_axisY);
}

void KisPressureScatterOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisCurveOption::readOptionSetting(setting);
    m_axisX = setting->getBool(SCATTER_X, true);
    m_axisY = setting->getBool(SCATTER_Y, true);

    // backward compatibility: test for a "scatter amount" property
    //                         and use this value if it does exist
    if (setting->hasProperty(SCATTER_AMOUNT) && !setting->hasProperty("ScatterValue"))
        KisCurveOption::setValue(setting->getDouble(SCATTER_AMOUNT));
}

QPointF KisPressureScatterOption::apply(const KisPaintInformation& info, qreal width, qreal height) const
{
    if ((!m_axisX && !m_axisY) || (!isChecked())) {
        return info.pos();
    }

    // just use the most significant dimension for calculations
    qreal diameter = qMax(width, height);
    qreal sensorValue = computeSizeLikeValue(info);

    qreal jitter = (2.0 * info.randomSource()->generateNormalized() - 1.0) * diameter * sensorValue;
    QPointF result(0.0, 0.0);

    if (m_axisX && m_axisY) {
        qreal jitterY = (2.0 * info.randomSource()->generateNormalized() - 1.0) * diameter * sensorValue;
        result = QPointF(jitter, jitterY);
        return info.pos() + result;
    }

    qreal drawingAngle = info.drawingAngle();
    QVector2D movement(cos(drawingAngle), sin(drawingAngle));
    if (m_axisX) {
        movement *= jitter;
        result = movement.toPointF();
    }
    else if (m_axisY) {
        QVector2D movementNormal(-movement.y(), movement.x());
        movementNormal *= jitter;
        result = movementNormal.toPointF();
    }

    return info.pos() + result;
}

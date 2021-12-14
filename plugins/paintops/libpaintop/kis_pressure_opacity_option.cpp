/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "kis_pressure_opacity_option.h"
#include <klocalizedstring.h>
#include <kis_painter.h>
#include <KoColor.h>

KisPressureOpacityOption::KisPressureOpacityOption()
    : KisCurveOption(KoID("Opacity", i18n("Opacity")), KisPaintOpOption::GENERAL, true)
{
    m_checkable = false;
}


void KisPressureOpacityOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisCurveOption::writeOptionSetting(setting);
    setting->setProperty("OpacityVersion", "2");
}

void KisPressureOpacityOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisCurveOption::readOptionSetting(setting);
    if (setting->getString("OpacityVersion", "1") == "1") {
        KisDynamicSensorSP pressureSensor = sensor(PRESSURE, true);
        if (pressureSensor) {
            QList<QPointF> points = pressureSensor->curve().points();
            QList<QPointF> points_new;
            Q_FOREACH (const QPointF & p, points) {
                points_new.push_back(QPointF(p.x() * 0.5, p.y()));
            }
            pressureSensor->setCurve(KisCubicCurve(points_new));
        }
    }
}

quint8 KisPressureOpacityOption::apply(KisPainter* painter, const KisPaintInformation& info) const
{

    if (!isChecked()) {
        return painter->opacity();
    }
    quint8 origOpacity = painter->opacity();

    qreal opacity = (qreal)(origOpacity * computeSizeLikeValue(info));
    quint8 opacity2 = (quint8)qRound(qBound<qreal>(OPACITY_TRANSPARENT_U8, opacity, OPACITY_OPAQUE_U8));

    painter->setOpacityUpdateAverage(opacity2);
    return origOpacity;
}

qreal KisPressureOpacityOption::getOpacityf(const KisPaintInformation& info)
{
    if (!isChecked()) return 1.0;
    return computeSizeLikeValue(info);
}


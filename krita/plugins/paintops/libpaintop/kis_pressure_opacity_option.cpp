/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
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
#include "kis_pressure_opacity_option.h"
#include <klocale.h>
#include <kis_painter.h>
#include <KoColor.h>
#include <KoColorSpace.h>


KisPressureOpacityOption::KisPressureOpacityOption()
        : KisCurveOption(i18n("Opacity"), "Opacity", KisPaintOpOption::brushCategory(), true)
{
    m_checkable = false;
    setMinimumLabel(i18n("Transparent"));
    setMaximumLabel(i18n("Opaque"));
}


void KisPressureOpacityOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    KisCurveOption::writeOptionSetting(setting);
    setting->setProperty("OpacityVersion", "2");
}

void KisPressureOpacityOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    KisCurveOption::readOptionSetting(setting);
    if (setting->getString("OpacityVersion", "1") == "1") {
        QList<QPointF> points = sensor()->curve().points();
        QList<QPointF> points_new;
        foreach(const QPointF &p, points)
        {
            points_new.push_back( QPointF(p.x() * 0.5, p.y()));
        }
        sensor()->setCurve(KisCubicCurve(points_new));
    }
}

quint8 KisPressureOpacityOption::apply(KisPainter * painter, const KisPaintInformation& info) const
{

    if (!isChecked()) {
        return painter->opacity();
    }
    quint8 origOpacity = painter->opacity();

    qreal opacity = (qreal)(origOpacity * computeValue(info));
    quint8 opacity2 = (quint8)qRound(qBound<qreal>(OPACITY_TRANSPARENT_U8, opacity, OPACITY_OPAQUE_U8));

    painter->setOpacity(opacity2);
    return origOpacity;
}

qreal KisPressureOpacityOption::getOpacityf(const KisPaintInformation& info)
{
    return isChecked() ? computeValue(info) : 1.0;
}


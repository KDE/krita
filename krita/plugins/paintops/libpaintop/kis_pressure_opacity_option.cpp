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
        : KisCurveOption(i18n("Opacity"), "Opacity", true)
{
}


quint8 KisPressureOpacityOption::apply(KisPainter * painter, double pressure) const
{

    if (!isChecked()) {
        return painter->opacity();
    }
    quint8 origOpacity = painter->opacity();

    qint32 opacity;
    if (!customCurve()) {
        opacity = (qint32)(origOpacity * pressure / PRESSURE_DEFAULT);
    } else {
        opacity = (qint32)(origOpacity * scaleToCurve(pressure) / PRESSURE_DEFAULT);
    }
    painter->setOpacity((qint8)qBound<qint32>(OPACITY_TRANSPARENT, opacity, OPACITY_OPAQUE ));

    return origOpacity;
}

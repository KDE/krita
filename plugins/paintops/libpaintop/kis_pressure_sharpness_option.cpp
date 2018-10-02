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

#include "kis_pressure_sharpness_option.h"

#include <klocalizedstring.h>

#include <widgets/kis_curve_widget.h>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <kis_fixed_paint_device.h>
#include <brushengine/kis_paintop.h>

KisPressureSharpnessOption::KisPressureSharpnessOption()
    : KisCurveOption("Sharpness", KisPaintOpOption::GENERAL, false)
{
}

void KisPressureSharpnessOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisCurveOption::writeOptionSetting(setting);
    setting->setProperty(SHARPNESS_THRESHOLD, m_threshold);
}

void KisPressureSharpnessOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisCurveOption::readOptionSetting(setting);
    m_threshold = setting->getInt(SHARPNESS_THRESHOLD, 4);

    // backward compatibility: test for a "sharpness factor" property
    //                         and use this value if it does exist
    if (setting->hasProperty(SHARPNESS_FACTOR) && !setting->hasProperty("SharpnessValue"))
        KisCurveOption::setValue(setting->getDouble(SHARPNESS_FACTOR));
}

void KisPressureSharpnessOption::apply(const KisPaintInformation &info, const QPointF &pt, qint32 &x, qint32 &y, qreal &xFraction, qreal &yFraction) const
{
    if (!isChecked() || KisCurveOption::value() == 0.0) {
        // brush
        KisPaintOp::splitCoordinate(pt.x(), &x, &xFraction);
        KisPaintOp::splitCoordinate(pt.y(), &y, &yFraction);
    }
    else {
        qreal processedSharpness = computeSizeLikeValue(info);

        if (processedSharpness == 1.0) {
            // pen
            xFraction = 0.0;
            yFraction = 0.0;
            x = qRound(pt.x());
            y = qRound(pt.y());
        }
        else {
            // something in between
            qint32 xi = qRound(pt.x());
            qint32 yi = qRound(pt.y());

            qreal xf = processedSharpness * xi + (1.0 - processedSharpness) * pt.x();
            qreal yf = processedSharpness * yi + (1.0 - processedSharpness) * pt.y();

            KisPaintOp::splitCoordinate(xf, &x, &xFraction);
            KisPaintOp::splitCoordinate(yf, &y, &yFraction);
        }
    }
}

void KisPressureSharpnessOption::applyThreshold(KisFixedPaintDeviceSP dab)
{
    if (!isChecked()) return;
    const KoColorSpace * cs = dab->colorSpace();

    // Set all alpha > opaque/2 to opaque, the rest to transparent.
    // XXX: Using 4/10 as the 1x1 circle brush paints nothing with 0.5.
    quint8* dabPointer = dab->data();
    QRect rc = dab->bounds();

    int pixelSize = dab->pixelSize();
    int pixelCount = rc.width() * rc.height();

    for (int i = 0; i < pixelCount; i++) {
        quint8 alpha = cs->opacityU8(dabPointer);

        if (alpha < (m_threshold * OPACITY_OPAQUE_U8) / 100) {
            cs->setOpacity(dabPointer, OPACITY_TRANSPARENT_U8, 1);
        }
        else {
            cs->setOpacity(dabPointer, OPACITY_OPAQUE_U8, 1);
        }

        dabPointer += pixelSize;
    }
}

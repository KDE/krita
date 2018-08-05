/*
 *  Copyright (C) 2014 Mohit Goyal <mohit.bits2011@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include <kis_cross_device_color_picker.h>



class KisRandomConstAccessorNG;

KisSmudgeRadiusOption::KisSmudgeRadiusOption():
    KisRateOption("SmudgeRadius", KisPaintOpOption::GENERAL, true)
{
    setValueRange(0.0,300.0);
}

QRect KisSmudgeRadiusOption::sampleRect(const KisPaintInformation& info,
                                        qreal diameter,
                                        const QPoint &pos) const
{
    const qreal sliderValue = computeSizeLikeValue(info);
    const int smudgeRadius = ((sliderValue * diameter) * 0.5) / 100.0;

    return kisGrowRect(QRect(pos, QSize(1,1)), smudgeRadius + 1);
}

void KisSmudgeRadiusOption::apply(KoColor *resultColor,
                                  const KisPaintInformation& info,
                                  qreal diameter,
                                  qreal posx,
                                  qreal posy,
                                  KisPaintDeviceSP dev) const
{
    if (!isChecked()) return;

    qreal sliderValue = computeSizeLikeValue(info);

    int smudgeRadius = ((sliderValue * diameter) * 0.5) / 100.0;


    KoColor color(Qt::transparent, dev->colorSpace());

    if (smudgeRadius == 1) {
        dev->pixel(posx, posy, &color);
    } else {

        const KoColorSpace* cs = dev->colorSpace();
        const int pixelSize = cs->pixelSize();

        quint8* data = new quint8[pixelSize];
        quint8* pixels[2];
        qint16 weights[2];

        pixels[1] = new quint8[pixelSize];
        pixels[0] = new quint8[pixelSize];

        int loop_increment = 1;
        if(smudgeRadius >= 8)
        {
            loop_increment = (2*smudgeRadius)/16;
        }
        int i = 0;
        int k = 0;
        int j = 0;

        KisRandomConstAccessorSP accessor = dev->createRandomConstAccessorNG(0, 0);
        accessor->moveTo(posx, posy);
        memcpy(color.data(), accessor->rawDataConst(), pixelSize);

        for (int y = 0; y <= smudgeRadius; y = y + loop_increment) {
            for (int x = 0; x <= smudgeRadius; x = x + loop_increment) {

                for(j = 0;j < 2;j++)
                {
                    if(j == 1)
                    {
                        y = y*(-1);
                    }
                    for(k = 0;k < 2;k++)
                    {
                        if(k == 1)
                        {
                            x = x*(-1);
                        }
                        accessor->moveTo(posx + x, posy + y);
                        memcpy(pixels[1], accessor->rawDataConst(), pixelSize);
                        if(i == 0)
                        {
                            memcpy(pixels[0],accessor->rawDataConst(),pixelSize);
                        }
                        if (x == 0 && y == 0) {
                            // Because the sum of the weights must be 255,
                            // we cheat a bit, and weigh the center pixel differently in order
                            // to sum to 255 in total
                            // It's -(counts -1), because we'll add the center one implicitly
                            // through that calculation
                            weights[1] = (255 - ((i + 1) * (255 /(i+2) )) );
                        } else {
                            weights[1] = 255 /(i+2);
                        }


                        i++;
                        if (i>smudgeRadius){i=0;}
                        weights[0] = 255 - weights[1];
                        const quint8** cpixels = const_cast<const quint8**>(pixels);
                        cs->mixColorsOp()->mixColors(cpixels, weights,2, data);
                        memcpy(pixels[0],data,pixelSize);


                    }
                    x = x*(-1);
                }
                y = y*(-1);
            }

        }

        color = KoColor(pixels[0],cs);

        for (int l = 0; l < 2; l++){
            delete[] pixels[l];
        }
        delete[] data;
    }

    *resultColor = color.convertedTo(resultColor->colorSpace());
}

void KisSmudgeRadiusOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisCurveOption::writeOptionSetting(setting);
}

void KisSmudgeRadiusOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisCurveOption::readOptionSetting(setting);
}

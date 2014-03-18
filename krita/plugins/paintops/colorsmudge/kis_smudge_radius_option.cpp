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

#include <klocale.h>

#include <kis_painter.h>
#include <widgets/kis_curve_widget.h>


#include "kis_paint_device.h"


#include "KoPointerEvent.h"
#include "KoCanvasBase.h"
#include "kis_random_accessor_ng.h"
#include "KoColor.h"
#include "KoResourceServerProvider.h"
#include "KoColorSet.h"
#include <KoChannelInfo.h>
#include <KoMixColorsOp.h>

#include <KoColor.h>



 class KisRandomConstAccessorNG;

KisSmudgeRadiusOption::KisSmudgeRadiusOption(const QString& name, const QString& label, bool checked, const QString& category):
    KisRateOption(name, label, checked, category)
{
    setValueRange(0,100);

}

void KisSmudgeRadiusOption::apply(KisPainter& painter, const KisPaintInformation& info, qreal scale, qreal posx, qreal posy) const
{
    int sliderValue = computeValue(info);
    int smudgeRadius = sliderValue;

    KisPaintDeviceSP dev =  painter.device();
    KoColor color = painter.paintColor();
    if (smudgeRadius == 1) {
        dev->pixel(posx, posy, &color);
        painter.setPaintColor(color);
    } else {
        // radius 2 ==> 9 pixels, 3 => 9 pixels, etc
        static int counts[] = { 0, 1, 9, 25, 45, 69, 109, 145, 193, 249 };

        const KoColorSpace* cs = dev->colorSpace();
        int pixelSize = cs->pixelSize();

        quint8* data = new quint8[pixelSize];
        static quint8** pixels = new quint8*[2];
        qint16* weights = new qint16[2];

        pixels[1] = new quint8[pixelSize];
        pixels[0] = new quint8[pixelSize];

        int loop_increment = 1;
        if(smudgeRadius >= 10)
        {
            loop_increment = smudgeRadius/16;
        }
        int i = 0;
        KisRandomConstAccessorSP accessor = dev->createRandomConstAccessorNG(0, 0);

        for (int y = -smudgeRadius; y <= smudgeRadius; y = y + loop_increment) {
            for (int x = -smudgeRadius; x <= smudgeRadius; x = x + loop_increment) {
                if (((x * x) + (y * y)) < smudgeRadius * smudgeRadius) {

                    accessor->moveTo(posx + x, posy + y);
                    memcpy(pixels[1], accessor->oldRawData(), pixelSize);
                    if(i == 0)
                    {
                        memcpy(pixels[0],accessor->oldRawData(),pixelSize);
                    }
                    if (x == 0 && y == 0) {
                        // Because the sum of the weights must be 255,
                        // we cheat a bit, and weigh the center pixel differently in order
                        // to sum to 255 in total
                        // It's -(counts -1), because we'll add the center one implicitly
                        // through that calculation
                        weights[1] = (255 - (i + 1) * (255 /(i+2)));
                    } else {
                        weights[1] = 255 /(i+2);
                    }
                    i++;
                    weights[0] = 255 - weights[1];
                    const quint8** cpixels = const_cast<const quint8**>(pixels);
                    cs->mixColorsOp()->mixColors(cpixels, weights,2, data);
                    memcpy(pixels[0],data,pixelSize);
                }

            }
        }
        // Weird, I can't do that directly :/

        painter.setPaintColor(KoColor(data, cs));

        for (i = 0; i < 2; i++){
            delete[] pixels[i];
        }
        //delete[] pixels;
        delete[] data;
    }


}

void KisSmudgeRadiusOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    KisCurveOption::writeOptionSetting(setting);
}

void KisSmudgeRadiusOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    KisCurveOption::readOptionSetting(setting);
}

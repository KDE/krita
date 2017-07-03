/*
 * Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com
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

#ifndef __KIS_QMIC_SIMPLE_CONVERTOR_H
#define __KIS_QMIC_SIMPLE_CONVERTOR_H

#include <QVector>
#include <QRect>
#include <kis_paint_device.h>
#include <gmic.h>

class QImage;

class KisQmicSimpleConvertor
{
public:
    static QImage convertToQImage(gmic_image<float>& gmicImage, float gmicMaxChannelValue = 255.0);
    static void convertFromQImage(const QImage &image, gmic_image<float> *gmicImage, float gmicUnitValue = 1.0);

    // output gmic image will have max channel 1.0 as in Krita's float rgba color-space
    static void convertToGmicImage(KisPaintDeviceSP dev, gmic_image<float> *gmicImage, QRect rc = QRect());
    // gmicMaxChannelValue indicates if the gmic image pixels rgb has range 0..255 or 0..1.0
    static void convertFromGmicImage(gmic_image<float>& gmicImage, KisPaintDeviceSP dst, float gmicMaxChannelValue);

    /// Fast versions
    static void convertFromGmicFast(gmic_image<float>& gmicImage, KisPaintDeviceSP dst, float gmicUnitValue);
    static void convertToGmicImageFast(KisPaintDeviceSP dev, gmic_image<float> *gmicImage, QRect rc = QRect());
};

#endif

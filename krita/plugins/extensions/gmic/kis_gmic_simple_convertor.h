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

#include <QVector>

#ifndef __KIS_GMIC_SIMPLE_CONVERTOR_H
#define __KIS_GMIC_SIMPLE_CONVERTOR_H

#include <gmic.h>
#include <kis_paint_device.h>


class KisGmicSimpleConvertor
{
public:
    KisGmicSimpleConvertor();
    ~KisGmicSimpleConvertor();

public:
    static QImage convertToQImage(gmic_image<float>& gmicImage);
    static void convertFromQImage(const QImage &image, gmic_image<float>& gmicImage, qreal multiplied = 255.0);

    // dev will be converted to float32 colorspace in-place to save some memory!!!
    void convertToGmicImage(KisPaintDeviceSP dev, gmic_image<float>& gmicImage, QRect rc = QRect());
    KisPaintDeviceSP convertFromGmicImage(gmic_image<float>& gmicImage);

    void setMultiplier(qreal multiplier) { m_multiplier = multiplier; }
private:
    qreal m_multiplier;
};

#endif

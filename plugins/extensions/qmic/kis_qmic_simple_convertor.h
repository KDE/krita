/*
 * SPDX-FileCopyrightText: 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_QMIC_SIMPLE_CONVERTOR_H
#define __KIS_QMIC_SIMPLE_CONVERTOR_H

#include <QRect>
#include <kis_paint_device.h>

#include "kis_qmic_interface.h"

class QImage;

class KisQmicSimpleConvertor
{
public:
    static QString blendingModeToString(QString blendMode);
    static QString stringToBlendingMode(QString str);

    static QImage convertToQImage(const KisQMicImage &gmicImage,
                                  float gmicMaxChannelValue = 255.0);
    static void convertFromQImage(const QImage &image,
                                  KisQMicImage &gmicImage,
                                  float gmicUnitValue = 1.0);

    // output gmic image will have max channel 255.0
    static void convertToGmicImage(KisPaintDeviceSP dev,
                                   KisQMicImage &gmicImage,
                                   QRect rc = QRect());
    // gmicMaxChannelValue indicates if the gmic image pixels rgb has range 0..255 or 0..1.0
    static void convertFromGmicImage(const KisQMicImage &gmicImage,
                                     KisPaintDeviceSP dst,
                                     float gmicMaxChannelValue);

    /// Fast versions
    static void convertFromGmicFast(const KisQMicImage &gmicImage,
                                    KisPaintDeviceSP dst,
                                    float gmicUnitValue);
    static void convertToGmicImageFast(KisPaintDeviceSP dev,
                                       KisQMicImage &gmicImage,
                                       QRect rc = QRect());
};

#endif

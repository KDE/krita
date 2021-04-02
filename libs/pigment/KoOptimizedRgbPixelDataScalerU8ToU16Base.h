/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KoOptimizedRgbPixelDataScalerU8ToU16BASE_H
#define KoOptimizedRgbPixelDataScalerU8ToU16BASE_H

#include <QtGlobal>
#include "kritapigment_export.h"


class KRITAPIGMENT_EXPORT KoOptimizedRgbPixelDataScalerU8ToU16Base
{
public:
    virtual ~KoOptimizedRgbPixelDataScalerU8ToU16Base();

    virtual void convertU8ToU16(const quint8 *src, int srcRowStride,
                                quint8 *dst, int dstRowStride,
                                int numRows, int numColumns) const = 0;

    virtual void convertU16ToU8(const quint8 *src, int srcRowStride,
                                quint8 *dst, int dstRowStride,
                                int numRows, int numColumns) const = 0;
};

#endif // KoOptimizedRgbPixelDataScalerU8ToU16BASE_H

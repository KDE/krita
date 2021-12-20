/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KoOptimizedRgbPixelDataScalerU8ToU16BASE_H
#define KoOptimizedRgbPixelDataScalerU8ToU16BASE_H

#include <QtGlobal>
#include "kritapigment_export.h"

/**
 * @brief Converts an RGB-like color space between U8 and U16 formats
 *
 * In some places we need to extend precision of the color space
 * in a very efficient way. It is specifically needed in the
 * colorsmudge engine, because it operates at an extremely low
 * levels of opacity. The conversion should also happen very
 * efficiently, because colorsmudge requests it on the fly right
 * when the user is painting on the canvas.
 *
 * The actual implementation is placed in class
 * `KoOptimizedRgbPixelDataScalerU8ToU16`.
 *
 * To create a scaler, just call a factory. It will create a version
 * of the scaler optimized for your CPU architecture.
 *
 * \code{.cpp}
 * QScopedPointer<KoOptimizedRgbPixelDataScalerU8ToU16Base> scaler(
 *     KoOptimizedRgbPixelDataScalerU8ToU16Factory::create());
 *
 * // ...
 *
 * // convert the data from U8 to U16
 * scaler->convertU8ToU16(src, srcRowStride,
 *                        dst, dstRowStride,
 *                        numRows, numColumns);
 *
 * // ...
 *
 * // convert the data back from U16 to U8
 * scaler->convertU16ToU8(src, srcRowStride,
 *                        dst, dstRowStride,
 *                        numRows, numColumns);
 *
 * \endcode
 */
class KRITAPIGMENT_EXPORT KoOptimizedRgbPixelDataScalerU8ToU16Base
{
public:
    KoOptimizedRgbPixelDataScalerU8ToU16Base(int channelsPerPixel);

    virtual ~KoOptimizedRgbPixelDataScalerU8ToU16Base();

    virtual void convertU8ToU16(const quint8 *src, int srcRowStride,
                                quint8 *dst, int dstRowStride,
                                int numRows, int numColumns) const = 0;

    virtual void convertU16ToU8(const quint8 *src, int srcRowStride,
                                quint8 *dst, int dstRowStride,
                                int numRows, int numColumns) const = 0;

    int channelsPerPixel() const;

protected:
    int m_channelsPerPixel;
};

#endif // KoOptimizedRgbPixelDataScalerU8ToU16BASE_H

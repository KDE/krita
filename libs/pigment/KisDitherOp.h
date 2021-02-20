/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "kritapigment_export.h"

#include <array>
#include <cmath>

#include <QScopedPointer>
#include <QtGlobal>

class KoColorSpace;
class KoID;

enum DitherType {
    DITHER_NONE = 0,
    DITHER_FAST = 1,
    DITHER_BEST = 2,

    DITHER_BAYER,
    DITHER_BLUE_NOISE,
};

class KRITAPIGMENT_EXPORT KisDitherOp
{
public:
    virtual ~KisDitherOp() = default;
    virtual void dither(const quint8 *src, quint8 *dst, int x, int y) const = 0;
    virtual void dither(const quint8 *srcRowStart, int srcRowStride, quint8 *dstRowStart, int dstRowStride, int x, int y, int columns, int rows) const = 0;

    /**
     * @return the identifier of this op's source depth
     */
    virtual KoID sourceDepthId() const = 0;

    /**
     * @return the identifier of this op's destination depth
     */
    virtual KoID destinationDepthId() const = 0;

    /**
     * @return the identifier of this op's type
     */
    virtual DitherType type() const = 0;
};

/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISMASKINGBRUSHCOMPOSITEOPBASE_H
#define KISMASKINGBRUSHCOMPOSITEOPBASE_H

#include <QtGlobal>

#include <kritaui_export.h>

class KRITAUI_EXPORT KisMaskingBrushCompositeOpBase
{
public:
    virtual ~KisMaskingBrushCompositeOpBase() = default;
    virtual void composite(const quint8 *srcRowStart, int srcRowStride,
                           quint8 *dstRowStart, int dstRowStride,
                           int columns, int rows) = 0;
};

#endif // KISMASKINGBRUSHCOMPOSITEOPBASE_H

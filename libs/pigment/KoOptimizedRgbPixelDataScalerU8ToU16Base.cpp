/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoOptimizedRgbPixelDataScalerU8ToU16Base.h"

KoOptimizedRgbPixelDataScalerU8ToU16Base::KoOptimizedRgbPixelDataScalerU8ToU16Base(int channelsPerPixel)
    : m_channelsPerPixel(channelsPerPixel)
{

}

KoOptimizedRgbPixelDataScalerU8ToU16Base::~KoOptimizedRgbPixelDataScalerU8ToU16Base()
{
}

int KoOptimizedRgbPixelDataScalerU8ToU16Base::channelsPerPixel() const
{
    return m_channelsPerPixel;
}

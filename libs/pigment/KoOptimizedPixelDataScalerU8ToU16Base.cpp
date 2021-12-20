/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoOptimizedPixelDataScalerU8ToU16Base.h"

KoOptimizedPixelDataScalerU8ToU16Base::KoOptimizedPixelDataScalerU8ToU16Base(int channelsPerPixel)
    : m_channelsPerPixel(channelsPerPixel)
{

}

KoOptimizedPixelDataScalerU8ToU16Base::~KoOptimizedPixelDataScalerU8ToU16Base()
{
}

int KoOptimizedPixelDataScalerU8ToU16Base::channelsPerPixel() const
{
    return m_channelsPerPixel;
}

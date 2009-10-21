/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoCtlMixColorsOp.h"
#include "KoCtlColorSpace.h"
#include "KoCtlAccumulator.h"
#include "KoCtlColorSpaceInfo.h"

KoCtlMixColorsOp::KoCtlMixColorsOp(const KoCtlColorSpace* _colorSpace, const KoCtlColorSpaceInfo* _info) : m_colorSpace(_colorSpace)
{
    m_accumulators = _info->accumulators();
}

KoCtlMixColorsOp::~KoCtlMixColorsOp()
{
}

void KoCtlMixColorsOp::mixColors(const quint8 * const* colors, const qint16 *weights, quint32 nColors, quint8 *dst) const
{
    foreach(KoCtlAccumulator* accumulator, m_accumulators) {
        accumulator->reset();
    }
    int alphaPos = m_colorSpace->alphaPos();
    // Compute the total for each channel by summing each colors multiplied by the weightlabcache
    double totalAlpha = 0.0;
    while (nColors--) {
        const quint8* color = *colors;
        double alphaTimesWeight;

        if (alphaPos != -1) {
            alphaTimesWeight = m_colorSpace->alpha(color) / 255.0;
        } else {
            alphaTimesWeight = 1.0;
        }
        alphaTimesWeight *= *weights;

        for (int i = 0; i < m_colorSpace->channelCount(); i++) {
            if (i != alphaPos) {
                m_accumulators[i]->mix(color, alphaTimesWeight);
            }
        }
        totalAlpha += alphaTimesWeight;
        ++colors;
        ++weights;
    }
    if (totalAlpha > 255.0) totalAlpha = 255.0;

    if (totalAlpha > 0) {
        double inv = 1.0 / totalAlpha;
        for (int i = 0; i < m_colorSpace->channelCount(); i++) {
            if (i != alphaPos) {
                m_accumulators[i]->affect(dst, inv);
            }
        }
        m_colorSpace->setAlpha(dst, totalAlpha, 1);
    } else {
        memset(dst, 0, m_colorSpace->pixelSize());
    }
}

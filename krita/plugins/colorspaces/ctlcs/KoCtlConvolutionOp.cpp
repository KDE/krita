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

#include "KoCtlConvolutionOp.h"
#include "KoCtlColorSpace.h"
#include "KoCtlAccumulator.h"
#include "KoCtlColorSpaceInfo.h"

KoCtlConvolutionOp::KoCtlConvolutionOp(KoCtlColorSpace* _colorSpace, const KoCtlColorSpaceInfo* _info) : m_colorSpace(_colorSpace)
{
    m_accumulators = _info->accumulators();
}

KoCtlConvolutionOp::~KoCtlConvolutionOp()
{
    qDeleteAll(m_accumulators);
}

void KoCtlConvolutionOp::convolveColors(const quint8* const* colors, const qreal* kernelValues, quint8 *dst, qreal factor, qreal offset, qint32 nPixels, const QBitArray & channelFlags) const
{
    foreach(KoCtlAccumulator* accumulator, m_accumulators) {
        accumulator->reset();
    }

    qreal totalWeight = 0;
    qreal totalWeightTransparent = 0;
    int channelsNb = m_colorSpace->channelCount();
    int alphaPos = m_colorSpace->alphaPos();

    for (; nPixels--; colors++, kernelValues++) {
        qreal weight = *kernelValues;
        if (weight != 0) {
            if (m_colorSpace->alpha(*colors) == 0) {
                totalWeightTransparent += weight;
            } else {
                for (int i = 0; i < channelsNb; i++) {
                    m_accumulators[i]->mix(colors[i], weight);
                }
            }
            totalWeight += weight;
        }
    }

    bool allChannels = channelFlags.isEmpty();
    Q_ASSERT(allChannels || channelFlags.size() == channelsNb);
    if (totalWeightTransparent == 0) {
        for (int i = 0; i < channelsNb; i++) {
            if ((allChannels and i != (uint)alphaPos)
                    or(not allChannels and channelFlags.testBit(i))) {
                m_accumulators[i]->affect(dst, factor, offset);
            }
        }
    } else if (totalWeightTransparent != totalWeight) {
        if (totalWeight == factor) {
            qint64 a = (totalWeight - totalWeightTransparent);
            for (int i = 0; i < channelsNb; i++) {
                if (allChannels || channelFlags.testBit(i)) {
                    if (i == (uint)alphaPos) {
                        m_accumulators[i]->affect(dst, totalWeight, offset);
                    } else {
                        m_accumulators[i]->affect(dst, a, offset);
                    }
                }
            }
        } else {
            qreal a = totalWeight / (factor * (totalWeight - totalWeightTransparent));     // use qreal as it easily saturate
            for (int i = 0; i < channelsNb; i++) {
                if (allChannels || channelFlags.testBit(i)) {
                    if (i == (uint)alphaPos) {
                        m_accumulators[i]->affect(dst, factor, offset);
                    } else {
                        m_accumulators[i]->affect(dst, a, offset);
                    }
                }
            }
        }
    }
}

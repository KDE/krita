/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

#ifndef KO_CONVOLUTION_OP_IMPL_H
#define KO_CONVOLUTION_OP_IMPL_H

#include "DebugPigment.h"
#include "KoColorSpaceMaths.h"
#include "KoConvolutionOp.h"
#include "KoColorSpaceTraits.h"

template<class _CSTrait>
class KoConvolutionOpImpl : public KoConvolutionOp
{
    typedef typename KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::compositetype compositetype;
    typedef typename _CSTrait::channels_type channels_type;
public:

    KoConvolutionOpImpl() { }

    ~KoConvolutionOpImpl() override { }

    /**
     * Calculates a weighted average of the pixels, mentioned in @p colors
     * using weight values from @p kernelValues
     *
     * Note:
     * It behaves in a quite unclear way, when at least one pixel is
     * fully transparent. There are three cases:
     * Case A) None of the pixels is fully transparent.
     *    * Every color channel AND alpha channel of @p dst stores a sum
     *      of the corresponding channels from @p colors, divided by @p factor
     *      and incremented by @p offset
     * Case B) At least one pixel of @p colors is transparent and @p factor
     * stores a weight of the kernel (sum of it's items).
     *    * Every color channel of @p dst stores a sum of the corresponding
     *      channels from non-transparent pixels, divided by a weight
     *      of non-transparent pixels and incremented by @p offset.
     *    * Alpha channel of @p dst stores a sum of the corresponding
     *      channels from non-transparent pixels, divided by a weight
     *      of all the pixels (equals to @p factor) and incremented
     *      by @p offset.
     * Case C) At least one pixel of @p colors is transparent and @p factor
     * is set to an arbitrary value.
     *    * Every color channel of @p dst stores a sum of the corresponding
     *      channels from non-transparent pixels, divided by a "scaled
     *      down factor" and incremented by @p offset. "Scaled
     *      down factor" is calculated in the following way:
     *
     *                                   [weight of non-transparent pixels]
     *      scaledDownFactor = @p factor * ----------------------------------
     *                                       [weight of all the pixels]
     *
     *    * Alpha channel of @p dst stores a sum of the corresponding
     *      channels from non-transparent pixels, divided by unscaled
     *      @p factor and incremented by @p offset.
     */

    void convolveColors(const quint8* const* colors, const qreal* kernelValues, quint8 *dst, qreal factor, qreal offset, qint32 nPixels, const QBitArray & channelFlags) const override {

        // Create and initialize to 0 the array of totals
        qreal totals[_CSTrait::channels_nb];

        qreal totalWeight = 0;
        qreal totalWeightTransparent = 0;

        memset(totals, 0, sizeof(qreal) * _CSTrait::channels_nb);

        for (; nPixels--; colors++, kernelValues++) {
            qreal weight = *kernelValues;
            const channels_type* color = _CSTrait::nativeArray(*colors);
            if (weight != 0) {
                if (_CSTrait::opacityU8(*colors) == 0) {
                    totalWeightTransparent += weight;
                } else {
                    for (uint i = 0; i < _CSTrait::channels_nb; i++) {
                        totals[i] += color[i] * weight;
                    }
                }
                totalWeight += weight;
            }
        }

        typename _CSTrait::channels_type* dstColor = _CSTrait::nativeArray(dst);

        bool allChannels = channelFlags.isEmpty();
        Q_ASSERT(allChannels || channelFlags.size() == (int)_CSTrait::channels_nb);
        if (totalWeightTransparent == 0) {
            // Case A)
            for (uint i = 0; i < _CSTrait::channels_nb; i++) {
                if (allChannels || channelFlags.testBit(i)) {
                    compositetype v = totals[i] / factor + offset;
                    dstColor[ i ] = CLAMP(v, KoColorSpaceMathsTraits<channels_type>::min,
                                          KoColorSpaceMathsTraits<channels_type>::max);
                }
            }
        } else if (totalWeightTransparent != totalWeight) {
            if (totalWeight == factor) {
                // Case B)
                qint64 a = (totalWeight - totalWeightTransparent);
                for (uint i = 0; i < _CSTrait::channels_nb; i++) {
                    if (allChannels || channelFlags.testBit(i)) {
                        if (i == (uint)_CSTrait::alpha_pos) {
                            compositetype v = totals[i] / totalWeight + offset;
                            dstColor[ i ] = CLAMP(v, KoColorSpaceMathsTraits<channels_type>::min,
                                                  KoColorSpaceMathsTraits<channels_type>::max);
                        } else {
                            compositetype v = totals[i] / a + offset;
                            dstColor[ i ] = CLAMP(v, KoColorSpaceMathsTraits<channels_type>::min,
                                                  KoColorSpaceMathsTraits<channels_type>::max);
                        }
                    }
                }
            } else {
                // Case C)
                qreal a = qreal(totalWeight) / (factor * (totalWeight - totalWeightTransparent));     // use qreal as it easily saturate
                for (uint i = 0; i < _CSTrait::channels_nb; i++) {
                    if (allChannels || channelFlags.testBit(i)) {
                        if (i == (uint)_CSTrait::alpha_pos) {
                            compositetype v = totals[i] / factor + offset;
                            dstColor[ i ] = CLAMP(v, KoColorSpaceMathsTraits<channels_type>::min,
                                                  KoColorSpaceMathsTraits<channels_type>::max);
                        } else {
                            compositetype v = (compositetype)(totals[i] * a + offset);
                            dstColor[ i ] = CLAMP(v, KoColorSpaceMathsTraits<channels_type>::min,
                                                  KoColorSpaceMathsTraits<channels_type>::max);
                        }
                    }
                }
            }
        }

    }
};

#endif

/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
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

#ifndef KOMIXCOLORSOPIMPL_H
#define KOMIXCOLORSOPIMPL_H

#include "KoColorSpace.h"

template<class _CSTrait>
class KoMixColorsOpImpl : public KoMixColorsOp
{
public:
    KoMixColorsOpImpl() {
    }
    virtual ~KoMixColorsOpImpl() { }
    virtual void mixColors(const quint8 * const* colors, const qint16 *weights, quint32 nColors, quint8 *dst) const {
        // Create and initialize to 0 the array of totals
        typename KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::compositetype totals[_CSTrait::channels_nb];
        typename KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::compositetype totalAlpha = 0;

        memset(totals, 0, sizeof(totals));

        // Compute the total for each channel by summing each colors multiplied by the weightlabcache

        while (nColors--) {
            const typename _CSTrait::channels_type* color = _CSTrait::nativeArray(*colors);
            typename KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::compositetype alphaTimesWeight;

            if (_CSTrait::alpha_pos != -1) {
                alphaTimesWeight = color[_CSTrait::alpha_pos];
            } else {
                alphaTimesWeight = KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::unitValue;
            }

            alphaTimesWeight *= *weights;

            for (int i = 0; i < (int)_CSTrait::channels_nb; i++) {
                if (i != _CSTrait::alpha_pos) {
                    totals[i] += color[i] * alphaTimesWeight;
                }
            }

            totalAlpha += alphaTimesWeight;
            colors++;
            weights++;
        }

        // set totalAlpha to the minimum between its value and the unit value of the channels
        const int sumOfWeights = 255;

        if (totalAlpha > KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::unitValue * sumOfWeights) {
            totalAlpha = KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::unitValue * sumOfWeights;
        }

        typename _CSTrait::channels_type* dstColor = _CSTrait::nativeArray(dst);

        if (totalAlpha > 0) {

            for (int i = 0; i < (int)_CSTrait::channels_nb; i++) {
                if (i != _CSTrait::alpha_pos) {

                    typename KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::compositetype v = totals[i] / totalAlpha;

                    if (v > KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::max) {
                        v = KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::max;
                    }
                    if (v < KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::min) {
                        v = KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::min;
                    }
                    dstColor[ i ] = v;
                }
            }

            if (_CSTrait::alpha_pos != -1) {
                dstColor[ _CSTrait::alpha_pos ] = totalAlpha / sumOfWeights;
            }
        } else {
            memset(dst, 0, sizeof(typename _CSTrait::channels_type) * _CSTrait::channels_nb);
        }
    }
};

#endif

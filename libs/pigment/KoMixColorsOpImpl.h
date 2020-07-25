/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2007 Emanuele Tamponi <emanuele@valinor.it>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KOMIXCOLORSOPIMPL_H
#define KOMIXCOLORSOPIMPL_H

#include "KoMixColorsOp.h"

#include <type_traits>
#include <KisCppQuirks.h>

template <typename T>
static inline T safeDivideWithRound(T dividend,
                                    std::enable_if_t<std::is_floating_point<T>::value, T> divisor) {
    return dividend / divisor;
}

template <typename T>
static inline T safeDivideWithRound(T dividend,
                                    std::enable_if_t<std::is_integral<T>::value, T> divisor) {
    return (dividend + divisor / 2) / divisor;
}



template<class _CSTrait>
class KoMixColorsOpImpl : public KoMixColorsOp
{
public:
    KoMixColorsOpImpl() {
    }
    ~KoMixColorsOpImpl() override { }
    void mixColors(const quint8 * const* colors, const qint16 *weights, quint32 nColors, quint8 *dst, int weightSum = 255) const override {
        mixColorsImpl(ArrayOfPointers(colors), WeightsWrapper(weights, weightSum), nColors, dst);
    }

    void mixColors(const quint8 *colors, const qint16 *weights, quint32 nColors, quint8 *dst, int weightSum = 255) const override {
        mixColorsImpl(PointerToArray(colors, _CSTrait::pixelSize), WeightsWrapper(weights, weightSum), nColors, dst);
    }

    void mixColors(const quint8 * const* colors, quint32 nColors, quint8 *dst) const override {
        mixColorsImpl(ArrayOfPointers(colors), NoWeightsSurrogate(nColors), nColors, dst);
    }

    void mixColors(const quint8 *colors, quint32 nColors, quint8 *dst) const override {
        mixColorsImpl(PointerToArray(colors, _CSTrait::pixelSize), NoWeightsSurrogate(nColors), nColors, dst);
    }

    void mixTwoColorArrays(const quint8* colorsA, const quint8* colorsB, quint32 nColors, qreal weight, quint8* dst) const override {
        const quint8* pixelA = colorsA;
        const quint8* pixelB = colorsB;
        weight = qBound(0.0, weight, 1.0);
        for (int i = 0; i < nColors; i++) {
            const quint8* colors[2];
            colors[0] = pixelA;
            colors[1] = pixelB;
            qint16 weights[2];
            weights[1] = qRound(weight * 255.0);
            weights[0] = 255 - weights[1];
            mixColorsImpl(ArrayOfPointers(colors), WeightsWrapper(weights, 255), 2, dst);

            pixelA += _CSTrait::pixelSize;
            pixelB += _CSTrait::pixelSize;
            dst += _CSTrait::pixelSize;
        }
    }

    void mixArrayWithColor(const quint8* colorArray, const quint8* color, quint32 nColors, qreal weight, quint8* dst) const override {
        const quint8* pixelA = colorArray;
        weight = qBound(0.0, weight, 1.0);
        for (int i = 0; i < nColors; i++) {
            const quint8* colors[2];
            colors[0] = pixelA;
            colors[1] = color;
            qint16 weights[2];
            weights[1] = qRound(weight * 255.0);
            weights[0] = 255 - weights[1];
            mixColorsImpl(ArrayOfPointers(colors), WeightsWrapper(weights, 255), 2, dst);

            pixelA += _CSTrait::pixelSize;
            dst += _CSTrait::pixelSize;
        }
    }

private:
    struct ArrayOfPointers {
        ArrayOfPointers(const quint8 * const* colors)
            : m_colors(colors)
        {
        }

        const quint8* getPixel() const {
            return *m_colors;
        }

        void nextPixel() {
            m_colors++;
        }

    private:
        const quint8 * const * m_colors;
    };

    struct PointerToArray {
        PointerToArray(const quint8 *colors, int pixelSize)
            : m_colors(colors),
              m_pixelSize(pixelSize)
        {
        }

        const quint8* getPixel() const {
            return m_colors;
        }

        void nextPixel() {
            m_colors += m_pixelSize;
        }

    private:
        const quint8 *m_colors;
        const int m_pixelSize;
    };

    struct WeightsWrapper
    {
        typedef typename KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::compositetype compositetype;

        WeightsWrapper(const qint16 *weights, int weightSum)
            : m_weights(weights), m_sumOfWeights(weightSum)
        {
        }

        inline void nextPixel() {
            m_weights++;
        }

        inline void premultiplyAlphaWithWeight(compositetype &alpha) const {
            alpha *= *m_weights;
        }

        inline int normalizeFactor() const {
            return m_sumOfWeights;
        }

    private:
        const qint16 *m_weights;
        int m_sumOfWeights {0};
    };

    struct NoWeightsSurrogate
    {
        typedef typename KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::compositetype compositetype;

        NoWeightsSurrogate(int numPixels)
            : m_numPixles(numPixels)
        {
        }

        inline void nextPixel() {
        }

        inline void premultiplyAlphaWithWeight(compositetype &) const {
        }

        inline int normalizeFactor() const {
            return m_numPixles;
        }

    private:
        const int m_numPixles;
    };

    template<class AbstractSource, class WeightsWrapper>
    void mixColorsImpl(AbstractSource source, WeightsWrapper weightsWrapper, quint32 nColors, quint8 *dst) const {
        // Create and initialize to 0 the array of totals
        typename KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::compositetype totals[_CSTrait::channels_nb];
        typename KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::compositetype totalAlpha = 0;

        memset(totals, 0, sizeof(totals));

        // Compute the total for each channel by summing each colors multiplied by the weightlabcache

        while (nColors--) {
            const typename _CSTrait::channels_type* color = _CSTrait::nativeArray(source.getPixel());
            typename KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::compositetype alphaTimesWeight;

            if (_CSTrait::alpha_pos != -1) {
                alphaTimesWeight = color[_CSTrait::alpha_pos];
            } else {
                alphaTimesWeight = KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::unitValue;
            }

            weightsWrapper.premultiplyAlphaWithWeight(alphaTimesWeight);

            for (int i = 0; i < (int)_CSTrait::channels_nb; i++) {
                if (i != _CSTrait::alpha_pos) {
                    totals[i] += color[i] * alphaTimesWeight;
                }
            }

            totalAlpha += alphaTimesWeight;
            source.nextPixel();
            weightsWrapper.nextPixel();
        }

        // set totalAlpha to the minimum between its value and the unit value of the channels
        const typename KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::compositetype sumOfWeights = weightsWrapper.normalizeFactor();

        if (totalAlpha > KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::unitValue * sumOfWeights) {
            totalAlpha = KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::unitValue * sumOfWeights;
        }

        typename _CSTrait::channels_type* dstColor = _CSTrait::nativeArray(dst);

        /**
         * FIXME: The following code relies on the unit value for floating point spaces being 1.0
         * We should be using the division functions in KoColorSpaceMaths for this, but right now
         * it is not clear how to call these functions.
         **/
        if (totalAlpha > 0) {

            for (int i = 0; i < (int)_CSTrait::channels_nb; i++) {
                if (i != _CSTrait::alpha_pos) {

                    typename KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::compositetype v = safeDivideWithRound(totals[i], totalAlpha);

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
                dstColor[ _CSTrait::alpha_pos ] = safeDivideWithRound(totalAlpha, sumOfWeights);
            }
        } else {
            memset(dst, 0, sizeof(typename _CSTrait::channels_type) * _CSTrait::channels_nb);
        }
    }

};

#endif

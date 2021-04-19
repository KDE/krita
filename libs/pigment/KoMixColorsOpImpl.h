/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2007 Emanuele Tamponi <emanuele@valinor.it>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KOMIXCOLORSOPIMPL_H
#define KOMIXCOLORSOPIMPL_H

#include "KoMixColorsOp.h"

#include <QtGlobal>
#include <type_traits>
#include <KisCppQuirks.h>
#include <KoColorSpaceMaths.h>
#include "kis_debug.h"
#include "kis_global.h"

#define SANITY_CHECKS

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

    Mixer* createMixer() const override;

    void mixColors(const quint8 * const* colors, const qint16 *weights, int nColors, quint8 *dst, int weightSum = 255) const override {
        mixColorsImpl(ArrayOfPointers(colors), WeightsWrapper(weights, weightSum), nColors, dst);
    }

    void mixColors(const quint8 *colors, const qint16 *weights, int nColors, quint8 *dst, int weightSum = 255) const override {
        mixColorsImpl(PointerToArray(colors, _CSTrait::pixelSize), WeightsWrapper(weights, weightSum), nColors, dst);
    }

    void mixColors(const quint8 * const* colors, int nColors, quint8 *dst) const override {
        mixColorsImpl(ArrayOfPointers(colors), NoWeightsSurrogate(nColors), nColors, dst);
    }

    void mixColors(const quint8 *colors, int nColors, quint8 *dst) const override {
        mixColorsImpl(PointerToArray(colors, _CSTrait::pixelSize), NoWeightsSurrogate(nColors), nColors, dst);
    }

    void mixTwoColorArrays(const quint8* colorsA, const quint8* colorsB, int nColors, qreal weight, quint8* dst) const override {
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

    void mixArrayWithColor(const quint8* colorArray, const quint8* color, int nColors, qreal weight, quint8* dst) const override {
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
    class MixerImpl;

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
        typedef typename KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::mixtype mixtype;

        WeightsWrapper(const qint16 *weights, int weightSum)
            : m_weights(weights), m_sumOfWeights(weightSum)
        {
        }

        inline void nextPixel() {
            m_weights++;
        }

        inline void premultiplyAlphaWithWeight(mixtype &alpha) const {
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
        typedef typename KoColorSpaceMathsTraits<typename _CSTrait::channels_type>::mixtype mixtype;

        NoWeightsSurrogate(int numPixels)
            : m_numPixles(numPixels)
        {
        }

        inline void nextPixel() {
        }

        inline void premultiplyAlphaWithWeight(mixtype &) const {
        }

        inline int normalizeFactor() const {
            return m_numPixles;
        }

    private:
        const int m_numPixles;
    };

    class MixDataResult {
        using channels_type = typename _CSTrait::channels_type;
        using mix_type = typename KoColorSpaceMathsTraits<channels_type>::mixtype;
        using MathsTraits = KoColorSpaceMathsTraits<channels_type>;

        mix_type totals[_CSTrait::channels_nb];
        mix_type totalAlpha = 0;
        qint64 normalizeFactor = 0;

#ifdef SANITY_CHECKS
        qint64 m_numPixels = 0;
#endif

    public:
        MixDataResult() {
            memset(totals, 0, sizeof(totals));
        }

        void computeMixedColor(quint8 *dst) {
#ifdef SANITY_CHECKS

            const mix_type maxSaneNumPixels =
                std::numeric_limits<mix_type>::max() / pow2(mix_type(MathsTraits::unitValue));

            if (m_numPixels > maxSaneNumPixels) {
                qWarning() << "SANITY CHECK FAILED: KoMixColorOp got too many pixels to mix, the containing type may overflow";
                qWarning() << "    " << ppVar(m_numPixels);
                qWarning() << "    " << ppVar(maxSaneNumPixels);
            }
#endif

            // set totalAlpha to the minimum between its value and the unit value of the channels
            const mix_type sumOfWeights = normalizeFactor;

            if (totalAlpha > MathsTraits::unitValue * sumOfWeights) {
                totalAlpha = MathsTraits::unitValue * sumOfWeights;
            }

            channels_type* dstColor = _CSTrait::nativeArray(dst);

            /**
             * FIXME: The following code relies on the unit value for floating point spaces being 1.0
             * We should be using the division functions in KoColorSpaceMaths for this, but right now
             * it is not clear how to call these functions.
             **/
            if (totalAlpha > 0) {

                for (int i = 0; i < (int)_CSTrait::channels_nb; i++) {
                    if (i != _CSTrait::alpha_pos) {

                        mix_type v = safeDivideWithRound(totals[i], totalAlpha);

                        if (v > MathsTraits::max) {
                            v = MathsTraits::max;
                        }
                        if (v < MathsTraits::min) {
                            v = MathsTraits::min;
                        }
                        dstColor[ i ] = v;
                    }
                }

                if (_CSTrait::alpha_pos != -1) {
                    dstColor[ _CSTrait::alpha_pos ] = safeDivideWithRound(totalAlpha, sumOfWeights);
                }
            } else {
                memset(dst, 0, sizeof(channels_type) * _CSTrait::channels_nb);
            }
        }

        template<class AbstractSource, class WeightsWrapper>
        void accumulateColors(AbstractSource source, WeightsWrapper weightsWrapper, int nColors) {
            // Compute the total for each channel by summing each colors multiplied by the weightlabcache

#ifdef SANITY_CHECKS
            m_numPixels += nColors;
#endif

            while (nColors--) {
                const channels_type* color = _CSTrait::nativeArray(source.getPixel());
                mix_type alphaTimesWeight;

                if (_CSTrait::alpha_pos != -1) {
                    alphaTimesWeight = color[_CSTrait::alpha_pos];
                } else {
                    alphaTimesWeight = MathsTraits::unitValue;
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

            normalizeFactor += weightsWrapper.normalizeFactor();
        }

        qint64 currentWeightsSum() const
        {
            return normalizeFactor;
        }
    };

    template<class AbstractSource, class WeightsWrapper>
    void mixColorsImpl(AbstractSource source, WeightsWrapper weightsWrapper, int nColors, quint8 *dst) const {
        MixDataResult result;
        result.accumulateColors(source, weightsWrapper, nColors);
        result.computeMixedColor(dst);
    }

};

template<class _CSTrait>
class KoMixColorsOpImpl<_CSTrait>::MixerImpl : public KoMixColorsOp::Mixer
{
public:
    MixerImpl()
    {
    }

    void accumulate(const quint8 *data, const qint16 *weights, int weightSum, int nPixels) override
    {
        result.accumulateColors(PointerToArray(data, _CSTrait::pixelSize), WeightsWrapper(weights, weightSum), nPixels);
    }

    void accumulateAverage(const quint8 *data, int nPixels) override
    {
        result.accumulateColors(PointerToArray(data, _CSTrait::pixelSize), NoWeightsSurrogate(nPixels), nPixels);
    }

    void computeMixedColor(quint8 *data) override
    {
        result.computeMixedColor(data);
    }

    qint64 currentWeightsSum() const override
    {
        return result.currentWeightsSum();
    }

private:
    MixDataResult result;
};

template<class _CSTrait>
KoMixColorsOp::Mixer *KoMixColorsOpImpl<_CSTrait>::createMixer() const
{
    return new MixerImpl();
}

#endif

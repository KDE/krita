/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCOLORPAIRSAMPLER_H
#define KISCOLORPAIRSAMPLER_H

#include <boost/iterator/iterator_facade.hpp>

#include <KoColor.h>
#include <KoColorModelStandardIds.h>

struct KisColorPairSampler
{
    std::vector<qreal> redColorValues = {0.0};
    std::vector<qreal> greenColorValues = {0.0};
    std::vector<qreal> blueColorValues = {0.0};
    std::vector<qreal> alphaValues = {1.0};

    size_t numSamples() const {
        return
            alphaValues.size() *
            alphaValues.size() *
            alphaValues.size() *
            redColorValues.size() *
            redColorValues.size() *
            greenColorValues.size() *
            greenColorValues.size() *
            blueColorValues.size() *
            blueColorValues.size();
    }

    struct SampleIndex {
        SampleIndex(size_t index, const KisColorPairSampler &sampler)
        {
            KIS_ASSERT(index >= 0);
            KIS_ASSERT(index < sampler.numSamples());

            size_t rest = index;

            dstBlueColorIndex = rest % sampler.blueColorValues.size();
            rest /= sampler.blueColorValues.size();

            dstGreenColorIndex = rest % sampler.greenColorValues.size();
            rest /= sampler.greenColorValues.size();

            dstRedColorIndex = rest % sampler.redColorValues.size();
            rest /= sampler.redColorValues.size();

            srcBlueColorIndex = rest % sampler.blueColorValues.size();
            rest /= sampler.blueColorValues.size();

            srcGreenColorIndex = rest % sampler.greenColorValues.size();
            rest /= sampler.greenColorValues.size();

            srcRedColorIndex = rest % sampler.redColorValues.size();
            rest /= sampler.redColorValues.size();

            dstAlphaIndex = rest % sampler.alphaValues.size();
            rest /= sampler.alphaValues.size();

            srcAlphaIndex = rest % sampler.alphaValues.size();
            rest /= sampler.alphaValues.size();

            KIS_ASSERT(rest < sampler.alphaValues.size());
            opacityIndex = rest;
        }

        SampleIndex(const SampleIndex &rhs) = default;
        SampleIndex(SampleIndex &&rhs) = default;
        SampleIndex& operator=(const SampleIndex &rhs) = default;
        SampleIndex& operator=(SampleIndex &&rhs) = default;

        size_t opacityIndex = 0;
        size_t srcAlphaIndex = 0;
        size_t dstAlphaIndex = 0;
        size_t srcRedColorIndex = 0;
        size_t srcGreenColorIndex = 0;
        size_t srcBlueColorIndex = 0;
        size_t dstRedColorIndex = 0;
        size_t dstGreenColorIndex = 0;
        size_t dstBlueColorIndex = 0;
    };

    class const_iterator
        : public boost::iterator_facade <const_iterator,
                                        boost::none_t,
                                        boost::random_access_traversal_tag,
                                        boost::none_t>
    {
        static size_t clampIndex(int index, const KisColorPairSampler &sampler) {
            const size_t numSamples = sampler.numSamples();
            KIS_ASSERT(numSamples > 0);
            return numSamples > 0 ?
                std::min<size_t>(numSamples - 1, index) : 0;
        }

    public:
        const_iterator(const KisColorPairSampler *sampler, int index)
            : m_sampler(sampler)
            , m_index(index)
            , m_sampleIndex(clampIndex(index, *sampler), *sampler)
        {}

        KoColor srcColor(const KoColorSpace *colorSpace) const {
            return createColor(m_sampler->redColorValues[m_sampleIndex.srcRedColorIndex],
                               m_sampler->greenColorValues[m_sampleIndex.srcGreenColorIndex],
                               m_sampler->blueColorValues[m_sampleIndex.srcBlueColorIndex],
                               m_sampler->alphaValues[m_sampleIndex.srcAlphaIndex],
                               colorSpace);

        }

        KoColor dstColor(const KoColorSpace *colorSpace) const {
            return createColor(m_sampler->redColorValues[m_sampleIndex.dstRedColorIndex],
                               m_sampler->greenColorValues[m_sampleIndex.dstGreenColorIndex],
                               m_sampler->blueColorValues[m_sampleIndex.dstBlueColorIndex],
                               m_sampler->alphaValues[m_sampleIndex.dstAlphaIndex],
                               colorSpace);
        }

        qreal opacity() const {
            return m_sampler->alphaValues[m_sampleIndex.opacityIndex];
        }

        qreal srcAlpha() const {
            return m_sampler->alphaValues[m_sampleIndex.srcAlphaIndex];
        }

        qreal dstAlpha() const {
            return m_sampler->alphaValues[m_sampleIndex.dstAlphaIndex];
        }

        qreal srcColor() const {
            return m_sampler->redColorValues[m_sampleIndex.srcRedColorIndex];
        }

        qreal dstColor() const {
            return m_sampler->redColorValues[m_sampleIndex.dstRedColorIndex];
        }

    private:
        static KoColor createColorU (qreal redF, qreal greenF, qreal blueF, qreal alphaF, const KoColorSpace *colorSpace) {
            using namespace Arithmetic;

            auto scaleChannel = [] (qreal value) -> quint16 {
                return qFuzzyCompare(value, 0.5) ?
                    KoColorSpaceMathsTraits<quint16>::halfValue :
                    qRound(qBound(0.0, value, 1.0) * unitValue<quint16>());
            };

            /**
             * Some composite ops reply on the halfValue to decide about the
             * processing they do, so we should make sure that float and uint16
             * modes use the same half-point in the test
             */
            const quint16 red = scaleChannel(redF);
            const quint16 green = scaleChannel(greenF);
            const quint16 blue = scaleChannel(blueF);
            const quint16 alpha = qRound(qBound(0.0, alphaF, 1.0) * unitValue<quint16>());

            KoColor c(colorSpace);
            quint16 *ptr = reinterpret_cast<quint16*>(c.data());
            ptr[0] = blue;
            ptr[1] = green;
            ptr[2] = red;
            ptr[3] = alpha;
            return c;
        };

        static inline KoColor createColor(qreal red, qreal green, qreal blue, qreal alpha, const KoColorSpace *colorSpace) {


            if (colorSpace->hasHighDynamicRange()) {
                KoColor result(colorSpace);

                QVector<float> colors;
                colors.resize(4);
                colors[0] = red;
                colors[1] = green;
                colors[2] = blue;
                colors[3] = alpha;

                colorSpace->fromNormalisedChannelsValue(result.data(), colors);

                return result;

            } else {
                return createColorU(red, green, blue, alpha, colorSpace);
            }
        };

    private:
        friend class boost::iterator_core_access;

        void increment() {
            m_index++;
            m_sampleIndex = SampleIndex(clampIndex(m_index, *m_sampler), *m_sampler);
        }

        bool equal(const_iterator const& other) const {
            return m_index == other.m_index &&
                m_sampler == other.m_sampler;
        }

        void decrement() {
            m_index--;
            m_sampleIndex = SampleIndex(clampIndex(m_index, *m_sampler), *m_sampler);
        }

        void advance(difference_type n) {
            m_index += n;
            m_sampleIndex = SampleIndex(clampIndex(m_index, *m_sampler), *m_sampler);
        }

        difference_type distance_to(const const_iterator &rhs) {
            return rhs.m_index - m_index;
        }

    private:
        const KisColorPairSampler *m_sampler;
        size_t m_index;
        SampleIndex m_sampleIndex;
    };

    const_iterator begin() const {
        return const_iterator(this, 0);
    }
    const_iterator end() const {
        return const_iterator(this, numSamples());
    }
};


#endif // KISCOLORPAIRSAMPLER_H

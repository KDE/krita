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
    std::vector<qreal> colorValues;
    std::vector<qreal> alphaValues;

    size_t numSamples() const {
        return
            alphaValues.size() *
            alphaValues.size() *
            alphaValues.size() *
            colorValues.size() *
            colorValues.size();
    }

    struct SampleIndex {
        SampleIndex(size_t index, const KisColorPairSampler &sampler)
        {
            KIS_ASSERT(index >= 0);
            KIS_ASSERT(index < sampler.numSamples());

            size_t rest = index;

            dstColorIndex = rest % sampler.colorValues.size();
            rest /= sampler.colorValues.size();

            srcColorIndex = rest % sampler.colorValues.size();
            rest /= sampler.colorValues.size();

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
        size_t srcColorIndex = 0;
        size_t dstColorIndex = 0;
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
            return createColor(m_sampler->colorValues[m_sampleIndex.srcColorIndex],
                               m_sampler->alphaValues[m_sampleIndex.srcAlphaIndex],
                               colorSpace);

        }

        KoColor dstColor(const KoColorSpace *colorSpace) const {
            return createColor(m_sampler->colorValues[m_sampleIndex.dstColorIndex],
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
            return m_sampler->colorValues[m_sampleIndex.srcColorIndex];
        }

        qreal dstColor() const {
            return m_sampler->colorValues[m_sampleIndex.dstColorIndex];
        }

    private:
        static KoColor createColorU (qreal colorF, qreal alphaF, const KoColorSpace *colorSpace) {
            using namespace Arithmetic;

            /**
             * Some composite ops reply on the halfValue to decide about the
             * processing they do, so we should make sure that float and uint16
             * modes use the same half-point in the test
             */
            const quint16 color = qFuzzyCompare(colorF, 0.5) ?
                KoColorSpaceMathsTraits<quint16>::halfValue :
                qRound(qBound(0.0, colorF, 1.0) * unitValue<quint16>());
            const quint16 alpha = qRound(qBound(0.0, alphaF, 1.0) * unitValue<quint16>());

            KoColor c(colorSpace);
            quint16 *ptr = reinterpret_cast<quint16*>(c.data());
            ptr[2] = color;
            ptr[3] = alpha;
            return c;
        };

        static KoColor createColorF32(qreal color, qreal alpha, const KoColorSpace *colorSpace) {
            KoColor c(colorSpace);
            float *ptr = reinterpret_cast<float*>(c.data());
            ptr[0] = color;
            ptr[3] = alpha;
            return c;
        };

        static KoColor createColorF16(qreal color, qreal alpha, const KoColorSpace *colorSpace) {
            KoColor c(colorSpace);
            half *ptr = reinterpret_cast<half*>(c.data());
            ptr[0] = color;
            ptr[3] = alpha;
            return c;
        };

        static KoColor createColor(qreal color, qreal alpha, const KoColorSpace *colorSpace) {
            KoColor result;

            if (colorSpace->colorDepthId() == Float32BitsColorDepthID) {
                result = createColorF32(color, alpha, colorSpace);
            } else if (colorSpace->colorDepthId() == Float16BitsColorDepthID) {
                result = createColorF16(color, alpha, colorSpace);
            } else if (colorSpace->colorDepthId() == Integer16BitsColorDepthID) {
                result = createColorU(color, alpha, colorSpace);
            } else {
                qFatal("bit depth is not implemented");
            }
            return result;
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

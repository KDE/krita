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
    public:
        const_iterator()
            : m_sampler(0),
            m_index(0) {}

        const_iterator(const KisColorPairSampler *sampler, int index)
            : m_sampler(sampler),
            m_index(index) {}

        KoColor srcColor(const KoColorSpace *colorSpace) const {
            SampleIndex idx(m_index, *m_sampler);
            return createColor(m_sampler->colorValues[idx.srcColorIndex],
                               m_sampler->alphaValues[idx.srcAlphaIndex],
                               colorSpace);

        }

        KoColor dstColor(const KoColorSpace *colorSpace) const {
            SampleIndex idx(m_index, *m_sampler);
            return createColor(m_sampler->colorValues[idx.dstColorIndex],
                               m_sampler->alphaValues[idx.dstAlphaIndex],
                               colorSpace);
        }

        qreal opacity() const {
            SampleIndex idx(m_index, *m_sampler);
            return m_sampler->alphaValues[idx.opacityIndex];
        }

    private:
        KoColor createColorU (qreal colorF, qreal alphaF, const KoColorSpace *colorSpace) const {
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

        KoColor createColorF(qreal color, qreal alpha, const KoColorSpace *colorSpace) const {
            KoColor c(colorSpace);
            float *ptr = reinterpret_cast<float*>(c.data());
            ptr[0] = color;
            ptr[3] = alpha;
            return c;
        };

        KoColor createColor(qreal color, qreal alpha, const KoColorSpace *colorSpace) const {
            KoColor result;

            if (colorSpace->colorDepthId() == Float32BitsColorDepthID) {
                result = createColorF(color, alpha, colorSpace);
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
        }

        bool equal(const_iterator const& other) const {
            return m_index == other.m_index &&
                m_sampler == other.m_sampler;
        }

        void decrement() {
            m_index--;
        }

        void advance(difference_type n) {
            m_index += n;
        }

        difference_type distance_to(const const_iterator &rhs) {
            return rhs.m_index - m_index;
        }

    private:
        const KisColorPairSampler *m_sampler;
        size_t m_index;
    };

    const_iterator begin() const {
        return const_iterator(this, 0);
    }
    const_iterator end() const {
        return const_iterator(this, numSamples());
    }
};


#endif // KISCOLORPAIRSAMPLER_H

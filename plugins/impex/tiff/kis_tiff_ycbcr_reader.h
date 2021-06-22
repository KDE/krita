/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_TIFF_YCBCR_READER_H_
#define _KIS_TIFF_YCBCR_READER_H_

#include <cmath>
#include <cstdint>
#include <memory>

#include <kis_buffer_stream.h>
#include <kis_global.h>
#include <kis_iterator_ng.h>
#include <kis_paint_device.h>

#include "kis_tiff_reader.h"

namespace KisTIFFYCbCr
{
enum Position { POSITION_CENTERED = 1, POSITION_COSITED = 2 };
}

template<typename T> class KisTIFFYCbCrReader : public KisTIFFReaderBase
{
public:
    using type = T;

    /**
     * @param hsub horizontal subsampling of Cb and Cr
     * @param hsub vertical subsampling of Cb and Cr
     */
    KisTIFFYCbCrReader(KisPaintDeviceSP device,
                       quint32 width,
                       quint32 height,
                       quint8 *poses,
                       int32_t alphapos,
                       uint16_t sourceDepth,
                       uint16_t sampleformat,
                       uint16_t nbcolorssamples,
                       uint16_t extrasamplescount,
                       bool premultipliedAlpha,
                       KoColorTransformation *transformProfile,
                       KisTIFFPostProcessor *postprocessor,
                       uint16_t hsub,
                       uint16_t vsub)
        : KisTIFFReaderBase(device, poses, alphapos, sourceDepth, sampleformat, nbcolorssamples, extrasamplescount, premultipliedAlpha, transformProfile, postprocessor)
        , m_hsub(hsub)
        , m_vsub(vsub)
    {
        // Initialize the buffer
        m_imageWidth = width;
        if (2 * (m_imageWidth / 2) != m_imageWidth)
            m_imageWidth++;
        m_bufferWidth = m_imageWidth / m_hsub;
        m_imageHeight = height;
        if (2 * (m_imageHeight / 2) != m_imageHeight)
            m_imageHeight++;
        m_bufferHeight = m_imageHeight / m_vsub;
        m_bufferCb = std::make_unique<T[]>(m_bufferWidth * m_bufferHeight);
        m_bufferCr = std::make_unique<T[]>(m_bufferWidth * m_bufferHeight);
    }

    ~KisTIFFYCbCrReader() override = default;

    uint32_t copyDataToChannels(quint32 x, quint32 y, quint32 dataWidth, KisBufferStreamBase *tiffstream) override
    {
        return copyDataToChannelsImpl(x, y, dataWidth, tiffstream);
    }

    void finalize() override
    {
        return finalizeImpl();
    }

private:
    template<typename U = T, typename std::enable_if<!std::numeric_limits<U>::is_integer, void>::type * = nullptr> uint32_t copyDataToChannelsImpl(quint32 x, quint32 y, quint32 dataWidth, KisBufferStreamBase *tiffstream)
    {
        size_t numcols = dataWidth / m_hsub;
        size_t buffPos = y / m_vsub * m_bufferWidth + x / m_hsub;
        for (size_t index = 0; index < numcols; index++) {
            KisHLineIteratorSP it = paintDevice()->createHLineIteratorNG(x + m_hsub * index, y, m_hsub);
            for (int vindex = 0; vindex < m_vsub; vindex++) {
                do {
                    T *d = reinterpret_cast<T *>(it->rawData());
                    d[0] = static_cast<T>(tiffstream->nextValue());
                    d[3] = std::numeric_limits<T>::max();
                    for (int k = 0; k < nbExtraSamples(); k++) {
                        if (k == alphaPos())
                            d[3] = static_cast<T>(tiffstream->nextValue());
                        else
                            tiffstream->nextValue();
                    }
                } while (it->nextPixel());
                it->nextRow();
            }
            m_bufferCb[buffPos] = static_cast<T>(tiffstream->nextValue());
            m_bufferCr[buffPos] = static_cast<T>(tiffstream->nextValue());
            buffPos++;
        }
        return m_vsub;
    }

    template<typename U = T, typename std::enable_if<std::numeric_limits<U>::is_integer, void>::type * = nullptr> uint32_t copyDataToChannelsImpl(quint32 x, quint32 y, quint32 dataWidth, KisBufferStreamBase *tiffstream)
    {
        size_t numcols = dataWidth / m_hsub;
        double coeff = std::numeric_limits<T>::max() / (double)(std::pow(2.0, this->sourceDepth()) - 1);
        //     dbgFile <<" depth expension coefficient :" << coeff;
        //     dbgFile <<" y =" << y;
        size_t buffPos = y / m_vsub * m_bufferWidth + x / m_hsub;
        for (size_t index = 0; index < numcols; index++) {
            KisHLineIteratorSP it = paintDevice()->createHLineIteratorNG(x + m_hsub * index, y, m_hsub);
            for (int vindex = 0; vindex < m_vsub; vindex++) {
                do {
                    T *d = reinterpret_cast<T *>(it->rawData());
                    d[0] = static_cast<T>(tiffstream->nextValue() * coeff);
                    d[3] = std::numeric_limits<T>::max();
                    for (int k = 0; k < nbExtraSamples(); k++) {
                        if (k == alphaPos())
                            d[3] = static_cast<T>(tiffstream->nextValue() * coeff);
                        else
                            tiffstream->nextValue();
                    }
                } while (it->nextPixel());
                it->nextRow();
            }
            m_bufferCb[buffPos] = static_cast<T>(tiffstream->nextValue() * coeff);
            m_bufferCr[buffPos] = static_cast<T>(tiffstream->nextValue() * coeff);
            buffPos++;
        }
        return m_vsub;
    }

    template<typename U = T, typename std::enable_if<!std::numeric_limits<U>::is_integer, void>::type * = nullptr> void finalizeImpl()
    {
        KisHLineIteratorSP it = paintDevice()->createHLineIteratorNG(0, 0, m_imageWidth);
        for (size_t y = 0; y < m_imageHeight; y++) {
            size_t x = 0;
            do {
                T *d = reinterpret_cast<T *>(it->rawData());
                size_t index = x / m_hsub + y / m_vsub * m_bufferWidth;
                d[1] = m_bufferCb[index];
                d[2] = m_bufferCr[index];
                ++x;

                if (this->hasPremultipliedAlpha()) {
                    auto unmultipliedColorsConsistent = [](T *d) { return !(std::abs(d[3]) < std::numeric_limits<T>::epsilon()); };

                    auto checkUnmultipliedColorsConsistent = [this](const T *d) {
                        const T alpha = std::abs(d[3]);

                        if (alpha >= static_cast<T>(0.01)) {
                            return true;
                        } else {
                            for (size_t i = 0; i < this->nbColorsSamples(); i++) {
                                if (!qFuzzyCompare(T(d[i] * alpha), d[i])) {
                                    return false;
                                }
                            }
                            return true;
                        }
                    };

                    if (!unmultipliedColorsConsistent(d)) {
                        while (1) {
                            T newAlpha = d[3];

                            for (quint8 i = 0; i < this->nbColorsSamples(); i++) {
                                d[i] = std::lroundf(d[i] * newAlpha);
                            }

                            d[3] = newAlpha;

                            if (checkUnmultipliedColorsConsistent(d)) {
                                break;
                            }

                            newAlpha += std::numeric_limits<T>::epsilon();
                        }
                    } else {
                        const T alpha = d[3];
                        for (quint8 i = 0; i < this->nbColorsSamples(); i++) {
                            d[i] = std::lroundf(d[i] * alpha);
                        }
                    }
                }
            } while (it->nextPixel());
            it->nextRow();
        }
    }

    template<typename U = T, typename std::enable_if<std::numeric_limits<U>::is_integer, void>::type * = nullptr> void finalizeImpl()
    {
        KisHLineIteratorSP it = paintDevice()->createHLineIteratorNG(0, 0, m_imageWidth);
        for (size_t y = 0; y < m_imageHeight; y++) {
            size_t x = 0;
            do {
                T *d = reinterpret_cast<T *>(it->rawData());
                size_t index = x / m_hsub + y / m_vsub * m_bufferWidth;
                d[1] = m_bufferCb[index];
                d[2] = m_bufferCr[index];
                ++x;

                if (this->hasPremultipliedAlpha()) {
                    const T alpha = d[3];
                    const float factor = alpha == 0 ? 0 : static_cast<float>(std::numeric_limits<T>::max()) / alpha;

                    for (quint8 i = 0; i < this->nbColorsSamples(); i++) {
                        d[i] = std::lroundf(d[i] * factor);
                    }
                }
            } while (it->nextPixel());
            it->nextRow();
        }
    }

private:
    std::unique_ptr<T[]> m_bufferCb;
    std::unique_ptr<T[]> m_bufferCr;
    quint32 m_bufferWidth, m_bufferHeight;
    uint16_t m_hsub;
    uint16_t m_vsub;
    quint32 m_imageWidth, m_imageHeight;
};

#endif

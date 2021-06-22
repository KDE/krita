/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_TIFF_READER_H_
#define _KIS_TIFF_READER_H_

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <tiffio.h>

#include <kis_buffer_stream.h>
#include <kis_debug.h>
#include <kis_global.h>
#include <kis_iterator_ng.h>
#include <kis_paint_device.h>
#include <kis_types.h>

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif // HAVE_OPENEXR

class KisBufferStreamBase;

class KisTIFFPostProcessor
{
public:
    KisTIFFPostProcessor(uint32_t nbcolorssamples)
        : m_nbcolorssamples(nbcolorssamples)
    {
    }
    virtual ~KisTIFFPostProcessor() = default;

public:
    virtual void postProcess(void *) = 0;

protected:
    inline uint32_t nbColorsSamples()
    {
        return m_nbcolorssamples;
    }

private:
    uint32_t m_nbcolorssamples;
};

template<typename T> class KisTIFFPostProcessorDummy : public KisTIFFPostProcessor
{
public:
    KisTIFFPostProcessorDummy(uint32_t nbcolorssamples)
        : KisTIFFPostProcessor(nbcolorssamples)
    {
    }
    ~KisTIFFPostProcessorDummy() override = default;

    void postProcess(void *) override
    {
    }
};

template<typename T> class KisTIFFPostProcessorInvert : public KisTIFFPostProcessor
{
public:
    KisTIFFPostProcessorInvert(uint32_t nbcolorssamples)
        : KisTIFFPostProcessor(nbcolorssamples)
    {
    }
    ~KisTIFFPostProcessorInvert() override
    {
    }

public:
    void postProcess(void *data) override
    {
        postProcessImpl(reinterpret_cast<T *>(data));
    }

private:
    template<typename U = T, typename std::enable_if<std::numeric_limits<U>::is_signed, void>::type * = nullptr> inline void postProcessImpl(T *data)
    {
        for (uint32_t i = 0; i < this->nbColorsSamples(); i++) {
            data[i] = -data[i];
        }
    }

    template<typename U = T, typename std::enable_if<!std::numeric_limits<U>::is_signed, void>::type * = nullptr> inline void postProcessImpl(T *data)
    {
        for (uint32_t i = 0; i < this->nbColorsSamples(); i++) {
            data[i] = std::numeric_limits<T>::max() - data[i];
        }
    }
};

template<typename T> class KisTIFFPostProcessorCIELABtoICCLAB : public KisTIFFPostProcessor
{
public:
    KisTIFFPostProcessorCIELABtoICCLAB(uint32_t nbcolorssamples)
        : KisTIFFPostProcessor(nbcolorssamples)
    {
    }
    ~KisTIFFPostProcessorCIELABtoICCLAB() override
    {
    }

public:
    void postProcess(void *data) override
    {
        postProcessImpl(reinterpret_cast<T *>(data));
    }

private:
    template<typename U = T, typename std::enable_if<!std::numeric_limits<U>::is_integer, void>::type * = nullptr> inline void postProcessImpl(T *data)
    {
        for (uint32_t i = 1; i < this->nbColorsSamples(); i++) {
            data[i] += 128.0f;
        }
    }

    template<typename U = T, typename std::enable_if<std::numeric_limits<U>::is_integer, void>::type * = nullptr> inline void postProcessImpl(T *data)
    {
        for (uint32_t i = 1; i < this->nbColorsSamples(); i++) {
            data[i] += std::numeric_limits<T>::max() / 2;
        }
    }
};

class KisTIFFReaderBase
{
public:
    KisTIFFReaderBase(KisPaintDeviceSP device,
                      quint8 *poses,
                      int32_t alphapos,
                      uint16_t sourceDepth,
                      uint16_t sample_format,
                      uint16_t nbcolorssamples,
                      uint16_t extrasamplescount,
                      bool premultipliedAlpha,
                      KoColorTransformation *transformProfile,
                      KisTIFFPostProcessor *postprocessor)
        : m_device(device)
        , m_alphapos(alphapos)
        , m_sourceDepth(sourceDepth)
        , m_sample_format(sample_format)
        , m_nbcolorssamples(nbcolorssamples)
        , m_nbextrasamples(extrasamplescount)
        , m_premultipliedAlpha(premultipliedAlpha)
        , m_poses(poses)
        , m_transformProfile(transformProfile)
        , mpostProcessImpl(postprocessor)
    {

    }
    virtual ~KisTIFFReaderBase()
    {
    }

public:
    /**
     * This function copy data from the tiff stream to the paint device starting at the given position.
     * @param x horizontal start position
     * @param y vertical start position
     * @param dataWidth width of the data to copy
     * @param tiffstream source of data
     *
     * @return the number of line which were copied
     */
    virtual uint32_t copyDataToChannels(quint32 x, quint32 y, quint32 dataWidth, KisBufferStreamBase *tiffstream) = 0;
    /**
     * This function is called when all data has been read and should be used for any postprocessing.
     */
    virtual void finalize()
    {
    }

protected:
    inline KisPaintDeviceSP paintDevice()
    {
        return m_device;
    }

    inline qint32 alphaPos()
    {
        return m_alphapos;
    }

    inline quint16 sourceDepth()
    {
        return m_sourceDepth;
    }
    inline uint16_t sampleFormat()
    {
        return m_sample_format;
    }

    inline quint16 nbColorsSamples()
    {
        return m_nbcolorssamples;
    }

    inline quint16 nbExtraSamples()
    {
        return m_nbextrasamples;
    }

    inline bool hasPremultipliedAlpha()
    {
        return m_premultipliedAlpha;
    }

    inline quint8 *poses()
    {
        return m_poses;
    }

    inline KoColorTransformation *transform()
    {
        return m_transformProfile;
    }

    inline KisTIFFPostProcessor *postProcessor()
    {
        return mpostProcessImpl;
    }

private:
    KisPaintDeviceSP m_device;
    qint32 m_alphapos;
    quint16 m_sourceDepth;
    uint16_t m_sample_format;
    quint16 m_nbcolorssamples;
    quint16 m_nbextrasamples;
    bool m_premultipliedAlpha;
    quint8 *m_poses;
    KoColorTransformation *m_transformProfile;
    KisTIFFPostProcessor *mpostProcessImpl;
};

template<typename T> class KisTIFFReaderTarget : public KisTIFFReaderBase
{
public:
    using type = T;

    KisTIFFReaderTarget(KisPaintDeviceSP device,
                        quint8 *poses,
                        int32_t alphapos,
                        uint16_t sourceDepth,
                        uint16_t sample_format,
                        uint16_t nbcolorssamples,
                        uint16_t extrasamplescount,
                        bool premultipliedAlpha,
                        KoColorTransformation *transformProfile,
                        KisTIFFPostProcessor *postprocessor,
                        T alphaValue)
        : KisTIFFReaderBase(device, poses, alphapos, sourceDepth, sample_format, nbcolorssamples, extrasamplescount, premultipliedAlpha, transformProfile, postprocessor)
        , m_alphaValue(alphaValue)
    {
    }
public:
    uint32_t copyDataToChannels(quint32 x, quint32 y, quint32 dataWidth, KisBufferStreamBase *tiffstream) override
    {
        return _copyDataToChannels(x, y, dataWidth, tiffstream);
    }

private:
    template<typename U = T, typename std::enable_if<!std::numeric_limits<U>::is_integer, void>::type * = nullptr> uint32_t _copyDataToChannels(quint32 x, quint32 y, quint32 dataWidth, KisBufferStreamBase *tiffstream)
    {
        KisHLineIteratorSP it = this->paintDevice()->createHLineIteratorNG(x, y, dataWidth);
        do {
            T *d = reinterpret_cast<T *>(it->rawData());
            quint8 i;
            for (i = 0; i < this->nbColorsSamples(); i++) {
                // XXX: for half this should use the bit constructor (plus downcast to uint16_t) (same as in the rest accesses)
                const uint32_t v = tiffstream->nextValue();
                std::memcpy(&d[this->poses()[i]], &v, sizeof(T));
            }
            this->postProcessor()->postProcess(d);
            if (this->transform()) {
                this->transform()->transform(reinterpret_cast<quint8 *>(d), reinterpret_cast<quint8 *>(d), 1);
            }
            d[this->poses()[i]] = m_alphaValue;
            for (quint8 k = 0; k < this->nbExtraSamples(); k++) {
                if (k == this->alphaPos()) {
                    const uint32_t v = tiffstream->nextValue();
                    std::memcpy(&d[this->poses()[i]], &v, sizeof(T));
                } else {
                    (void)tiffstream->nextValue();
                }
            }

            if (this->hasPremultipliedAlpha()) {
                auto unmultipliedColorsConsistent = [this, i](T *d) { return !(std::abs(d[this->poses()[i]]) < std::numeric_limits<T>::epsilon()); };

                auto checkUnmultipliedColorsConsistent = [this, i](const T *d) {
                    const T alpha = std::abs(d[this->poses()[i]]);

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
                        T newAlpha = d[this->poses()[i]];

                        for (quint8 i = 0; i < this->nbColorsSamples(); i++) {
                            d[i] = std::lroundf(d[i] * newAlpha);
                        }

                        d[this->poses()[i]] = newAlpha;

                        if (checkUnmultipliedColorsConsistent(d)) {
                            break;
                        }

                        newAlpha += std::numeric_limits<T>::epsilon();
                    }
                } else {
                    const T alpha = d[this->poses()[i]];
                    for (quint8 i = 0; i < this->nbColorsSamples(); i++) {
                        d[i] = std::lroundf(d[i] * alpha);
                    }
                }
            }
        } while (it->nextPixel());
        return 1;
    }

    template<typename U = T, typename std::enable_if<std::numeric_limits<U>::is_integer, void>::type * = nullptr> uint32_t _copyDataToChannels(quint32 x, quint32 y, quint32 dataWidth, KisBufferStreamBase *tiffstream)
    {
        KisHLineIteratorSP it = this->paintDevice()->createHLineIteratorNG(x, y, dataWidth);
        const double coeff = std::numeric_limits<T>::max() / static_cast<double>(std::pow(2.0, this->sourceDepth()) - 1);
        const bool no_coeff = !std::is_same<T, uint8_t>::value && this->sourceDepth() == sizeof(T) * CHAR_BIT;
        //    dbgFile <<" depth expension coefficient :" << coeff;
        do {
            T *d = reinterpret_cast<T *>(it->rawData());
            quint8 i;
            for (i = 0; i < this->nbColorsSamples(); i++) {
                if (no_coeff) {
                    d[this->poses()[i]] = static_cast<T>(tiffstream->nextValue());
                } else {
                    d[this->poses()[i]] = static_cast<T>(tiffstream->nextValue() * coeff);
                }
            }
            this->postProcessor()->postProcess(d);
            if (this->transform()) {
                this->transform()->transform(reinterpret_cast<quint8 *>(d), reinterpret_cast<quint8 *>(d), 1);
            }
            d[this->poses()[i]] = m_alphaValue;
            for (quint8 k = 0; k < this->nbExtraSamples(); k++) {
                if (k == this->alphaPos()) {
                    d[this->poses()[i]] = no_coeff ? static_cast<T>(tiffstream->nextValue()) : static_cast<T>(tiffstream->nextValue() * coeff);
                } else {
                    tiffstream->nextValue();
                }
            }

            if (hasPremultipliedAlpha()) {
                const T alpha = d[poses()[i]];
                const float factor = alpha == 0 ? 0 : static_cast<float>(std::numeric_limits<T>::max()) / alpha;

                for (quint8 i = 0; i < nbColorsSamples(); i++) {
                    d[i] = std::lroundf(d[i] * factor);
                }
            }
        } while (it->nextPixel());
        return 1;
    }

private:
    T m_alphaValue;
};

class KisTIFFReaderFromPalette : public KisTIFFReaderBase
{
public:
    using type = uint16_t;

    KisTIFFReaderFromPalette(KisPaintDeviceSP device,
                             uint16_t *red,
                             uint16_t *green,
                             uint16_t *blue,
                             quint8 *poses,
                             int32_t alphapos,
                             uint16_t sourceDepth,
                             uint16_t sample_format,
                             uint16_t nbcolorssamples,
                             bool premultipliedAlpha,
                             uint8_t extrasamplescount,
                             KoColorTransformation *transformProfile,
                             KisTIFFPostProcessor *postprocessor)
        : KisTIFFReaderBase(device, poses, alphapos, sourceDepth, sample_format, nbcolorssamples, extrasamplescount, premultipliedAlpha, transformProfile, postprocessor)
        , m_red(red)
        , m_green(green)
        , m_blue(blue)
    {
    }
public:
    uint32_t copyDataToChannels(quint32 x, quint32 y, quint32 dataWidth, KisBufferStreamBase *tiffstream) override
    {
        KisHLineIteratorSP it = paintDevice()->createHLineIteratorNG(static_cast<int>(x), static_cast<int>(y), static_cast<int>(dataWidth));
        do {
            KisTIFFReaderFromPalette::type *d = reinterpret_cast<KisTIFFReaderFromPalette::type *>(it->rawData());
            uint32_t index = tiffstream->nextValue();
            d[2] = m_red[index];
            d[1] = m_green[index];
            d[0] = m_blue[index];
            d[3] = std::numeric_limits<KisTIFFReaderFromPalette::type>::max();

        } while (it->nextPixel());
        return 1;
    }

private:
    uint16_t *m_red, *m_green, *m_blue;
};

#endif

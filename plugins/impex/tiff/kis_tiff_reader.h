/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_TIFF_READER_H_
#define _KIS_TIFF_READER_H_

// On some platforms, tiffio.h #defines 0 in a bad
// way for C++, as (void *)0 instead of using the correct
// C++ value 0. Include stdio.h first to get the right one.
#include <stdio.h>
#include <tiffio.h>

#include <kis_paint_device.h>
#include <kis_types.h>
#include <kis_global.h>

#define quint32_MAX 4294967295u

class KisBufferStreamBase;

class KisTIFFPostProcessor
{
public:
    KisTIFFPostProcessor(uint8 nbcolorssamples) : m_nbcolorssamples(nbcolorssamples) { }
    virtual ~KisTIFFPostProcessor() {}
public:
    virtual void postProcess8bit(quint8*) { }
    virtual void postProcess16bit(quint16*) { }
    virtual void postProcess32bit(quint32*) { }
protected:
    inline uint8 nbColorsSamples() {
        return m_nbcolorssamples;
    }
private:
    uint8 m_nbcolorssamples;
};

class KisTIFFPostProcessorInvert : public KisTIFFPostProcessor
{
public:
    KisTIFFPostProcessorInvert(uint8 nbcolorssamples) : KisTIFFPostProcessor(nbcolorssamples) {}
    ~KisTIFFPostProcessorInvert() override {}
public:
    void postProcess8bit(quint8* data) override {
        for (int i = 0; i < nbColorsSamples(); i++) {
            data[i] = quint8_MAX - data[i];
        }
    }
    void postProcess16bit(quint16* data) override {
        quint16* d = (quint16*) data;
        for (int i = 0; i < nbColorsSamples(); i++) {
            d[i] = quint16_MAX - d[i];
        }
    }
    void postProcess32bit(quint32* data) override {
        quint32* d = (quint32*) data;
        for (int i = 0; i < nbColorsSamples(); i++) {
            d[i] = quint32_MAX - d[i];
        }
    }
};

class KisTIFFPostProcessorCIELABtoICCLAB : public KisTIFFPostProcessor
{
public:
    KisTIFFPostProcessorCIELABtoICCLAB(uint8 nbcolorssamples) : KisTIFFPostProcessor(nbcolorssamples) {}
    ~KisTIFFPostProcessorCIELABtoICCLAB() override {}
public:
    void postProcess8bit(quint8* data) override {
        qint8* ds = (qint8*) data;
        for (int i = 1; i < nbColorsSamples(); i++) {
            ds[i] = data[i] + quint8_MAX / 2;
        }
    }
    void postProcess16bit(quint16* data) override {
        quint16* d = (quint16*) data;
        qint16* ds = (qint16*) data;
        for (int i = 1; i < nbColorsSamples(); i++) {
            ds[i] = d[i] + quint16_MAX / 2;
        }
    }
    void postProcess32bit(quint32* data) override {
        quint32* d = (quint32*) data;
        qint32* ds = (qint32*) data;
        for (int i = 1; i < nbColorsSamples(); i++) {
            ds[i] = d[i] + quint32_MAX / 2;
        }
    }
};


class KisTIFFReaderBase
{
public:
    KisTIFFReaderBase(KisPaintDeviceSP device, quint8* poses, int8 alphapos, uint8 sourceDepth,
                      uint16 sample_format, uint8 nbcolorssamples, uint8 extrasamplescount,
                      KoColorTransformation* transformProfile, KisTIFFPostProcessor* postprocessor)
        : m_device(device)
        , m_alphapos(alphapos)
        , m_sourceDepth(sourceDepth)
        , m_sample_format(sample_format)
        , m_nbcolorssamples(nbcolorssamples)
        , m_nbextrasamples(extrasamplescount)
        , m_poses(poses)
        , m_transformProfile(transformProfile)
        , m_postprocess(postprocessor)
    {

    }
    virtual ~KisTIFFReaderBase() {}
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
    virtual uint copyDataToChannels(quint32 x, quint32 y, quint32 dataWidth, KisBufferStreamBase* tiffstream) = 0;
    /**
     * This function is called when all data has been read and should be used for any postprocessing.
     */
    virtual void finalize() { }
protected:

    inline KisPaintDeviceSP paintDevice() {
        return m_device;
    }

    inline quint8 alphaPos() {
        return m_alphapos;
    }

    inline quint8 sourceDepth() {
        return m_sourceDepth;
    }
    inline uint16 sampleFormat() {
        return m_sample_format;
    }

    inline quint8 nbColorsSamples() {
        return m_nbcolorssamples;
    }

    inline quint8 nbExtraSamples() {
        return m_nbextrasamples;
    }

    inline quint8* poses() {
        return m_poses;
    }

    inline KoColorTransformation* transform() {
        return m_transformProfile;
    }

    inline KisTIFFPostProcessor* postProcessor() {
        return m_postprocess;
    }

private:
    KisPaintDeviceSP m_device;
    qint8 m_alphapos;
    quint8 m_sourceDepth;
    uint16 m_sample_format;
    quint8 m_nbcolorssamples;
    quint8 m_nbextrasamples;
    quint8* m_poses;
    KoColorTransformation* m_transformProfile;
    KisTIFFPostProcessor* m_postprocess;
};

class KisTIFFReaderTarget8bit : public KisTIFFReaderBase
{
public:
    KisTIFFReaderTarget8bit(KisPaintDeviceSP device, quint8* poses, int8 alphapos, uint8 sourceDepth, uint16 sample_format, uint8 nbcolorssamples, uint8 extrasamplescount, KoColorTransformation* transformProfile, KisTIFFPostProcessor* postprocessor)
        : KisTIFFReaderBase(device, poses, alphapos, sourceDepth, sample_format, nbcolorssamples, extrasamplescount, transformProfile, postprocessor)
    {
    }
public:
    uint copyDataToChannels(quint32 x, quint32 y, quint32 dataWidth, KisBufferStreamBase* tiffstream) override;
};


class KisTIFFReaderTarget16bit : public KisTIFFReaderBase
{
public:
    KisTIFFReaderTarget16bit(KisPaintDeviceSP device, quint8* poses, int8 alphapos, uint8 sourceDepth, uint16 sample_format, uint8 nbcolorssamples, uint8 extrasamplescount, KoColorTransformation* transformProfile, KisTIFFPostProcessor* postprocessor, uint16 _alphaValue)
        : KisTIFFReaderBase(device, poses, alphapos, sourceDepth, sample_format, nbcolorssamples, extrasamplescount, transformProfile, postprocessor),
          m_alphaValue(_alphaValue)
    {
    }
public:
    uint copyDataToChannels(quint32 x, quint32 y, quint32 dataWidth, KisBufferStreamBase* tiffstream) override ;
private:
    uint16 m_alphaValue;
};

class KisTIFFReaderTarget32bit : public KisTIFFReaderBase
{
public:
    KisTIFFReaderTarget32bit(KisPaintDeviceSP device, quint8* poses, int8 alphapos, uint8 sourceDepth, uint16 sample_format, uint8 nbcolorssamples, uint8 extrasamplescount, KoColorTransformation* transformProfile, KisTIFFPostProcessor* postprocessor, uint32 _alphaValue)
        : KisTIFFReaderBase(device, poses, alphapos, sourceDepth, sample_format, nbcolorssamples, extrasamplescount, transformProfile, postprocessor),
          m_alphaValue(_alphaValue)
    {
    }
public:
    uint copyDataToChannels(quint32 x, quint32 y, quint32 dataWidth, KisBufferStreamBase* tiffstream) override ;
private:
    uint32 m_alphaValue;
};

class KisTIFFReaderFromPalette : public  KisTIFFReaderBase
{
public:
    KisTIFFReaderFromPalette(KisPaintDeviceSP device, uint16 *red, uint16 *green, uint16 *blue, quint8* poses, int8 alphapos, uint8 sourceDepth, uint16 sample_format, uint8 nbcolorssamples, uint8 extrasamplescount, KoColorTransformation* transformProfile, KisTIFFPostProcessor* postprocessor)
        : KisTIFFReaderBase(device, poses, alphapos, sourceDepth, sample_format, nbcolorssamples, extrasamplescount, transformProfile, postprocessor), m_red(red), m_green(green), m_blue(blue)
    {
    }
public:
    uint copyDataToChannels(quint32 x, quint32 y, quint32 dataWidth, KisBufferStreamBase* tiffstream) override ;
private:
    uint16 *m_red, *m_green, *m_blue;
};

#endif

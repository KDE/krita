/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_TIFF_YCBCR_READER_H_
#define _KIS_TIFF_YCBCR_READER_H_

#include "kis_tiff_reader.h"

namespace KisTIFFYCbCr
{
enum Position {
    POSITION_CENTERED = 1,
    POSITION_COSITED = 2
};
}

class KisTIFFYCbCrReaderTarget8Bit : public KisTIFFReaderBase
{
public:
    /**
     * @param hsub horizontal subsampling of Cb and Cr
     * @param hsub vertical subsampling of Cb and Cr
     */
    KisTIFFYCbCrReaderTarget8Bit(KisPaintDeviceSP device, quint32 width, quint32 height, quint8* poses,
                                 int8 alphapos, uint8 sourceDepth, uint16 sampleformat, uint8 nbcolorssamples, uint8 extrasamplescount,
                                 KoColorTransformation* transformProfile, KisTIFFPostProcessor* postprocessor, uint16 hsub, uint16 vsub);
    ~KisTIFFYCbCrReaderTarget8Bit() override;
    uint copyDataToChannels(quint32 x, quint32 y, quint32 dataWidth, KisBufferStreamBase* tiffstream) override;
    void finalize() override;
private:
    quint8* m_bufferCb;
    quint8* m_bufferCr;
    quint32 m_bufferWidth, m_bufferHeight;
    uint16 m_hsub;
    uint16 m_vsub;
    quint32 m_imageWidth, m_imageHeight;

};

class KisTIFFYCbCrReaderTarget16Bit : public KisTIFFReaderBase
{
public:
    /**
     * @param hsub horizontal subsampling of Cb and Cr
     * @param hsub vertical subsampling of Cb and Cr
     */
    KisTIFFYCbCrReaderTarget16Bit(KisPaintDeviceSP device, quint32 width, quint32 height, quint8* poses,
                                  int8 alphapos, uint8 sourceDepth, uint16 sampleformat, uint8 nbcolorssamples, uint8 extrasamplescount,
                                  KoColorTransformation* transformProfile, KisTIFFPostProcessor* postprocessor, uint16 hsub, uint16 vsub);
    ~KisTIFFYCbCrReaderTarget16Bit() override;
    uint copyDataToChannels(quint32 x, quint32 y, quint32 dataWidth, KisBufferStreamBase* tiffstream) override;
    void finalize() override;
private:
    quint16* m_bufferCb;
    quint16* m_bufferCr;
    quint32 m_bufferWidth, m_bufferHeight;
    uint16 m_hsub;
    uint16 m_vsub;
    quint32 m_imageWidth, m_imageHeight;

};


#endif

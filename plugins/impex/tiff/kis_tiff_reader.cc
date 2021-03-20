/*
 *  SPDX-FileCopyrightText: 2005-2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tiff_reader.h"

#include <math.h>

#include <kis_debug.h>

#include <kis_paint_device.h>
#include "kis_iterator_ng.h"
#include "kis_buffer_stream.h"

#include <KoColorSpaceConstants.h>
#include <KoColorSpaceTraits.h>

uint KisTIFFReaderTarget8bit::copyDataToChannels(quint32 x, quint32 y, quint32 dataWidth, KisBufferStreamBase* tiffstream)
{
    KisHLineIteratorSP it = paintDevice()->createHLineIteratorNG(x, y, dataWidth);
    double coeff = quint8_MAX / (double)(pow(2.0, sourceDepth()) - 1);
//         dbgFile <<" depth expension coefficient :" << coeff;
    do {
        quint8 *d = it->rawData();
        quint8 i;
        for (i = 0; i < nbColorsSamples() ; i++) {
            d[poses()[i]] = (quint8)(tiffstream->nextValue() * coeff);
        }
        postProcessor()->postProcess8bit(d);
        if (transform()) transform()->transform(d, d, 1);
        d[poses()[i]] = quint8_MAX;
        for (int k = 0; k < nbExtraSamples(); k++) {
            if (k == alphaPos())
                d[poses()[i]] = (quint32)(tiffstream->nextValue() * coeff);
            else
                tiffstream->nextValue();
        }

    } while (it->nextPixel());
    return 1;
}
uint KisTIFFReaderTarget16bit::copyDataToChannels(quint32 x, quint32 y, quint32 dataWidth, KisBufferStreamBase* tiffstream)
{
    KisHLineIteratorSP it = paintDevice()->createHLineIteratorNG(x, y, dataWidth);
    double coeff = quint16_MAX / (double)(pow(2.0, sourceDepth()) - 1);
    bool no_coeff = (sourceDepth() == 16);
//         dbgFile <<" depth expension coefficient :" << coeff;
    do {
        quint16 *d = reinterpret_cast<quint16 *>(it->rawData());
        quint8 i;
        for (i = 0; i < nbColorsSamples(); i++) {
            d[poses()[i]] = no_coeff ? tiffstream->nextValue() : (quint16)(tiffstream->nextValue() * coeff);
        }
        postProcessor()->postProcess16bit(d);
        if (transform()) transform()->transform((quint8*)d, (quint8*)d, 1);
        d[poses()[i]] = m_alphaValue;
        for (int k = 0; k < nbExtraSamples(); k++) {
            if (k == alphaPos())
                d[poses()[i]] = no_coeff ? tiffstream->nextValue() : (quint16)(tiffstream->nextValue() * coeff);
            else
                tiffstream->nextValue();
        }

    } while (it->nextPixel());
    return 1;
}

uint KisTIFFReaderTarget32bit::copyDataToChannels(quint32 x, quint32 y, quint32 dataWidth, KisBufferStreamBase* tiffstream)
{
    KisHLineIteratorSP it = paintDevice()->createHLineIteratorNG(x, y, dataWidth);
    double coeff = quint32_MAX / (double)(pow(2.0, sourceDepth()) - 1);
    bool no_coeff = (sourceDepth() == 32);
//    dbgFile <<" depth expension coefficient :" << coeff;
    do {
        quint32 *d = reinterpret_cast<quint32 *>(it->rawData());
        quint8 i;
        for (i = 0; i < nbColorsSamples(); i++) {
            d[poses()[i]] = no_coeff ? tiffstream->nextValue() : (quint32)(tiffstream->nextValue() * coeff);
        }
        postProcessor()->postProcess32bit(d);
        if (transform()) transform()->transform((quint8*)d, (quint8*)d, 1);
        d[poses()[i]] = m_alphaValue;
        for (int k = 0; k < nbExtraSamples(); k++) {
            if (k == alphaPos())
                d[poses()[i]] = no_coeff ? tiffstream->nextValue() : (quint32)(tiffstream->nextValue() * coeff);
            else
                tiffstream->nextValue();
        }

    } while (it->nextPixel());
    return 1;
}
uint KisTIFFReaderFromPalette::copyDataToChannels(quint32 x, quint32 y, quint32 dataWidth,  KisBufferStreamBase* tiffstream)
{
    KisHLineIteratorSP it = paintDevice()->createHLineIteratorNG(x, y, dataWidth);
    do {
        quint16* d = reinterpret_cast<quint16 *>(it->rawData());
        uint32 index = tiffstream->nextValue();
        d[2] = m_red[index];
        d[1] = m_green[index];
        d[0] = m_blue[index];
        d[3] = quint16_MAX;

    } while (it->nextPixel());
    return 1;
}

/*
 *  Copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_tiff_reader.h"

#include <kis_debug.h>

#include <kis_iterators_pixel.h>
#include <kis_paint_device.h>

#include "kis_buffer_stream.h"

uint KisTIFFReaderTarget8bit::copyDataToChannels(quint32 x, quint32 y, quint32 dataWidth, KisBufferStreamBase* tiffstream)
{
    KisHLineIterator it = paintDevice() -> createHLineIterator(x, y, dataWidth);
    double coeff = quint8_MAX / (double)(pow(2, sourceDepth()) - 1);
//         dbgFile <<" depth expension coefficient :" << coeff;
    while (!it.isDone()) {
        quint8 *d = it.rawData();
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
        ++it;
    }
    return 1;
}
uint KisTIFFReaderTarget16bit::copyDataToChannels(quint32 x, quint32 y, quint32 dataWidth, KisBufferStreamBase* tiffstream)
{
    KisHLineIterator it = paintDevice() -> createHLineIterator(x, y, dataWidth);
    double coeff = quint16_MAX / (double)(pow(2, sourceDepth()) - 1);
//         dbgFile <<" depth expension coefficient :" << coeff;
    while (!it.isDone()) {
        quint16 *d = reinterpret_cast<quint16 *>(it.rawData());
        quint8 i;
        for (i = 0; i < nbColorsSamples(); i++) {
            d[poses()[i]] = (quint16)(tiffstream->nextValue() * coeff);
        }
        postProcessor()->postProcess16bit(d);
        if (transform()) transform()->transform((quint8*)d, (quint8*)d, 1);
        d[poses()[i]] = quint16_MAX;
        for (int k = 0; k < nbExtraSamples(); k++) {
            if (k == alphaPos())
                d[poses()[i]] = (quint16)(tiffstream->nextValue() * coeff);
            else
                tiffstream->nextValue();
        }
        ++it;
    }
    return 1;
}

uint KisTIFFReaderTarget32bit::copyDataToChannels(quint32 x, quint32 y, quint32 dataWidth, KisBufferStreamBase* tiffstream)
{
    KisHLineIterator it = paintDevice() -> createHLineIterator(x, y, dataWidth);
    double coeff = quint32_MAX / (double)(pow(2, sourceDepth()) - 1);
//         dbgFile <<" depth expension coefficient :" << coeff;
    while (!it.isDone()) {
        quint32 *d = reinterpret_cast<quint32 *>(it.rawData());
        quint8 i;
        for (i = 0; i < nbColorsSamples(); i++) {
            d[poses()[i]] = (quint32)(tiffstream->nextValue() * coeff);
        }
        postProcessor()->postProcess32bit(d);
        if (transform()) transform()->transform((quint8*)d, (quint8*)d, 1);
        d[poses()[i]] = quint32_MAX;
        for (int k = 0; k < nbExtraSamples(); k++) {
            if (k == alphaPos())
                d[poses()[i]] = (quint32)(tiffstream->nextValue() * coeff);
            else
                tiffstream->nextValue();
        }
        ++it;
    }
    return 1;
}
uint KisTIFFReaderFromPalette::copyDataToChannels(quint32 x, quint32 y, quint32 dataWidth,  KisBufferStreamBase* tiffstream)
{
    KisHLineIterator it = paintDevice() -> createHLineIterator(x, y, dataWidth);
    while (!it.isDone()) {
        quint16* d = reinterpret_cast<quint16 *>(it.rawData());
        uint32 index = tiffstream->nextValue();
        d[2] = m_red[index];
        d[1] = m_green[index];
        d[0] = m_blue[index];
        d[3] = quint16_MAX;
        ++it;
    }
    return 1;
}

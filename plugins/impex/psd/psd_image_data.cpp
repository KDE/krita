/*
 *   SPDX-FileCopyrightText: 2011 Siddharth Sharma <siddharth.kde@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <psd_image_data.h>

#include <QtEndian>

#include <QFile>
#include <kis_debug.h>
#include <QVector>
#include <QByteArray>
#include <QBuffer>

#include <KoChannelInfo.h>
#include <KoColorSpace.h>
#include <KoColorSpaceMaths.h>
#include <KoColorSpaceTraits.h>

#include "psd_utils.h"
#include "compression.h"

#include "kis_iterator_ng.h"
#include "kis_paint_device.h"

#include <asl/kis_asl_reader_utils.h>

#include "psd_pixel_utils.h"


PSDImageData::PSDImageData(PSDHeader *header)
{
    m_header = header;
}

PSDImageData::~PSDImageData() {

}

bool PSDImageData::read(QIODevice &io, KisPaintDeviceSP dev)
{
    psdread(io, m_compression);
    quint64 start = io.pos();
    m_channelSize = m_header->channelDepth/8;
    m_channelDataLength = quint64(m_header->height) * m_header->width * m_channelSize;

    dbgFile << "Reading Image Data Block: compression" << m_compression << "channelsize" << m_channelSize << "number of channels" << m_header->nChannels;

    switch (m_compression) {

    case 0: // Uncompressed

        for (int channel = 0; channel < m_header->nChannels; channel++) {
            m_channelOffsets << 0;
            ChannelInfo channelInfo;
            channelInfo.channelId = channel;
            channelInfo.compressionType = Compression::Uncompressed;
            channelInfo.channelDataStart = start;
            channelInfo.channelDataLength = quint64(m_header->width) * m_header->height * m_channelSize;
            start += channelInfo.channelDataLength;
            m_channelInfoRecords.append(channelInfo);

        }

        break;

    case 1: // RLE
    {
        quint32 rlelength = 0;

        // The start of the actual channel data is _after_ the RLE rowlengths block
        if (m_header->version == 1) {
            start += m_header->nChannels * m_header->height * 2;
        }
        else if (m_header->version == 2) {
            start += m_header->nChannels * m_header->height * 4;
        }

        for (int channel = 0; channel < m_header->nChannels; channel++) {
            m_channelOffsets << 0;
            quint64 sumrlelength = 0;
            ChannelInfo channelInfo;
            channelInfo.channelId = channel;
            channelInfo.channelDataStart = start;
            channelInfo.compressionType = Compression::RLE;
            for (quint32 row = 0; row < m_header->height; row++ ) {
                if (m_header->version == 1) {
                    quint16 rlelength16; // use temporary variable to not cast pointers and not rely on endianness
                    psdread(io, rlelength16);
                    rlelength = rlelength16;
                }
                else if (m_header->version == 2) {
                    psdread(io, rlelength);
                }
                channelInfo.rleRowLengths.append(rlelength);
                sumrlelength += rlelength;
            }
            channelInfo.channelDataLength = sumrlelength;
            start += channelInfo.channelDataLength;
            m_channelInfoRecords.append(channelInfo);
        }

        break;
    }
    case 2: // ZIP without prediction
    case 3: // ZIP with prediction
    default:
        break;
    }

    if (!m_channelInfoRecords.isEmpty()) {
        QVector<ChannelInfo*> infoRecords;

        QVector<ChannelInfo>::iterator it = m_channelInfoRecords.begin();
        QVector<ChannelInfo>::iterator end = m_channelInfoRecords.end();

        for (; it != end; ++it) {
            infoRecords << &(*it);
        }

        const QRect imageRect(0, 0, m_header->width, m_header->height);

        try {
            PsdPixelUtils::readChannels(io, dev,
                                        m_header->colormode,
                                        m_channelSize,
                                        imageRect,
                                        infoRecords);
        } catch (KisAslReaderUtils::ASLParseException &) {
            dev->clear();
            return true;
        }
    }

    return true;
}

bool PSDImageData::write(QIODevice &io, KisPaintDeviceSP dev, bool hasAlpha)
{
    // XXX: make the compression setting configurable. For now, always use RLE.
    psdwrite(io, (quint16)Compression::RLE);

    // now write all the channels in display order
    // fill in the channel chooser, in the display order, but store the pixel index as well.
    QRect rc(0, 0, m_header->width, m_header->height);

    const int channelSize = m_header->channelDepth / 8;
    const psd_color_mode colorMode = m_header->colormode;

    QVector<PsdPixelUtils::ChannelWritingInfo> writingInfoList;

    bool writeAlpha = hasAlpha &&
        dev->colorSpace()->channelCount() != dev->colorSpace()->colorChannelCount();

    const int numChannels =
        writeAlpha ?
        dev->colorSpace()->channelCount() :
        dev->colorSpace()->colorChannelCount();

    for (int i = 0; i < numChannels; i++) {
        const int rleOffset = io.pos();

        int channelId = writeAlpha && i == numChannels - 1 ? -1 : i;

        writingInfoList <<
            PsdPixelUtils::ChannelWritingInfo(channelId, -1, rleOffset);

        io.seek(io.pos() + rc.height() * sizeof(quint16));
    }

    PsdPixelUtils::writePixelDataCommon(io, dev, rc,
                                        colorMode, channelSize,
                                        false, false, writingInfoList);
    return true;
}

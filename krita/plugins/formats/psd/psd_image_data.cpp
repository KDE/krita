/*
 *   Copyright (C) 2011 by Siddharth Sharma <siddharth.kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include <psd_image_data.h>

#include <netinet/in.h> // htonl

#include <QFile>
#include <QDebug>
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

bool PSDImageData::read(QIODevice *io, KisPaintDeviceSP dev ) {



    psdread(io, &m_compression);
    quint64 start = io->pos();
    m_channelSize = m_header->channelDepth/8;
    m_channelDataLength = m_header->height * m_header->width * m_channelSize;

    dbgFile << "Reading Image Data Block: compression" << m_compression << "channelsize" << m_channelSize << "number of channels" << m_header->nChannels;

    switch (m_compression) {

    case 0: // Uncompressed

        for (int channel = 0; channel < m_header->nChannels; channel++) {
            m_channelOffsets << 0;
            ChannelInfo channelInfo;
            channelInfo.channelId = channel;
            channelInfo.compressionType = Compression::Uncompressed;
            channelInfo.channelDataStart = start;
            channelInfo.channelDataLength = m_header->width * m_header->height * m_channelSize;
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
            quint32 sumrlelength = 0;
            ChannelInfo channelInfo;
            channelInfo.channelId = channel;
            channelInfo.channelDataStart = start;
            channelInfo.compressionType = Compression::RLE;
            for (quint32 row = 0; row < m_header->height; row++ ) {
                if (m_header->version == 1) {
                    psdread(io,(quint16*)&rlelength);
                }
                else if (m_header->version == 2) {
                    psdread(io,&rlelength);
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
        } catch (KisAslReaderUtils::ASLParseException &e) {
            dev->clear();
            return true;
        }

    }

    return true;
}

bool PSDImageData::write(QIODevice *io, KisPaintDeviceSP dev)
{
    // XXX: make the compression settting configurable. For now, always use RLE.
    psdwrite(io, (quint16)Compression::RLE);

    // now write all the channels in display order
    // fill in the channel chooser, in the display order, but store the pixel index as well.
    QRect rc(0, 0, m_header->width, m_header->height);
    QVector<quint8* > tmp = dev->readPlanarBytes(0, 0, rc.width(), rc.height());
    // then reorder the planes to fit the psd model -- alpha first, then display order
    QVector<quint8* > planes;
    QList<KoChannelInfo*> origChannels = dev->colorSpace()->channels();

    quint8* alphaPlane = 0;
    foreach(KoChannelInfo *ch, KoChannelInfo::displayOrderSorted(origChannels)) {
        int channelIndex = KoChannelInfo::displayPositionToChannelIndex(ch->displayPosition(), origChannels);
        //qDebug() << ppVar(ch->name()) << ppVar(ch->pos()) << ppVar(ch->displayPosition()) << ppVar(channelIndex);
        if (ch->channelType() == KoChannelInfo::ALPHA) {
            alphaPlane = tmp[channelIndex];
        } else {
            planes.append(tmp[channelIndex]);
        }
    }
    planes.append(alphaPlane); // alpha is last, in contrast with layers, where it's first.
    // now planes are holding pointers to quint8 arrays
    tmp.clear();

    // Now fix up the cmyk channels, we need to invert them
    if (m_header->colormode == CMYK || m_header->colormode == CMYK64) {
        for (int i = 0; i < 4; ++i) {
            if (m_header->channelDepth == 8) {
                for (int j = 0; j < rc.width() * rc.height(); ++j) {
                    planes[i][j] = 255 - planes[i][j];
                }
            }
            else if (m_header->channelDepth == 16) {
                quint16 val;
                for (int j = 0; j < rc.width() * rc.height(); ++j) {
                    val = reinterpret_cast<quint16*>(planes[i])[j];
                    val = quint16_MAX - ntohs(val);
                    reinterpret_cast<quint16*>(planes[i])[j] = val;
                }
            }
        }
    }

    quint64 channelLengthPos = io->pos();
    // write zero's for the channel lengths section
    for (uint i = 0; i < dev->colorSpace()->channelCount() * rc.height(); ++i) {
        psdwrite(io, (quint16)0);
    }
    // here the actual channel data starts
    quint64 channelStartPos = io->pos();

    for (int channelInfoIndex = 0; channelInfoIndex  < planes.size(); ++channelInfoIndex) {
        quint8 *plane = planes[channelInfoIndex];

        quint32 stride = (m_header->channelDepth / 8) * rc.width();
        for (qint32 row = 0; row < rc.height(); ++row) {

            QByteArray uncompressed = QByteArray::fromRawData((const char*)plane + row * stride, stride);
            if (m_header->channelDepth == 8) {
            } else if (m_header->channelDepth == 16) {
                quint16 *dataPtr = reinterpret_cast<quint16 *>(uncompressed.data());
                for (int i = 0; i < rc.width(); i++) {
                    quint16 val = htons(*dataPtr);
                    *dataPtr = val;
                    ++dataPtr;
                }
            } else if (m_header->channelDepth == 32) {
                quint32 *dataPtr = reinterpret_cast<quint32 *>(uncompressed.data());
                for (int i = 0; i < rc.width(); i++) {
                    quint32 val = htonl(*dataPtr);
                    *dataPtr = val;
                    ++dataPtr;
                }
            }
            QByteArray compressed = Compression::compress(uncompressed, Compression::RLE);

            io->seek(channelLengthPos);
            psdwrite(io, (quint16)compressed.size());
            channelLengthPos +=2;
            io->seek(channelStartPos);

            if (io->write(compressed) != compressed.size()) {
                error = "Could not write image data";
                return false;
            }

            channelStartPos += compressed.size();
        }
    }

    qDeleteAll(planes);
    planes.clear();

    return true;
}

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

#include <netinet/in.h> // htonl

#include <QFile>
#include <QDebug>
#include <QVector>
#include <QByteArray>
#include <QBuffer>

#include <KoColorSpace.h>
#include <KoColorSpaceMaths.h>
#include <KoColorSpaceTraits.h>

#include <psd_image_data.h>
#include "psd_utils.h"
#include "compression.h"

PSDImageData::PSDImageData(PSDHeader *header)
{
    m_header = header;
}

PSDImageData::~PSDImageData() {

}

bool PSDImageData::read(KisPaintDeviceSP dev ,QIODevice *io) {

    qDebug() << "Position before read " << io->pos();
    psdread(io, &m_compression);
    qDebug() << "COMPRESSION TYPE " << m_compression;
    quint64 start = io->pos();
    m_channelSize = m_header->channelDepth/8;
    m_channelDataLength = m_header->height * m_header->width * m_channelSize;

    switch (m_compression) {

    case 0: // Uncompressed

        for (int channel = 0; channel < m_header->nChannels; channel++) {

            ChannelInfo channelInfo;
            channelInfo.channelId = channel;
            channelInfo.compressionType = Compression::Uncompressed;
            channelInfo.channelDataStart = start;
            channelInfo.channelDataLength = m_header->width * m_header->height * m_channelSize;
            start += channelInfo.channelDataLength;
            m_channelInfoRecords.append(channelInfo);

        }

        for (int channel = 0; channel < m_header->nChannels; channel++) {

            qDebug() << "Channel ID: " << m_channelInfoRecords[channel].channelId;
            qDebug() << "Channel Compression Type: " << m_channelInfoRecords[channel].compressionType;
            qDebug() << "Channel Data Start: " << m_channelInfoRecords[channel].channelDataStart;
            qDebug() << "Channel Data Length: " << m_channelInfoRecords[channel].channelDataLength;
            qDebug() << "---------------------------------------------------";
        }

        switch (m_header->colormode) {
        case Bitmap:
            break;
        case Grayscale:
            break;
        case Indexed:
            break;
        case RGB:
            qDebug()<<"RGB";
            doRGB(dev, io);
            break;
        case CMYK:
            doCMYK(dev,io);
            break;
        case MultiChannel:
            break;
        case DuoTone:
            break;
        case Lab:
            doLAB(dev, io);
            break;
        case UNKNOWN:
            break;
        default:
            break;
        }

        break;

    case 1: // RLE
        qDebug()<<"RLE ENCODED";
        quint32 rlelength;
        quint32 sumrlelength;

        for (int channel=0; channel < m_header->nChannels; channel++) {
            ChannelInfo channelInfo;
            channelInfo.channelId = channel;
            channelInfo.channelDataStart = start;
            channelInfo.compressionType = Compression::RLE;
            for (int row = 0; row < m_header->height; row++ ) {
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
            start +=sumrlelength;
            m_channelInfoRecords.append(channelInfo);
        }

        for (int channel = 0; channel < m_header->nChannels; channel++) {
            qDebug() << "Channel ID: " << m_channelInfoRecords[channel].channelId;
            qDebug() << "Channel Compression Type: " << m_channelInfoRecords[channel].compressionType;
            qDebug() << "Channel Data Start: " << m_channelInfoRecords[channel].channelDataStart;
            qDebug() << "Channel Data Length: " << m_channelInfoRecords[channel].channelDataLength;
            qDebug() << "---------------------------------------------------";
        }

        switch (m_header->colormode) {
        case Bitmap:
            break;
        case Grayscale:
            break;
        case Indexed:
            break;
        case RGB:
            doRGB(dev,io);
            break;
        case CMYK:
            break;
        case MultiChannel:
            break;
        case DuoTone:
            break;
        case Lab:
            break;
        case UNKNOWN:
            break;
        default:
            break;
        }

        break;

    case 2: // ZIP without prediction
        qDebug()<<"ZIP without prediction";

        switch (m_header->colormode) {
        case Bitmap:
            break;
        case Grayscale:
            break;
        case Indexed:
            break;
        case RGB:
            break;
        case CMYK:
            break;
        case MultiChannel:
            break;
        case DuoTone:
            break;
        case Lab:
            break;
        case UNKNOWN:
            break;
        default:
            break;
        }

        break;

    case 3: // ZIP with prediction
        qDebug()<<"ZIP with prediction";

        switch (m_header->colormode) {
        case Bitmap:
            break;
        case Grayscale:
            break;
        case Indexed:
            break;
        case RGB:
            break;
        case CMYK:
            break;
        case MultiChannel:
            break;
        case DuoTone:
            break;
        case Lab:
            break;
        case UNKNOWN:
            break;
        default:
            break;
        }

        break;

    default:
        break;

    }

    return true;
}

bool PSDImageData::doRGB(KisPaintDeviceSP dev, QIODevice *io) {

    int channelOffset = 0;
    
    for (int row = 0; row <m_header->height; row++) {
        
        KisHLineIterator it = dev->createHLineIterator(0, row, m_header->width);
        QVector<QByteArray> vectorBytes;

        for (int channel = 0; channel < m_header->nChannels; channel++) {

            io->seek(m_channelInfoRecords[channel].channelDataStart + channelOffset);

            switch (m_compression){

            case Compression::Uncompressed:

                vectorBytes.append(io->read(m_header->width*m_channelSize));

                // Debug
                qDebug() << "channel: " << m_channelInfoRecords[channel].channelId << "is at position " << io->pos();

                break;

            case Compression::RLE:

                QByteArray CompressedBytes;
                QBuffer buffer;
                CompressedBytes = io->read(m_channelInfoRecords[channel].rleRowLengths[row]);
                // buffer.write(Compression::uncompress());
                //vectorBytes.append(buffer);

                // Debug
                qDebug()<<"CompressedBytes data"<<CompressedBytes;
                qDebug()<<"Row length read: "<<m_channelInfoRecords[channel].rleRowLengths;
                qDebug()<<"IO Position before Read: "<<io->pos();
                qDebug() << "channel: " << m_channelInfoRecords[channel].channelId << "is at position " << io->pos();

                break;

            case Compression::ZIP:
                break;

            case Compression::ZIPWithPrediction:
                break;

            default:
                break;
            }


        }
        channelOffset += (m_header->width * m_channelSize);
        
        qDebug() << "------------------------------------------";
        qDebug() << "channel offset:"<< channelOffset ;
        qDebug() << "------------------------------------------";

        for (int col = 0; col < m_header->width; col++) {

            if (m_channelSize == 1) {

                quint8 red = ntohs(reinterpret_cast<const quint8 *>(vectorBytes[0].constData())[col]);
                KoRgbU8Traits::setRed(it.rawData(), red);

                quint8 green = ntohs(reinterpret_cast<const quint8 *>(vectorBytes[1].constData())[col]);
                KoRgbU8Traits::setGreen(it.rawData(), green);

                quint8 blue = ntohs(reinterpret_cast<const quint8 *>(vectorBytes[2].constData())[col]);
                KoRgbU8Traits::setBlue(it.rawData(), blue);

            }

            else if (m_channelSize == 2) {

                quint16 red = ntohs(reinterpret_cast<const quint16 *>(vectorBytes[0].constData())[col]);
                KoRgbU16Traits::setRed(it.rawData(), red);

                quint16 green = ntohs(reinterpret_cast<const quint16 *>(vectorBytes[1].constData())[col]);
                KoRgbU16Traits::setGreen(it.rawData(), green);

                quint16 blue = ntohs(reinterpret_cast<const quint16 *>(vectorBytes[2].constData())[col]);
                KoRgbU16Traits::setBlue(it.rawData(), blue);

            }

            // XXX see implementation Openexr
            else if (m_channelSize == 4) {

                quint16 red = ntohs(reinterpret_cast<const quint16 *>(vectorBytes.constData())[col]);
                KoRgbU16Traits::setRed(it.rawData(), red);

                quint16 green = ntohs(reinterpret_cast<const quint16 *>(vectorBytes.constData())[col]);
                KoRgbU16Traits::setGreen(it.rawData(), green);

                quint16 blue = ntohs(reinterpret_cast<const quint16 *>(vectorBytes.constData())[col]);
                KoRgbU16Traits::setBlue(it.rawData(), blue);

            }

            dev->colorSpace()->setOpacity(it.rawData(), OPACITY_OPAQUE_U8, 1);
            ++it;
        }

    }

    return true;
}


bool PSDImageData::doCMYK(KisPaintDeviceSP dev, QIODevice *io) {
    int channelOffset = 0;

    for (int row = 0; row <m_header->height; row++) {

        KisHLineIterator it = dev->createHLineIterator(0, row, m_header->width);
        QVector<QByteArray> vectorBytes;

        for (int channel = 0; channel < m_header->nChannels; channel++) {

            io->seek(m_channelInfoRecords[channel].channelDataStart + channelOffset);
            vectorBytes.append(io->read(m_header->width*m_channelSize));
            qDebug() << "channel: " << m_channelInfoRecords[channel].channelId << "is at position " << io->pos();

        }
        channelOffset += (m_header->width * m_channelSize);

        qDebug() << "------------------------------------------";
        qDebug() << "channel offset:"<< channelOffset ;
        qDebug() << "------------------------------------------";

        for (int col = 0; col < m_header->width; col++) {

            if (m_channelSize == 1) {

                quint8 C = ntohs(reinterpret_cast<const quint8 *>(vectorBytes[0].constData())[col]);
                KoCmykTraits<quint8>::setC(it.rawData(),C);

                quint8 M = ntohs(reinterpret_cast<const quint8 *>(vectorBytes[1].constData())[col]);
                KoCmykTraits<quint8>::setM(it.rawData(),M);

                quint8 Y = ntohs(reinterpret_cast<const quint8 *>(vectorBytes[2].constData())[col]);
                KoCmykTraits<quint8>::setY(it.rawData(),Y);

                quint8 K = ntohs(reinterpret_cast<const quint8 *>(vectorBytes[3].constData())[col]);
                KoCmykTraits<quint8>::setK(it.rawData(),K);

            }

            else if (m_channelSize == 2) {


                quint16 C = ntohs(reinterpret_cast<const quint16 *>(vectorBytes[0].constData())[col]);
                KoCmykTraits<quint16>::setC(it.rawData(),C);

                quint16 M = ntohs(reinterpret_cast<const quint16 *>(vectorBytes[1].constData())[col]);
                KoCmykTraits<quint16>::setM(it.rawData(),M);

                quint16 Y = ntohs(reinterpret_cast<const quint16 *>(vectorBytes[2].constData())[col]);
                KoCmykTraits<quint16>::setY(it.rawData(),Y);

                quint16 K = ntohs(reinterpret_cast<const quint16 *>(vectorBytes[3].constData())[col]);
                KoCmykTraits<quint16>::setK(it.rawData(),K);

            }

            else if (m_channelSize == 4) {

                quint32 C = ntohs(reinterpret_cast<const quint32 *>(vectorBytes[0].constData())[col]);
                KoCmykTraits<quint32>::setC(it.rawData(),C);

                quint32 M = ntohs(reinterpret_cast<const quint32 *>(vectorBytes[1].constData())[col]);
                KoCmykTraits<quint32>::setM(it.rawData(),M);

                quint32 Y = ntohs(reinterpret_cast<const quint32 *>(vectorBytes[2].constData())[col]);
                KoCmykTraits<quint32>::setY(it.rawData(),Y);

                quint32 K = ntohs(reinterpret_cast<const quint32 *>(vectorBytes[3].constData())[col]);
                KoCmykTraits<quint32>::setK(it.rawData(),K);

            }

            dev->colorSpace()->setOpacity(it.rawData(), OPACITY_OPAQUE_U8, 1);
            ++it;
        }

    }
    return true;
}

bool PSDImageData::doLAB(KisPaintDeviceSP dev, QIODevice *io) {
    int channelOffset = 0;

    for (int row = 0; row <m_header->height; row++) {

        KisHLineIterator it = dev->createHLineIterator(0, row, m_header->width);
        QVector<QByteArray> vectorBytes;

        for (int channel = 0; channel < m_header->nChannels; channel++) {

            io->seek(m_channelInfoRecords[channel].channelDataStart + channelOffset);
            vectorBytes.append(io->read(m_header->width*m_channelSize));
            qDebug() << "channel: " << m_channelInfoRecords[channel].channelId << "is at position " << io->pos();

        }
        channelOffset += (m_header->width * m_channelSize);

        qDebug() << "------------------------------------------";
        qDebug() << "channel offset:"<< channelOffset ;
        qDebug() << "------------------------------------------";

        for (int col = 0; col < m_header->width; col++) {

            if (m_channelSize == 1) {

                quint8 L = ntohs(reinterpret_cast<const quint8 *>(vectorBytes[0].constData())[col]);
                KoLabTraits<quint8>::setL(it.rawData(),L);

                quint8 A = ntohs(reinterpret_cast<const quint8 *>(vectorBytes[1].constData())[col]);
                KoLabTraits<quint8>::setA(it.rawData(),A);

                quint8 B = ntohs(reinterpret_cast<const quint8 *>(vectorBytes[2].constData())[col]);
                KoLabTraits<quint8>::setB(it.rawData(),B);

            }

            else if (m_channelSize == 2) {

                quint16 L = ntohs(reinterpret_cast<const quint16 *>(vectorBytes[0].constData())[col]);
                KoLabTraits<quint16>::setL(it.rawData(),L);

                quint16 A = ntohs(reinterpret_cast<const quint16 *>(vectorBytes[1].constData())[col]);
                KoLabTraits<quint16>::setA(it.rawData(),A);

                quint16 B = ntohs(reinterpret_cast<const quint16 *>(vectorBytes[2].constData())[col]);
                KoLabTraits<quint16>::setB(it.rawData(),B);

            }

            else if (m_channelSize == 4) {

                quint32 L = ntohs(reinterpret_cast<const quint32 *>(vectorBytes[0].constData())[col]);
                KoLabTraits<quint32>::setL(it.rawData(),L);

                quint32 A = ntohs(reinterpret_cast<const quint32 *>(vectorBytes[1].constData())[col]);
                KoLabTraits<quint32>::setA(it.rawData(),A);

                quint32 B = ntohs(reinterpret_cast<const quint32 *>(vectorBytes[2].constData())[col]);
                KoLabTraits<quint32>::setB(it.rawData(),B);

            }

            dev->colorSpace()->setOpacity(it.rawData(), OPACITY_OPAQUE_U8, 1);
            ++it;
        }

    }

    return true;

}

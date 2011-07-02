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

bool PSDImageData::read(KisPaintDeviceSP dev ,QIODevice *io, PSDHeader *header) {

    qDebug() << "Position before read " << io->pos();
    psdread(io, &m_compression);
    qDebug() << "COMPRESSION TYPE " << m_compression;
    quint64 start = io->pos();
    m_channelSize = header->channelDepth/8;
    m_channelDataLength = header->height * header->width * m_channelSize;

    switch (m_compression) {

    case 0: // Uncompressed
        for (int channel = 0; channel < header->nChannels; channel++) {
            ChannelInfo channelInfo;
            channelInfo.channelId = channel;
            channelInfo.compressionType = Compression::Uncompressed;
            channelInfo.channelDataStart = start;
            channelInfo.channelDataLength = header->width * header->height * m_channelSize;
            start += channelInfo.channelDataLength;
            m_channelInfoRecords.append(channelInfo);
        }

        for (int channel = 0; channel < header->nChannels; channel++) {
            qDebug() << "Channel ID: " << m_channelInfoRecords[channel].channelId;
            qDebug() << "Channel Compression Type: " << m_channelInfoRecords[channel].compressionType;
            qDebug() << "Channel Data Start: " << m_channelInfoRecords[channel].channelDataStart;
            qDebug() << "Channel Data Length: " << m_channelInfoRecords[channel].channelDataLength;
            qDebug() << "---------------------------------------------------";
        }

        switch (header->colormode) {
        case Bitmap:
            break;
        case Grayscale:
            break;
        case Indexed:
            break;
        case RGB:
            qDebug()<<"RGB";
            doRGB(dev,io,header);
            break;
        case CMYK:
            break;
        case MultiChannel:
            break;
        case DuoTone:
            break;
        case Lab:
            doLAB(dev,io,header);
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

        for (int channel=0; channel < header->nChannels; channel++) {
            ChannelInfo channelInfo;
            channelInfo.channelId = channel;
            for (int row = 0; row < header->height; row++ ) {
                if (header->version == 1) {
                    psdread(io,(quint16*)&rlelength);
                }
                else if (header->version == 2) {
                    psdread(io,&rlelength);
                }
                channelInfo.rleRowLengths.append(rlelength);
                sumrlelength += rlelength;
            }
            m_channelInfoRecords.append(channelInfo);
        }

        for (int channel = 0; channel < header->nChannels; channel++) {
            m_channelInfoRecords[channel].channelDataStart = start;
            m_channelInfoRecords[channel].channelDataLength = sumrlelength;
        }

        switch (header->colormode) {
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

    case 2: // ZIP without prediction
        qDebug()<<"ZIP without prediction";

        switch (header->colormode) {
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

        switch (header->colormode) {
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

bool PSDImageData::doRGB(KisPaintDeviceSP dev, QIODevice *io, PSDHeader *header) {

    int channelOffset = 0;
    
    for (int row = 0; row <header->height; row++) {
        
        KisHLineIterator it = dev->createHLineIterator(0, row, header->width);
        QVector<QByteArray> vectorBytes;

        for (int channel = 0; channel < header->nChannels; channel++) {

            io->seek(m_channelInfoRecords[channel].channelDataStart + channelOffset);
            vectorBytes.append(io->read(header->width*m_channelSize));
            qDebug() << "channel: " << m_channelInfoRecords[channel].channelId << "is at position " << io->pos();

        }
        channelOffset += (header->width * m_channelSize);
        
        qDebug() << "------------------------------------------";
        qDebug() << "channel offset:"<< channelOffset ;
        qDebug() << "------------------------------------------";

        for (int col = 0; col < header->width; col++) {

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

        }
        ++it;
    }

    return true;
}


bool PSDImageData::doCMYK(KisPaintDeviceSP dev, QIODevice *io, PSDHeader *header) {

    return true;
}

bool PSDImageData::doLAB(KisPaintDeviceSP dev, QIODevice *io, PSDHeader *header) {


    qDebug()<<"Channel Size: "<<m_channelSize;
    int row,col,index;
    QByteArray l,a,b;
    l = io->read(m_channelDataLength);
    a = io->read(m_channelDataLength);
    b = io->read(m_channelDataLength);

    //XXX: this code is only valid for images with channelsize == 2


    for (row = 0; row < header->height; row++) {
        KisHLineIterator it = dev->createHLineIterator(0, row, header->width);
        for ( col = 0; col < header->width; col++) {
            index = row * header->width + col;

            if (m_channelSize == 1) {

                quint8 L = ntohs(reinterpret_cast<const quint8 *>(l.constData())[index]);
                KoLabTraits<quint8>::setL(it.rawData(),L);

                quint8 A = ntohs(reinterpret_cast<const quint8 *>(a.constData())[index]);
                KoLabTraits<quint8>::setA(it.rawData(),A);

                quint8 B = ntohs(reinterpret_cast<const quint8 *>(b.constData())[index]);
                KoLabTraits<quint8>::setB(it.rawData(),B);

            }

            else if (m_channelSize == 2) {

                quint16 L = ntohs(reinterpret_cast<const quint16 *>(l.constData())[index]);
                KoLabTraits<quint16>::setL(it.rawData(),L);

                quint16 A = ntohs(reinterpret_cast<const quint16 *>(a.constData())[index]);
                KoLabTraits<quint16>::setA(it.rawData(),A);

                quint16 B = ntohs(reinterpret_cast<const quint16 *>(b.constData())[index]);
                KoLabTraits<quint16>::setB(it.rawData(),B);

            }

            else if (m_channelSize == 4) {

                quint32 L = ntohs(reinterpret_cast<const quint32 *>(l.constData())[index]);
                KoLabTraits<quint32>::setL(it.rawData(),L);

                quint32 A = ntohs(reinterpret_cast<const quint32 *>(a.constData())[index]);
                KoLabTraits<quint32>::setA(it.rawData(),A);

                quint32 B = ntohs(reinterpret_cast<const quint32 *>(b.constData())[index]);
                KoLabTraits<quint32>::setB(it.rawData(),B);

            }

            dev->colorSpace()->setOpacity(it.rawData(), OPACITY_OPAQUE_U8, 1);

            ++it;
        }
    }

    return true;
}

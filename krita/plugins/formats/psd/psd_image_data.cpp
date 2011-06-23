/***************************************************************************
 *   Copyright (C) 2011 by SIDDHARTH SHARMA siddharth.kde@gmail.com        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/

// XXX: we need this include to be able to convert byte order. Photoshop stores shorts like AB, we need BA to set the values

#include <netinet/in.h> // htonl

#include <QFile>
#include <QDebug>

#include <KoColorSpace.h>
#include <KoColorSpaceMaths.h>
#include <KoColorSpaceTraits.h>

#include <psd_image_data.h>
#include "psd_utils.h"
#include "compression.h"

PSDImageData::PSDImageData(PSDHeader *header)
{
}
PSDImageData::~PSDImageData(){

}

bool PSDImageData::read(KisPaintDeviceSP dev ,QIODevice *io, PSDHeader *header){

    qDebug() << "Position before read " << io->pos();
    psdread(io, &compression);
    qDebug() << "COMPRESSION TYPE " << compression;
    quint64 start = io->pos();
    channelSize = header->channelDepth/8;
    channelDataLength = header->height * header->width * channelSize;

    switch(compression){

    case 0: // Uncompressed
        for (int channel = 0; channel < header->nChannels; channel++) {

            ChannelInfo channelInfo;
            channelInfo.channelId = channel;
            channelInfo.compressionType = Compression::Uncompressed;
            channelInfo.channelDataStart = start;
            channelInfo.channelDataLength = header->width * header->height * channelSize;
            start += channelInfo.channelDataLength;
            channelInfoRecords.append(channelInfo);

        }

        for (int channel = 0; channel < header->nChannels; channel++) {
            qDebug()<<"Channel ID: "<<channelInfoRecords[channel].channelId;
            qDebug()<<"Channel Compression Type: "<<channelInfoRecords[channel].compressionType;
            qDebug()<<"Channe Data Start: "<<channelInfoRecords[channel].channelDataStart;
            qDebug()<<"Channel Data Length: "<<channelInfoRecords[channel].channelDataLength<<endl;
            qDebug()<<"---------------------------------------------------"<<endl;

        }

        switch(header->colormode){
        case Bitmap:
            break;
        case Grayscale:
            break;
        case Indexed:
            break;
        case RGB: qDebug()<<"RGB";
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
        /* quint16 rlelength;
        quint32 sumrlelength;
        for (int channel=0; channel < header->nChannels; channel++){
            channelInfoRecords[channel].channelId = channel;
            for (int row = 0; row < header->height; row++ ){
                if (header->version == 1){
            //        psdread(io,(quint8)&rlelength);
                }
                else if (header->version == 2){
                    psdread(io,&rlelength);
                }
                channelInfoRecords[channel].rleRowLengths.append(rlelength);
                sumrlelength += rlelength;
            }
        }

        for (int channel = 0; channel < header->nChannels; channel++){
            channelInfoRecords[channel].channelDataStart = start;
            channelInfoRecords[channel].channelDataLength = sumrlelength;
        }
*/
        switch(colormode){
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

        switch(colormode){
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

        switch(colormode){
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

bool PSDImageData::doRGB(KisPaintDeviceSP dev, QIODevice *io, PSDHeader *header){
    int row,col,index;
    if (channelSize == 2) {
        for (row = 0; row < header->height; row++) {
            KisHLineIterator it = dev->createHLineIterator(0, row, header->width);
            for ( col = 0; col < header->width; col++) {
                index = row * header->width + col;
                io->seek(channelInfoRecords[0].channelDataStart);
                data = io->read(channelSize);
                quint16 red = ntohs(reinterpret_cast<const quint16 *>(data.constData())[index]);
                KoRgbU16Traits::setRed(it.rawData(), red);

                io->seek(channelInfoRecords[1].channelDataStart);
                data = io->read(channelSize);
                quint16 green = ntohs(reinterpret_cast<const quint16 *>(data.constData())[index]);
                KoRgbU16Traits::setGreen(it.rawData(), green);

                io->seek(channelInfoRecords[2].channelDataStart);
                data = io->read(channelSize);
                quint16 blue = ntohs(reinterpret_cast<const quint16 *>(data.constData())[index]);
                KoRgbU16Traits::setBlue(it.rawData(), blue);

                dev->colorSpace()->setOpacity(it.rawData(), OPACITY_OPAQUE_U8, 1);
                ++it;
            }
        }
    }
    /*
    // XXX: this code is only valid for images with channelsize == 2
    if (channelSize == 2) {

        // Actually, the next step will be to not slurp all the data into a huge QByteArray, but use
        // position and seek to read the bytes, this takes to much memory.
        r = io->read(channelDataLength);
        g = io->read(channelDataLength);
        b = io->read(channelDataLength);


        for (row = 0; row < header->height; row++) {
            KisHLineIterator it = dev->createHLineIterator(0, row, header->width);
            for ( col = 0; col < header->width; col++) {

                // since we're in two-bytes/channel mode anyway, don't multiply with the channelsize.
                // we will convert the _byte_ array to an array of _shorts_, so the index points to
                // the short.
                index = row * header->width + col;

                // step 1: get the constData() -- which is the quint8 array in the byte array. We use const to avoid having Qt make a copy
                // step 2: reinterpret_cast that array to an array of const quint16 (const short)
                // step 3: get the quint16 at the index position
                // step 4: convert from network to host byte order
                quint16 red = ntohs(reinterpret_cast<const quint16 *>(r.constData())[index]);
                // step 5: set the value
                KoRgbU16Traits::setRed(it.rawData(), red);

                // same here
                quint16 green = ntohs(reinterpret_cast<const quint16 *>(g.constData())[index]);
                KoRgbU16Traits::setGreen(it.rawData(), green);

                //  same here
                quint16 blue = ntohs(reinterpret_cast<const quint16 *>(b.constData())[index]);
                KoRgbU16Traits::setBlue(it.rawData(), blue);

                dev->colorSpace()->setOpacity(it.rawData(), OPACITY_OPAQUE_U8, 1);

                ++it;
            }
        }
    }*/
    return true;
}


bool PSDImageData::doCMYK(KisPaintDeviceSP dev, QIODevice *io, PSDHeader *header){
    QByteArray c,m,y,k;
    c = io->read(channelDataLength);
    m = io->read(channelDataLength);
    y = io->read(channelDataLength);
    k = io->read(channelDataLength);

    int row,col,index;
    if (channelSize == 2) {
        for (row = 0; row < header->height; row++) {
            KisHLineIterator it = dev->createHLineIterator(0, row, header->width);
            for ( col = 0; col < header->width; col++) {
                index = row * header->width + col;

                //  quint16 C = ntohs(reinterpret_cast<const quint16 *>(c.constData())[index]);
                //    KoCmykTraits<0,4,5>(it.rawData(),C);

                //     quint16 M = ntohs(reinterpret_cast<const quint16 *>(m.constData())[index]);
                //     KoCmykTraits<M>(it.rawData(),M);
                /*
                quint16 Y = ntohs(reinterpret_cast<const quint16 *>(y.constData())[index]);
                KoCmykTraits<quint16>::setyellow(it.rawData(),Y);

                quint16 K = ntohs(reinterpret_cast<const quint16 *>(k.constData())[index]);
                KoCmykTraits<quint16>::setblack(it.rawData(),K);
*/
                dev->colorSpace()->setOpacity(it.rawData(), OPACITY_OPAQUE_U8, 1);

                ++it;
            }
        }
    }
    return true;
}

bool PSDImageData::doLAB(KisPaintDeviceSP dev, QIODevice *io, PSDHeader *header){
    //use KoLabU16Traits::setL, KoCmykU8Traits::setC and so on
    qDebug()<<"Channel Size: "<<channelSize;
    int row,col,index;
    QByteArray l,a,b;
    l = io->read(channelDataLength);
    a = io->read(channelDataLength);
    b = io->read(channelDataLength);

    // XXX: this code is only valid for images with channelsize == 2
    if (channelSize == 2) {
        for (row = 0; row < header->height; row++) {
            KisHLineIterator it = dev->createHLineIterator(0, row, header->width);
            for ( col = 0; col < header->width; col++) {
                index = row * header->width + col;

                quint16 L = ntohs(reinterpret_cast<const quint16 *>(l.constData())[index]);
                KoLabTraits<quint16>::setL(it.rawData(),L);

                quint16 A = ntohs(reinterpret_cast<const quint16 *>(a.constData())[index]);
                KoLabTraits<quint16>::setA(it.rawData(),A);

                quint16 B = ntohs(reinterpret_cast<const quint16 *>(b.constData())[index]);
                KoLabTraits<quint16>::setB(it.rawData(),B);

                dev->colorSpace()->setOpacity(it.rawData(), OPACITY_OPAQUE_U8, 1);

                ++it;
            }
        }
    }
    return true;
}

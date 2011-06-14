/*
 *  Copyright (c) 2011 siddharth SHARMA <siddharth.kde@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <QFile>
#include <QImage>
#include <QDebug>
#include <QBuffer>

#include "psd_image_data.h"
#include "psd_utils.h"
#include "compression.h"

#include <KoColorSpace.h>
#include <KoColorSpaceMaths.h>
#include <KoColorSpaceTraits.h>


PSDImageData::PSDImageData(PSDHeader *header)
{
}
PSDImageData::~PSDImageData(){

}

bool PSDImageData::read(KisPaintDeviceSP dev ,QIODevice *io, PSDHeader *header){

    qDebug() << "Position before read " << io->pos();

    // Types start with a capital letter, variables with a lower case letter, so _c_ompression, not _C_ompression
    psdread(io, &compression);
    qDebug() << "COMPRESSION TYPE " << compression;
    switch(compression){

    case 0: // Raw Data
        readRawData(dev,io,header);
        break;

    case 1: // RLE
        readRLEData(dev,io,header);
        qDebug()<<"RLE ENCODED";
        break;

    case 2: // ZIP without prediction
        qDebug()<<"ZIP without prediction";
        break;

    case 3: // ZIP with prediction
        qDebug()<<"ZIP with prediction";
        break;

    default:
        break;

    }

    return true;
}

bool PSDImageData::readRawData(KisPaintDeviceSP dev , QIODevice *io, PSDHeader *header)
{
    qDebug() << "compression: "<< compression;
    qDebug() << "Position after read " << io->pos();

    quint16 channelSize = header->channelDepth/8;
    qDebug() << "channelSize  " << channelSize << endl;

    channelDataLength = header->height * header->width * channelSize;

    qDebug() << "Height: " << header->height;
    qDebug() << "Width: "  << header->width;
    qDebug() << "channelDataLength  " << channelDataLength << endl;

    QByteArray r,g,b,a;

    r = io->read(channelDataLength);
    g = io->read(channelDataLength);
    b = io->read(channelDataLength);

    int row,col,index;
    for (row = 0; row < header->height; row++) {
        KisHLineIterator it = dev->createHLineIterator(0, row, header->width);
        for ( col = 0; col < header->width; col++) {
            index = (row * header->width + col) * channelSize;
            KoRgbU16Traits::setRed(it.rawData(),r[index]);
            KoRgbU16Traits::setGreen(it.rawData(),g[index]);
            KoRgbU16Traits::setBlue(it.rawData(),b[index]);
            dev->colorSpace()->setOpacity(it.rawData(), OPACITY_OPAQUE_U8, 1);
            ++it;
        }
    }
    qDebug() << "Position after reading all three channels" << io->pos();

    return true;
}

bool PSDImageData::readRLEData(KisPaintDeviceSP dev, QIODevice *io, PSDHeader *header){

    return true;
}


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

#include "psd_image_data.h"
#include "psd_utils.h"

#include "KoColorSpaceMaths.h"

PSDImageData::PSDImageData(PSDHeader *header)
{
}
PSDImageData::~PSDImageData(){

}

bool PSDImageData::read(QIODevice *io, PSDHeader *header){

    qDebug() << "Position before read " << io->pos();

    // Types start with a capital letter, variables with a lower case letter, so _c_ompression, not _C_ompression
    psdread(io, &compression);
    switch(compression){

      case 0: // Raw Data
            readRawData(io,header);
            break;

      case 1: // RLE
        break;

      case 2: // ZIP without prediction
        break;

      case 3: // ZIP with prediction
        break;

      default:
        break;

    }

    return true;
}

bool PSDImageData::readRawData(QIODevice *io, PSDHeader *header)
{
    QImage image(header->width,header->height, QImage::Format_RGB32 );

    qDebug() << "compression: "<< compression;
    qDebug() << "Position after read " << io->pos();

    quint16 channelSize = header->channelDepth/8;
    qDebug() << "channelSize  " << channelSize << endl;

    channelDataLength = header->height * header->width * channelSize;

    qDebug() << "channelDataLength  " << channelDataLength << endl;
    QByteArray r,g,b,a;
    quint8 rs,gs,bs;
    KoColorSpaceMaths<quint16, quint8> *kcsm;
    r = io->read(channelDataLength);
    g = io->read(channelDataLength);
    b = io->read(channelDataLength);

    for (int k = 1; k <= channelDataLength; k++){
        rs = kcsm->scaleToA(r[k]);
        gs = kcsm->scaleToA(g[k]);
        bs = kcsm->scaleToA(b[k]);
        qDebug() << "Iterating over the data in the channel arrays. Iteration" << k
                 << "Channel size" << channelSize
                 << "red value" << rs
                 << "green value" << gs
                 << "blue value" << bs;
    
        for (int row = 0; row < header->height; row++) {
            qDebug() << "\tIterating over the rows. Current row" << row
                     << "r, g, b" << rs << gs << bs;

            for (int col = 0; col < header->width; col++) {
               qDebug() << "\t\titerating over the columns. Current column" << col
                        << "r, g, b" << rs << gs << bs;

                image.setPixel(QPoint(col, row), qRgb(rs, gs, bs));
            }
        }
    }

    qDebug() << "Position after reading all three channels" << io->pos();

    image.save("test.png");
    return true;
}


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

// XXX: we need this include to be able to convert byte order. Photoshop stores shorts like AB, we need BA to set the values
#include <netinet/in.h> // htonl

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

#include <math.h>

PSDImageData::PSDImageData(PSDHeader *header)
{
}
PSDImageData::~PSDImageData(){

}

bool PSDImageData::read(KisPaintDeviceSP dev ,QIODevice *io, PSDHeader *header){

    qDebug() << "Position before read " << io->pos();
    quint32 channelSize = header->channelDepth/8;
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


    int row,col,index;

    qDebug() << "compression: "<< compression;
    qDebug() << "Position after read " << io->pos();

    channelSize = header->channelDepth/8;
    channelDataLength = header->height * header->width * channelSize;

    qDebug() << "Height: " << header->height;
    qDebug() << "Width: "  << header->width;
    qDebug() << "channelDataLength  " << channelDataLength << endl;

    switch(header->colormode){

    case 0:  // bitmap
        break;

    case 1: // grayscale
        break;

    case 2: // indexed
        break;

    // Use enums, not intes
    case 3: // RGB


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

                    // same here
                    quint16 blue = ntohs(reinterpret_cast<const quint16 *>(b.constData())[index]);
                    KoRgbU16Traits::setBlue(it.rawData(), blue);

                    dev->colorSpace()->setOpacity(it.rawData(), OPACITY_OPAQUE_U8, 1);

                    ++it;
                }

            }
        }
        break;

    case 4:

        /*

        RGB -> CMYK             CMYK -> RGB
        ___________             ___________

        K = min(1-R,1-G,1-B)    R = 1-min(1,C*(1-K)+K)
        C = (1-R-K)/(1-K)       G = 1-min(1,M*(1-K)+K)
        M = (1-G-K)/(1-K)       B = 1-min(1,Y*(1-K)+K)
        Y = (1-B-K)/(1-K)

       */

        cba = io->read(channelDataLength);
        mba = io->read(channelDataLength);
        yba = io->read(channelDataLength);
        kba = io->read(channelDataLength);

        quint64 C,M,Y,K;
        quint8 Red,Green,Blue;

        for (row = 0; row < header->height; row++){
            KisHLineIterator it = dev->createHLineIterator(0, row, header->width);
            for ( col = 0; col < header->width; col++) {
                index = (row * header->width + col) * channelSize;

                C = (255 - cba[index]) / 255;
                M = (255 - mba[index]) / 255;
                Y = (255 - yba[index]) / 255;
                K = (255 - kba[index]) / 255;

                Red = (1.0 - (C * (1 - K) + K)) * 255;
                Green = (1.0 - (M * (1 - K) + K)) * 255;
                Blue =  (1.0 - (Y * (1 - K) + K)) * 255;

                if (Red < 0) Red = 0;
                else if (Red > 255) Red = 255;
                if (Green < 0) Green = 0;
                else if (Green > 255) Green = 255;
                if (Blue < 0) Blue = 0;
                else if (Blue > 255) Blue = 255;

                //XXX

                KoRgbU16Traits::setRed(it.rawData(),Red);
                KoRgbU16Traits::setGreen(it.rawData(),Green);
                KoRgbU16Traits::setBlue(it.rawData(),Blue);
                dev->colorSpace()->setOpacity(it.rawData(), OPACITY_OPAQUE_U8, 1);
            }
        }

        break;

    case 7:
        break;

    case 8:  // Duotone

        r = io->read(channelDataLength);
        g = io->read(channelDataLength);
        b = io->read(channelDataLength);
        break;

    case 9:  // Lab

        // http://en.wikipedia.org/wiki/Lab_color_space
        // L for lightness and a and b for the color-opponent dimension

        lb = io->read(channelDataLength);
        ab = io->read(channelDataLength);
        bb = io->read(channelDataLength);

        quint64 exp_l,exp_a,exp_b,     // Exponents
                l_coef,a_coef,b_coef;  // Coeficients

        l_coef = 2.55;
        a_coef = 1.00;
        b_coef = 1.00;

        /*     for (row = 0; row < header->height; row++){
            KisHLineIterator it = dev->createHLineIterator(0, row, header->width);
            for ( col = 0; col < header->width; col++) {
                index = (row * header->width + col) * channelSize;

                exp_l = lb[index];
                exp_a = ab[index];
                exp_b = bb[index];

                quint16 L = exp_l / l_coef;
                quint16 A = ( exp_a / a_coef - 127.5 );
                quint16 B = ( exp_b / b_coef - 127.5 );

                // 1) convert values to XYZ and then to RGB
                // 2) here we use standards Observer = 2, Illuminant = D65
                // 3) For presicion we use 3 decimal places

                const quint64 rX = 95.047;
                const quint64 rY = 100.000;
                const quint64 rZ = 108.883;

                // XXX Need to check whether i do it right or not

    /*            quint64 vY = (L + 16.0) / 116.0;
                quint64 vX =  A / 500.0 + vY;
                quint64 vZ =  vY - B / 200.0;

                quint64 vX3 = vX * vX * vX;
                quint64 vY3 = vY * vY * vY;
                quint64 vZ3 = vZ * vZ * vZ;
*/
        /*          if ( vY3 > 0.008856 )
                    vY = vY3;
                else
                    vY = (vY - 16 / 116 ) / 7.787;

                if ( vX3 > 0.008856 )
                    vX = vX3;
                else
                    vY = (vY - 16 / 116 ) / 7.787;

                if ( vZ3 > 0.008856 )
                    vZ = vZ3;
                else
                    vZ = (vZ - 16 / 116 ) / 7.787;*/

        // Converted values from LAB -> XYZ
        /*
                quint64 X = rX * vX;
                quint64 Y = rY * vY;
                quint64 Z = rZ * vZ;

                quint64 cX = X / 100.0;
                quint64 cY = Y / 100.0;
                quint64 cZ = Z / 100.0;

                quint64 vR = cX * 3.2406 + cY * ( -1.5372 ) + cZ * ( -0.4986 );
                quint64 vG = cX * ( -0.9689 ) + cY * 1.8758 + cZ * 0.0415;
                quint64 vB = cX * 0.0557 + vY * ( -0.2040 ) + cZ * 1.0570;

                      SUDO CODE
                if (vR > 0.0031308)
                  ;//    vR = 1.055 * (Power(vR, 1 / 2.4)) - 0.055;
                    else
                      vR = 12.92 * vR;

                    if (var_G > 0.0031308)
                   ;//   vG = 1.055 * (Power(vG, 1 / 2.4)) - 0.055;
                    else
                      vG = 12.92 * vG;

                    if (vB > 0.0031308)
                     ;// vB = 1.055 * (Power(vB, 1 / 2.4)) - 0.055;
            }
        }*/
        break;

    case 10:
        break;

    case 11:
        break;

    case 12:
        break;

    case 13:
        break;

    case 14:
        break;

    default:
        break;

    }

    qDebug() << "Position after reading all three channels" << io->pos();

    return true;
}

bool PSDImageData::readRLEData(KisPaintDeviceSP dev, QIODevice *io, PSDHeader *header){

    return true;
}


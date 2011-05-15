/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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
#include "psd_colormode_block.h"

#include "psd.h"
#include "psd_utils.h"
#include <QtCore/QByteArray>
#include <qrgb.h>

PSDColorModeBlock::PSDColorModeBlock(PSDColorMode colormode)
    : blocksize(0)
    , colormode(colormode)
{
}

bool PSDColorModeBlock::read(QIODevice* io)
{
    // get length
    psdread(io, &blocksize);

    if (blocksize == 0) {
        if (colormode == Indexed || colormode == DuoTone) {
            error = "Blocksize of 0 and Indexed or DuoTone colormode";
            return false;
        }
        else {
            return true;
        }
    }
    if (colormode == Indexed && blocksize != 768) {
        error = QString("Indexed mode, but block size is %1.").arg(blocksize);
        return false;
    }

    data = io->read(blocksize);
    if ((quint32)data.size() != blocksize) return false;

    if (colormode == Indexed) {
     indextoRGB(data); 
      qFatal("TODO: Compute the colormap");
        return false;
    }
    return valid();
}

void PSDColorModeBlock::indextoRGB(QByteArray data)
{
 // dont know if it correct or not need more work 
int rgb[2];
 QRgb color;
for(int i=0;i<=256;)
{
  rgb[0]=data[i];
  rgb[1]=data[++i];
  rgb[2]=data[++i];
  color = qRgb(rgb[0],rgb[1],rgb[2]);
  i=+3;
}
return color;
}



bool PSDColorModeBlock::write(QIODevice* io)
{
    Q_UNUSED(io);
    Q_ASSERT(valid());
    if (!valid()) {
        error = "Cannot write an invalid Color Mode Block";
        return false;
    }
    qFatal("TODO: implement writing the colormode block");
    return false;
}

bool PSDColorModeBlock::valid()
{
    if (blocksize == 0 && (colormode == Indexed || colormode == DuoTone)) {
        error = "Blocksize of 0 and Indexed or DuoTone colormode";
        return false;
    }
    if (colormode == Indexed && blocksize != 768) {
        error = QString("Indexed mode, but block size is %1.").arg(blocksize);
        return false;
    }
    if (colormode == DuoTone && blocksize == 0) {
        error == QString("DuoTone mode, but data block is empty");
        return false;
    }
    if ((quint32)data.size() != blocksize) {
        error = QString("Data size is %1, but block size is %2").arg(data.size()).arg(blocksize);
        return false;
    }
    return true;
}

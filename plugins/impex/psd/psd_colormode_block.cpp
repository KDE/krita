/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "psd_colormode_block.h"

#include <psd_utils.h>
#include <QByteArray>
#include <QColor>

PSDColorModeBlock::PSDColorModeBlock(psd_color_mode colormode)
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
        int i = 0;
        while (i <= 767) {
            colormap.append(qRgb(data[i],data[i + 1],data[i + 2]));
            i += 2;
        }
    }
    else {
        duotoneSpecification = data;
    }
    return valid();
}



bool PSDColorModeBlock::write(QIODevice* io)
{
    if (!valid()) {
        error = "Cannot write an invalid Color Mode Block";
        return false;
    }
    if (colormap.size() > 0 && colormode == Indexed) {
        error = "Cannot write indexed color data";
        return false;
    }
    else if (duotoneSpecification.size() > 0 && colormode == DuoTone) {
        quint32 blocksize = duotoneSpecification.size();
        psdwrite(io, blocksize);
        if (io->write(duotoneSpecification.constData(), blocksize) != blocksize) {
            error = "Failed to write duotone specification";
            return false;
        }
    }
    else {
        quint32 i = 0;
        psdwrite(io, i);
    }
    return true;
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
        error = QString("DuoTone mode, but data block is empty");
        return false;
    }
    if ((quint32)data.size() != blocksize) {
        error = QString("Data size is %1, but block size is %2").arg(data.size()).arg(blocksize);
        return false;
    }
    return true;
}

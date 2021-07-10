/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "psd_header.h"

#include <QByteArray>
#include <QIODevice>

#include "psd_utils.h"
#include <QtEndian>

struct Header {
    char signature[4]; // 8PBS
    char version[2]; // 1 or 2
    char padding[6];
    char nChannels[2]; // 1 - 56
    char height[4]; // 1-30,000 or 1 - 300,000
    char width[4]; // 1-30,000 or 1 - 300,000
    char channelDepth[2]; // 1, 8, 16
    char colormode[2]; // 0-9
};

PSDHeader::PSDHeader()
    : version(0)
    , nChannels(0)
    , height(0)
    , width(0)
    , channelDepth(0)
    , colormode(COLORMODE_UNKNOWN)
{
}

bool PSDHeader::read(QIODevice &device)
{
    Header header;
    quint64 bytesRead = device.read((char *)&header, sizeof(Header));
    if (bytesRead != sizeof(Header)) {
        error = "Could not read header: not enough bytes";
        return false;
    }

    signature = QString(header.signature);
    memcpy(&version, header.version, 2);
    version = qFromBigEndian(version);
    memcpy(&nChannels, header.nChannels, 2);
    nChannels = qFromBigEndian(nChannels);
    memcpy(&height, header.height, 4);
    height = qFromBigEndian(height);
    memcpy(&width, header.width, 4);
    width = qFromBigEndian(width);
    memcpy(&channelDepth, header.channelDepth, 2);
    channelDepth = qFromBigEndian(channelDepth);
    memcpy(&colormode, header.colormode, 2);
    colormode = (psd_color_mode)qFromBigEndian((quint16)colormode);

    return valid();
}

bool PSDHeader::write(QIODevice *device)
{
    if (!valid())
        return false;
    if (!psdwrite(device, signature))
        return false;
    if (!psdwrite(device, version))
        return false;
    if (!psdpad(device, 6))
        return false;
    if (!psdwrite(device, nChannels))
        return false;
    if (!psdwrite(device, height))
        return false;
    if (!psdwrite(device, width))
        return false;
    if (!psdwrite(device, channelDepth))
        return false;
    if (!psdwrite(device, (quint16)colormode))
        return false;
    return true;
}

bool PSDHeader::valid()
{
    if (signature != "8BPS") {
        error = "Not a PhotoShop document. Signature is: " + signature;
        return false;
    }
    if (version < 1 || version > 2) {
        error = QString("Wrong version: %1").arg(version);
        return false;
    }
    if (nChannels < 1 || nChannels > 56) {
        error = QString("Channel count out of range: %1").arg(nChannels);
        return false;
    }
    if (version == 1) {
        if (height < 1 || height > 30000) {
            error = QString("Height out of range: %1").arg(height);
            return false;
        }
        if (width < 1 || width > 30000) {
            error = QString("Width out of range: %1").arg(width);
            return false;
        }
    } else /* if (version == 2) */ {
        if (height < 1 || height > 300000) {
            error = QString("Height out of range: %1").arg(height);
            return false;
        }
        if (width < 1 || width > 300000) {
            error = QString("Width out of range: %1").arg(width);
            return false;
        }
    }
    if (channelDepth != 1 && channelDepth != 8 && channelDepth != 16) {
        error = QString("Channel depth incorrect: %1").arg(channelDepth);
        return false;
    }
    if (colormode < 0 || colormode > 9) {
        error = QString("Colormode is out of range: %1").arg(colormode);
        return false;
    }

    return true;
}

QDebug operator<<(QDebug dbg, const PSDHeader &header)
{
#ifndef NODEBUG
    dbg.nospace() << "(valid: " << const_cast<PSDHeader *>(&header)->valid();
    dbg.nospace() << ", signature: " << header.signature;
    dbg.nospace() << ", version:" << header.version;
    dbg.nospace() << ", number of channels: " << header.nChannels;
    dbg.nospace() << ", height: " << header.height;
    dbg.nospace() << ", width: " << header.width;
    dbg.nospace() << ", channel depth: " << header.channelDepth;
    dbg.nospace() << ", color mode: ";
    switch (header.colormode) {
    case (Bitmap):
        dbg.nospace() << "Bitmap";
        break;
    case (Grayscale):
        dbg.nospace() << "Grayscale";
        break;
    case (Indexed):
        dbg.nospace() << "Indexed";
        break;
    case (RGB):
        dbg.nospace() << "RGB";
        break;
    case (CMYK):
        dbg.nospace() << "CMYK";
        break;
    case (MultiChannel):
        dbg.nospace() << "MultiChannel";
        break;
    case (DuoTone):
        dbg.nospace() << "DuoTone";
        break;
    case (Lab):
        dbg.nospace() << "Lab";
        break;
    default:
        dbg.nospace() << "Unknown";
    };
    dbg.nospace() << ")";
#endif
    return dbg.nospace();
}

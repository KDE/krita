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
#include "psd_header.h"

#include <QByteArray>
#include <QIODevice>

#include "psd_utils.h"
#include <netinet/in.h> // htonl

struct Header {
    char signature[4];    // 8PBS
    char version[2];      // 1 or 2
    char padding[6];
    char nChannels[2];    // 1 - 56
    char height[4];       // 1-30,000 or 1 - 300,000
    char width[4];        // 1-30,000 or 1 - 300,000
    char channelDepth[2]; // 1, 8, 16
    char colormode[2];    // 0-9
};


PSDHeader::PSDHeader()
    : m_version(0)
    , m_nChannels(0)
    , m_height(0)
    , m_width(0)
    , m_channelDepth(0)
    , m_colormode(UNKNOWN)
{
}

bool PSDHeader::read(QIODevice* device)
{
    Header header;
    quint64 bytesRead = device->read((char*)&header, sizeof(Header));
    if (bytesRead != sizeof(Header)) {
        error = "Could not read header: not enough bytes";
        return false;
    }

    m_signature = QString(header.signature);
    memcpy(&m_version, header.version, 2);
    m_version = ntohs(m_version);
    memcpy(&m_nChannels, header.nChannels, 2);
    m_nChannels = ntohs(m_nChannels);
    memcpy(&m_height, header.height, 4);
    m_height = ntohl(m_height);
    memcpy(&m_width, header.width, 4);
    m_width = ntohl(m_width);
    memcpy(&m_channelDepth, header.channelDepth, 2);
    m_channelDepth = ntohs(m_channelDepth);
    memcpy(&m_colormode, header.colormode, 2);
    m_colormode = (PSDColorMode)ntohs((quint16)m_colormode);

    return valid();
}

bool PSDHeader::write(QIODevice* device)
{
    Q_ASSERT(valid());
    if (!valid()) return false;
    if (!psdwrite(device, m_signature)) return false;
    if (!psdwrite(device, m_version)) return false;
    if (!psdpad(device, 6)) return false;
    if (!psdwrite(device, m_nChannels)) return false;
    if (!psdwrite(device, m_height)) return false;
    if (!psdwrite(device, m_width)) return false;
    if (!psdwrite(device, m_channelDepth)) return false;
    if (!psdwrite(device, (quint16)m_colormode)) return false;
    return true;
}

bool PSDHeader::valid()
{
    if (m_signature != "8BPS") {
        error = "Not a PhotoShop document. Signature is: " + m_signature;
        return false;
    }
    if (m_version < 1 || m_version > 2) {
        error = QString("Wrong version: %1").arg(m_version);
        return false;
    }
    if (m_nChannels < 1 || m_nChannels > 56) {
        error = QString("Channel count out of range: %1").arg(m_nChannels);
        return false;
    }
    if (m_version == 1) {
        if (m_height < 1 || m_height > 30000) {
            error = QString("Height out of range: %1").arg(m_height);
            return false;
        }
        if (m_width < 1 || m_width > 30000) {
            error = QString("Width out of range: %1").arg(m_width);
            return false;
        }
    }
    else if (m_version == 2) {
        if (m_height < 1 || m_height > 300000) {
            error = QString("Height out of range: %1").arg(m_height);
            return false;
        }
        if (m_width < 1 || m_width > 300000) {
            error = QString("Width out of range: %1").arg(m_width);
            return false;
        }
    }
    if (m_channelDepth != 1 && m_channelDepth != 8 && m_channelDepth != 16) {
        error = QString("Channel depth incorrect: %1").arg(m_channelDepth);
        return false;
    }
    if (m_colormode < 0 || m_colormode > 9) {
        error = QString("Colormode is out of range: %1").arg(m_colormode);
        return false;
    }

    return true;
}

QDebug operator<<(QDebug dbg, const PSDHeader& header)
{
#ifndef NODEBUG
    dbg.nospace() << "(valid: " << const_cast<PSDHeader*>(&header)->valid();
    dbg.nospace() << ", signature: " << header.m_signature;
    dbg.nospace() << ", version:" << header.m_version;
    dbg.nospace() << ", number of channels: " << header.m_nChannels;
    dbg.nospace() << ", height: " << header.m_height;
    dbg.nospace() << ", width: " << header.m_width;
    dbg.nospace() << ", channel depth: " << header.m_channelDepth;
    dbg.nospace() << ", color mode: ";
    switch(header.m_colormode) {
    case(Bitmap):
        dbg.nospace() << "Bitmap";
        break;
    case(Grayscale):
        dbg.nospace() << "Grayscale";
        break;
    case(Indexed):
        dbg.nospace() << "Indexed";
        break;
    case(RGB):
        dbg.nospace() << "RGB";
        break;
    case(CMYK):
        dbg.nospace() << "CMYK";
        break;
    case(MultiChannel):
        dbg.nospace() << "MultiChannel";
        break;
    case(DuoTone):
        dbg.nospace() << "DuoTone";
        break;
    case(Lab):
        dbg.nospace() << "Lab";
        break;
    default:
        dbg.nospace() << "Unknown";
    };
    dbg.nospace() << ")";
#endif
    return dbg.nospace();
}

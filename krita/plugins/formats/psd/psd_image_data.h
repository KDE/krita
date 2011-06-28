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

#ifndef PSD_IMAGE_DATA_H
#define PSD_IMAGE_DATA_H

#include <kis_paint_device.h>
#include <kis_types.h>

#include <psd.h>
#include <psd_header.h>
#include <compression.h>

#include <QFile>
class QIODevice;


class PSDImageData
{

public:
    PSDImageData(PSDHeader *header);
    virtual ~PSDImageData();

    bool read(KisPaintDeviceSP dev ,QIODevice *io, PSDHeader *header);
    bool doRGB(KisPaintDeviceSP dev ,QIODevice *io, PSDHeader *header);
    bool doCMYK(KisPaintDeviceSP dev ,QIODevice *io, PSDHeader *header);
    bool doLAB(KisPaintDeviceSP dev ,QIODevice *io, PSDHeader *header);

private:
    quint16 compression;
    quint64 channelDataLength;
    quint32 channelSize;

    struct ChannelInfo {
        qint16 channelId;
        Compression::CompressionType compressionType;
        quint64 channelDataStart;
        quint64 channelDataLength;
        QVector<quint32> rleRowLengths;
    };

    QVector<ChannelInfo> channelInfoRecords;
    PSDColorMode colormode;
};

#endif // PSD_IMAGE_DATA_H

/*
 *   SPDX-FileCopyrightText: 2011 Siddharth Sharma <siddharth.kde@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef PSD_IMAGE_DATA_H
#define PSD_IMAGE_DATA_H

#include <kis_paint_device.h>
#include <kis_types.h>

#include <psd.h>
#include <psd_header.h>
#include <compression.h>
#include <psd_layer_record.h>

#include <QFile>
class QIODevice;


class PSDImageData
{

public:
    PSDImageData(PSDHeader *header);
    virtual ~PSDImageData();

    bool read(QIODevice &io, KisPaintDeviceSP dev);
    bool write(QIODevice *io, KisPaintDeviceSP dev, bool hasAlpha);


    QString error;

private:
    bool readRGB(QIODevice &io, KisPaintDeviceSP dev);
    bool readCMYK(QIODevice &io, KisPaintDeviceSP dev);
    bool readLAB(QIODevice &io, KisPaintDeviceSP dev);
    bool readGrayscale(QIODevice &io, KisPaintDeviceSP dev);

    PSDHeader *m_header;

    quint16 m_compression;
    quint64 m_channelDataLength;
    quint32 m_channelSize;

    QVector<ChannelInfo> m_channelInfoRecords;
    QVector<int> m_channelOffsets; // this doesn't need to be global
};

#endif // PSD_IMAGE_DATA_H

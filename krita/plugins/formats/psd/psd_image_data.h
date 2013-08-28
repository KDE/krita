/*
 *   Copyright (C) 2011 by Siddharth Sharma <siddharth.kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.                              
 *                                                                    
 *   This program is distributed in the hope that it will be useful,  
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of   
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    
 *   GNU General Public License for more details.                     
 *                                                                    
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/> 
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

    bool read(QIODevice *io, KisPaintDeviceSP dev);
    bool write(QIODevice *io, KisPaintDeviceSP dev);


    QString error;

private:

    bool readRGB(QIODevice *io, KisPaintDeviceSP dev);
    bool readCMYK(QIODevice *io, KisPaintDeviceSP dev);
    bool readLAB(QIODevice *io, KisPaintDeviceSP dev);
    bool readGrayscale(QIODevice *io, KisPaintDeviceSP dev);

    PSDHeader *m_header;
    
    quint16 m_compression;
    quint64 m_channelDataLength;
    quint32 m_channelSize;

    QVector<ChannelInfo> m_channelInfoRecords;
    QVector<int> m_channelOffsets; // this doesn't need to be global
};

#endif // PSD_IMAGE_DATA_H

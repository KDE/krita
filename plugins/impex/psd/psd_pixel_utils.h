/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __PSD_PIXEL_UTILS_H
#define __PSD_PIXEL_UTILS_H

#include <QVector>
#include <QRect>

#include "psd.h"
#include "kis_types.h"

class QIODevice;
struct ChannelInfo;
struct ChannelWritingInfo;

namespace PsdPixelUtils
{

    struct ChannelWritingInfo {
        ChannelWritingInfo() : channelId(0), sizeFieldOffset(-1), rleBlockOffset(-1) {}
        ChannelWritingInfo(qint16 _channelId, int _sizeFieldOffset) : channelId(_channelId), sizeFieldOffset(_sizeFieldOffset), rleBlockOffset(-1) {}
        ChannelWritingInfo(qint16 _channelId, int _sizeFieldOffset, int _rleBlockOffset) : channelId(_channelId), sizeFieldOffset(_sizeFieldOffset), rleBlockOffset(_rleBlockOffset) {}

        qint16 channelId;
        int sizeFieldOffset;
        int rleBlockOffset;
    };

    void readChannels(QIODevice *io,
                      KisPaintDeviceSP device,
                      psd_color_mode colorMode,
                      int channelSize,
                      const QRect &layerRect,
                      QVector<ChannelInfo*> infoRecords);

    void readAlphaMaskChannels(QIODevice *io,
                               KisPaintDeviceSP device,
                               int channelSize,
                               const QRect &layerRect,
                               QVector<ChannelInfo*> infoRecords);

    void writeChannelDataRLE(QIODevice *io,
                             const quint8 *plane,
                             const int channelSize,
                             const QRect &rc,
                             const qint64 sizeFieldOffset,
                             const qint64 rleBlockOffset,
                             const bool writeCompressionType);

    void writePixelDataCommon(QIODevice *io,
                              KisPaintDeviceSP dev,
                              const QRect &rc,
                              psd_color_mode colorMode,
                              int channelSize,
                              bool alphaFirst,
                              const bool writeCompressionType,
                              QVector<ChannelWritingInfo> &writingInfoList);

}

#endif /* __PSD_PIXEL_UTILS_H */

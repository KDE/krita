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
#ifndef PSD_LAYER_RECORD_H
#define PSD_LAYER_RECORD_H

#include <QString>
#include <QVector>
#include <QByteArray>
#include <QBitArray>

#include <kis_types.h>
#include <kis_paint_device.h>
#include <kis_node.h>

#include "psd.h"
#include "psd_header.h"

#include "compression.h"

#include "psd_additional_layer_info_block.h"

class QIODevice;

enum psd_layer_type {
    psd_layer_type_normal,
    psd_layer_type_hidden,
    psd_layer_type_folder,
    psd_layer_type_solid_color,
    psd_layer_type_gradient_fill,
    psd_layer_type_pattern_fill,
    psd_layer_type_levels,
    psd_layer_type_curves,
    psd_layer_type_brightness_contrast,
    psd_layer_type_color_balance,
    psd_layer_type_hue_saturation,
    psd_layer_type_selective_color,
    psd_layer_type_threshold,
    psd_layer_type_invert,
    psd_layer_type_posterize,
    psd_layer_type_channel_mixer,
    psd_layer_type_gradient_map,
    psd_layer_type_photo_filter,
};

struct  ChannelInfo {

    ChannelInfo()
        : channelId(0)
        , compressionType(Compression::Unknown)
        , channelDataStart(0)
        , channelDataLength(0)
        , channelOffset(0)
        , channelInfoPosition(0)
    {}

    qint16 channelId; // 0 red, 1 green, 2 blue, -1 transparency, -2 user-supplied layer mask
    Compression::CompressionType compressionType;
    quint64 channelDataStart;
    quint64 channelDataLength;
    QVector<quint32> rleRowLengths;
    int channelOffset; // where the channel data starts
    int channelInfoPosition; // where the channelinfo record is saved in the file
};

class PSDLayerRecord
{
public:

    PSDLayerRecord(const PSDHeader &header);

    ~PSDLayerRecord()
    {
        qDeleteAll(channelInfoRecords);
    }

    QRect channelRect(ChannelInfo *channel) const;

    bool read(QIODevice* io);
    bool readPixelData(QIODevice* io, KisPaintDeviceSP device);
    bool readMask(QIODevice* io, KisPaintDeviceSP dev, ChannelInfo *channel);

    bool write(QIODevice* io, KisNodeSP node);
    bool writePixelData(QIODevice* io);

    bool valid();

    QString error;

    qint32 top;
    qint32 left;
    qint32 bottom;
    qint32 right;

    quint16 nChannels;

    QVector<ChannelInfo*> channelInfoRecords;

    QString blendModeKey;

    quint8 opacity;
    quint8 clipping;
    bool   transparencyProtected;
    bool   visible;
    bool   irrelevant;

    struct LayerMaskData {
        qint32 top;
        qint32 left;
        qint32 bottom;
        qint32 right;
        quint8 defaultColor; // 0 or 255
        bool positionedRelativeToLayer;
        bool disabled;
        bool invertLayerMaskWhenBlending;
        quint8 userMaskDensity;
        double userMaskFeather;
        quint8 vectorMaskDensity;
        double vectorMaskFeather;
    };

    LayerMaskData layerMask;

    struct LayerBlendingRanges {

        QByteArray data;

        quint8 blackValues[2];
        quint8 whiteValues[2];
        quint32 compositeGrayBlendDestinationRange;
        QVector<QPair<quint32, quint32> > sourceDestinationRanges;
    };

    LayerBlendingRanges blendingRanges;

    QString layerName; // pascal, not unicode!

    PsdAdditionalLayerInfoBlock infoBlocks;

private:

    bool doRGB(KisPaintDeviceSP dev ,QIODevice *io);
    bool doCMYK(KisPaintDeviceSP dev ,QIODevice *io);
    bool doLAB(KisPaintDeviceSP dev ,QIODevice *io);
    bool doGrayscale(KisPaintDeviceSP dev ,QIODevice *io);

    KisNodeSP m_node;
    const PSDHeader m_header;
};

QDebug operator<<(QDebug dbg, const PSDLayerRecord& layer);
QDebug operator<<(QDebug dbg, const ChannelInfo& layer);

#endif // PSD_LAYER_RECORD_H

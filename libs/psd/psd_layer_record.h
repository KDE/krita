/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef PSD_LAYER_RECORD_H
#define PSD_LAYER_RECORD_H

#include "kritapsd_export.h"

#include <QBitArray>
#include <QByteArray>
#include <QString>
#include <QVector>

#include <kis_node.h>
#include <kis_paint_device.h>
#include <kis_types.h>
#include <psd.h>

#include "psd_additional_layer_info_block.h"
#include "psd_header.h"

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

struct KRITAPSD_EXPORT ChannelInfo {
    ChannelInfo()
        : channelId(0)
        , compressionType(psd_compression_type::Unknown)
        , channelDataStart(0)
        , channelDataLength(0)
        , channelOffset(0)
        , channelInfoPosition(0)
    {
    }

    qint16 channelId; // 0 red, 1 green, 2 blue, -1 transparency, -2 user-supplied layer mask
    psd_compression_type compressionType;
    quint64 channelDataStart;
    quint64 channelDataLength;
    QVector<quint32> rleRowLengths;
    int channelOffset; // where the channel data starts
    int channelInfoPosition; // where the channelinfo record is saved in the file
};

class KRITAPSD_EXPORT PSDLayerRecord
{
public:
    PSDLayerRecord(const PSDHeader &header);

    ~PSDLayerRecord()
    {
        qDeleteAll(channelInfoRecords);
    }

    QRect channelRect(ChannelInfo *channel) const;

    bool read(QIODevice &io);
    bool readPixelData(QIODevice &io, KisPaintDeviceSP device);
    bool readMask(QIODevice &io, KisPaintDeviceSP dev, ChannelInfo *channel);

    void write(QIODevice &io,
               KisPaintDeviceSP layerContentDevice,
               KisNodeSP onlyTransparencyMask,
               const QRect &maskRect,
               psd_section_type sectionType,
               const QDomDocument &stylesXmlDoc,
               bool useLfxsLayerStyleFormat);
    void writePixelData(QIODevice &io, psd_compression_type compressionType);

    bool valid();

    QString error;

    qint32 top;
    qint32 left;
    qint32 bottom;
    qint32 right;

    quint16 nChannels;

    QVector<ChannelInfo *> channelInfoRecords;

    QString blendModeKey;
    bool isPassThrough;

    quint8 opacity;
    quint8 clipping;
    bool transparencyProtected;
    bool visible;
    bool irrelevant;

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
        struct LayerBlendingRange {
            std::array<quint8, 2> blackValues;
            std::array<quint8, 2> whiteValues;
        };

        QByteArray data;

        QPair<LayerBlendingRange, LayerBlendingRange> compositeGrayRange;
        QVector<QPair<LayerBlendingRange, LayerBlendingRange>> sourceDestinationRanges;
    };

    LayerBlendingRanges blendingRanges;

    QString layerName; // pascal, not unicode!

    PsdAdditionalLayerInfoBlock infoBlocks;

private:
    template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
    bool readImpl(QIODevice &io);

    template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
    void writeImpl(QIODevice &io,
                   KisPaintDeviceSP layerContentDevice,
                   KisNodeSP onlyTransparencyMask,
                   const QRect &maskRect,
                   psd_section_type sectionType,
                   const QDomDocument &stylesXmlDoc,
                   bool useLfxsLayerStyleFormat);

    template<psd_byte_order = psd_byte_order::psdBigEndian>
    void writeTransparencyMaskPixelData(QIODevice &io);

    template<psd_byte_order = psd_byte_order::psdBigEndian>
    void writePixelDataImpl(QIODevice &io, psd_compression_type compressionType);

    KisPaintDeviceSP convertMaskDeviceIfNeeded(KisPaintDeviceSP dev);

private:
    KisPaintDeviceSP m_layerContentDevice;
    KisNodeSP m_onlyTransparencyMask;
    QRect m_onlyTransparencyMaskRect;
    qint64 m_transparencyMaskSizeOffset;

    const PSDHeader m_header;
};

KRITAPSD_EXPORT QDebug operator<<(QDebug dbg, const PSDLayerRecord &layer);
KRITAPSD_EXPORT QDebug operator<<(QDebug dbg, const ChannelInfo &layer);

inline QDebug &operator<<(QDebug dbg, const PSDLayerRecord::LayerBlendingRanges::LayerBlendingRange &data)
{
    return dbg << data.blackValues[0] << data.blackValues[1] << data.whiteValues[0] << data.whiteValues[1];
}

#endif // PSD_LAYER_RECORD_H

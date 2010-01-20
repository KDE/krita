/*
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
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
/*
 * Constants and defines taken from gimp and psdparse
 */
#ifndef PSD_H
#define PSD_H

#include <QPair>
#include <QString>

const int MAX_CHANNELS = 56;


/**
 * Image color/depth modes
 */
enum PSDColorMode {
    Bitmap = 0,
    Grayscale,
    Indexed,
    RGB,
    CMYK,
    MultiChannel,
    DuoTone,
    Lab,
    Gray16,
    RGB48,
    Lab48,
    CMYK64,
    DeepMultichannel,
    Duotone16,
    UNKNOWN = 9000
 };


/**
 * color samplers, apparently distict from PSDColormode
 */
namespace PSDColorSampler {
enum PSDColorSamplers {
    RGB,
    HSB,
    CMYK,
    PANTONE, // LAB
    FOCOLTONE, // CMYK
    TRUMATCH, // CMYK
    TOYO, // LAB
    LAB,
    GRAYSCALE,
    HKS, // CMYK
    DIC, // LAB
    TOTAL_INK,
    MONITOR_RGB,
    DUOTONE,
    OPACITY,
    ANPA      = 3000 // LAB
};
};

namespace Compression {
enum CompressionType {
    Uncompressed = 0,
    RLE,
    ZIP,
    ZIPWithPrediction,
    Unknown
};
};

/**
 * Convert PsdColorMode to pigment colormodelid and colordepthid.
 * @see KoColorModelStandardIds
 *
 * @return a QPair containing ColorModelId and ColorDepthID
 */
QPair<QString, QString> psd_colormode_to_colormodelid(PSDColorMode colormode, quint16 channelDepth);


/**
 * Convert the Photoshop blend mode strings to Pigment compositeop id's
 */
QString psd_blendmode_to_composite_op(const QString& blendmode);
#endif // PSD_H

/*
 *  SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef PSD_ADDITIONAL_LAYER_INFO_BLOCK_H
#define PSD_ADDITIONAL_LAYER_INFO_BLOCK_H

#include "kritapsd_export.h"

#include <QBitArray>
#include <QByteArray>
#include <QDomDocument>
#include <QIODevice>
#include <QString>
#include <QVector>
#include <boost/function.hpp>

#include <kis_types.h>

#include <kis_node.h>
#include <kis_paint_device.h>
#include <psd.h>

#include "psd_header.h"

// additional layer information

// LEVELS
// Level record
struct KRITAPSD_EXPORT psd_layer_level_record {
    quint16 input_floor; // (0...253)
    quint16 input_ceiling; // (2...255)
    quint16 output_floor; // 255). Matched to input floor.
    quint16 output_ceiling; // (0...255)
    float gamma; // Short integer from 10...999 representing 0.1...9.99. Applied to all image data.
};

// Levels settings files are loaded and saved in the Levels dialog.
struct KRITAPSD_EXPORT psd_layer_levels {
    psd_layer_level_record record[29]; // 29 sets of level records, each level containing 5 qint8 integers
    // Photoshop CS (8.0) Additional information
    // At the end of the Version 2 file is the following information:
    quint16 extra_level_count; // Count of total level record structures. Subtract the legacy number of level record structures, 29, to determine how many are
                               // remaining in the file for reading.
    psd_layer_level_record *extra_record; // Additianol level records according to count
    quint8 lookup_table[3][256];
};

// CURVES
// The following is the data for each curve specified by count above
struct KRITAPSD_EXPORT psd_layer_curves_data {
    quint16 channel_index; // Before each curve is a channel index.
    quint16 point_count; // Count of points in the curve (qint8 integer from 2...19)
    quint16 output_value[19]; // All coordinates have range 0 to 255
    quint16 input_value[19];
};

// Curves file format
struct KRITAPSD_EXPORT psd_layer_curves {
    quint16 curve_count; // Count of curves in the file.
    psd_layer_curves_data *curve;
    quint8 lookup_table[3][256];
};

// BRIGHTNESS AND CONTRAST
struct KRITAPSD_EXPORT psd_layer_brightness_contrast {
    qint8 brightness;
    qint8 contrast;
    qint8 mean_value; // for brightness and contrast
    qint8 Lab_color;
    quint8 lookup_table[256];
};

// COLOR BALANCE
struct KRITAPSD_EXPORT psd_layer_color_balance {
    qint8 cyan_red[3]; // (-100...100). shadows, midtones, highlights
    qint8 magenta_green[3];
    qint8 yellow_blue[3];
    bool preserve_luminosity;
    quint8 lookup_table[3][256];
};

// HUE/SATURATION
// Hue/Saturation settings files are loaded and saved in Photoshop¡¯s Hue/Saturation dialog
struct KRITAPSD_EXPORT psd_layer_hue_saturation {
    quint8 hue_or_colorization; // 0 = Use settings for hue-adjustment; 1 = Use settings for colorization.
    qint8 colorization_hue; // Photoshop 5.0: The actual values are stored for the new version. Hue is - 180...180, Saturation is 0...100, and Lightness is
                            // -100...100.
    qint8 colorization_saturation; // Photoshop 4.0: Three qint8 integers Hue, Saturation, and Lightness from ¨C100...100.
    qint8 colorization_lightness; // The user interface represents hue as ¨C180...180, saturation as 0...100, and Lightness as -100...1000, as the traditional
                                  // HSB color wheel, with red = 0.
    qint8 master_hue; // Master hue, saturation and lightness values.
    qint8 master_saturation;
    qint8 master_lightness;
    qint8 range_values[6][4]; // For RGB and CMYK, those values apply to each of the six hextants in the HSB color wheel: those image pixels nearest to red,
                              // yellow, green, cyan, blue, or magenta. These numbers appear in the user interface from ¨C60...60, however the slider will
                              // reflect each of the possible 201 values from ¨C100...100.
    qint8 setting_values[6][3]; // For Lab, the first four of the six values are applied to image pixels in the four Lab color quadrants, yellow, green, blue,
                                // and magenta. The other two values are ignored ( = 0). The values appear in the user interface from ¨C90 to 90.
    quint8 lookup_table[6][360];
};

// SELECTIVE COLOR
// Selective Color settings files are loaded and saved in Photoshop¡¯s Selective Color dialog.
struct KRITAPSD_EXPORT psd_layer_selective_color {
    quint16 correction_method; // 0 = Apply color correction in relative mode; 1 = Apply color correction in absolute mode.
    qint8 cyan_correction[10]; // Amount of cyan correction. Short integer from ¨C100...100.
    qint8 magenta_correction[10]; // Amount of magenta correction. Short integer from ¨C100...100.
    qint8 yellow_correction[10]; // Amount of yellow correction. Short integer from ¨C100...100.
    qint8 black_correction[10]; // Amount of black correction. Short integer from ¨C100...100.
};

// THRESHOLD
struct KRITAPSD_EXPORT psd_layer_threshold {
    quint16 level; // (1...255)
};

// INVERT
// no parameter

// POSTERIZE
struct KRITAPSD_EXPORT psd_layer_posterize {
    quint16 levels; // (2...255)
    quint8 lookup_table[256];
};

// CHANNEL MIXER
struct KRITAPSD_EXPORT psd_layer_channel_mixer {
    bool monochrome;
    qint8 red_cyan[4]; // RGB or CMYK color plus constant for the mixer settings. 4 * 2 bytes of color with 2 bytes of constant.
    qint8 green_magenta[4]; // (-200...200)
    qint8 blue_yellow[4];
    qint8 black[4];
    qint8 constant[4];
};

// PHOTO FILTER
struct KRITAPSD_EXPORT psd_layer_photo_filter {
    qint32 x_color; // 4 bytes each for XYZ color
    qint32 y_color;
    qint32 z_color;
    qint32 density; // (1...100)
    bool preserve_luminosity;
};

#include <kis_psd_layer_style.h>

struct KRITAPSD_EXPORT psd_layer_solid_color {
    quint32 id;
    QColor fill_color;
};

struct KRITAPSD_EXPORT psd_layer_gradient_fill {
    quint32 id;
    double angle;
    psd_gradient_style style;
    qint32 scale;
    bool reverse; // Is gradient reverse
    bool dithered; // Is gradient dithered
    bool align_with_layer;
    psd_gradient_color gradient_color;
};

struct KRITAPSD_EXPORT psd_layer_pattern_fill {
    quint32 id;
    psd_pattern_info pattern_info;
    qint32 scale;
};

struct KRITAPSD_EXPORT psd_layer_type_face {
    qint8 mark; // Mark value
    qint32 font_type; // Font type data
    qint8 font_name[256]; // Pascal string of font name
    qint8 font_family_name[256]; // Pascal string of font family name
    qint8 font_style_name[256]; // Pascal string of font style name
    qint8 script; // Script value
    qint32 number_axes_vector; // Number of design axes vector to follow
    qint32 *vector; // Design vector value
};

struct KRITAPSD_EXPORT psd_layer_type_style {
    qint8 mark; // Mark value
    qint8 face_mark; // Face mark value
    qint32 size; // Size value
    qint32 tracking; // Tracking value
    qint32 kerning; // Kerning value
    qint32 leading; // Leading value
    qint32 base_shift; // Base shift value
    bool auto_kern; // Auto kern on/off
    bool rotate; // Rotate up/down
};

struct KRITAPSD_EXPORT psd_layer_type_line {
    qint32 char_count; // Character count value
    qint8 orientation; // Orientation value
    qint8 alignment; // Alignment value
    qint8 actual_char; // Actual character as a double byte character
    qint8 style; // Style value
};

struct KRITAPSD_EXPORT psd_layer_type_tool {
    double transform_info[6]; // 6 * 8 double precision numbers for the transform information
    qint8 faces_count; // Count of faces
    psd_layer_type_face *face;
    qint8 styles_count; // Count of styles
    psd_layer_type_style *style;
    qint8 type; // Type value
    qint32 scaling_factor; // Scaling factor value
    qint32 character_count; // Character count value
    qint32 horz_place; // Horizontal placement
    qint32 vert_place; // Vertical placement
    qint32 select_start; // Select start value
    qint32 select_end; // Select end value
    qint8 lines_count; // Line count
    psd_layer_type_line *line;
    QColor color;
    bool anti_alias; // Anti alias on/off
};

/**
 * @brief The PsdAdditionalLayerInfoBlock class implements the Additional Layer Information block
 *
 * See: https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/#50577409_71546
 */
class KRITAPSD_EXPORT PsdAdditionalLayerInfoBlock
{
public:
    PsdAdditionalLayerInfoBlock(const PSDHeader &header);

    typedef boost::function<bool(QIODevice *)> ExtraLayerInfoBlockHandler;

    void setExtraLayerInfoBlockHandler(ExtraLayerInfoBlockHandler handler);

    bool read(QIODevice *io);
    bool write(QIODevice *io, KisNodeSP node);

    void writeLuniBlockEx(QIODevice *io, const QString &layerName);
    void writeLsctBlockEx(QIODevice *io, psd_section_type sectionType, bool isPassThrough, const QString &blendModeKey);
    void writeLfx2BlockEx(QIODevice *io, const QDomDocument &stylesXmlDoc, bool useLfxsLayerStyleFormat);
    void writePattBlockEx(QIODevice *io, const QDomDocument &patternsXmlDoc);

    bool valid();

    const PSDHeader &m_header;
    QString error;
    QStringList keys; // List of all the keys that we've seen

    QString unicodeLayerName;
    QDomDocument layerStyleXml;
    QVector<QDomDocument> embeddedPatterns;

    psd_section_type sectionDividerType;
    QString sectionDividerBlendMode;

private:
    void readImpl(QIODevice *io);

private:
    ExtraLayerInfoBlockHandler m_layerInfoBlockHandler;
};

#endif // PSD_ADDITIONAL_LAYER_INFO_BLOCK_H

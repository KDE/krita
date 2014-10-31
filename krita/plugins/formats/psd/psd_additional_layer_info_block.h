/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef PSD_ADDITIONAL_LAYER_INFO_BLOCK_H
#define PSD_ADDITIONAL_LAYER_INFO_BLOCK_H

#include <QString>
#include <QVector>
#include <QByteArray>
#include <QBitArray>
#include <QIODevice>

#include <kis_types.h>
#include <kis_paint_device.h>
#include <kis_node.h>

#include "psd.h"
#include "psd_header.h"

enum psd_gradient_style {
    psd_gradient_style_linear,				// 'Lnr '
    psd_gradient_style_radial,				// 'Rdl '
    psd_gradient_style_angle,				// 'Angl'
    psd_gradient_style_reflected,			// 'Rflc'
    psd_gradient_style_diamond				// 'Dmnd'
};

enum psd_color_stop_type {
    psd_color_stop_type_foreground_color,	// 'FrgC'
    psd_color_stop_type_background_Color,	// 'BckC'
    psd_color_stop_type_user_stop			// 'UsrS'
};

enum psd_technique_type {
    psd_technique_softer,
    psd_technique_precise,
    psd_technique_slope_limit,
};

enum psd_stroke_position {
    psd_stroke_outside,
    psd_stroke_inside,
    psd_stroke_center
};

enum psd_fill_type {
    psd_fill_solid_color,
    psd_fill_gradient,
    psd_fill_pattern,
};

enum psd_glow_source {
    psd_glow_center,
    psd_glow_edge,
};

enum psd_bevel_style {
    psd_bevel_outer_bevel,
    psd_bevel_inner_bevel,
    psd_bevel_emboss,
    psd_bevel_pillow_emboss,
    psd_bevel_stroke_emboss,
};

enum psd_direction {
    psd_direction_up,
    psd_direction_down
};

// additional layer information

// LEVELS
// Level record
struct psd_layer_level_record {
    quint16 input_floor; // (0...253)
    quint16 input_ceiling; // (2...255)
    quint16 output_floor; // 255). Matched to input floor.
    quint16 output_ceiling; // (0...255)
    float gamma; // Short integer from 10...999 representing 0.1...9.99. Applied to all image data.
};

// Levels settings files are loaded and saved in the Levels dialog.
struct psd_layer_levels
{
    psd_layer_level_record record[29]; // 29 sets of level records, each level containing 5 qint8 integers
    // Photoshop CS (8.0) Additional information
    // At the end of the Version 2 file is the following information:
    quint16 extra_level_count; // Count of total level record structures. Subtract the legacy number of level record structures, 29, to determine how many are remaining in the file for reading.
    psd_layer_level_record *extra_record; // Additianol level records according to count
    quint8 lookup_table[3][256];
};


// CURVES
// The following is the data for each curve specified by count above
struct psd_layer_curves_data
{
    quint16 channel_index; // Before each curve is a channel index.
    quint16 point_count; // Count of points in the curve (qint8 integer from 2...19)
    quint16 output_value[19]; // All coordinates have range 0 to 255
    quint16 input_value[19];
};

// Curves file format
struct psd_layer_curves
{
    quint16 curve_count; // Count of curves in the file.
    psd_layer_curves_data * curve;
    quint8 lookup_table[3][256];
};


// BRIGHTNESS AND CONTRAST
struct psd_layer_brightness_contrast
{
    qint8 brightness;
    qint8 contrast;
    qint8 mean_value; // for brightness and contrast
    qint8 Lab_color;
    quint8 lookup_table[256];
};


// COLOR BALANCE
struct psd_layer_color_balance
{
    qint8 cyan_red[3]; // (-100...100). shadows, midtones, highlights
    qint8 magenta_green[3];
    qint8 yellow_blue[3];
    bool preserve_luminosity;
    quint8 lookup_table[3][256];
};


// HUE/SATURATION
// Hue/Saturation settings files are loaded and saved in Photoshop¡¯s Hue/Saturation dialog
struct psd_layer_hue_saturation
{
    quint8 hue_or_colorization; // 0 = Use settings for hue-adjustment; 1 = Use settings for colorization.
    qint8 colorization_hue; // Photoshop 5.0: The actual values are stored for the new version. Hue is - 180...180, Saturation is 0...100, and Lightness is -100...100.
    qint8 colorization_saturation;// Photoshop 4.0: Three qint8 integers Hue, Saturation, and Lightness from ¨C100...100.
    qint8 colorization_lightness; // The user interface represents hue as ¨C180...180, saturation as 0...100, and Lightness as -100...1000, as the traditional HSB color wheel, with red = 0.
    qint8 master_hue; // Master hue, saturation and lightness values.
    qint8 master_saturation;
    qint8 master_lightness;
    qint8 range_values[6][4]; // For RGB and CMYK, those values apply to each of the six hextants in the HSB color wheel: those image pixels nearest to red, yellow, green, cyan, blue, or magenta. These numbers appear in the user interface from ¨C60...60, however the slider will reflect each of the possible 201 values from ¨C100...100.
    qint8 setting_values[6][3]; // For Lab, the first four of the six values are applied to image pixels in the four Lab color quadrants, yellow, green, blue, and magenta. The other two values are ignored ( = 0). The values appear in the user interface from ¨C90 to 90.
    quint8 lookup_table[6][360];
};


// SELECTIVE COLOR
// Selective Color settings files are loaded and saved in Photoshop¡¯s Selective Color dialog.
struct psd_layer_selective_color
{
    quint16 correction_method; // 0 = Apply color correction in relative mode; 1 = Apply color correction in absolute mode.
    qint8 cyan_correction[10]; // Amount of cyan correction. Short integer from ¨C100...100.
    qint8 magenta_correction[10]; // Amount of magenta correction. Short integer from ¨C100...100.
    qint8 yellow_correction[10]; // Amount of yellow correction. Short integer from ¨C100...100.
    qint8 black_correction[10]; // Amount of black correction. Short integer from ¨C100...100.
};


// THRESHOLD
struct psd_layer_threshold
{
    quint16 level; // (1...255)
} ;


// INVERT
// no parameter


// POSTERIZE
struct psd_layer_posterize
{
    quint16 levels; // (2...255)
    quint8 lookup_table[256];
};


// CHANNEL MIXER
struct psd_layer_channel_mixer
{
    bool monochrome;
    qint8 red_cyan[4]; // RGB or CMYK color plus constant for the mixer settings. 4 * 2 bytes of color with 2 bytes of constant.
    qint8 green_magenta[4]; // (-200...200)
    qint8 blue_yellow[4];
    qint8 black[4];
    qint8 constant[4];
};


// GRADIENT MAP
// Each color stop
struct psd_gradient_color_stop
{
    qint32 location; // Location of color stop
    qint32 midpoint; // Midpoint of color stop
    QColor actual_color;
    psd_color_stop_type color_stop_type;
};

// Each transparency stop
struct psd_gradient_transparency_stop
{
    qint32 location; // Location of transparency stop
    qint32 midpoint; // Midpoint of transparency stop
    qint8 opacity; // Opacity of transparency stop
};

// Gradient settings (Photoshop 6.0)
struct psd_layer_gradient_map
{
    bool reverse; // Is gradient reverse
    bool dithered; // Is gradient dithered
    qint32 name_length;
    quint16 *name; // Name of the gradient: Unicode string, padded
    qint8 number_color_stops; // Number of color stops to follow
    psd_gradient_color_stop * color_stop;
    qint8 number_transparency_stops;// Number of transparency stops to follow
    psd_gradient_transparency_stop * transparency_stop;
    qint8 expansion_count; // Expansion count ( = 2 for Photoshop 6.0)
    qint8 interpolation; // Interpolation if length above is non-zero
    qint8 length; // Length (= 32 for Photoshop 6.0)
    qint8 mode; // Mode for this gradient
    qint32 random_number_seed; // Random number seed
    qint8 showing_transparency_flag;// Flag for showing transparency
    qint8 using_vector_color_flag;// Flag for using vector color
    qint32 roughness_factor; // Roughness factor
    QColor min_color;
    QColor max_color;
    QColor lookup_table[256];
};


// PHOTO FILTER
struct psd_layer_photo_filter
{
    qint32 x_color; // 4 bytes each for XYZ color
    qint32 y_color;
    qint32 z_color;
    qint32 density; // (1...100)
    bool preserve_luminosity;
};


// EFFECTS

struct psd_gradient_color {
    qint32 smoothness;
    qint32 name_length;
    quint16 * name; // Name of the gradient: Unicode string, padded
    qint8 number_color_stops; // Number of color stops to follow
    psd_gradient_color_stop * color_stop;
    qint8 number_transparency_stops;// Number of transparency stops to follow
    psd_gradient_transparency_stop *transparency_stop;
};

struct psd_pattern {
    psd_color_mode color_mode; // The image mode of the file.
    qint8 height; // Point: vertical, 2 bytes and horizontal, 2 bytes
    qint8 width;
    qint32 name_length; // Name: Unicode string
    quint16 * name;
    quint8 unique_id[256]; // Unique ID for this pattern: Pascal string
    qint32 version;
    qint8 top; // Rectangle: top, left, bottom, right
    qint8 left;
    qint8 bottom;
    qint8 right;
    qint32 max_channel; // Max channels
    qint32 channel_number;
    QColor * image_data;
};

// dsdw, isdw: http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm#50577409_22203
struct psd_layer_effects_drop_shadow {
    bool effect_enable; // Effect enabled

    QString blend_mode; // Blend mode: 4 bytes for signature and 4 bytes for key
    QColor color;
    QColor native_color;
    quint8 opacity; // Opacity as a percent
    qint32 angle; // Angle in degrees
    bool use_global_light; // Use this angle in all of the layer effects
    qint32 distance; // Distance in pixels
    qint32 spread; // Intensity as a percent
    qint32 size; // Blur value in pixels

    quint8 contour_lookup_table[256];
    bool anti_aliased;
    qint32 noise;
    bool knocks_out;
};

// isdw: http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm#50577409_22203
struct psd_layer_effects_inner_shadow {
    bool effect_enable; // Effect enabled

    QString blend_mode; // Blend mode: 4 bytes for signature and 4 bytes for key
    QColor color;
    QColor native_color;
    quint8 opacity; // Opacity as a percent
    qint32 angle; // Angle in degrees
    bool use_global_light; // Use this angle in all of the layer effects
    qint32 distance; // Distance in pixels
    qint32 choke; // Intensity as a percent
    qint32 size; // Blur value in pixels

    quint8 contour_lookup_table[256];
    bool anti_aliased;
    qint32 noise;
};

// oglw: http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm#50577409_25738
struct psd_layer_effects_outer_glow {
    bool effect_enable; // Effect enabled

    QString blend_mode; // Blend mode: 4 bytes for signature and 4 bytes for key
    quint8 opacity; // Opacity as a percent
    qint32 noise;
    psd_fill_type fill_type;
    QColor color;
    QColor native_color;
    psd_gradient_color gradient_color;

    psd_technique_type technique;
    qint32 spread;
    qint32 size;

    quint8 contour_lookup_table[256];
    bool anti_aliased;
    qint32 range;
    qint32 jitter;
};

// iglw: http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm#50577409_27692
struct psd_layer_effects_inner_glow {
    bool effect_enable; // Effect enabled

    QString blend_mode; // Blend mode: 4 bytes for signature and 4 bytes for key
    quint8 opacity; // Opacity as a percent
    qint32 noise;
    psd_fill_type fill_type;
    QColor color;
    QColor native_color;
    psd_gradient_color gradient_color;

    psd_technique_type technique;
    psd_glow_source source;
    qint32 choke;
    qint32 size;

    quint8 contour_lookup_table[256];
    bool anti_aliased;
    qint32 range;
    qint32 jitter;
};

struct psd_pattern_info {
    qint32 name_length;
    quint16 * name;
    quint8 identifier[256];
};

// bevl: http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm#50577409_31889
struct psd_layer_effects_bevel_emboss {
    bool effect_enable; // Effect enabled

    psd_bevel_style style; // Bevel style
    psd_technique_type technique;
    qint32 depth;
    psd_direction direction; // Up or down
    qint32 size; // Strength. Depth in pixels
    qint32 soften; // Blur value in pixels.

    qint32 angle; // Angle in degrees
    bool use_global_light; // Use this angle in all of the layer effects
    qint32 altitude;
    quint8 gloss_contour_lookup_table[256];
    bool gloss_anti_aliased;
    QString highlight_blend_mode; // Highlight blend mode: 4 bytes for signature and 4 bytes for the key
    QColor highlight_color;
    QColor real_highlight_color;
    quint8 highlight_opacity; // Hightlight opacity as a percent
    QString shadow_blend_mode; // Shadow blend mode: 4 bytes for signature and 4 bytes for the key
    QColor shadow_color;
    QColor real_shadow_color;
    quint8 shadow_opacity; // Shadow opacity as a percent

    bool contour_enable;
    quint8 contour_lookup_table[256];
    bool contour_anti_aliased;
    qint32 contour_range;

    bool texture_enable;
    psd_pattern_info texture_pattern_info;
    qint32 texture_scale;
    qint32 texture_depth;
    bool texture_invert;
    bool texture_link;
    qint32 texture_horz_phase;
    qint32 texture_vert_phase;
};

struct psd_layer_effects_satin {
    bool effect_enable; // Effect enabled

    QString blend_mode; // Blend mode: 4 bytes for signature and 4 bytes for key
    QColor color;
    quint8 opacity; // Opacity as a percent
    qint32 angle; // Angle in degrees
    qint32 distance;
    qint32 size;
    quint8 contour_lookup_table[256];
    bool anti_aliased;
    bool invert;
};

// sofi: http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm#50577409_70055
struct psd_layer_effects_color_overlay {
    // Effects layer, solid fill (added in Photoshop 7.0)
    bool effect_enable; // Effect enabled

    QString blend_mode; // Key for blend mode
    QColor color;
    quint8 opacity; // Opacity as a percent
    QColor native_color;
};

struct psd_layer_effects_gradient_overlay {
    bool effect_enable; // Effect enabled

    QString blend_mode; // Blend mode: 4 bytes for signature and 4 bytes for key
    quint8 opacity; // Opacity as a percent
    psd_gradient_color gradient_color;
    bool reverse;
    psd_gradient_style style;
    bool align_width_layer;
    qint32 angle; // Angle in degrees
    qint32 scale;
    qint32 horz_offset;
    qint32 vert_offset;
};

struct psd_layer_effects_pattern_overlay {
    bool effect_enable; // Effect enabled

    QString blend_mode; // Blend mode: 4 bytes for signature and 4 bytes for key
    QColor color;
    quint8 opacity; // Opacity as a percent
    qint32 scale;
    bool link_with_layer;
    psd_pattern_info pattern_info;
    qint32 horz_phase;
    qint32 vert_phase;
};


struct psd_layer_effects_stroke {
    bool effect_enable; // Effect enabled

    qint32 size;
    psd_stroke_position position;
    QString blend_mode; // Blend mode: 4 bytes for signature and 4 bytes for key
    quint8 opacity; // Opacity as a percent
    psd_fill_type fill_type;

    QColor fill_color;

    psd_gradient_color gradient_color;
    bool gradient_reverse;
    psd_gradient_style gradient_style;
    bool gradient_align;
    qint32 gradient_angle;
    qint32 gradient_scale;
    qint32 gradient_horz_offset;
    qint32 gradient_vert_offset;

    psd_pattern_info pattern_info;
    qint32 pattern_scale;
    bool pattern_link;
    qint32 pattern_horz_phase;
    qint32 pattern_vert_phase;
};

struct psd_layer_effects {
    qint8 effects_count; // Effects count: may be 6 (for the 6 effects in Photoshop 5 and 6) or 7 (for Photoshop 7.0)
    bool visible; // common state info, visible: always true
    psd_layer_effects_drop_shadow drop_shadow;
    psd_layer_effects_inner_shadow inner_shadow;
    psd_layer_effects_outer_glow outer_glow;
    psd_layer_effects_inner_glow inner_glow;
    psd_layer_effects_bevel_emboss bevel_emboss;
    psd_layer_effects_satin satin;
    psd_layer_effects_color_overlay color_overlay;
    psd_layer_effects_gradient_overlay gradient_overlay;
    psd_layer_effects_pattern_overlay pattern_overlay;
    psd_layer_effects_stroke stroke;

    QVector<bool> fill;
    QVector<bool> valid;
    QVector<QString> blend_mode;
    QVector<quint8> opacity;
    QVector<QColor> * image_data;
    QVector<qint32> left;
    QVector<qint32> top;
    QVector<qint32> right;
    QVector<qint32> bottom;
    QVector<qint32> width;
    QVector<qint32> height;
};

struct psd_layer_solid_color {
    quint32 id;
    QColor fill_color;
};

struct psd_layer_gradient_fill {
    quint32 id;
    double angle;
    psd_gradient_style style;
    qint32 scale;
    bool reverse; // Is gradient reverse
    bool dithered; // Is gradient dithered
    bool align_with_layer;
    psd_gradient_color gradient_color;
};

struct psd_layer_pattern_fill {
    quint32 id;
    psd_pattern_info pattern_info;
    qint32 scale;
};

struct psd_layer_type_face {
    qint8 mark; // Mark value
    qint32 font_type; // Font type data
    qint8 font_name[256]; // Pascal string of font name
    qint8 font_family_name[256]; // Pascal string of font family name
    qint8 font_style_name[256]; // Pascal string of font style name
    qint8 script; // Script value
    qint32 number_axes_vector; // Number of design axes vector to follow
    qint32 * vector; // Design vector value
};

struct psd_layer_type_style {
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

struct psd_layer_type_line {
    qint32 char_count; // Character count value
    qint8 orientation; // Orientation value
    qint8 alignment; // Alignment value
    qint8 actual_char; // Actual character as a double byte character
    qint8 style; // Style value
};

struct psd_layer_type_tool {
    double transform_info[6]; // 6 * 8 double precision numbers for the transform information
    qint8 faces_count; // Count of faces
    psd_layer_type_face * face;
    qint8 styles_count; // Count of styles
    psd_layer_type_style * style;
    qint8 type; // Type value
    qint32 scaling_factor; // Scaling factor value
    qint32 character_count; // Character count value
    qint32 horz_place; // Horizontal placement
    qint32 vert_place; // Vertical placement
    qint32 select_start; // Select start value
    qint32 select_end; // Select end value
    qint8 lines_count; // Line count
    psd_layer_type_line * line;
    QColor color;
    bool anti_alias; // Anti alias on/off
};

/**
 * @brief The PsdAdditionalLayerInfoBlock class implements the Additional Layer Information block
 *
 * See: http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/#50577409_71546
 */
class PsdAdditionalLayerInfoBlock
{
public:
    PsdAdditionalLayerInfoBlock();

    bool read(QIODevice* io);
    bool write(QIODevice* io, KisNodeSP node);

    bool valid();

    QString error;


    QString key;

};

#endif // PSD_ADDITIONAL_LAYER_INFO_BLOCK_H

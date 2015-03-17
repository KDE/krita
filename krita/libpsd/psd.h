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
#include <QColor>
#include <QVector>

#include <KoColorModelStandardIds.h>
#include <KoCompositeOpRegistry.h>

#include "libkispsd_export.h"

#include "kis_debug.h"



const int MAX_CHANNELS = 56;

typedef qint32  Fixed;                  /* Represents a fixed point implied decimal */

/**
 * Image color/depth modes
 */
enum psd_color_mode {
    Bitmap = 0,
    Grayscale=1,
    Indexed=2,
    RGB=3,
    CMYK=4,
    MultiChannel=7,
    DuoTone=8,
    Lab=9,
    Gray16,
    RGB48,
    Lab48,
    CMYK64,
    DeepMultichannel,
    Duotone16,
    UNKNOWN = 9000
 };


/**
 * Color samplers, apparently distict from PSDColormode
 */
namespace psd_color_sampler {
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
}

// EFFECTS
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

enum psd_section_type {
    psd_other = 0,
    psd_open_folder,
    psd_closed_folder,
    psd_bounding_divider
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
    quint8 height; // Point: vertical, 2 bytes and horizontal, 2 bytes
    quint8 width;
    QString name;
    QString uuid;
    qint32 version;
    quint8 top; // Rectangle: top, left, bottom, right
    quint8 left;
    quint8 bottom;
    quint8 right;
    qint32 max_channel; // Max channels
    qint32 channel_number;
    QVector<QRgb> color_table;
};

struct psd_layer_effects_context {
    psd_layer_effects_context()
        : global_angle(120),
          keep_original(false)
    {
    }

    qint32 global_angle;
    bool keep_original;
};

#define PSD_LOOKUP_TABLE_SIZE 256

class KoAbstractGradient;

// dsdw, isdw: http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm#50577409_22203
class LIBKISPSD_EXPORT psd_layer_effects_shadow_base {
public:
    psd_layer_effects_shadow_base()
        : m_invertsSelection(false)
        , m_edgeHidden(true)
        , m_effectEnabled(false)
        , m_blendMode(COMPOSITE_MULT)
        , m_color(Qt::black)
        , m_nativeColor(Qt::black)
        , m_opacity(75)
        , m_angle(120)
        , m_useGlobalLight(true)
        , m_distance(21)
        , m_spread(0)
        , m_size(21)
        , m_antiAliased(0)
        , m_noise(0)
        , m_knocksOut(false)
        , m_fillType(psd_fill_solid_color)
        , m_technique(psd_technique_softer)
        , m_range(100)
        , m_jitter(0)
        , m_gradient(0)
    {
        for(int i = 0; i < PSD_LOOKUP_TABLE_SIZE; ++i) {
            m_contourLookupTable[i] = i;
        }
    }

    virtual ~psd_layer_effects_shadow_base() {
    }

    QPoint calculateOffset(const psd_layer_effects_context *context) const;

    void setEffectEnabled(bool value) {
        m_effectEnabled = value;
    }

    bool effectEnabled() const {
        return m_effectEnabled;
    }

    QString blendMode() const {
        return m_blendMode;
    }

    QColor color() const {
        return m_color;
    }

    QColor nativeColor() const {
        return m_nativeColor;
    }

    quint8 opacity() const {
        return m_opacity;
    }

    qint32 angle() const {
        return m_angle;
    }

    bool useGlobalLight() const {
        return m_useGlobalLight;
    }

    qint32 distance() const {
        return m_distance;
    }

    qint32 spread() const {
        return m_spread;
    }

    qint32 size() const {
        return m_size;
    }

    const quint8* contourLookupTable() const {
        return m_contourLookupTable;
    }

    bool antiAliased() const {
        return m_antiAliased;
    }

    qint32 noise() const {
        return m_noise;
    }

    bool knocksOut() const {
        return m_knocksOut;
    }

    bool invertsSelection() const {
        return m_invertsSelection;
    }

    bool edgeHidden() const {
        return m_edgeHidden;
    }

    psd_fill_type fillType() const {
        return m_fillType;
    }

    psd_technique_type technique() const {
        return m_technique;
    }

    qint32 range() const {
        return m_range;
    }

    qint32 jitter() const {
        return m_jitter;
    }

    KoAbstractGradient* gradient() const {
        return m_gradient;
    }

protected:
    /**
     * All the setters are protected by default.  Should be made
     * public by descendants manually when needed.
     */

    void setBlendMode(QString value) {
        m_blendMode = value;
    }

    void setColor(QColor value) {
        m_color = value;
    }

    void setNativeColor(QColor value) {
        m_nativeColor = value;
    }

    void setOpacity(quint8 value) {
        m_opacity = value;
    }

    void setAngle(qint32 value) {
        m_angle = value;
    }

    void setUseGlobalLight(bool value) {
        m_useGlobalLight = value;
    }

    void setDistance(qint32 value) {
        m_distance = value;
    }

    void setSpread(qint32 value) {
        m_spread = value;
    }

    void setSize(qint32 value) {
        m_size = value;
    }

    void setContourLookupTable(quint8* value) {
        memcpy(m_contourLookupTable, value, PSD_LOOKUP_TABLE_SIZE * sizeof(quint8));
    }

    void setAntiAliased(bool value) {
        m_antiAliased = value;
    }

    void setNoise(qint32 value) {
        m_noise = value;
    }

    void setKnocksOut(bool value) {
        m_knocksOut = value;
    }

    void setInvertsSelection(bool value) {
        m_invertsSelection = value;
    }

    void setEdgeHidden(bool value) {
        m_edgeHidden = value;
    }

    void setFillType(psd_fill_type value) {
        m_fillType = value;
    }

    void setTechnique(psd_technique_type value) {
        m_technique = value;
    }

    void setRange(qint32 value) {
        m_range = value;
    }

    void setJitter(qint32 value) {
        m_jitter = value;
    }

    void setGradient(KoAbstractGradient* value) {
        m_gradient = value;
    }

private:
    // internal
    bool m_invertsSelection;
    bool m_edgeHidden;

private:

    bool m_effectEnabled; // Effect enabled

    QString m_blendMode; // already in Krita format!
    QColor m_color;
    QColor m_nativeColor;
    quint8 m_opacity; // Opacity as a percent (0...100)
    qint32 m_angle; // Angle in degrees
    bool m_useGlobalLight; // Use this angle in all of the layer effects
    qint32 m_distance; // Distance in pixels
    qint32 m_spread; // Intensity as a percent
    qint32 m_size; // Blur value in pixels

    quint8 m_contourLookupTable[PSD_LOOKUP_TABLE_SIZE];
    bool m_antiAliased;
    qint32 m_noise;
    bool m_knocksOut;

    // for Outer/Inner Glow
    psd_fill_type m_fillType;
    psd_technique_type m_technique;
    qint32 m_range;
    qint32 m_jitter;
    KoAbstractGradient *m_gradient;
};

class LIBKISPSD_EXPORT psd_layer_effects_shadow_common : public psd_layer_effects_shadow_base
{
public:
    using psd_layer_effects_shadow_base::setBlendMode;
    using psd_layer_effects_shadow_base::setColor;
    using psd_layer_effects_shadow_base::setOpacity;
    using psd_layer_effects_shadow_base::setAngle;
    using psd_layer_effects_shadow_base::setUseGlobalLight;
    using psd_layer_effects_shadow_base::setDistance;
    using psd_layer_effects_shadow_base::setSpread;
    using psd_layer_effects_shadow_base::setSize;
    using psd_layer_effects_shadow_base::setContourLookupTable;
    using psd_layer_effects_shadow_base::setAntiAliased;
    using psd_layer_effects_shadow_base::setNoise;
};

class LIBKISPSD_EXPORT psd_layer_effects_drop_shadow : public psd_layer_effects_shadow_common
{
public:
    using psd_layer_effects_shadow_base::setKnocksOut;
};

// isdw: http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm#50577409_22203
class LIBKISPSD_EXPORT psd_layer_effects_inner_shadow : public psd_layer_effects_shadow_common
{
public:
    psd_layer_effects_inner_shadow() {
        setKnocksOut(true);
        setInvertsSelection(true);
        setEdgeHidden(false);
    }
};

class LIBKISPSD_EXPORT psd_layer_effects_glow_common : public psd_layer_effects_shadow_base
{
public:
    psd_layer_effects_glow_common() {
        setKnocksOut(true);
        setDistance(0);
        setBlendMode(COMPOSITE_LINEAR_DODGE);
    }

    using psd_layer_effects_shadow_base::setBlendMode;
    using psd_layer_effects_shadow_base::setColor;
    using psd_layer_effects_shadow_base::setOpacity;

    using psd_layer_effects_shadow_base::setSpread;
    using psd_layer_effects_shadow_base::setSize;
    using psd_layer_effects_shadow_base::setContourLookupTable;
    using psd_layer_effects_shadow_base::setAntiAliased;
    using psd_layer_effects_shadow_base::setNoise;

    using psd_layer_effects_shadow_base::setFillType;
    using psd_layer_effects_shadow_base::setTechnique;
    using psd_layer_effects_shadow_base::setRange;
    using psd_layer_effects_shadow_base::setJitter;
    using psd_layer_effects_shadow_base::setGradient;
};

// oglw: http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm#50577409_25738
class LIBKISPSD_EXPORT psd_layer_effects_outer_glow : public psd_layer_effects_glow_common
{
};

// iglw: http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm#50577409_27692
class LIBKISPSD_EXPORT psd_layer_effects_inner_glow : public psd_layer_effects_glow_common
{
public:
    psd_layer_effects_inner_glow() {
        setInvertsSelection(true);
        setEdgeHidden(false);
    }
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


/**
 * Convert PsdColorMode to pigment colormodelid and colordepthid.
 * @see KoColorModelStandardIds
 *
 * @return a QPair containing ColorModelId and ColorDepthID
 */
QPair<QString, QString> LIBKISPSD_EXPORT psd_colormode_to_colormodelid(psd_color_mode colormode, quint16 channelDepth);


/**
 * Convert the Photoshop blend mode strings to Pigment compositeop id's
 */
QString LIBKISPSD_EXPORT psd_blendmode_to_composite_op(const QString& blendmode);
QString LIBKISPSD_EXPORT composite_op_to_psd_blendmode(const QString& compositeOp);

#endif // PSD_H

/*
 *  SPDX-FileCopyrightText: 2010 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
/*
 * Constants and defines taken from gimp and psdparse
 */
#ifndef PSD_H
#define PSD_H

#include "kritapsdutils_export.h"

#include <QColor>
#include <QPair>
#include <QString>
#include <QVector>
#include <cstdint>

#include <KisLinkedResourceWrapper.h>
#include <KoColorModelStandardIds.h>
#include <KoCompositeOpRegistry.h>
#include <KoPattern.h>
#include <resources/KoAbstractGradient.h>

const int MAX_CHANNELS = 56;

typedef qint32 Fixed; /* Represents a fixed point implied decimal */

enum class psd_byte_order : std::uint_least8_t {
    psdBigEndian = 0,
    psdLittleEndian,
    psdInvalidByteOrder = 255,
};

/**
 * Image color/depth modes
 */
enum psd_color_mode {
    Bitmap = 0,
    Grayscale = 1,
    Indexed = 2,
    RGB = 3,
    CMYK = 4,
    MultiChannel = 7,
    DuoTone = 8,
    Lab = 9,
    Gray16,
    RGB48,
    Lab48,
    CMYK64,
    DeepMultichannel,
    Duotone16,
    COLORMODE_UNKNOWN = 9000
};

/**
 * Color samplers, apparently distict from PSDColormode
 */
namespace psd_color_sampler
{
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
    ANPA = 3000 // LAB
};
}

// EFFECTS
enum psd_gradient_style {
    psd_gradient_style_linear, // 'Lnr '
    psd_gradient_style_radial, // 'Rdl '
    psd_gradient_style_angle, // 'Angl'
    psd_gradient_style_reflected, // 'Rflc'
    psd_gradient_style_diamond // 'Dmnd'
};

enum psd_color_stop_type {
    psd_color_stop_type_foreground_color, // 'FrgC'
    psd_color_stop_type_background_Color, // 'BckC'
    psd_color_stop_type_user_stop // 'UsrS'
};

enum psd_technique_type {
    psd_technique_softer,
    psd_technique_precise,
    psd_technique_slope_limit,
};

enum psd_stroke_position { psd_stroke_outside, psd_stroke_inside, psd_stroke_center };

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

enum psd_direction { psd_direction_up, psd_direction_down };

enum psd_section_type { psd_other = 0, psd_open_folder, psd_closed_folder, psd_bounding_divider };

// GRADIENT MAP
// Each color stop
struct psd_gradient_color_stop {
    qint32 location; // Location of color stop
    qint32 midpoint; // Midpoint of color stop
    QColor actual_color;
    psd_color_stop_type color_stop_type;
};

// Each transparency stop
struct psd_gradient_transparency_stop {
    qint32 location; // Location of transparency stop
    qint32 midpoint; // Midpoint of transparency stop
    qint8 opacity; // Opacity of transparency stop
};

// Gradient settings (Photoshop 6.0)
struct psd_layer_gradient_map {
    bool reverse; // Is gradient reverse
    bool dithered; // Is gradient dithered
    qint32 name_length;
    quint16 *name; // Name of the gradient: Unicode string, padded
    qint8 number_color_stops; // Number of color stops to follow
    psd_gradient_color_stop *color_stop;
    qint8 number_transparency_stops; // Number of transparency stops to follow
    psd_gradient_transparency_stop *transparency_stop;
    qint8 expansion_count; // Expansion count ( = 2 for Photoshop 6.0)
    qint8 interpolation; // Interpolation if length above is non-zero
    qint8 length; // Length (= 32 for Photoshop 6.0)
    qint8 mode; // Mode for this gradient
    qint32 random_number_seed; // Random number seed
    qint8 showing_transparency_flag; // Flag for showing transparency
    qint8 using_vector_color_flag; // Flag for using vector color
    qint32 roughness_factor; // Roughness factor
    QColor min_color;
    QColor max_color;
    QColor lookup_table[256];
};

struct psd_gradient_color {
    qint32 smoothness;
    qint32 name_length;
    quint16 *name; // Name of the gradient: Unicode string, padded
    qint8 number_color_stops; // Number of color stops to follow
    psd_gradient_color_stop *color_stop;
    qint8 number_transparency_stops; // Number of transparency stops to follow
    psd_gradient_transparency_stop *transparency_stop;
};

struct psd_pattern {
    psd_color_mode color_mode = Bitmap; // The image mode of the file.
    quint8 height = 0; // Point: vertical, 2 bytes and horizontal, 2 bytes
    quint8 width = 0;
    QString name;
    QString uuid;
    qint32 version = 0;
    quint8 top = 0; // Rectangle: top, left, bottom, right
    quint8 left = 0;
    quint8 bottom = 0;
    quint8 right = 0;
    qint32 max_channel = 0; // Max channels
    qint32 channel_number = 0;
    QVector<QRgb> color_table;
};

struct psd_layer_effects_context {
    psd_layer_effects_context()
        : keep_original(false)
    {
    }

    bool keep_original;
};

#define PSD_LOOKUP_TABLE_SIZE 256

// dsdw, isdw: https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm#50577409_22203
class KRITAPSDUTILS_EXPORT psd_layer_effects_shadow_base
{
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
    {
        for (int i = 0; i < PSD_LOOKUP_TABLE_SIZE; ++i) {
            m_contourLookupTable[i] = i;
        }
    }

    virtual ~psd_layer_effects_shadow_base();

    QPoint calculateOffset(const psd_layer_effects_context *context) const;

    void setEffectEnabled(bool value)
    {
        m_effectEnabled = value;
    }

    bool effectEnabled() const
    {
        return m_effectEnabled;
    }

    QString blendMode() const
    {
        return m_blendMode;
    }

    QColor color() const
    {
        return m_color;
    }

    QColor nativeColor() const
    {
        return m_nativeColor;
    }

    qint32 opacity() const
    {
        return m_opacity;
    }

    qint32 angle() const
    {
        return m_angle;
    }

    bool useGlobalLight() const
    {
        return m_useGlobalLight;
    }

    qint32 distance() const
    {
        return m_distance;
    }

    qint32 spread() const
    {
        return m_spread;
    }

    qint32 size() const
    {
        return m_size;
    }

    const quint8 *contourLookupTable() const
    {
        return m_contourLookupTable;
    }

    bool antiAliased() const
    {
        return m_antiAliased;
    }

    qint32 noise() const
    {
        return m_noise;
    }

    bool knocksOut() const
    {
        return m_knocksOut;
    }

    bool invertsSelection() const
    {
        return m_invertsSelection;
    }

    bool edgeHidden() const
    {
        return m_edgeHidden;
    }

    psd_fill_type fillType() const
    {
        return m_fillType;
    }

    psd_technique_type technique() const
    {
        return m_technique;
    }

    qint32 range() const
    {
        return m_range;
    }

    qint32 jitter() const
    {
        return m_jitter;
    }

    KoAbstractGradientSP gradient(KisResourcesInterfaceSP resourcesInterface) const
    {
        return m_gradientWrapper.isValid() ? m_gradientWrapper.resource(resourcesInterface) : KoAbstractGradientSP();
    }

public:
    void setBlendMode(QString value)
    {
        m_blendMode = value;
    }

    void setColor(QColor value)
    {
        m_color = value;
    }

    void setNativeColor(QColor value)
    {
        m_nativeColor = value;
    }

    void setOpacity(qint32 value)
    {
        m_opacity = value;
    }

    void setAngle(qint32 value)
    {
        m_angle = value;
    }

    void setUseGlobalLight(bool value)
    {
        m_useGlobalLight = value;
    }

    void setDistance(qint32 value)
    {
        m_distance = value;
    }

    void setSpread(qint32 value)
    {
        m_spread = value;
    }

    void setSize(qint32 value)
    {
        m_size = value;
    }

    void setContourLookupTable(const quint8 *value)
    {
        memcpy(m_contourLookupTable, value, PSD_LOOKUP_TABLE_SIZE * sizeof(quint8));
    }

    void setAntiAliased(bool value)
    {
        m_antiAliased = value;
    }

    void setNoise(qint32 value)
    {
        m_noise = value;
    }

    void setKnocksOut(bool value)
    {
        m_knocksOut = value;
    }

    void setInvertsSelection(bool value)
    {
        m_invertsSelection = value;
    }

    void setEdgeHidden(bool value)
    {
        m_edgeHidden = value;
    }

    void setFillType(psd_fill_type value)
    {
        m_fillType = value;
    }

    void setTechnique(psd_technique_type value)
    {
        m_technique = value;
    }

    void setRange(qint32 value)
    {
        m_range = value;
    }

    void setJitter(qint32 value)
    {
        m_jitter = value;
    }

    void setGradient(KoAbstractGradientSP value)
    {
        m_gradientWrapper = value;
    }

    virtual void scaleLinearSizes(qreal scale);

private:
    // internal
    bool m_invertsSelection;
    bool m_edgeHidden;

private:
    bool m_effectEnabled; // Effect enabled

    QString m_blendMode; // already in Krita format!
    QColor m_color;
    QColor m_nativeColor;
    qint32 m_opacity; // Opacity as a percent (0...100)
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
    KisLinkedResourceWrapper<KoAbstractGradient> m_gradientWrapper;
};

class KRITAPSDUTILS_EXPORT psd_layer_effects_shadow_common : public psd_layer_effects_shadow_base
{
public:
    /// FIXME: 'using' is not supported by MSVC, so please refactor in
    ///        some other way to ensure that the setters are not used
    ///        in the classes we don't want

    // using psd_layer_effects_shadow_base::setBlendMode;
    // using psd_layer_effects_shadow_base::setColor;
    // using psd_layer_effects_shadow_base::setOpacity;
    // using psd_layer_effects_shadow_base::setAngle;
    // using psd_layer_effects_shadow_base::setUseGlobalLight;
    // using psd_layer_effects_shadow_base::setDistance;
    // using psd_layer_effects_shadow_base::setSpread;
    // using psd_layer_effects_shadow_base::setSize;
    // using psd_layer_effects_shadow_base::setContourLookupTable;
    // using psd_layer_effects_shadow_base::setAntiAliased;
    // using psd_layer_effects_shadow_base::setNoise;

    ~psd_layer_effects_shadow_common() override;
};

class KRITAPSDUTILS_EXPORT psd_layer_effects_drop_shadow : public psd_layer_effects_shadow_common
{
public:
    /// FIXME: 'using' is not supported by MSVC, so please refactor in
    ///        some other way to ensure that the setters are not used
    ///        in the classes we don't want

    // using psd_layer_effects_shadow_base::setKnocksOut;
    ~psd_layer_effects_drop_shadow();
};

// isdw: https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm#50577409_22203
class KRITAPSDUTILS_EXPORT psd_layer_effects_inner_shadow : public psd_layer_effects_shadow_common
{
public:
    psd_layer_effects_inner_shadow()
    {
        setKnocksOut(false);
        setInvertsSelection(true);
        setEdgeHidden(false);
    }

    ~psd_layer_effects_inner_shadow() override;
};

class KRITAPSDUTILS_EXPORT psd_layer_effects_glow_common : public psd_layer_effects_shadow_base
{
public:
    psd_layer_effects_glow_common()
    {
        setKnocksOut(true);
        setDistance(0);
        setBlendMode(COMPOSITE_LINEAR_DODGE);
        setColor(Qt::white);
    }
    ~psd_layer_effects_glow_common() override;
    /// FIXME: 'using' is not supported by MSVC, so please refactor in
    ///        some other way to ensure that the setters are not used
    ///        in the classes we don't want

    // using psd_layer_effects_shadow_base::setBlendMode;
    // using psd_layer_effects_shadow_base::setColor;
    // using psd_layer_effects_shadow_base::setOpacity;

    // using psd_layer_effects_shadow_base::setSpread;
    // using psd_layer_effects_shadow_base::setSize;
    // using psd_layer_effects_shadow_base::setContourLookupTable;
    // using psd_layer_effects_shadow_base::setAntiAliased;
    // using psd_layer_effects_shadow_base::setNoise;

    // using psd_layer_effects_shadow_base::setFillType;
    // using psd_layer_effects_shadow_base::setTechnique;
    // using psd_layer_effects_shadow_base::setRange;
    // using psd_layer_effects_shadow_base::setJitter;
    // using psd_layer_effects_shadow_base::setGradient;
};

// oglw: https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm#50577409_25738
class KRITAPSDUTILS_EXPORT psd_layer_effects_outer_glow : public psd_layer_effects_glow_common
{
public:
    ~psd_layer_effects_outer_glow() override;
};

// iglw: https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm#50577409_27692
class KRITAPSDUTILS_EXPORT psd_layer_effects_inner_glow : public psd_layer_effects_glow_common
{
public:
    psd_layer_effects_inner_glow()
        : m_source(psd_glow_edge)
    {
        setInvertsSelection(true);
        setEdgeHidden(false);
        setKnocksOut(false);
    }

    psd_glow_source source() const
    {
        return m_source;
    }
    void setSource(psd_glow_source value)
    {
        m_source = value;
    }
    ~psd_layer_effects_inner_glow() override;

private:
    psd_glow_source m_source;
};

struct psd_layer_effects_satin : public psd_layer_effects_shadow_base {
    psd_layer_effects_satin()
    {
        setInvert(false);
        setUseGlobalLight(false);
        setDistance(8);
        setSize(7);
        setSpread(0);
        setKnocksOut(true);
        setEdgeHidden(false);
        setBlendMode(COMPOSITE_LINEAR_BURN);
    }

    /// FIXME: 'using' is not supported by MSVC, so please refactor in
    ///        some other way to ensure that the setters are not used
    ///        in the classes we don't want

    // using psd_layer_effects_shadow_base::setBlendMode;
    // using psd_layer_effects_shadow_base::setColor;
    // using psd_layer_effects_shadow_base::setOpacity;

    // // NOTE: no global light setting explicitly!
    // using psd_layer_effects_shadow_base::setAngle;
    // using psd_layer_effects_shadow_base::setDistance;

    // using psd_layer_effects_shadow_base::setSize;

    // using psd_layer_effects_shadow_base::setContourLookupTable;
    // using psd_layer_effects_shadow_base::setAntiAliased;

    bool invert() const
    {
        return m_invert;
    }

    void setInvert(bool value)
    {
        m_invert = value;
    }

private:
    bool m_invert;
};

struct psd_pattern_info {
    qint32 name_length;
    quint16 *name;
    quint8 identifier[256];
};

// bevl: https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm#50577409_31889
struct psd_layer_effects_bevel_emboss : public psd_layer_effects_shadow_base {
    psd_layer_effects_bevel_emboss()
        : m_style(psd_bevel_inner_bevel)
        , m_technique(psd_technique_softer)
        , m_depth(100)
        , m_direction(psd_direction_up)
        , m_soften(0)
        ,

        m_altitude(30)
        ,

        m_glossAntiAliased(false)
        ,

        m_highlightBlendMode(COMPOSITE_SCREEN)
        , m_highlightColor(Qt::white)
        , m_highlightOpacity(75)
        ,

        m_shadowBlendMode(COMPOSITE_MULT)
        , m_shadowColor(Qt::black)
        , m_shadowOpacity(75)
        ,

        m_contourEnabled(false)
        , m_contourRange(100)
        ,

        m_textureEnabled(false)
        , m_textureScale(100)
        , m_textureDepth(100)
        , m_textureInvert(false)
        ,

        m_textureAlignWithLayer(true)
        , m_textureHorizontalPhase(0)
        , m_textureVerticalPhase(0)
    {
        for (int i = 0; i < PSD_LOOKUP_TABLE_SIZE; ++i) {
            m_glossContourLookupTable[i] = i;
        }
    }

    /// FIXME: 'using' is not supported by MSVC, so please refactor in
    ///        some other way to ensure that the setters are not used
    ///        in the classes we don't want

    // using psd_layer_effects_shadow_base::setSize;

    // using psd_layer_effects_shadow_base::setAngle;
    // using psd_layer_effects_shadow_base::setUseGlobalLight;

    // using psd_layer_effects_shadow_base::setContourLookupTable;
    // using psd_layer_effects_shadow_base::setAntiAliased;

    psd_bevel_style style() const
    {
        return m_style;
    }
    void setStyle(psd_bevel_style value)
    {
        m_style = value;
    }

    psd_technique_type technique() const
    {
        return m_technique;
    }
    void setTechnique(psd_technique_type value)
    {
        m_technique = value;
    }

    int depth() const
    {
        return m_depth;
    }
    void setDepth(int value)
    {
        m_depth = value;
    }

    psd_direction direction() const
    {
        return m_direction;
    }
    void setDirection(psd_direction value)
    {
        m_direction = value;
    }

    int soften() const
    {
        return m_soften;
    }
    void setSoften(int value)
    {
        m_soften = value;
    }

    int altitude() const
    {
        return m_altitude;
    }
    void setAltitude(int value)
    {
        m_altitude = value;
    }

    const quint8 *glossContourLookupTable() const
    {
        return m_glossContourLookupTable;
    }

    void setGlossContourLookupTable(const quint8 *value)
    {
        memcpy(m_glossContourLookupTable, value, PSD_LOOKUP_TABLE_SIZE * sizeof(quint8));
    }

    bool glossAntiAliased() const
    {
        return m_glossAntiAliased;
    }
    void setGlossAntiAliased(bool value)
    {
        m_glossAntiAliased = value;
    }

    QString highlightBlendMode() const
    {
        return m_highlightBlendMode;
    }
    void setHighlightBlendMode(QString value)
    {
        m_highlightBlendMode = value;
    }

    QColor highlightColor() const
    {
        return m_highlightColor;
    }
    void setHighlightColor(QColor value)
    {
        m_highlightColor = value;
    }

    qint32 highlightOpacity() const
    {
        return m_highlightOpacity;
    }
    void setHighlightOpacity(qint32 value)
    {
        m_highlightOpacity = value;
    }

    QString shadowBlendMode() const
    {
        return m_shadowBlendMode;
    }
    void setShadowBlendMode(QString value)
    {
        m_shadowBlendMode = value;
    }

    QColor shadowColor() const
    {
        return m_shadowColor;
    }
    void setShadowColor(QColor value)
    {
        m_shadowColor = value;
    }

    qint32 shadowOpacity() const
    {
        return m_shadowOpacity;
    }
    void setShadowOpacity(qint32 value)
    {
        m_shadowOpacity = value;
    }

    bool contourEnabled() const
    {
        return m_contourEnabled;
    }
    void setContourEnabled(bool value)
    {
        m_contourEnabled = value;
    }

    int contourRange() const
    {
        return m_contourRange;
    }
    void setContourRange(int value)
    {
        m_contourRange = value;
    }

    bool textureEnabled() const
    {
        return m_textureEnabled;
    }
    void setTextureEnabled(bool value)
    {
        m_textureEnabled = value;
    }

    KoPatternSP texturePattern(KisResourcesInterfaceSP interface) const
    {
        return m_texturePatternLink.isValid() ? m_texturePatternLink.resource(interface) : KoPatternSP();
    }

    void setTexturePattern(KoPatternSP value)
    {
        m_texturePatternLink = value;
    }

    int textureScale() const
    {
        return m_textureScale;
    }
    void setTextureScale(int value)
    {
        m_textureScale = value;
    }

    int textureDepth() const
    {
        return m_textureDepth;
    }
    void setTextureDepth(int value)
    {
        m_textureDepth = value;
    }

    bool textureInvert() const
    {
        return m_textureInvert;
    }
    void setTextureInvert(bool value)
    {
        m_textureInvert = value;
    }

    bool textureAlignWithLayer() const
    {
        return m_textureAlignWithLayer;
    }
    void setTextureAlignWithLayer(bool value)
    {
        m_textureAlignWithLayer = value;
    }

    void setTexturePhase(const QPointF &phase)
    {
        m_textureHorizontalPhase = phase.x();
        m_textureVerticalPhase = phase.y();
    }

    QPointF texturePhase() const
    {
        return QPointF(m_textureHorizontalPhase, m_textureVerticalPhase);
    }

    int textureHorizontalPhase() const
    {
        return m_textureHorizontalPhase;
    }
    void setTextureHorizontalPhase(int value)
    {
        m_textureHorizontalPhase = value;
    }

    int textureVerticalPhase() const
    {
        return m_textureVerticalPhase;
    }
    void setTextureVerticalPhase(int value)
    {
        m_textureVerticalPhase = value;
    }

    void scaleLinearSizes(qreal scale) override
    {
        psd_layer_effects_shadow_base::scaleLinearSizes(scale);
        m_soften *= scale;
        m_textureScale *= scale;
    }

private:
    psd_bevel_style m_style;
    psd_technique_type m_technique;
    int m_depth;
    psd_direction m_direction; // Up or down
    int m_soften; // Blur value in pixels.

    int m_altitude;

    quint8 m_glossContourLookupTable[256];
    bool m_glossAntiAliased;

    QString m_highlightBlendMode; // already in Krita format
    QColor m_highlightColor;
    qint32 m_highlightOpacity; // Highlight opacity as a percent

    QString m_shadowBlendMode; // already in Krita format
    QColor m_shadowColor;
    qint32 m_shadowOpacity; // Shadow opacity as a percent

    bool m_contourEnabled;
    int m_contourRange;

    bool m_textureEnabled;
    KisLinkedResourceWrapper<KoPattern> m_texturePatternLink;
    int m_textureScale;
    int m_textureDepth;
    bool m_textureInvert;

    bool m_textureAlignWithLayer;
    int m_textureHorizontalPhase; // 0..100%
    int m_textureVerticalPhase; // 0..100%
};

struct psd_layer_effects_overlay_base : public psd_layer_effects_shadow_base {
    psd_layer_effects_overlay_base()
        : m_scale(100)
        , m_alignWithLayer(true)
        , m_dither(false)
        , m_reverse(false)
        , m_style(psd_gradient_style_linear)
        , m_gradientXOffset(0)
        , m_gradientYOffset(0)
        , m_horizontalPhase(0)
        , m_verticalPhase(0)
    {
        setUseGlobalLight(false);
    }

    /// FIXME: 'using' is not supported by MSVC, so please refactor in
    ///        some other way to ensure that the setters are not used
    ///        in the classes we don't want

    // using psd_layer_effects_shadow_base::setBlendMode;
    // using psd_layer_effects_shadow_base::setOpacity;

    int scale() const
    {
        return m_scale;
    }

    bool alignWithLayer() const
    {
        return m_alignWithLayer;
    }

    bool dither() const
    {
        return m_dither;
    }

    bool reverse() const
    {
        return m_reverse;
    }

    psd_gradient_style style() const
    {
        return m_style;
    }

    int gradientXOffset() const
    {
        return m_gradientXOffset;
    }

    int gradientYOffset() const
    {
        return m_gradientYOffset;
    }

    KoPatternSP pattern(KisResourcesInterfaceSP resourcesInterface) const
    {
        return m_patternLink.isValid() ? m_patternLink.resource(resourcesInterface) : KoPatternSP();
    }

    int horizontalPhase() const
    {
        return m_horizontalPhase;
    }

    int verticalPhase() const
    {
        return m_verticalPhase;
    }

    // refactor that
public:
    void setScale(int value)
    {
        m_scale = value;
    }

    void setAlignWithLayer(bool value)
    {
        m_alignWithLayer = value;
    }

    void setDither(bool value)
    {
        m_dither = value;
    }

    void setReverse(bool value)
    {
        m_reverse = value;
    }

    void setStyle(psd_gradient_style value)
    {
        m_style = value;
    }

    void setGradientOffset(const QPointF &pt)
    {
        m_gradientXOffset = qRound(pt.x());
        m_gradientYOffset = qRound(pt.y());
    }

    QPointF gradientOffset() const
    {
        return QPointF(m_gradientXOffset, m_gradientYOffset);
    }

    void setPattern(KoPatternSP value)
    {
        m_patternLink = KisLinkedResourceWrapper<KoPattern>(value);
    }

    void setPatternPhase(const QPointF &phase)
    {
        m_horizontalPhase = phase.x();
        m_verticalPhase = phase.y();
    }

    QPointF patternPhase() const
    {
        return QPointF(m_horizontalPhase, m_verticalPhase);
    }

    void scaleLinearSizes(qreal scale) override
    {
        psd_layer_effects_shadow_base::scaleLinearSizes(scale);
        m_scale *= scale;
    }

private:
    // Gradient+Pattern
    int m_scale;
    bool m_alignWithLayer;

    // Gradient
    bool m_dither;
    bool m_reverse;
    psd_gradient_style m_style;
    int m_gradientXOffset; // 0..100%
    int m_gradientYOffset; // 0..100%

    // Pattern
    KisLinkedResourceWrapper<KoPattern> m_patternLink;
    int m_horizontalPhase; // 0..100%
    int m_verticalPhase; // 0..100%

protected:
    /// FIXME: 'using' is not supported by MSVC, so please refactor in
    ///        some other way to ensure that the setters are not used
    ///        in the classes we don't want

    // must be called in the derived classes' c-tor
    // using psd_layer_effects_shadow_base::setFillType;
};

// sofi: https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm#50577409_70055
struct psd_layer_effects_color_overlay : public psd_layer_effects_overlay_base {
    psd_layer_effects_color_overlay()
    {
        setFillType(psd_fill_solid_color);
        setColor(Qt::white);
    }

    /// FIXME: 'using' is not supported by MSVC, so please refactor in
    ///        some other way to ensure that the setters are not used
    ///        in the classes we don't want

    // using psd_layer_effects_shadow_base::setColor;
};

struct psd_layer_effects_gradient_overlay : public psd_layer_effects_overlay_base {
    psd_layer_effects_gradient_overlay()
    {
        setFillType(psd_fill_gradient);
        setAngle(90);
        setReverse(false);
        setScale(100);
        setAlignWithLayer(true);
        setStyle(psd_gradient_style_linear);
    }

public:
    /// FIXME: 'using' is not supported by MSVC, so please refactor in
    ///        some other way to ensure that the setters are not used
    ///        in the classes we don't want

    // using psd_layer_effects_shadow_base::setGradient;
    // using psd_layer_effects_shadow_base::setAngle;

    // using psd_layer_effects_overlay_base::setReverse;
    // using psd_layer_effects_overlay_base::setScale;
    // using psd_layer_effects_overlay_base::setAlignWithLayer;
    // using psd_layer_effects_overlay_base::setStyle;

    // using psd_layer_effects_overlay_base::setGradientOffset;
    // using psd_layer_effects_overlay_base::gradientOffset;
};

struct psd_layer_effects_pattern_overlay : public psd_layer_effects_overlay_base {
    psd_layer_effects_pattern_overlay()
    {
        setFillType(psd_fill_pattern);
        setScale(100);
        setAlignWithLayer(true);
    }

    /// FIXME: 'using' is not supported by MSVC, so please refactor in
    ///        some other way to ensure that the setters are not used
    ///        in the classes we don't want

    // using psd_layer_effects_overlay_base::setScale;
    // using psd_layer_effects_overlay_base::setAlignWithLayer;

    // using psd_layer_effects_overlay_base::setPattern;

    // using psd_layer_effects_overlay_base::setPatternPhase;
    // using psd_layer_effects_overlay_base::patternPhase;

private:
    // These are unused
    /*int m_scale;
    bool m_alignWithLayer;
    KoPatternSP m_pattern;

    int m_horizontalPhase;
    int m_verticalPhase;*/
};

struct psd_layer_effects_stroke : public psd_layer_effects_overlay_base {
    psd_layer_effects_stroke()
        : m_position(psd_stroke_outside)
    {
        setFillType(psd_fill_solid_color);
        setColor(Qt::black);

        setAngle(90);
        setReverse(false);
        setScale(100);
        setAlignWithLayer(true);
        setStyle(psd_gradient_style_linear);

        setScale(100);
        setAlignWithLayer(true);
    }

    /// FIXME: 'using' is not supported by MSVC, so please refactor in
    ///        some other way to ensure that the setters are not used
    ///        in the classes we don't want

    // using psd_layer_effects_shadow_base::setFillType;
    // using psd_layer_effects_shadow_base::setSize;

    // using psd_layer_effects_shadow_base::setColor;

    // using psd_layer_effects_shadow_base::setGradient;
    // using psd_layer_effects_shadow_base::setAngle;
    // using psd_layer_effects_overlay_base::setReverse;
    // using psd_layer_effects_overlay_base::setScale;
    // using psd_layer_effects_overlay_base::setAlignWithLayer;
    // using psd_layer_effects_overlay_base::setStyle;

    // using psd_layer_effects_overlay_base::setGradientOffset;
    // using psd_layer_effects_overlay_base::gradientOffset;

    // using psd_layer_effects_overlay_base::setPattern;

    // using psd_layer_effects_overlay_base::setPatternPhase;
    // using psd_layer_effects_overlay_base::patternPhase;

    psd_stroke_position position() const
    {
        return m_position;
    }
    void setPosition(psd_stroke_position value)
    {
        m_position = value;
    }

private:
    psd_stroke_position m_position;
};

/**
 * Convert PsdColorMode to pigment colormodelid and colordepthid.
 * @see KoColorModelStandardIds
 *
 * @return a QPair containing ColorModelId and ColorDepthID
 */
QPair<QString, QString> KRITAPSDUTILS_EXPORT psd_colormode_to_colormodelid(psd_color_mode colormode, quint16 channelDepth);

/**
 * Convert the Photoshop blend mode strings to Pigment compositeop id's
 */
QString KRITAPSDUTILS_EXPORT psd_blendmode_to_composite_op(const QString &blendmode);
QString KRITAPSDUTILS_EXPORT composite_op_to_psd_blendmode(const QString &compositeOp);

#endif // PSD_H

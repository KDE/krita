/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_TEXTURE_OPTION_H
#define KIS_TEXTURE_OPTION_H

#include <kritapaintop_export.h>

#include <kis_paint_device.h>
#include <kis_cached_paint_device.h>
#include <kis_types.h>
#include "kis_paintop_option.h"
#include "kis_pressure_texture_strength_option.h"
#include <resources/KoAbstractGradient.h>
#include <resources/KoCachedGradient.h>

#include "KisTextureMaskInfo.h"

#include <QRect>

class KisTextureChooser;
class KoPattern;
class KoResource;
class KisPropertiesConfiguration;
class KisPaintopLodLimitations;
class KisResourcesInterface;


enum KisBrushTextureFlag
{
    None = 0x0,
    SupportsLightnessMode = 0x1,
    SupportsGradientMode = 0x2,
};
Q_DECLARE_FLAGS(KisBrushTextureFlags, KisBrushTextureFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(KisBrushTextureFlags)

class PAINTOP_EXPORT KisTextureOption : public KisPaintOpOption
{
    Q_OBJECT
public:

    explicit KisTextureOption(KisBrushTextureFlags flags = None);
    ~KisTextureOption() override;

public Q_SLOTS:

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;
    void lodLimitations(KisPaintopLodLimitations *l) const override;

private Q_SLOTS:

    void resetGUI(KoResourceSP ); /// called when a new pattern is selected


private:
    /// UI Widget that stores all the texture options
    KisTextureChooser* m_textureOptions;
};

class PAINTOP_EXPORT KisTextureProperties
{
public:
    KisTextureProperties(int levelOfDetail, KisBrushTextureFlags flags = None);

    enum TexturingMode {
        MULTIPLY,
        SUBTRACT,
        LIGHTNESS,
        GRADIENT,
        DARKEN,
        OVERLAY,
        COLOR_DODGE,
        COLOR_BURN,
        LINEAR_DODGE,
        LINEAR_BURN,
        HARD_MIX_PHOTOSHOP,
        HARD_MIX_SOFTER_PHOTOSHOP,
        HEIGHT,
        LINEAR_HEIGHT,
        HEIGHT_PHOTOSHOP,
        LINEAR_HEIGHT_PHOTOSHOP
    };

    bool m_enabled;

    /**
     * @brief apply combine the texture map with the dab
     * @param dab the colored, final representation of the dab, after mirroring and everything.
     * @param offset the position of the dab on the image. used to calculate the position of the mask pattern
     * @param info the paint information
     */
    void apply(KisFixedPaintDeviceSP dab, const QPoint& offset, const KisPaintInformation & info);
    void fillProperties(const KisPropertiesConfigurationSP setting, KisResourcesInterfaceSP resourcesInterface, KoCanvasResourcesInterfaceSP canvasResourcesInterface);
    QList<KoResourceSP> prepareEmbeddedResources(const KisPropertiesConfigurationSP setting, KisResourcesInterfaceSP resourcesInterface);
    bool applyingGradient() const;

    static bool applyingGradient(const KisPropertiesConfiguration *settings);

private:
    void applyLightness(KisFixedPaintDeviceSP dab, const QPoint& offset, const KisPaintInformation& info);
    void applyGradient(KisFixedPaintDeviceSP dab, const QPoint& offset, const KisPaintInformation& info);

private:

    int m_offsetX;
    int m_offsetY;
    TexturingMode m_texturingMode;
    KoAbstractGradientSP m_gradient;
    KoCachedGradient m_cachedGradient;

    int m_levelOfDetail;

private:
    KisPressureTextureStrengthOption m_strengthOption;
    KisTextureMaskInfoSP m_maskInfo;
    KisBrushTextureFlags m_flags;
    KisCachedPaintDevice m_cachedPaintDevice;
};

#endif // KIS_TEXTURE_OPTION_H

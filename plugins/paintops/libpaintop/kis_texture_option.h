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
#include <resources/KoAbstractGradient.h>
#include <resources/KoCachedGradient.h>

#include "KisTextureMaskInfo.h"

#include <QRect>

class KoPattern;
class KoResource;
class KisPropertiesConfiguration;
class KisResourcesInterface;

#include <KisStandardOptions.h>
#include <KisTextureOptionData.h>


class PAINTOP_EXPORT KisTextureOption
{
public:
    KisTextureOption(const KisPropertiesConfiguration *setting, KisResourcesInterfaceSP resourcesInterface,
                     KoCanvasResourcesInterfaceSP canvasResourcesInterface,
                     int levelOfDetail,
                     KisBrushTextureFlags flags = None);

    bool m_enabled {false};

    /**
     * @brief apply combine the texture map with the dab
     * @param dab the colored, final representation of the dab, after mirroring and everything.
     * @param offset the position of the dab on the image. used to calculate the position of the mask pattern
     * @param info the paint information
     */
    void apply(KisFixedPaintDeviceSP dab, const QPoint& offset, const KisPaintInformation & info);
    static QList<KoResourceLoadResult> prepareLinkedResources(const KisPropertiesConfigurationSP setting, KisResourcesInterfaceSP resourcesInterface);
    static QList<KoResourceLoadResult> prepareEmbeddedResources(const KisPropertiesConfigurationSP setting, KisResourcesInterfaceSP resourcesInterface);
    bool applyingGradient() const;

    static bool applyingGradient(const KisPropertiesConfiguration *settings);

private:
    void applyLightness(KisFixedPaintDeviceSP dab, const QPoint& offset, const KisPaintInformation& info);
    void applyGradient(KisFixedPaintDeviceSP dab, const QPoint& offset, const KisPaintInformation& info);
    void fillProperties(const KisPropertiesConfiguration *setting, KisResourcesInterfaceSP resourcesInterface, KoCanvasResourcesInterfaceSP canvasResourcesInterface);
private:

    int m_offsetX {0};
    int m_offsetY {0};
    KisTextureOptionData::TexturingMode m_texturingMode {KisTextureOptionData::MULTIPLY};
    KoAbstractGradientSP m_gradient;
    KoCachedGradient m_cachedGradient;

    int m_levelOfDetail {0};

private:
    KisStrengthOption m_strengthOption;
    KisTextureMaskInfoSP m_maskInfo;
    KisBrushTextureFlags m_flags;
    KisCachedPaintDevice m_cachedPaintDevice;
};

#endif // KIS_TEXTURE_OPTION_H

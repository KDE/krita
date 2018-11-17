/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2012
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_TEXTURE_OPTION_H
#define KIS_TEXTURE_OPTION_H

#include <kritapaintop_export.h>

#include <kis_paint_device.h>
#include <kis_types.h>
#include "kis_paintop_option.h"
#include "kis_pressure_texture_strength_option.h"

#include "KisTextureMaskInfo.h"

#include <QRect>

class KisTextureChooser;
class KoPattern;
class KoResource;
class KisPropertiesConfiguration;
class KisPaintopLodLimitations;

class PAINTOP_EXPORT KisTextureOption : public KisPaintOpOption
{
    Q_OBJECT
public:

    explicit KisTextureOption();
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
    KisTextureProperties(int levelOfDetail);

    enum TexturingMode {
        MULTIPLY,
        SUBTRACT
    };

    bool m_enabled;

    /**
     * @brief apply combine the texture map with the dab
     * @param dab the colored, final representation of the dab, after mirroring and everything.
     * @param offset the position of the dab on the image. used to calculate the position of the mask pattern
     */
    void apply(KisFixedPaintDeviceSP dab, const QPoint& offset, const KisPaintInformation & info);
    void fillProperties(const KisPropertiesConfigurationSP setting);

private:

    int m_offsetX;
    int m_offsetY;
    TexturingMode m_texturingMode;

    int m_levelOfDetail;

private:
    KisPressureTextureStrengthOption m_strengthOption;
    KisTextureMaskInfoSP m_maskInfo;
};

#endif // KIS_TEXTURE_OPTION_H

/*
 *  Copyright (c) 2010 Sven Langkamp <sven.langkamp@gmail.com>
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

#ifndef KIS_BRUSH_BASED_PAINTOP_SETTINGS_H
#define KIS_BRUSH_BASED_PAINTOP_SETTINGS_H

#include <brushengine/kis_paintop_settings.h>
#include <kritapaintop_export.h>
#include <kis_outline_generation_policy.h>
#include <kis_brush.h>
#include <kis_shared.h>
#include <kis_shared_ptr.h>


class PAINTOP_EXPORT KisBrushBasedPaintOpSettings : public KisOutlineGenerationPolicy<KisPaintOpSettings>
{
public:
    KisBrushBasedPaintOpSettings();
    ~KisBrushBasedPaintOpSettings() override {}

    ///Reimplemented
    bool paintIncremental() override;

    using KisPaintOpSettings::brushOutline;
    QPainterPath brushOutline(const KisPaintInformation &info, const OutlineMode &mode) override;

    ///Reimplemented
    bool isValid() const override;

    ///Reimplemented
    bool isLoadable() override;

    KisBrushSP brush() const;

    KisPaintOpSettingsSP clone() const override;

    void setAngle(qreal value);
    qreal angle();

    void setSpacing(qreal spacing);
    qreal spacing();

    void setAutoSpacing(bool active, qreal coeff);

    bool autoSpacingActive();
    qreal autoSpacingCoeff();

    void setPaintOpSize(qreal value) override;
    qreal paintOpSize() const override;

    QList<KisUniformPaintOpPropertySP> uniformProperties(KisPaintOpSettingsSP settings) override;

protected:

    void onPropertyChanged() override;
    QPainterPath brushOutlineImpl(const KisPaintInformation &info, const OutlineMode &mode, qreal additionalScale);
    mutable KisBrushSP m_savedBrush;
    QList<KisUniformPaintOpPropertyWSP> m_uniformProperties;

private:

    Q_DISABLE_COPY(KisBrushBasedPaintOpSettings)

};

class KisBrushBasedPaintOpSettings;
typedef KisPinnedSharedPtr<KisBrushBasedPaintOpSettings> KisBrushBasedPaintOpSettingsSP;

#endif // KIS_BRUSH_BASED_PAINTOP_SETTINGS_H

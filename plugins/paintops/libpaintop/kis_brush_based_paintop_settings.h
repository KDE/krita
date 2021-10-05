/*
 *  SPDX-FileCopyrightText: 2010 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    KisBrushBasedPaintOpSettings(KisResourcesInterfaceSP resourcesInterface);
    ~KisBrushBasedPaintOpSettings() override {}

    ///Reimplemented
    bool paintIncremental() override;

    using KisPaintOpSettings::brushOutline;
    QPainterPath brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom) override;

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

    QList<KisUniformPaintOpPropertySP> uniformProperties(KisPaintOpSettingsSP settings, QPointer<KisPaintOpPresetUpdateProxy> updateProxy) override;

    virtual bool hasPatternSettings() const override;

    QList<int> requiredCanvasResources() const override;

    void setResourceCacheInterface(KoResourceCacheInterfaceSP cacheInterface) override;
    void regenerateResourceCache(KoResourceCacheInterfaceSP cacheInterface) override;

protected:

    void onPropertyChanged() override;
    QPainterPath brushOutlineImpl(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom, qreal additionalScale);
    mutable KisBrushSP m_savedBrush;
    QList<KisUniformPaintOpPropertyWSP> m_uniformProperties;

private:

    Q_DISABLE_COPY(KisBrushBasedPaintOpSettings)

};

class KisBrushBasedPaintOpSettings;
typedef KisPinnedSharedPtr<KisBrushBasedPaintOpSettings> KisBrushBasedPaintOpSettingsSP;

#endif // KIS_BRUSH_BASED_PAINTOP_SETTINGS_H

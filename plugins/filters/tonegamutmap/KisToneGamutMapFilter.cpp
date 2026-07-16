/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-only
 */
#include "KisToneGamutMapFilter.h"

#include <kis_filter_category_ids.h>
#include <filter/kis_filter_registry.h>
#include <kis_filter_configuration.h>
#include <KisGlobalResourcesInterface.h>

#include <KoColorModelStandardIds.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
#include <kis_color_space_selector.h>

#include <KoCompositeColorTransformation.h>

#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include "ui_kistonegamutmapwidget.h"

const QString TONE_MAP_ENABLED = "toneMap/Enabled";
const QString GAMUT_MAP_ENABLED = "gamutMap/Enabled";
const QString GAMUT_MAP_TARGET_PROFILE = "gamutMap/TargetProfile";

K_PLUGIN_FACTORY_WITH_JSON(KisToneGamutMapFilterFactory, "kritatonegamutmapfilter.json", registerPlugin<KisToneGamutMapFilterPlugin>();)

KisToneGamutMapFilterPlugin::KisToneGamutMapFilterPlugin(QObject *parent, const QVariantList &): QObject(parent)
{
    KisFilterRegistry::instance()->add(KisFilterSP(new KisToneGamutMapFilter()));
}

KisToneGamutMapFilterPlugin::~KisToneGamutMapFilterPlugin()
{

}

KisToneGamutMapFilter::KisToneGamutMapFilter()
    : KisColorTransformationFilter(id(), FiltersCategoryColorId, i18n("Tone/Gamut Map..."))
{
    setSupportsThreading(true);

    /// While as a colortransformation this filter supports threading,
    /// Adjustment layers and painting are disabled for two reasons:
    ///
    /// 1. There's a ton of different tone and gamut mapping algorithms.
    /// This way, we avoid being tied to a specific one, and can experiment.
    /// 2. Some of these implementations are very slow, and thus not suited
    /// for dynamic updating.
    setSupportsAdjustmentLayers(false);
    setSupportsPainting(false);
}

KoColorTransformation *KisToneGamutMapFilter::createTransformation(const KoColorSpace *cs, const KisFilterConfigurationSP config) const
{
    QVector<KoColorTransformation*> allTransforms;

    if (config->getProperty(TONE_MAP_ENABLED).toBool()) {
        QHash<QString, QVariant> params;
        KoColorTransformation *toneMap = cs->createColorTransformation("tonemap_adjustment", params);
        allTransforms << toneMap;
    }

    if (config->getProperty(GAMUT_MAP_ENABLED).toBool()) {
        QHash<QString, QVariant> params;
        params["targetColorProfileName"] = config->getProperty(GAMUT_MAP_TARGET_PROFILE);
        KoColorTransformation *gamutMap = cs->createColorTransformation("gamutmap_clip_adjustment", params);
        allTransforms << gamutMap;
    }

    return KoCompositeColorTransformation::createOptimizedCompositeTransform(allTransforms);
}

KisFilterConfigurationSP KisToneGamutMapFilter::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisFilterConfigurationSP config = factoryConfiguration(resourcesInterface);
    config->setProperty(TONE_MAP_ENABLED, true);
    config->setProperty(GAMUT_MAP_ENABLED, true);
    config->setProperty(GAMUT_MAP_TARGET_PROFILE, "sRGB-elle-V2-srgbtrc.icc");
    return config;
}

KisConfigWidget *KisToneGamutMapFilter::createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev, bool useForMasks) const
{
    Q_UNUSED(useForMasks)
    return new KisToneGamutMapWidget(parent, dev);
}


KisToneGamutMapWidget::KisToneGamutMapWidget(QWidget *parent, KisPaintDeviceSP dev)
    : KisConfigWidget(parent)
    , ui(new Ui::KisToneGamutMapWidget)
{
    Q_ASSERT(dev);
    ui->setupUi(this);

    ui->targetGamut->showDepth(false);
    ui->targetGamut->showColorBrowserButton(false);

    connect(ui->chkGamutMap, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationItemChanged()));
    connect(ui->chkToneMap, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationItemChanged()));
    connect(ui->targetGamut, SIGNAL(colorSpaceChanged(const KoColorSpace*)), SIGNAL(sigConfigurationItemChanged()));
}

KisToneGamutMapWidget::~KisToneGamutMapWidget()
{
    delete ui;
}

void KisToneGamutMapWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    QVariant value;
    if (config->getProperty(TONE_MAP_ENABLED, value)) {
        ui->chkToneMap->setChecked(value.toBool());
    }
    if (config->getProperty(GAMUT_MAP_ENABLED, value)) {
        ui->chkGamutMap->setChecked(value.toBool());
    }
    if (config->getProperty(GAMUT_MAP_TARGET_PROFILE, value)) {
        const KoColorProfile *profile = KoColorSpaceRegistry::instance()->profileByName(value.toString());
        if (profile) {
            ui->targetGamut->setCurrentColorSpace(
                KoColorSpaceRegistry::instance()->colorSpace(profile->colorModelID(),
                                                             Integer16BitsColorDepthID.id(),
                                                             profile));
        }
    }
    Q_EMIT sigConfigurationItemChanged();
}

KisPropertiesConfigurationSP KisToneGamutMapWidget::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("tone_gamut_map", 1, KisGlobalResourcesInterface::instance());
    config->setProperty(TONE_MAP_ENABLED, ui->chkToneMap->isChecked());
    config->setProperty(GAMUT_MAP_ENABLED, ui->chkGamutMap->isChecked());
    if (ui->targetGamut->currentColorSpace()) {
        config->setProperty(GAMUT_MAP_TARGET_PROFILE, ui->targetGamut->currentColorSpace()->profile()->name());
    }
    return config;
}


#include "KisToneGamutMapFilter.moc"

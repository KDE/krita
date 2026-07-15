/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-only
 */
#include "KisToneGamutMapFilter.h"
#include <kis_filter_category_ids.h>
#include <filter/kis_filter_registry.h>
#include <KoColorModelStandardIds.h>

#include <KoCompositeColorTransformation.h>

#include <kpluginfactory.h>
#include <klocalizedstring.h>

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
    setSupportsAdjustmentLayers(false);
    setSupportsPainting(false);
}

KoColorTransformation *KisToneGamutMapFilter::createTransformation(const KoColorSpace *cs, const KisFilterConfigurationSP config) const
{
    QHash<QString, QVariant> params;
    params["targetColorProfileName"] = "sRGB-elle-V2-srgbtrc.icc";

    const KoColorSpace *workingCs = KoColorSpaceRegistry::instance()->colorSpace(LABAColorModelID.id(), Float32BitsColorDepthID.id());

    KoColorTransformation *toneMap = cs->createColorTransformation("tonemap_adjustment", params);
    KoColorTransformation *gamutMap = cs->createColorTransformation("gamutmap_clip_adjustment", params);

    QVector<KoColorTransformation*> allTransforms;
    allTransforms << toneMap;
    allTransforms << gamutMap;

    return KoCompositeColorTransformation::createOptimizedCompositeTransform(allTransforms);
}

#include "KisToneGamutMapFilter.moc"

/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "patterngenerator.h"

#include <QPoint>


#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include <KoColor.h>
#include <KisResourceTypes.h>
#include <resources/KoPattern.h>

#include <kis_debug.h>
#include <kis_fill_painter.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_layer.h>
#include <generator/kis_generator_registry.h>
#include <kis_global.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <kis_pattern_chooser.h>
#include <KisResourcesInterface.h>
#include <KoResourceLoadResult.h>

#include "kis_wdg_pattern.h"
#include "ui_wdgpatternoptions.h"

K_PLUGIN_FACTORY_WITH_JSON(KritaPatternGeneratorFactory, "kritapatterngenerator.json", registerPlugin<KritaPatternGenerator>();)

KritaPatternGenerator::KritaPatternGenerator(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisGeneratorRegistry::instance()->add(new PatternGenerator());
}

KritaPatternGenerator::~KritaPatternGenerator()
{
}

/****************************************************************************/
/*              KoPatternGeneratorConfiguration                             */
/****************************************************************************/

class PatternGeneratorConfiguration : public KisFilterConfiguration
{
public:
    PatternGeneratorConfiguration(const QString & name, qint32 version, KisResourcesInterfaceSP resourcesInterface)
        : KisFilterConfiguration(name, version, resourcesInterface)
    {
    }

    PatternGeneratorConfiguration(const PatternGeneratorConfiguration &rhs)
        : KisFilterConfiguration(rhs)
    {
    }

    virtual KisFilterConfigurationSP clone() const override {
        return new PatternGeneratorConfiguration(*this);
    }

    KoResourceLoadResult pattern(KisResourcesInterfaceSP resourcesInterface) const
    {
        const QString patternMD5 = getString("pattern/md5");
        const QString patternName = getString("pattern", "Grid01.pat");
        auto source = resourcesInterface->source<KoPattern>(ResourceType::Patterns);
        return source.bestMatchLoadResult(patternMD5, "", patternName);
    }

    KoPatternSP pattern() const {
        return pattern(resourcesInterface()).resource<KoPattern>();
    }

    QTransform transform() const {
        QTransform transform;

        transform.shear(getDouble("transform_shear_x", 0.0), getDouble("transform_shear_y", 0.0));

        transform.scale(getDouble("transform_scale_x", 1.0), getDouble("transform_scale_y", 1.0));
        transform.rotate(getDouble("transform_rotation_x", 0.0), Qt::XAxis);
        transform.rotate(getDouble("transform_rotation_y", 0.0), Qt::YAxis);
        transform.rotate(getDouble("transform_rotation_z", 0.0), Qt::ZAxis);

        transform.translate(getInt("transform_offset_x", 0), getInt("transform_offset_y", 0));
        return transform;
    }

    QList<KoResourceLoadResult> linkedResources(KisResourcesInterfaceSP globalResourcesInterface) const override
    {
        return {pattern(globalResourcesInterface)};
    }
};



/****************************************************************************/
/*              KoPatternGenerator                                          */
/****************************************************************************/

PatternGenerator::PatternGenerator()
    : KisGenerator(id(), KoID("basic"), i18n("&Pattern..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(true);
}

KisFilterConfigurationSP PatternGenerator::factoryConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    return new PatternGeneratorConfiguration(id().id(), 1, resourcesInterface);
}

KisFilterConfigurationSP PatternGenerator::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisFilterConfigurationSP config = factoryConfiguration(resourcesInterface);

    auto source = resourcesInterface->source<KoPattern>(ResourceType::Patterns);

    if (!source.fallbackResource()) {
        return config;
    }

    config->setProperty("pattern/md5", QVariant::fromValue(source.fallbackResource()->md5Sum()));
    config->setProperty("pattern", QVariant::fromValue(source.fallbackResource()->name()));

    config->setProperty("transform_shear_x", QVariant::fromValue(0.0));
    config->setProperty("transform_shear_y", QVariant::fromValue(0.0));

    config->setProperty("transform_scale_x", QVariant::fromValue(1.0));
    config->setProperty("transform_scale_y", QVariant::fromValue(1.0));

    config->setProperty("transform_rotation_x", QVariant::fromValue(0.0));
    config->setProperty("transform_rotation_y", QVariant::fromValue(0.0));
    config->setProperty("transform_rotation_z", QVariant::fromValue(0.0));

    config->setProperty("transform_offset_x", QVariant::fromValue(0));
    config->setProperty("transform_offset_y", QVariant::fromValue(0));

    config->setProperty("transform_keep_scale_aspect", QVariant::fromValue(true));

    return config;
}

KisConfigWidget * PatternGenerator::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool) const
{
    Q_UNUSED(dev);
    return new KisWdgPattern(parent);
}

void PatternGenerator::generate(KisProcessingInformation dstInfo,
                                 const QSize& size,
                                 const KisFilterConfigurationSP _config,
                                 KoUpdater* progressUpdater) const
{
    KisPaintDeviceSP dst = dstInfo.paintDevice();

    Q_ASSERT(!dst.isNull());

    const PatternGeneratorConfiguration *config =
        dynamic_cast<const PatternGeneratorConfiguration*>(_config.data());

    KIS_SAFE_ASSERT_RECOVER_RETURN(config);
    KoPatternSP pattern = config->pattern();
    QTransform transform = config->transform();

    KisFillPainter gc(dst);
    gc.setPattern(pattern);
    gc.setProgress(progressUpdater);
    gc.setChannelFlags(config->channelFlags());
    gc.setOpacity(OPACITY_OPAQUE_U8);
    gc.setSelection(dstInfo.selection());
    gc.setWidth(size.width());
    gc.setHeight(size.height());
    gc.setFillStyle(KisFillPainter::FillStylePattern);
    /**
     * HACK ALERT: using "no-compose" version of `fillRect` discards all the opacity,
     * selection, and channel flags options. Though it doesn't seem that we have a any
     * GUI in Krita that actually passes a selection to the generator itself. Fill
     * layers apply their settings on a later stage of the compositing pipeline.
     */
    gc.fillRectNoCompose(QRect(dstInfo.topLeft(), size), pattern, transform);
    gc.end();

}

#include "patterngenerator.moc"

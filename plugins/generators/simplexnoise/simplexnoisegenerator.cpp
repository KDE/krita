/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2019 Eoin O 'Neill <eoinoneill1991@gmail.com>
 * SPDX-FileCopyrightText: 2019 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "simplexnoisegenerator.h"
#include "ui_wdgsimplexnoiseoptions.h"
#include "kis_wdg_simplex_noise.h"
#include "3rdparty/c-open-simplex/open-simplex-noise.h"

#include <KisSequentialIteratorProgress.h>
#include <KoUpdater.h>
#include <QCryptographicHash>
#include <filter/kis_filter_configuration.h>
#include <generator/kis_generator_registry.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpaceRegistry.h>
#include <kis_processing_information.h>
#include <kpluginfactory.h>

K_PLUGIN_FACTORY_WITH_JSON(KritaSimplexNoiseGeneratorFactory, "kritasimplexnoisegenerator.json", registerPlugin<KisSimplexNoiseGeneratorHandle>();)

KisSimplexNoiseGeneratorHandle::KisSimplexNoiseGeneratorHandle(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisGeneratorRegistry::instance()->add(new KisSimplexNoiseGenerator());

}

KisSimplexNoiseGeneratorHandle::~KisSimplexNoiseGeneratorHandle()
{
}

KisSimplexNoiseGenerator::KisSimplexNoiseGenerator() : KisGenerator(id(), KoID("basic"), i18n("&Simplex Noise..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(true);
}

void KisSimplexNoiseGenerator::generate(KisProcessingInformation dst, const QSize &size, const KisFilterConfigurationSP config, KoUpdater *progressUpdater) const
{
    KisPaintDeviceSP device = dst.paintDevice();
    Q_ASSERT(!device.isNull());

    osn_context *noise_context;

    QRect bounds = QRect(dst.topLeft(), size);
    QRect whole_image_bounds = device->defaultBounds()->bounds();

    const KoColorSpace *cs = device->colorSpace();
    const KoColorSpace *src = KoColorSpaceRegistry::instance()->colorSpace(GrayAColorModelID.id(), Float32BitsColorDepthID.id(), "Gray-D50-elle-V2-srgbtrc.icc");
    KoColorConversionTransformation *conv = KoColorSpaceRegistry::instance()->createColorConverter(src, cs, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());

    KisSequentialIteratorProgress it(device, bounds, progressUpdater);

    QVariant property;

    const uint default_seed = (config->getProperty("seed", property)) ? property.toUInt() : 0;
    const QString custom_seed_string = (config->getProperty("custom_seed_string", property)) ? property.toString() : "";
    const bool use_custom_seed = !custom_seed_string.trimmed().isEmpty();

    const uint seed = use_custom_seed ? seedFromString(custom_seed_string) : default_seed;
    open_simplex_noise(seed, &noise_context);

    double frequency = (config && config->getProperty("frequency", property)) ? property.toDouble() : 25.0;
    double ratio_x = (config && config->getProperty("ratio_x", property)) ? property.toDouble() : 1.0;
    double ratio_y = (config && config->getProperty("ratio_y", property)) ? property.toDouble() : 1.0;

    bool looping = (config && config->getProperty("looping", property)) ? property.toBool() : false;

    if( looping ){
        float major_radius = 0.5f * frequency * ratio_x;
        float minor_radius = 0.5f * frequency * ratio_y;
        while(it.nextPixel()){
            double x_phase = (double)it.x() / (double)whole_image_bounds.width() * M_PI * 2;
            double y_phase = (double)it.y() / (double)(whole_image_bounds.height()) * M_PI * 2;
            double x_coordinate = major_radius * map_range(cos(x_phase), -1.0, 1.0, 0.0, 1.0);
            double y_coordinate = major_radius * map_range(sin(x_phase), -1.0, 1.0, 0.0, 1.0);
            double z_coordinate = minor_radius * map_range(cos(y_phase), -1.0, 1.0, 0.0, 1.0);
            double w_coordinate = minor_radius * map_range(sin(y_phase), -1.0, 1.0, 0.0, 1.0);
            double value = open_simplex_noise4(noise_context, x_coordinate, y_coordinate, z_coordinate, w_coordinate);
            value = map_range(value, -1.0, 1.0, 0.0, 1.0);

            KoColor c(src);
            reinterpret_cast<float *>(c.data())[0] = value;
            c.setOpacity(OPACITY_OPAQUE_F);

            conv->transform(c.data(), it.rawData(), 1);
        }
    } else {
        while(it.nextPixel()){
            double x_phase = (double)it.x() / (double)(whole_image_bounds.width()) * ratio_x;
            double y_phase = (double)it.y() / (double)(whole_image_bounds.height()) * ratio_y;
            double value = open_simplex_noise4(noise_context, x_phase * frequency, y_phase * frequency, x_phase * frequency, y_phase * frequency);
            value = map_range(value, -1.0, 1.0, 0.0, 1.0);

            KoColor c(src);
            reinterpret_cast<float *>(c.data())[0] = value;
            c.setOpacity(OPACITY_OPAQUE_F);

            conv->transform(c.data(), it.rawData(), 1);
        }
    }
    delete conv;
    open_simplex_noise_free(noise_context);
}

KisFilterConfigurationSP KisSimplexNoiseGenerator::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisFilterConfigurationSP config = factoryConfiguration(resourcesInterface);
    config->setProperty("looping", false);
    config->setProperty("frequency", 25.0);
    uint seed = static_cast<uint>(rand());
    config->setProperty("seed", seed);
    config->setProperty("custom_seed_string", "");
    config->setProperty("ratio_x", 1.0f);
    config->setProperty("ratio_y", 1.0f);
    return config;
}

KisConfigWidget * KisSimplexNoiseGenerator::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool) const
{
    Q_UNUSED(dev);
    return new KisWdgSimplexNoise((KisFilter*)this, (QWidget*)parent);
}


uint KisSimplexNoiseGenerator::seedFromString(const QString &string) const
{
    QByteArray bytes = QCryptographicHash::hash(string.toUtf8(),QCryptographicHash::Md5);
    uint hash = 0;
    for( int index = 0; index < bytes.length(); index++){
        hash += rotateLeft(bytes[index], index % 32);
    }
    return hash;
}

quint64 KisSimplexNoiseGenerator::rotateLeft(const quint64 input, uint shift) const
{
    return (input << shift)|(input >> (64 - shift));
}

#include "simplexnoisegenerator.moc"


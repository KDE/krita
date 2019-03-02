/*
 * KDE. Krita Project.
 *
 * Copyright (c) 2019 Eoin O'Neill <eoinoneill1991@gmail.com>
 * Copyright (c) 2019 Emmet O'Neill <emmetoneill.pdx@gmail.com>
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

#include "simplexnoisegenerator.h"
#include "ui_wdgsimplexnoiseoptions.h"
#include "kis_wdg_simplex_noise.h"
#include "3rdparty/c-open-simplex/open-simplex-noise.h"

#include <QCryptographicHash>
#include <kpluginfactory.h>
#include <KoUpdater.h>
#include <kis_processing_information.h>
#include <KisSequentialIteratorProgress.h>
#include <filter/kis_filter_configuration.h>
#include <generator/kis_generator_registry.h>

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
    const KoColorSpace * cs = device->colorSpace();
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
            double x_phase = (double)it.x() / (double)bounds.width() * M_PI * 2;
            double y_phase = (double)it.y() / (double)(bounds.height()) * M_PI * 2;
            double x_coordinate = major_radius * map_range(cos(x_phase), -1.0, 1.0, 0.0, 1.0);
            double y_coordinate = major_radius * map_range(sin(x_phase), -1.0, 1.0, 0.0, 1.0);
            double z_coordinate = minor_radius * map_range(cos(y_phase), -1.0, 1.0, 0.0, 1.0);
            double w_coordinate = minor_radius * map_range(sin(y_phase), -1.0, 1.0, 0.0, 1.0);
            double value = open_simplex_noise4(noise_context, x_coordinate, y_coordinate, z_coordinate, w_coordinate);
            value = map_range(value, -1.0, 1.0, 0.0, 255.0);
            QColor color = qRgb(static_cast<int>(value),
                                static_cast<int>(value),
                                static_cast<int>(value));
            cs->fromQColor(color, it.rawData());
        }
    } else {
        while(it.nextPixel()){
            double x_phase = (double)it.x() / (double)(bounds.width()) * ratio_x;
            double y_phase = (double)it.y() / (double)(bounds.height()) * ratio_y;
            double value = open_simplex_noise4(noise_context, x_phase * frequency, y_phase * frequency, x_phase * frequency, y_phase * frequency);
            value = map_range(value, -1.0, 1.0, 0.0, 255.0);
            QColor color = qRgb(static_cast<int>(value),
                                static_cast<int>(value),
                                static_cast<int>(value));
            cs->fromQColor(color, it.rawData());
        }
    }

    open_simplex_noise_free(noise_context);
}

KisFilterConfigurationSP KisSimplexNoiseGenerator::factoryConfiguration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("simplex_noise", 1);
    config->setProperty("looping", false);
    config->setProperty("frequency", 25.0);
    uint seed = static_cast<uint>(rand());
    config->setProperty("seed", seed);
    config->setProperty("custom_seed_string", "");
    config->setProperty("ratio_x", 1.0f);
    config->setProperty("ratio_y", 1.0f);
    return config;
}

KisConfigWidget * KisSimplexNoiseGenerator::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev) const
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


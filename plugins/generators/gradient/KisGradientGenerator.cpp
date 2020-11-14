/*
 * KDE. Krita Project.
 *
 * Copyright (c) 2020 Deif Lou <ginoba@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <kpluginfactory.h>
#include <KoUpdater.h>
#include <kis_processing_information.h>
#include <filter/kis_filter_configuration.h>
#include <kis_gradient_painter.h>

#include "KisGradientGenerator.h"
#include "KisGradientGeneratorConfigWidget.h"

KisGradientGenerator::KisGradientGenerator() : KisGenerator(id(), KoID("basic"), i18n("&Gradient..."))
{
    setSupportsPainting(true);
}

void KisGradientGenerator::generate(KisProcessingInformation dst,
                                    const QSize &size,
                                    const KisFilterConfigurationSP config,
                                    KoUpdater *progressUpdater) const
{
    KisPaintDeviceSP device = dst.paintDevice();
    Q_ASSERT(!device.isNull());

    KIS_SAFE_ASSERT_RECOVER_RETURN(config);
    const KisGradientGeneratorConfiguration *generatorConfiguration =
        dynamic_cast<const KisGradientGeneratorConfiguration*>(config.data());

    device->clear(QRect(dst.topLeft(), size));

    QSize imageSize = device->defaultBounds()->imageBorderRect().size();
    QPair<QPointF, QPointF> positions =
        generatorConfiguration->absoluteCartesianPositionsInPixels(imageSize.width(), imageSize.height());

    KisGradientPainter painter(device);
    painter.setProgress(progressUpdater);
    painter.setGradientShape(generatorConfiguration->shape());
    painter.setGradient(generatorConfiguration->gradient());
    painter.paintGradient(
        positions.first,
        positions.second,
        generatorConfiguration->repeat(),
        generatorConfiguration->antiAliasThreshold(),
        generatorConfiguration->reverse(),
        QRect(dst.topLeft(), size)
    );
}

KisFilterConfigurationSP KisGradientGenerator::factoryConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    return new KisGradientGeneratorConfiguration(resourcesInterface);
}

KisFilterConfigurationSP KisGradientGenerator::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisGradientGeneratorConfiguration *config = new KisGradientGeneratorConfiguration(resourcesInterface);
    config->setDefaults();
    return config;
}

KisConfigWidget* KisGradientGenerator::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool) const
{
    Q_UNUSED(dev);
    return new KisGradientGeneratorConfigWidget(parent);
}

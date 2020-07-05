/*
 * This file is part of Krita
 *
 * Copyright (c) 2020 L. E. Segovia <amy@amyspark.me>
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

#include "generator.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <kis_debug.h>
#include <kpluginfactory.h>
#include <klocalizedstring.h>
#include <kis_fill_painter.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <kis_processing_information.h>
#include <KisSequentialIteratorProgress.h>
#include <KoUpdater.h>
#include <filter/kis_filter_configuration.h>
#include <generator/kis_generator_registry.h>

#include "SeExprExpressionContext.h"

#include "kis_wdg_seexpr.h"
#include "ui_wdgseexpr.h"

/****************************************************************************/
/*              KisSeExprGeneratorConfiguration                             */
/****************************************************************************/

class KisSeExprGeneratorConfiguration : public KisFilterConfiguration
{
public:
    KisSeExprGeneratorConfiguration(const QString &name, qint32 version, KisResourcesInterfaceSP resourcesInterface)
        : KisFilterConfiguration(name, version, resourcesInterface)
    {
    }

    KisSeExprGeneratorConfiguration(const KisSeExprGeneratorConfiguration &rhs)
        : KisFilterConfiguration(rhs)
    {
    }

    virtual KisFilterConfigurationSP clone() const override
    {
        return new KisSeExprGeneratorConfiguration(*this);
    }

    QString script() const
    {
        return this->getString("script", QStringLiteral(BASE_SCRIPT));
    }
};

K_PLUGIN_FACTORY_WITH_JSON(KritaSeExprGeneratorFactory, "generator.json", registerPlugin<KritaSeExprGenerator>();)

KritaSeExprGenerator::KritaSeExprGenerator(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisGeneratorRegistry::instance()->add(new KisSeExprGenerator());
}

KritaSeExprGenerator::~KritaSeExprGenerator()
{
}

KisSeExprGenerator::KisSeExprGenerator() : KisGenerator(id(), KoID("basic"), i18n("&SeExpr..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(false);
}

KisFilterConfigurationSP KisSeExprGenerator::factoryConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    return new KisSeExprGeneratorConfiguration(id().id(), 1, resourcesInterface);
}

KisFilterConfigurationSP KisSeExprGenerator::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisFilterConfigurationSP config = factoryConfiguration(resourcesInterface);

    QVariant script(QStringLiteral(BASE_SCRIPT));
    config->setProperty("script", script);
    return config;
}

KisConfigWidget *KisSeExprGenerator::createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev, bool) const
{
    Q_UNUSED(dev);
    return new KisWdgSeExpr(parent);
}

inline KisSeExprGenerator::RenderDepth KisSeExprGenerator::textureConversionStrategy(const KoColorSpace *cs) const
{
    KIS_ASSERT(cs);
    KoID colorDepth = cs->colorDepthId();

    if (cs->colorModelId() == RGBAColorModelID)
    {
        if (colorDepth == Float16BitsColorDepthID || colorDepth == Float32BitsColorDepthID || colorDepth == Float64BitsColorDepthID)
        {
            return RENDER_RGB_DIRECTLY;
        }
        else
        {
            return RENDER_BGR_AND_CLAMP;
        }
    }
    else
    {
        return RENDER_WITH_CONVERSION;
    }
}

void KisSeExprGenerator::generate(KisProcessingInformation dstInfo,
                                  const QSize &size,
                                  const KisFilterConfigurationSP config,
                                  KoUpdater *progressUpdater) const
{
    KisPaintDeviceSP device = dstInfo.paintDevice();

    Q_ASSERT(!device.isNull());
    Q_ASSERT(config);

    if (config)
    {
        QString script = config->getString("script");

        QRect bounds = QRect(dstInfo.topLeft(), size);
        QRect whole_image_bounds = device->defaultBounds()->bounds();

        const KoColorSpace *cs = device->colorSpace();
        const RenderDepth strategy = textureConversionStrategy(cs);
        KisSequentialIteratorProgress it(device, bounds, progressUpdater);

        SeExprExpressionContext expression(script);

        expression.m_vars["u"] = new SeExprVariable();
        expression.m_vars["v"] = new SeExprVariable();
        expression.m_vars["w"] = new SeExprVariable(whole_image_bounds.width());
        expression.m_vars["h"] = new SeExprVariable(whole_image_bounds.height());

        if (expression.isValid() && expression.returnType().isFP(3))
        {
            double pixel_stride_x = 1. / whole_image_bounds.width();
            double pixel_stride_y = 1. / whole_image_bounds.height();
            double &u = expression.m_vars["u"]->m_value;
            double &v = expression.m_vars["v"]->m_value;

            switch(strategy)
            {
                case RENDER_RGB_DIRECTLY:
                    // SeExpr already outputs floating-point RGB
                    while (it.nextPixel())
                    {
                        u = pixel_stride_x * (it.x() + .5);
                        v = pixel_stride_y * (it.y() + .5);

                        const double *value = expression.evalFP();

                        QVector<float> color = {
                            static_cast<float>(value[0]),
                            static_cast<float>(value[1]),
                            static_cast<float>(value[2]),
                            KoColorSpaceMathsTraits<float>::unitValue
                        };

                        cs->fromNormalisedChannelsValue(it.rawData(), color);
                    }
                    break;
                case RENDER_BGR_AND_CLAMP:
                    // Adjust all pixels to 0.0-1.0 and render them in BGRA order
                    while (it.nextPixel())
                    {
                        u = pixel_stride_x * (it.x() + .5);
                        v = pixel_stride_y * (it.y() + .5);

                        const double *value = expression.evalFP();

                        QVector<float> color = {
                            (float)qBound(value[2], 0.0, 1.0),
                            (float)qBound(value[1], 0.0, 1.0),
                            (float)qBound(value[0], 0.0, 1.0),
                            KoColorSpaceMathsTraits<float>::unitValue
                        };

                        cs->fromNormalisedChannelsValue(it.rawData(), color);
                    }
                    break;
                case RENDER_WITH_CONVERSION:
                    // Adjust all pixels to 0.0-1.0 and transform them via QColor
                    while (it.nextPixel())
                    {
                        u = pixel_stride_x * (it.x() + .5);
                        v = pixel_stride_y * (it.y() + .5);

                        const double *value = expression.evalFP();

                        QColor color = QColor::fromRgbF(
                            qBound(static_cast<float>(value[0]), 0.0f, 1.0f),
                            qBound(static_cast<float>(value[1]), 0.0f, 1.0f),
                            qBound(static_cast<float>(value[2]), 0.0f, 1.0f)
                        );

                        cs->fromQColor(color, it.rawData());
                    }
                    break;
            }
        }
    }
}

#include "generator.moc"

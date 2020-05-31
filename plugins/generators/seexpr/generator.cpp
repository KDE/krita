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

KisFilterConfigurationSP KisSeExprGenerator::defaultConfiguration() const
{
    KisFilterConfigurationSP config = factoryConfiguration();

    QVariant script(QStringLiteral(BASE_SCRIPT));
    config->setProperty("script", script);
    return config;
}

KisConfigWidget *KisSeExprGenerator::createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev, bool) const
{
    Q_UNUSED(dev);
    return new KisWdgSeExpr(parent);
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
        const KoColorSpace *cs = device->colorSpace();
        KisSequentialIteratorProgress it(device, bounds, progressUpdater);

        SeExprExpressionContext expression(script);

        expression.m_vars["u"] = new SeExprVariable();
        expression.m_vars["v"] = new SeExprVariable();
        expression.m_vars["w"] = new SeExprVariable(bounds.width());
        expression.m_vars["h"] = new SeExprVariable(bounds.height());

        if (expression.isValid() && expression.returnType().isFP(3))
        {
            double pixel_stride_x = 1. / bounds.width();
            double pixel_stride_y = 1. / bounds.height();
            double &u = expression.m_vars["u"]->m_value;
            double &v = expression.m_vars["v"]->m_value;

            while(it.nextPixel())
            {
                u = pixel_stride_x * (it.x() + .5);
                v = pixel_stride_y * (it.y() + .5);

                const qreal* value = expression.evalFP();
                QColor color;

                // SeExpr already outputs normalized RGB
                color.setRedF(value[0]);
                color.setGreenF(value[1]);
                color.setBlueF(value[2]);

                cs->fromQColor(color, it.rawData());
            }
        }
    }
}

#include "generator.moc"

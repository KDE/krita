/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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

#include "patterngenerator.h"

#include <QPoint>


#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include <KoColor.h>
#include <KoResourceServer.h>
#include <resources/KoPattern.h>
#include <KoResourceServerProvider.h>

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


#include "kis_wdg_pattern.h"
#include "ui_wdgpatternoptions.h"

K_PLUGIN_FACTORY_WITH_JSON(KritaPatternGeneratorFactory, "kritapatterngenerator.json", registerPlugin<KritaPatternGenerator>();)

KritaPatternGenerator::KritaPatternGenerator(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisGeneratorRegistry::instance()->add(new KoPatternGenerator());
}

KritaPatternGenerator::~KritaPatternGenerator()
{
}

KoPatternGenerator::KoPatternGenerator() : KisGenerator(id(), KoID("basic"), i18n("&Pattern..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(true);
}

KisFilterConfigurationSP KoPatternGenerator::defaultConfiguration() const
{
    KisFilterConfigurationSP config = factoryConfiguration();

    QVariant v;
    v.setValue(QString("Grid01.pat"));
    config->setProperty("pattern", v);

//    v.setValue(KoColor());
//    config->setProperty("color", v);

    return config;
}

KisConfigWidget * KoPatternGenerator::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool) const
{
    Q_UNUSED(dev);
    return new KisWdgPattern(parent);
}

void KoPatternGenerator::generate(KisProcessingInformation dstInfo,
                                 const QSize& size,
                                 const KisFilterConfigurationSP config,
                                 KoUpdater* progressUpdater) const
{
    KisPaintDeviceSP dst = dstInfo.paintDevice();

    Q_ASSERT(!dst.isNull());
    Q_ASSERT(config);

    if (!config) return;
    QString patternName = config->getString("pattern", "Grid01.pat");
    KoResourceServer<KoPattern> *rserver = KoResourceServerProvider::instance()->patternServer();
    KoPattern *pattern = rserver->resourceByName(patternName);

//    KoColor c = config->getColor("color");

    KisFillPainter gc(dst);
    gc.setPattern(pattern);
//    gc.setPaintColor(c);
    gc.setProgress(progressUpdater);
    gc.setChannelFlags(config->channelFlags());
    gc.setOpacity(OPACITY_OPAQUE_U8);
    gc.setSelection(dstInfo.selection());
    gc.setWidth(size.width());
    gc.setHeight(size.height());
    gc.setFillStyle(KisFillPainter::FillStylePattern);
    gc.fillRect(QRect(dstInfo.topLeft(), size), pattern);
    gc.end();

}

#include "patterngenerator.moc"

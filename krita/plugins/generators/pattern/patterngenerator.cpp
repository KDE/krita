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

#include <kis_debug.h>
#include <kcomponentdata.h>
#include <kpluginfactory.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kstandarddirs.h>

#include <KoColor.h>
#include <KoProgressUpdater.h>

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
#include <KoResourceServer.h>
#include <kis_pattern.h>
#include "kis_resource_server_provider.h"

#include "kis_wdg_pattern.h"
#include "ui_wdgpatternoptions.h"

K_PLUGIN_FACTORY(KritaPatternGeneratorFactory, registerPlugin<KritaPatternGenerator>();)
K_EXPORT_PLUGIN(KritaPatternGeneratorFactory("krita"))

KritaPatternGenerator::KritaPatternGenerator(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisGeneratorRegistry::instance()->add(new KisPatternGenerator());
}

KritaPatternGenerator::~KritaPatternGenerator()
{
}

KisPatternGenerator::KisPatternGenerator() : KisGenerator(id(), KoID("basic"), i18n("&Pattern..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(true);
    setSupportsIncrementalPainting(false);
}

KisFilterConfiguration* KisPatternGenerator::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration("pattern", 1);

    QVariant v;
    v.setValue(QString("Grid01.pat"));
    config->setProperty("pattern", v);

//    v.setValue(KoColor());
//    config->setProperty("color", v);

    return config;
}

KisConfigWidget * KisPatternGenerator::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev) const
{
    Q_UNUSED(dev);
    return new KisWdgPattern(parent);
}

void KisPatternGenerator::generate(KisProcessingInformation dstInfo,
                                 const QSize& size,
                                 const KisFilterConfiguration* config,
                                 KoUpdater* progressUpdater) const
{
    KisPaintDeviceSP dst = dstInfo.paintDevice();

    Q_ASSERT(!dst.isNull());
    Q_ASSERT(config);

    if (!config) return;
    QString patternName = config->getString("pattern", "Grid01.pat");
    KoResourceServer<KisPattern> *rserver = KisResourceServerProvider::instance()->patternServer();
    KisPattern *pattern = rserver->getResourceByName(patternName);

//    KoColor c = config->getColor("color");

    KisFillPainter gc(dst);
    gc.setPattern(pattern);
//    gc.setPaintColor(c);
    gc.setProgress(progressUpdater);
    gc.setChannelFlags(config->channelFlags());
    gc.setOpacity(100);
    gc.setSelection(dstInfo.selection());
    gc.setWidth(size.width());
    gc.setHeight(size.height());
    gc.setFillStyle(KisFillPainter::FillStylePattern);
    gc.setBounds(QRect(dstInfo.topLeft(), size));
    gc.fillRect(QRect(dstInfo.topLeft(), size), pattern);
    gc.end();

}

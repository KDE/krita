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

#include "colorgenerator.h"

#include <qpoint.h>

#include <kis_debug.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kpluginfactory.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kstandarddirs.h>

#include <KoProgressUpdater.h>
#include <KoColorSpaceRegistry.h>

#include <kis_fill_painter.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <generator/kis_generator_registry.h>
#include <kis_global.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>

#include "kis_wdg_color.h"
#include "ui_wdgcoloroptions.h"

K_PLUGIN_FACTORY(KritaColorGeneratorFactory, registerPlugin<KritaColorGenerator>();)
K_EXPORT_PLUGIN(KritaColorGeneratorFactory("krita"))

KritaColorGenerator::KritaColorGenerator(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    //setComponentData(KritaColorGeneratorFactory::componentData());
    KisGeneratorRegistry::instance()->add(new KisColorGenerator());
}

KritaColorGenerator::~KritaColorGenerator()
{
}

KisColorGenerator::KisColorGenerator() : KisGenerator(id(), KoID("basic"), i18n("&Solid Color..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(true);
    setSupportsPreview(true);
    setSupportsIncrementalPainting(false);
}

KisFilterConfiguration* KisColorGenerator::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration("color", 1);

    QVariant v;
    v.setValue(KoColor());
    config->setProperty("color", v);
    return config;
}

KisConfigWidget * KisColorGenerator::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, const KisImageWSP image) const
{
    Q_UNUSED(dev);
    Q_UNUSED(image);
    return new KisWdgColor(parent);
}

void KisColorGenerator::generate(KisProcessingInformation dstInfo,
                                 const QSize& size,
                                 const KisFilterConfiguration* config,
                                 KoUpdater* progressUpdater) const
{
    KisPaintDeviceSP dst = dstInfo.paintDevice();
    QPoint dstTopLeft = dstInfo.topLeft();

    Q_ASSERT(!dst.isNull());
    Q_ASSERT(config);

    QVariant value;
    KoColor c = (config && config->getProperty("color", value)) ? value.value<KoColor>() : KoColor();

    KisFillPainter gc(dst);
    gc.setProgress(progressUpdater);
    gc.setChannelFlags(config->channelFlags());
    gc.setOpacity(100);
    gc.setSelection(dstInfo.selection());
    gc.fillRect(QRect(dstInfo.topLeft(), size), c);
    gc.end();

}

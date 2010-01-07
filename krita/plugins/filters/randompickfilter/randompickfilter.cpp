/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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

#include "randompickfilter.h"
#include <stdlib.h>
#include <vector>

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
#include <KoUpdater.h>

#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_paint_device.h>
#include <kis_random_accessor.h>
#include <kis_random_generator.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>

#include "kis_wdg_random_pick.h"
#include "ui_wdgrandompickoptions.h"

K_PLUGIN_FACTORY(KritaRandomPickFilterFactory, registerPlugin<KritaRandomPickFilter>();)
K_EXPORT_PLUGIN(KritaRandomPickFilterFactory("krita"))

KritaRandomPickFilter::KritaRandomPickFilter(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    //setComponentData(KritaRandomPickFilterFactory::componentData());
    KisFilterRegistry::instance()->add(new KisFilterRandomPick());
}

KritaRandomPickFilter::~KritaRandomPickFilter()
{
}

KisFilterRandomPick::KisFilterRandomPick() : KisFilter(id(), categoryOther(), i18n("&Random Pick..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(true);
    setSupportsPreview(true);
    setSupportsIncrementalPainting(false);
}


void KisFilterRandomPick::process(KisConstProcessingInformation srcInfo,
                                  KisProcessingInformation dstInfo,
                                  const QSize& size,
                                  const KisFilterConfiguration* config,
                                  KoUpdater* progressUpdater
                                 ) const
{
    const KisPaintDeviceSP src = srcInfo.paintDevice();
    KisPaintDeviceSP dst = dstInfo.paintDevice();
    QPoint dstTopLeft = dstInfo.topLeft();
    QPoint srcTopLeft = srcInfo.topLeft();
    Q_UNUSED(config);
    Q_ASSERT(!src.isNull());
    Q_ASSERT(!dst.isNull());

    if (progressUpdater) {
        progressUpdater->setRange(0, size.width() * size.height());
    }
    int count = 0;

    const KoColorSpace * cs = src->colorSpace();

    QVariant value;
    int level = (config && config->getProperty("level", value)) ? value.toInt() : 50;
    int opacity = (config && config->getProperty("opacity", value)) ? value.toInt() : 100;

    int seedThreshold = rand();
    int seedH = rand();
    int seedV = rand();

    if (config) {
        seedThreshold = config->getInt("seedThreshold", seedThreshold);
        seedH = config->getInt("seedH", seedH);
        seedV = config->getInt("seedV", seedV);
    }
    KisRandomGenerator randT(seedThreshold);
    KisRandomGenerator randH(seedH);
    KisRandomGenerator randV(seedV);

    KisHLineIteratorPixel dstIt = dst->createHLineIterator(dstTopLeft.x(), dstTopLeft.y(), size.width(), dstInfo.selection());
    KisHLineConstIteratorPixel srcIt = src->createHLineConstIterator(srcTopLeft.x(), srcTopLeft.y(), size.width(), srcInfo.selection());
    KisRandomConstAccessorPixel srcRA = src->createRandomConstAccessor(0, 0, srcInfo.selection());

    double threshold = (100 - level) / 100.0;

    qint16 weights[2];
    weights[0] = (255 * opacity) / 100; weights[1] = 255 - weights[0];
    const quint8* pixels[2];
    KoMixColorsOp * mixOp = cs->mixColorsOp();
    for (int row = 0; row < size.height() && !(progressUpdater && progressUpdater->interrupted()); ++row) {
        while (!srcIt.isDone()) {
            if (randT.doubleRandomAt(srcIt.x(), srcIt.y()) > threshold) {
                int x = static_cast<int>(srcIt.x() + 2.5 * randH.doubleRandomAt(srcIt.x(), srcIt.y()));
                int y = static_cast<int>(srcIt.y() +  2.5 * randH.doubleRandomAt(srcIt.x(), srcIt.y()));
                srcRA.moveTo(x, y);
                pixels[0] = srcRA.oldRawData();
                pixels[1] = srcIt.oldRawData();
                mixOp->mixColors(pixels, weights, 2, dstIt.rawData());
            }
            ++srcIt;
            ++dstIt;
            if (progressUpdater) progressUpdater->setValue(++count);
        }
        srcIt.nextRow();
        dstIt.nextRow();
    }

}

KisConfigWidget * KisFilterRandomPick::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, const KisImageWSP image) const
{
    Q_UNUSED(image);
    Q_UNUSED(dev);
    return new KisWdgRandomPick((KisFilter*)this, (QWidget*)parent);
}

KisFilterConfiguration* KisFilterRandomPick::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration("randompick", 1);
    config->setProperty("level", 50);
    config->setProperty("windowsize", 2.5);
    config->setProperty("opacity", 100);
    config->setProperty("seedThreshold", rand());
    config->setProperty("seedH", rand());
    config->setProperty("seedV", rand());

    return config;
}

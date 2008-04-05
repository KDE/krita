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

#include "noisegenerator.h"

#include <qpoint.h>

#include <kis_debug.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kstandarddirs.h>

#include <KoProgressUpdater.h>
#include <KoColorSpaceRegistry.h>

#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <generator/kis_generator_registry.h>
#include <kis_global.h>
#include <kis_random_generator.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>

#include "kis_wdg_noise.h"
#include "ui_wdgnoiseoptions.h"

typedef KGenericFactory<KritaNoiseGenerator> KritaNoiseGeneratorFactory;
K_EXPORT_COMPONENT_FACTORY( kritanoisegenerator, KritaNoiseGeneratorFactory( "krita" ) )

KritaNoiseGenerator::KritaNoiseGenerator(QObject *parent, const QStringList &)
        : KParts::Plugin(parent)
{
    setComponentData(KritaNoiseGeneratorFactory::componentData());
    if (parent->inherits("KisGeneratorRegistry")) {
        KisGeneratorRegistry * manager = dynamic_cast<KisGeneratorRegistry *>(parent);
        if (manager) {
            manager->add(new KisNoiseGenerator());
        }
    }
}

KritaNoiseGenerator::~KritaNoiseGenerator()
{
}

KisNoiseGenerator::KisNoiseGenerator() : KisGenerator(id(), KoID("randomness"), i18n("&Random Noise..."))
{
    setColorSpaceIndependence( FULLY_INDEPENDENT );
    setSupportsPainting( true );
    setSupportsPreview( true );
    setSupportsIncrementalPainting( false );
}

KisFilterConfiguration* KisNoiseGenerator::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration("noise", 1);
    config->setProperty("level", 50 );
    config->setProperty("opacity", 100 );
    config->setProperty("seedThreshold", rand() );
    config->setProperty("seedRed", rand() );
    config->setProperty("seedGreen", rand() );
    config->setProperty("seedBlue", rand() );
    return config;
}

KisFilterConfigWidget * KisNoiseGenerator::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev) const
{
    Q_UNUSED(dev);
    qDebug() << "XXXXXXXXXXXX";
    return new KisWdgNoise(parent);
}

void KisNoiseGenerator::generate( KisProcessingInformation dstInfo,
                                  const QSize& size,
                                  const KisFilterConfiguration* config,
                                  KoUpdater* progressUpdater
        ) const
{
    KisPaintDeviceSP dst = dstInfo.paintDevice();
    QPoint dstTopLeft = dstInfo.topLeft();
    Q_ASSERT(!dst.isNull());

    if ( progressUpdater )
    {
        progressUpdater->setRange(0, size.width() * size.height() );
    }
    int count = 0;

    const KoColorSpace * cs = dst->colorSpace();

    QVariant value;
    int level = (config && config->getProperty("level", value)) ? value.toInt() : 50;
    int opacity = (config &&config->getProperty("opacity", value)) ? value.toInt() : 100;

    KisHLineIteratorPixel dstIt = dst->createHLineIterator(dstTopLeft.x(), dstTopLeft.y(), size.width(), dstInfo.selection());

    quint8* interm = new quint8[ cs->pixelSize() ];
    quint8* tmp = new quint8[ cs->pixelSize() ];
    double threshold = (100.0 - level) * 0.01;

    qint16 weights[2];
    weights[0] = (255 * opacity) / 100; weights[1] = 255 - weights[0];
    quint8* pixels[2];
    pixels[0] = interm;

    KoMixColorsOp * mixOp = cs->mixColorsOp();

    int seedAlpha = rand();
    int seedRed = rand();
    int seedGreen = rand();
    int seedBlue = rand();
    
    if( config )
    {
        seedAlpha = config->getInt( "seedAlpha", seedAlpha);
        seedRed = config->getInt( "seedRed", seedRed);
        seedGreen = config->getInt( "seedGreen", seedGreen);
        seedBlue = config->getInt( "seedBlue", seedBlue);
    }
    
    bool hasChannelFlags = !(config->channelFlags().isEmpty());

    KisRandomGenerator randr(seedRed);
    KisRandomGenerator randg(seedGreen);
    KisRandomGenerator randb(seedBlue);
    KisRandomGenerator randa(seedAlpha);
    
    QColor c2 = Qt::white;
    
    for ( int row = 0; row < size.height() && !(progressUpdater && progressUpdater->interrupted()); ++row ) {
        while(!dstIt.isDone() && !(progressUpdater && progressUpdater->interrupted()))
        {
            if(randa.doubleRandomAt( dstIt.x(), dstIt.y()) > threshold)
            {
                QColor c = qRgba(static_cast<int>((double)randr.doubleRandomAt( dstIt.x(), dstIt.y()) * 255),
                                static_cast<int>((double)randg.doubleRandomAt( dstIt.x(), dstIt.y()) * 255),
                                static_cast<int>((double)randb.doubleRandomAt( dstIt.x(), dstIt.y()) * 255),
                                static_cast<int>((double)randa.doubleRandomAt( dstIt.x(), dstIt.y()) * 255));
                                
                cs->fromQColor( c, interm, 0);
                cs->fromQColor( c2, pixels[1], 0);
                c2 = c;
                
                if (!hasChannelFlags) {
                    mixOp->mixColors( pixels, weights, 2,  dstIt.rawData() );
                }
                else {
                    mixOp->mixColors( pixels, weights, 2,  tmp );

                    for (int i = 0; i < config->channelFlags().size(); ++i) {
                        if (config->channelFlags().testBit(i)) {
                            dstIt.rawData()[i] = tmp[i]; // XXX: use the actual channel size!
                        }
                    }
                }
            }
            ++dstIt;
            ++dstIt;
            if(progressUpdater) progressUpdater->setValue( ++count );
        }
        dstIt.nextRow();
        dstIt.nextRow();
    }

    delete [] interm;
    delete [] tmp;
}

const KoColorSpace * KisNoiseGenerator::colorSpace()
{
    return KoColorSpaceRegistry::instance()->rgb8();
}

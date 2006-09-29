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

#include "noisefilter.h"

#include <stdlib.h>
#include <vector>

#include <qpoint.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kstandarddirs.h>
#include <ktempfile.h>

#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_filter_registry.h>
#include <kis_global.h>
#include <kis_types.h>

#include "kis_wdg_noise.h"
#include "ui_wdgnoiseoptions.h"

typedef KGenericFactory<KritaNoiseFilter> KritaNoiseFilterFactory;
K_EXPORT_COMPONENT_FACTORY( kritanoisefilter, KritaNoiseFilterFactory( "krita" ) )

KritaNoiseFilter::KritaNoiseFilter(QObject *parent, const QStringList &)
        : KParts::Plugin(parent)
{
    setInstance(KritaNoiseFilterFactory::instance());


    if (parent->inherits("KisFilterRegistry")) {
        KisFilterRegistry * manager = dynamic_cast<KisFilterRegistry *>(parent);
        manager->add(KisFilterSP(new KisFilterNoise()));
    }
}

KritaNoiseFilter::~KritaNoiseFilter()
{
}

KisFilterNoise::KisFilterNoise() : KisFilter(id(), "other", i18n("&Random Noise..."))
{
}

KisFilterConfiguration* KisFilterNoise::configuration(QWidget* w)
{
    KisWdgNoise* wN = dynamic_cast<KisWdgNoise*>(w);
    KisFilterConfiguration* config = new KisFilterConfiguration(id().id(), 1);
    if(wN)
    {
        config->setProperty("level", wN->widget()->intLevel->value() );
        config->setProperty("opacity", wN->widget()->intOpacity->value() );
    }
    return config;
}

KisFilterConfigWidget * KisFilterNoise::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev)
{
    return new KisWdgNoise((KisFilter*)this, (QWidget*)parent, i18n("Configuration of noise filter").ascii());
}

void KisFilterNoise::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* config, const QRect& rect)
{
/*    Q_ASSERT(src != 0);
    Q_ASSERT(dst != 0);*/
    
    setProgressTotalSteps(rect.width() * rect.height());

    KoColorSpace * cs = src->colorSpace();
    Q_INT32 psize = cs->pixelSize();
    
    QVariant value;
    int level = (config && config->getProperty("level", value)) ? value.toInt() : 50;
    int opacity = (config && config->getProperty("opacity", value)) ? value.toInt() : 100;
    
    KisRectIteratorPixel dstIt = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );
    KisRectIteratorPixel srcIt = src->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), false);
    
    Q_UINT8* interm = new Q_UINT8[ cs->pixelSize() ];
    Q_UINT32 threshold = (RAND_MAX / 100) * (100 - level);
    
    Q_UINT8 weights[2];
    weights[0] = (255 * opacity) / 100; weights[1] = 255 - weights[0];
    const Q_UINT8* pixels[2];
    pixels[0] = interm;
    while(!srcIt.isDone())
    {
        if(rand() > threshold)
        {
            QColor c = qRgb((double)rand()/RAND_MAX * 255,(double)rand()/RAND_MAX * 255,(double)rand()/RAND_MAX * 255);
            cs->fromQColor( c, interm, 0 );
            pixels[1] = srcIt.oldRawData();
            cs->mixColors( pixels, weights, 2, dstIt.rawData() );
        }
        ++srcIt;
        ++dstIt;
        incProgress();
    }
    
    delete [] interm;
    setProgressDone(); // Must be called even if you don't really support progression
}

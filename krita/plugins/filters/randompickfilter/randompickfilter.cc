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
#include <kis_random_accessor.h>
#include <kis_layer.h>
#include <kis_filter_registry.h>
#include <kis_global.h>
#include <kis_types.h>

#include "kis_wdg_random_pick.h"
#include "wdgrandompickoptions.h"

typedef KGenericFactory<KritaRandomPickFilter> KritaRandomPickFilterFactory;
K_EXPORT_COMPONENT_FACTORY( kritarandompickfilter, KritaRandomPickFilterFactory( "krita" ) )

KritaRandomPickFilter::KritaRandomPickFilter(QObject *parent, const char *name, const QStringList &)
        : KParts::Plugin(parent, name)
{
    setInstance(KritaRandomPickFilterFactory::instance());


    if (parent->inherits("KisFilterRegistry")) {
        KisFilterRegistry * manager = dynamic_cast<KisFilterRegistry *>(parent);
        manager->add(new KisFilterRandomPick());
    }
}

KritaRandomPickFilter::~KritaRandomPickFilter()
{
}

KisFilterRandomPick::KisFilterRandomPick() : KisFilter(id(), "other", i18n("&Random Pick..."))
{
}

KisFilterConfiguration* KisFilterRandomPick::configuration(QWidget* w)
{
    KisWdgRandomPick* wN = dynamic_cast<KisWdgRandomPick*>(w);
    KisFilterConfiguration* config = new KisFilterConfiguration(id().id(), 1);
    if(wN)
    {
        config->setProperty("level", wN->widget()->intLevel->value() );
        config->setProperty("windowsize", wN->widget()->intWindowSize->value() );
        config->setProperty("opacity", wN->widget()->intOpacity->value() );
    }
    return config;
}

KisFilterConfigWidget * KisFilterRandomPick::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev)
{
    return new KisWdgRandomPick((KisFilter*)this, (QWidget*)parent, i18n("Configuration of random pick filter").ascii());
}

void KisFilterRandomPick::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* config, const QRect& rect)
{
    Q_ASSERT(src != 0);
    Q_ASSERT(dst != 0);
    
    setProgressTotalSteps(rect.height() * rect.width());

    KisColorSpace * cs = src->colorSpace();
    Q_INT32 psize = cs->pixelSize();
    
    QVariant value;
    int level = (config && config->getProperty("level", value)) ? value.toInt() : 50;
    double windowsize = (config && config->getProperty("windowsize", value)) ? value.toInt() / 2. : 2.5;
    int opacity = (config && config->getProperty("opacity", value)) ? value.toInt() : 100;
    
    KisRectIteratorPixel dstIt = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );
    KisRectIteratorPixel srcIt = src->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), false);
    KisRandomAccessorPixel srcRA = src->createRandomAccessor(0, 0, false);
    
    Q_UINT32 threshold = (RAND_MAX / 100) * (100 - level);
    
    Q_UINT8 weights[2];
    weights[0] = (255 * opacity) / 100; weights[1] = 255 - weights[0];
    const Q_UINT8* pixels[2];
    while(!srcIt.isDone())
    {
        if(rand() > threshold)
        {
            int x = srcIt.x() + 2.5 * rand() / RAND_MAX;
            int y = srcIt.y() +  2.5 * rand() / RAND_MAX;
            srcRA.moveTo( x, y);
            pixels[0] = srcRA.oldRawData();
            pixels[1] = srcIt.oldRawData();
            cs->mixColors( pixels, weights, 2, dstIt.rawData() );
        }
        ++srcIt;
        ++dstIt;
        incProgress();
    }
    
    setProgressDone(); // Must be called even if you don't really support progression
}

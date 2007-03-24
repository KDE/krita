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
#include <kcomponentdata.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kstandarddirs.h>

#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_random_accessor.h>
#include <kis_layer.h>
#include <kis_filter_registry.h>
#include <kis_global.h>
#include <kis_types.h>

#include "kis_wdg_random_pick.h"
#include "ui_wdgrandompickoptions.h"

typedef KGenericFactory<KritaRandomPickFilter> KritaRandomPickFilterFactory;
K_EXPORT_COMPONENT_FACTORY( kritarandompickfilter, KritaRandomPickFilterFactory( "krita" ) )

KritaRandomPickFilter::KritaRandomPickFilter(QObject *parent, const QStringList &)
        : KParts::Plugin(parent)
{
    setComponentData(KritaRandomPickFilterFactory::componentData());


    if (parent->inherits("KisFilterRegistry")) {
        KisFilterRegistry * manager = dynamic_cast<KisFilterRegistry *>(parent);
        manager->add(KisFilterSP(new KisFilterRandomPick()));
    }
}

KritaRandomPickFilter::~KritaRandomPickFilter()
{
}

KisFilterRandomPick::KisFilterRandomPick() : KisFilter(id(), "other", i18n("&Random Pick..."))
{
}

KisFilterConfigWidget * KisFilterRandomPick::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev)
{
    Q_UNUSED(dev);
    return new KisWdgRandomPick((KisFilter*)this, (QWidget*)parent);
}

KisFilterConfiguration* KisFilterRandomPick::designerConfiguration(const KisPaintDeviceSP)
{
    KisFilterConfiguration* config = new KisFilterConfiguration("randompick", 1);
    config->setProperty("level", 50 );
    config->setProperty("windowsize", 2.5 );
    config->setProperty("opacity", 100 );
    return config;
}

void KisFilterRandomPick::process(const KisPaintDeviceSP src, const QPoint& srcTopLeft, KisPaintDeviceSP dst, const QPoint& dstTopLeft, const QSize& size, KisFilterConfiguration* config)
{
    Q_ASSERT(src != 0);
    Q_ASSERT(dst != 0);

    setProgressTotalSteps(size.height() * size.width());

    KoColorSpace * cs = src->colorSpace();

    QVariant value;
    int level = (config && config->getProperty("level", value)) ? value.toInt() : 50;
    int opacity = (config && config->getProperty("opacity", value)) ? value.toInt() : 100;

    KisRectIteratorPixel dstIt = dst->createRectIterator(dstTopLeft.x(), dstTopLeft.y(), size.width(), size.height());
    KisRectConstIteratorPixel srcIt = src->createRectConstIterator(srcTopLeft.x(), srcTopLeft.y(), size.width(), size.height());
    KisRandomConstAccessorPixel srcRA = src->createRandomConstAccessor(0, 0);

    Q_INT32 threshold = (RAND_MAX / 100) * (100 - level);

    Q_UINT8 weights[2];
    weights[0] = (255 * opacity) / 100; weights[1] = 255 - weights[0];
    const Q_UINT8* pixels[2];
    KoMixColorsOp * mixOp = cs->mixColorsOp();
    while(!srcIt.isDone())
    {
        if(rand() > threshold)
        {
            int x = static_cast<int>(srcIt.x() + 2.5 * rand() / RAND_MAX);
            int y = static_cast<int>(srcIt.y() +  2.5 * rand() / RAND_MAX);
            srcRA.moveTo( x, y);
            pixels[0] = srcRA.oldRawData();
            pixels[1] = srcIt.oldRawData();
            mixOp->mixColors( pixels, weights, 2, dstIt.rawData() );
        }
        ++srcIt;
        ++dstIt;
        incProgress();
    }

    setProgressDone(); // Must be called even if you don't really support progression
}

/*
 * This file is part of the KDE project
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  Inspired by a similar plugin for the digikam project from:
 *  Copyright (c) 2005 Gilles Caulier <caulier dot gilles at kdemail dot net>
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

#include "lenscorrectionfilter.h"

#include <stdlib.h>
#include <vector>

#include <qpoint.h>

#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kstandarddirs.h>
#include <ktempfile.h>

#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_filter_registry.h>
#include <kis_global.h>
#include <kis_layer.h>
#include <kis_random_sub_accessor.h>
#include <kis_types.h>

#include "kis_wdg_lens_correction.h"
#include "ui_wdglenscorrectionoptions.h"

typedef KGenericFactory<KritaLensCorrectionFilter> KritaLensCorrectionFilterFactory;
K_EXPORT_COMPONENT_FACTORY( kritalenscorrectionfilter, KritaLensCorrectionFilterFactory( "krita" ) )

KritaLensCorrectionFilter::KritaLensCorrectionFilter(QObject *parent, const QStringList &)
        : KParts::Plugin(parent)
{
    setInstance(KritaLensCorrectionFilterFactory::instance());


    if (parent->inherits("KisFilterRegistry")) {
        KisFilterRegistry * manager = dynamic_cast<KisFilterRegistry *>(parent);
        manager->add(KisFilterSP(new KisFilterLensCorrection()));
    }
}

KritaLensCorrectionFilter::~KritaLensCorrectionFilter()
{
}

KisFilterLensCorrection::KisFilterLensCorrection() : KisFilter(id(), "other", i18n("&Lens Correction..."))
{
}

KisFilterConfiguration* KisFilterLensCorrection::configuration(QWidget* w)
{
    QVariant value;
    KisWdgLensCorrection* wN = dynamic_cast<KisWdgLensCorrection*>(w);
    KisFilterConfiguration* config = new KisFilterConfiguration(id().id(), 1);
    if(wN)
    {
        config->setProperty("xcenter", wN->widget()->intXCenter->value() );
        config->setProperty("ycenter", wN->widget()->intYCenter->value() );
        config->setProperty("correctionnearcenter", wN->widget()->dblCorrectionNearCenter->value() );
        config->setProperty("correctionnearedges", wN->widget()->dblCorrectionNearEdges->value() );
        config->setProperty("brightness", wN->widget()->dblBrightness->value() );
    }
    return config;
}

KisFilterConfigWidget * KisFilterLensCorrection::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP /*dev*/)
{
    return new KisWdgLensCorrection((KisFilter*)this, (QWidget*)parent, i18n("Configuration of lens correction filter").ascii());
}

void KisFilterLensCorrection::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* config, const QRect& rawrect)
{
//     Q_ASSERT(src != 0);
//     Q_ASSERT(dst != 0);

    QRect layerrect = src->exactBounds();

    QRect workingrect = layerrect.intersect( rawrect );

    setProgressTotalSteps(workingrect.width() * workingrect.height());

    KoColorSpace* cs = dst->colorSpace();

    QVariant value;
    double xcenter = (config && config->getProperty("xcenter", value)) ? value.toInt() : 50;
    double ycenter = (config && config->getProperty("ycenter", value)) ? value.toInt() : 50;
    double correctionnearcenter = (config && config->getProperty("correctionnearcenter", value)) ? value.toDouble() : 0.;
    double correctionnearedges = (config && config->getProperty("correctionnearedges", value)) ? value.toDouble() : 0.;
    double brightness = ( (config && config->getProperty("brightness", value)) ? value.toDouble() : 0. );

    KisRectIteratorPixel dstIt = dst->createRectIterator(workingrect.x(), workingrect.y(), workingrect.width(), workingrect.height(), true );
    KisRandomSubAccessorPixel srcRSA = src->createRandomSubAccessor();

    double normallise_radius_sq = 4.0 / (layerrect.width() * layerrect.width() + layerrect.height() * layerrect.height());
    xcenter = layerrect.x() + layerrect.width() * xcenter / 100.0;
    ycenter = layerrect.y() + layerrect.height() * ycenter / 100.0;
    double mult_sq = correctionnearcenter / 200.0;
    double mult_qd = correctionnearedges / 200.0;

    Q_UINT16 lab[4];

    while(!dstIt.isDone())
    {
        double off_x = dstIt.x() - xcenter;
        double off_y = dstIt.y() - ycenter;
        double radius_sq = ( (off_x * off_x) + (off_y * off_y) ) * normallise_radius_sq;

        double radius_mult = radius_sq * mult_sq + radius_sq * radius_sq * mult_qd;
        double mag = radius_mult;
        radius_mult += 1.0;

        double srcX = xcenter + radius_mult * off_x;
        double srcY = ycenter + radius_mult * off_y;

        double brighten = 1.0 + mag * brightness;

        srcRSA.moveTo( KisPoint( srcX, srcY ) );
        srcRSA.sampledOldRawData( dstIt.rawData() );
        cs->toLabA16( dstIt.rawData(), (Q_UINT8*)lab, 1);
#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))

        lab[0] = CLAMP( lab[0] * static_cast<Q_UINT16>( brighten ), 0, 65535);
        cs->fromLabA16( (Q_UINT8*)lab, dstIt.rawData(), 1);

        ++dstIt;
        incProgress();
    }
    setProgressDone(); // Must be called even if you don't really support progression
}

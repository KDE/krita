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

#include <kis_debug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kstandarddirs.h>

#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include <kis_paint_device.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include <kis_layer.h>
#include <kis_random_sub_accessor.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>

#include "kis_wdg_lens_correction.h"
#include "ui_wdglenscorrectionoptions.h"

typedef KGenericFactory<KritaLensCorrectionFilter> KritaLensCorrectionFilterFactory;
K_EXPORT_COMPONENT_FACTORY(kritalenscorrectionfilter, KritaLensCorrectionFilterFactory("krita"))

KritaLensCorrectionFilter::KritaLensCorrectionFilter(QObject *parent, const QStringList &)
        : QObject(parent)
{
    //setComponentData(KritaLensCorrectionFilterFactory::componentData());
    KisFilterRegistry::instance()->add(new KisFilterLensCorrection());

}

KritaLensCorrectionFilter::~KritaLensCorrectionFilter()
{
}

KisFilterLensCorrection::KisFilterLensCorrection() : KisFilter(id(), categoryOther(), i18n("&Lens Correction..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(true);
    setSupportsPreview(true);
    setSupportsIncrementalPainting(false);
    setSupportsAdjustmentLayers(false);
}

KisFilterConfiguration* KisFilterLensCorrection::factoryConfiguration(const KisPaintDeviceSP) const
{
    QVariant value;
    KisFilterConfiguration* config = new KisFilterConfiguration("lenscorrection", 1);
    config->setProperty("xcenter", 50);
    config->setProperty("ycenter", 50);
    config->setProperty("correctionnearcenter", 0.);
    config->setProperty("correctionnearedges", 0.);
    config->setProperty("brightness", 0.);
    return config;
}

KisConfigWidget * KisFilterLensCorrection::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP , const KisImageWSP) const
{
    return new KisWdgLensCorrection((KisFilter*)this, (QWidget*)parent);
}

void KisFilterLensCorrection::process(KisConstProcessingInformation srcInfo,
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

    Q_ASSERT(src != 0);
    Q_ASSERT(dst != 0);

    QRect layerrect = src->exactBounds();

    QRect workingrect = layerrect.intersect(QRect(srcTopLeft, size));

    int cost = (size.width() * size.height()) / 100;
    if (cost == 0) cost = 1;
    int count = 0;

    KoColorSpace* cs = dst->colorSpace();

    QVariant value;
    double xcenter = (config && config->getProperty("xcenter", value)) ? value.toInt() : 50;
    double ycenter = (config && config->getProperty("ycenter", value)) ? value.toInt() : 50;
    double correctionnearcenter = (config && config->getProperty("correctionnearcenter", value)) ? value.toDouble() : 0.;
    double correctionnearedges = (config && config->getProperty("correctionnearedges", value)) ? value.toDouble() : 0.;
    double brightness = ((config && config->getProperty("brightness", value)) ? value.toDouble() : 0.);

    KisRectIteratorPixel dstIt = dst->createRectIterator(dstTopLeft.x(), dstTopLeft.y(), workingrect.width(), workingrect.height(), dstInfo.selection());
    KisRandomSubAccessorPixel srcRSA = src->createRandomSubAccessor(srcInfo.selection());

    double normallise_radius_sq = 4.0 / (layerrect.width() * layerrect.width() + layerrect.height() * layerrect.height());
    xcenter = layerrect.x() + layerrect.width() * xcenter / 100.0;
    ycenter = layerrect.y() + layerrect.height() * ycenter / 100.0;
    double mult_sq = correctionnearcenter / 200.0;
    double mult_qd = correctionnearedges / 200.0;

    quint16 lab[4];

    int tx = dstTopLeft.x() - srcTopLeft.x();
    int ty = dstTopLeft.y() - srcTopLeft.y();

    while (!dstIt.isDone()) {
        double off_x = dstIt.x() - xcenter;
        double off_y = dstIt.y() - ycenter;
        double radius_sq = ((off_x * off_x) + (off_y * off_y)) * normallise_radius_sq;

        double radius_mult = radius_sq * mult_sq + radius_sq * radius_sq * mult_qd;
        double mag = radius_mult;
        radius_mult += 1.0;

        double srcX = xcenter + radius_mult * off_x;
        double srcY = ycenter + radius_mult * off_y;

        double brighten = 1.0 + mag * brightness;

        srcRSA.moveTo(QPointF(srcX + tx, srcY + ty));
        srcRSA.sampledOldRawData(dstIt.rawData());
        cs->toLabA16(dstIt.rawData(), (quint8*)lab, 1);
#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))

        lab[0] = CLAMP(lab[0] * static_cast<quint16>(brighten), 0, 65535);
        cs->fromLabA16((quint8*)lab, dstIt.rawData(), 1);

        ++dstIt;
        if (progressUpdater) progressUpdater->setProgress((++count) / cost);
    }
}

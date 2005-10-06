/*
 * This file is part of Krita
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
  * Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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

#include <math.h>

#include <klocale.h>

#include <qlayout.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qlabel.h>

#include "kis_filter_config_widget.h"
#include "kis_brightness_contrast_filter.h"
#include "wdg_brightness_contrast.h"
#include "kis_colorspace.h"
#include "kis_paint_device_impl.h"
#include "kis_iterators_pixel.h"
#include "kis_iterator.h"
#include "kcurve.h"
#include "kis_histogram.h"
#include "kis_basic_histogram_producers.h"

KisBrightnessContrastFilterConfiguration::KisBrightnessContrastFilterConfiguration()
{
}

KisBrightnessContrastFilter::KisBrightnessContrastFilter()
    : KisFilter( id(), "adjust", "&Brightness/contrast...")
{

}

KisFilterConfigWidget * KisBrightnessContrastFilter::createConfigurationWidget(QWidget *parent, KisPaintDeviceImplSP dev)
{
    return new KisBrightnessContrastConfigWidget(parent, dev);
}

KisFilterConfiguration* KisBrightnessContrastFilter::configuration(QWidget *nwidget, KisPaintDeviceImplSP)
{
    KisBrightnessContrastConfigWidget* widget = (KisBrightnessContrastConfigWidget*)nwidget;

    if ( widget == 0 )
    {
        return new KisBrightnessContrastFilterConfiguration();
    } else {
        return widget->config();
    }
}

std::list<KisFilterConfiguration*> KisBrightnessContrastFilter::listOfExamplesConfiguration(KisPaintDeviceImplSP /*dev*/)
{
//XXX should really come up with a list of configurations
    std::list<KisFilterConfiguration*> list;
    list.insert(list.begin(), new KisBrightnessContrastFilterConfiguration( ));
    return list;
}


void KisBrightnessContrastFilter::process(KisPaintDeviceImplSP src, KisPaintDeviceImplSP dst, KisFilterConfiguration* config, const QRect& rect)
{
    KisBrightnessContrastFilterConfiguration* configBC = (KisBrightnessContrastFilterConfiguration*) config;

    KisColorAdjustment *adj = src->colorSpace()->createBrightnessContrastAdjustment(configBC->transfer);

    KisRectIteratorPixel dstIt = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );
    KisRectIteratorPixel srcIt = src->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), false);

    setProgressTotalSteps(rect.width() * rect.height());
    Q_INT32 pixelsProcessed = 0;

    while( ! srcIt.isDone()  && !cancelRequested())
    {
        Q_UINT32 npix;
        npix = srcIt.nConseqPixels();

        // change the brightness and contrast
        src->colorSpace()->applyAdjustment(srcIt.oldRawData(), dstIt.rawData(), adj, npix);

        srcIt+=npix;
        dstIt+=npix;

        pixelsProcessed++;
        setProgress(pixelsProcessed);
    }

    setProgressDone();
}

KisBrightnessContrastConfigWidget::KisBrightnessContrastConfigWidget(QWidget * parent, KisPaintDeviceImplSP dev, const char * name, WFlags f)
    : KisFilterConfigWidget(parent, name, f)
{
    int i;
    int height;
    m_page = new WdgBrightnessContrast(this);
    QHBoxLayout * l = new QHBoxLayout(this);
    Q_CHECK_PTR(l);

    l -> add(m_page);
    height = 256;
    connect( m_page->kCurve, SIGNAL(modified()), SIGNAL(sigPleaseUpdatePreview()));

    // Create the horizontal gradient label
    QPixmap hgradientpix(256, 1);
    QPainter hgp(&hgradientpix);
    hgp.setPen(QPen::QPen(QColor(0,0,0),1, Qt::SolidLine));
    for( i=0; i<256; ++i )
    {
        hgp.setPen(QColor(i,i,i));
        hgp.drawPoint(i, 0);
    }
    m_page->hgradient->setPixmap(hgradientpix);

    // Create the vertical gradient label
    QPixmap vgradientpix(1, 256);
    QPainter vgp(&vgradientpix);
    vgp.setPen(QPen::QPen(QColor(0,0,0),1, Qt::SolidLine));
    for( i=0; i<256; ++i )
    {
        vgp.setPen(QColor(i,i,i));
        vgp.drawPoint(0, 255-i);
    }
    m_page->vgradient->setPixmap(vgradientpix);

    KisHistogramProducerSP producer = new KisGenericLightnessHistogramProducer();
    KisHistogram histogram(dev, producer, LINEAR);
    QPixmap pix(256, height);
    pix.fill();
    QPainter p(&pix);
    p.setPen(QPen::QPen(Qt::gray,1, Qt::SolidLine));

    double highest = (double)histogram.calculations().getHighest();
    Q_INT32 bins = histogram.producer() -> numberOfBins();

    if (histogram.getHistogramType() == LINEAR) {
        double factor = (double)height / highest;
        for( i=0; i<bins; ++i ) {
            p.drawLine(i, height, i, height - int(histogram.getValue(i) * factor));
        }
    } else {
        double factor = (double)height / (double)log(highest);
        for( i = 0; i < bins; ++i ) {
            p.drawLine(i, height, i, height - int(log((double)histogram.getValue(i)) * factor));
        }
    }

    m_page->kCurve->setPixmap(pix);

}

KisBrightnessContrastFilterConfiguration * KisBrightnessContrastConfigWidget::config()
{
    KisBrightnessContrastFilterConfiguration * cfg = new KisBrightnessContrastFilterConfiguration();

    for(int i=0; i <256; i++)
    {
        Q_INT32 val;
        val = int(0xFFFF * m_page->kCurve->getCurveValue( i / 255.0));
        if(val >0xFFFF)
            val=0xFFFF;
        if(val <0)
            val = 0;

        cfg->transfer[i] = val;
    }

    return cfg;
}

/*
 * This file is part of Krita
 *
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
#include <qlayout.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qlabel.h>
#include <qcombobox.h>

#include "kis_filter_config_widget.h"
#include "kis_perchannel_filter.h"
#include "wdg_perchannel.h"
#include "kis_colorspace.h"
#include "kis_paint_device_impl.h"
#include "kis_iterators_pixel.h"
#include "kcurve.h"
#include "kis_histogram.h"
#include "kis_basic_histogram_producers.h"


KisPerChannelFilterConfiguration::KisPerChannelFilterConfiguration()
{
}

KisPerChannelFilter::KisPerChannelFilter()
    : KisFilter( id(), "adjust", "&Color Adjustment...")
{

}

KisFilterConfigWidget * KisPerChannelFilter::createConfigurationWidget(QWidget *parent, KisPaintDeviceImplSP dev)
{
    return new KisPerChannelConfigWidget(parent, dev);
}

KisFilterConfiguration* KisPerChannelFilter::configuration(QWidget *nwidget, KisPaintDeviceImplSP)
{
    KisPerChannelConfigWidget* widget = (KisPerChannelConfigWidget*)nwidget;

    if ( widget == 0 )
    {
        return new KisPerChannelFilterConfiguration();
    } else {
        return widget->config();
    }
}

std::list<KisFilterConfiguration*> KisPerChannelFilter::listOfExamplesConfiguration(KisPaintDeviceImplSP /*dev*/)
{
//XXX should really come up with a list of configurations
    std::list<KisFilterConfiguration*> list;
    list.insert(list.begin(), new KisPerChannelFilterConfiguration( ));
    return list;
}


void KisPerChannelFilter::process(KisPaintDeviceImplSP src, KisPaintDeviceImplSP dst, KisFilterConfiguration* config, const QRect& rect)
{
    KisPerChannelFilterConfiguration* configBC = (KisPerChannelFilterConfiguration*) config;

    KisColorAdjustment *adj = src->colorSpace()->createBrightnessContrastAdjustment(configBC->transfer);

    KisRectIteratorPixel dstIt = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );
    KisRectIteratorPixel srcIt = src->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), false);

    setProgressTotalSteps(rect.width() * rect.height());
    Q_INT32 pixelsProcessed = 0;

    while( ! srcIt.isDone()  && !cancelRequested())
    {
        Q_UINT32 npix;
        npix = srcIt.nConseqPixels();

        // change the channel values
        src->colorSpace()->applyAdjustment(srcIt.oldRawData(), dstIt.rawData(), adj, npix);

        srcIt+=npix;
        dstIt+=npix;

        pixelsProcessed++;
        setProgress(pixelsProcessed);
    }

    setProgressDone();
}

void KisPerChannelConfigWidget::setActiveChannel(int ch)
{
    int i;
    int height = 256;
printf("setactivechannel\n");
    QPixmap pix(256, height);
    pix.fill();
    QPainter p(&pix);
    p.setPen(QPen::QPen(Qt::gray,1, Qt::SolidLine));

    m_histogram->setChannel(ch);

    double highest = (double)m_histogram->calculations().getHighest();
    Q_INT32 bins = m_histogram->producer() -> numberOfBins();

    if (m_histogram->getHistogramType() == LINEAR) {
        double factor = (double)height / highest;
        for( i=0; i<bins; ++i ) {
            p.drawLine(i, height, i, height - int(m_histogram->getValue(i) * factor));
        }
    } else {
        double factor = (double)height / (double)log(highest);
        for( i = 0; i < bins; ++i ) {
            p.drawLine(i, height, i, height - int(log((double)m_histogram->getValue(i)) * factor));
        }
    }

    m_page->kCurve->setPixmap(pix);
}

KisPerChannelConfigWidget::KisPerChannelConfigWidget(QWidget * parent, KisPaintDeviceImplSP dev, const char * name, WFlags f)
    : KisFilterConfigWidget(parent, name, f)
{
    int i;
    int height;
    m_page = new WdgPerChannel(this);
    QHBoxLayout * l = new QHBoxLayout(this);
    Q_CHECK_PTR(l);

    m_dev = dev;

    l -> add(m_page);
    height = 256;
    connect( m_page->kCurve, SIGNAL(modified()), SIGNAL(sigPleaseUpdatePreview()));

    // Fill in the channel chooser
    QValueVector<KisChannelInfo *> channels = dev->colorSpace()->channels();
    for(i=0; i < dev->colorSpace()->nColorChannels(); i++)
        m_page->cmbChannel -> insertItem(channels.at(i)->name());
    connect( m_page->cmbChannel, SIGNAL(activated(int)), this, SLOT(setActiveChannel(int)));

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

    KisIDList keys = 
        KisHistogramProducerFactoryRegistry::instance()->listKeysCompatibleWith(m_dev->colorSpace());
    KisHistogramProducerFactory *hpf = KisHistogramProducerFactoryRegistry::instance()->get(*(keys.at(0)));
    m_histogram = new KisHistogram(m_dev, hpf->generate(), LINEAR);

    setActiveChannel(0);
}

KisPerChannelFilterConfiguration * KisPerChannelConfigWidget::config()
{
    KisPerChannelFilterConfiguration * cfg = new KisPerChannelFilterConfiguration();

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

#include "kis_perchannel_filter.moc"

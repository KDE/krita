/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Frederic Coiffier <fcoiffie@gmail.com>
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
#include <qspinbox.h>

#include "kis_filter_config_widget.h"
#include "kis_level_filter.h"
#include "wdg_level.h"
#include "kis_colorspace.h"
#include "kis_paint_device.h"
#include "kis_iterators_pixel.h"
#include "kis_iterator.h"
#include "kis_histogram.h"
#include "kis_basic_histogram_producers.h"
#include "kis_painter.h"
#include "kgradientslider.h"

KisLevelFilterConfiguration::KisLevelFilterConfiguration()
    : KisFilterConfiguration( "levels", 1 )
{
    whitevalue = 255;
    blackvalue = 0;
    gammavalue = 1.0;

    outwhitevalue = 0xFFFF;
    outblackvalue = 0;

    m_adjustment = 0;
}

KisLevelFilterConfiguration::~KisLevelFilterConfiguration()
{
    delete m_adjustment;
}

void KisLevelFilterConfiguration::fromXML( const QString& s )
{
    KisFilterConfiguration::fromXML(s);
    blackvalue = getInt( "blackvalue" );
    whitevalue = getInt( "whitevalue" );
    gammavalue = getDouble( "gammavalue" );
    outblackvalue = getInt( "outblackvalue" );
    outwhitevalue = getInt( "outwhitevalue" );
}

QString KisLevelFilterConfiguration::toString()
{
    m_properties.clear();
    setProperty("blackvalue", blackvalue);
    setProperty("whitevalue", whitevalue);
    setProperty("gammavalue", gammavalue);
    setProperty("outblackvalue", outblackvalue);
    setProperty("outwhitevalue", outwhitevalue);

    return KisFilterConfiguration::toString();
}

KisLevelFilter::KisLevelFilter()
    : KisFilter( id(), "adjust", i18n("&Levels"))
{

}

KisFilterConfigWidget * KisLevelFilter::createConfigurationWidget(QWidget *parent, KisPaintDeviceSP dev)
{
    return new KisLevelConfigWidget(parent, dev);
}

KisFilterConfiguration* KisLevelFilter::configuration(QWidget *nwidget)
{
    KisLevelConfigWidget* widget = (KisLevelConfigWidget*)nwidget;

    if ( widget == 0 )
    {
        return new KisLevelFilterConfiguration();
    } else {
        return widget->config();
    }
}

std::list<KisFilterConfiguration*> KisLevelFilter::listOfExamplesConfiguration(KisPaintDeviceSP /*dev*/)
{
    //XXX should really come up with a list of configurations
    std::list<KisFilterConfiguration*> list;
    list.insert(list.begin(), new KisLevelFilterConfiguration( ));
    return list;
}

bool KisLevelFilter::workWith(KisColorSpace* cs)
{
    return (cs->getProfile() != 0);
}


void KisLevelFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* config, const QRect& rect)
{
    
    if (!config) {
        kdWarning() << "No configuration object for level filter\n";
        return;
    }
    
    KisLevelFilterConfiguration* configBC = (KisLevelFilterConfiguration*) config;
    Q_ASSERT(config);

    if (src!=dst) {
        KisPainter gc(dst);
        gc.bitBlt(rect.x(), rect.y(), COMPOSITE_COPY, src, rect.x(), rect.y(), rect.width(), rect.height());
        gc.end();
    }

    if (configBC->m_adjustment == 0) {
        Q_UINT16 transfer[256];
        for (int i = 0; i < 256; i++) {
            if (i <= configBC->blackvalue)
                transfer[i] = configBC->outblackvalue;
            else if (i < configBC->whitevalue)
            {
                double a = (double)(i - configBC->blackvalue) / (double)(configBC->whitevalue - configBC->blackvalue);
                a = (double)(configBC->outwhitevalue - configBC->outblackvalue) * pow (a, (1.0 / configBC->gammavalue));
                transfer[i] = int(configBC->outblackvalue + a);
            }
            else
                transfer[i] = configBC->outwhitevalue;
        }
        configBC->m_adjustment = src->colorSpace()->createBrightnessContrastAdjustment(transfer);
    }
    
    KisRectIteratorPixel iter = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );

    setProgressTotalSteps(rect.width() * rect.height());
    Q_INT32 pixelsProcessed = 0;

    while( ! iter.isDone()  && !cancelRequested())
    {
        Q_UINT32 npix=0, maxpix = iter.nConseqPixels();
        Q_UINT8 selectedness = iter.selectedness();
        // The idea here is to handle stretches of completely selected and completely unselected pixels.
        // Partially selected pixels are handled one pixel at a time.
        switch(selectedness)
        {
            case MIN_SELECTED:
                while(iter.selectedness()==MIN_SELECTED && maxpix)
                {
                    --maxpix;
                    ++iter;
                    ++npix;
                }
                pixelsProcessed += npix;
                break;

            case MAX_SELECTED:
            {
                Q_UINT8 *firstPixel = iter.rawData();
                while(iter.selectedness()==MAX_SELECTED && maxpix)
                {
                    --maxpix;
                    if (maxpix != 0)
                        ++iter;
                    ++npix;
                }
                // adjust
                src->colorSpace()->applyAdjustment(firstPixel, firstPixel, configBC->m_adjustment, npix);
                pixelsProcessed += npix;
                ++iter;
                break;
            }

            default:
                // adjust, but since it's partially selected we also only partially adjust
                src->colorSpace()->applyAdjustment(iter.oldRawData(), iter.rawData(), configBC->m_adjustment, 1);
                const Q_UINT8 *pixels[2] = {iter.oldRawData(), iter.rawData()};
                Q_UINT8 weights[2] = {MAX_SELECTED - selectedness, selectedness};
                src->colorSpace()->mixColors(pixels, weights, 2, iter.rawData());
                ++iter;
                pixelsProcessed++;
                break;
        }
        setProgress(pixelsProcessed);
    }

    setProgressDone();
}

KisLevelConfigWidget::KisLevelConfigWidget(QWidget * parent, KisPaintDeviceSP dev, const char * name, WFlags f)
    : KisFilterConfigWidget(parent, name, f)
{
    m_page = new WdgLevel(this);
    histogram = NULL;

    m_page->ingradient->enableGamma(true);
    m_page->blackspin->setValue(0);
    m_page->whitespin->setValue(255);
    m_page->gammaspin->setNum(1.0);
    m_page->ingradient->modifyGamma(1.0);
    m_page->outblackspin->setValue(0);
    m_page->outwhitespin->setValue(255);

    QHBoxLayout * l = new QHBoxLayout(this);
    Q_CHECK_PTR(l);
    l->addWidget(m_page, 0, Qt::AlignTop);

    connect( m_page->blackspin, SIGNAL(valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->whitespin, SIGNAL(valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->ingradient, SIGNAL(modifiedGamma(double)), SIGNAL(sigPleaseUpdatePreview()));

    connect( m_page->blackspin, SIGNAL(valueChanged(int)), m_page->ingradient, SLOT(modifyBlack(int)));
    connect( m_page->whitespin, SIGNAL(valueChanged(int)), m_page->ingradient, SLOT(modifyWhite(int)));
    //connect( m_page->whitespin, SIGNAL(valueChanged(int)), m_page->ingradient, SLOT(modifyGamma()));

    connect( m_page->ingradient, SIGNAL(modifiedBlack(int)), m_page->blackspin, SLOT(setValue(int)));
    connect( m_page->ingradient, SIGNAL(modifiedWhite(int)), m_page->whitespin, SLOT(setValue(int)));
    connect( m_page->ingradient, SIGNAL(modifiedGamma(double)), m_page->gammaspin, SLOT(setNum(double)));


    connect( m_page->outblackspin, SIGNAL(valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->outwhitespin, SIGNAL(valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));

    connect( m_page->outblackspin, SIGNAL(valueChanged(int)), m_page->outgradient, SLOT(modifyBlack(int)));
    connect( m_page->outwhitespin, SIGNAL(valueChanged(int)), m_page->outgradient, SLOT(modifyWhite(int)));

    connect( m_page->outgradient, SIGNAL(modifiedBlack(int)), m_page->outblackspin, SLOT(setValue(int)));
    connect( m_page->outgradient, SIGNAL(modifiedWhite(int)), m_page->outwhitespin, SLOT(setValue(int)));

    connect( (QObject*)(m_page->chkLogarithmic), SIGNAL(toggled(bool)), this, SLOT(drawHistogram(bool)));

    KisHistogramProducerSP producer = new KisGenericLabHistogramProducer();
    histogram = new KisHistogram(dev, producer, LINEAR);
    m_histlog = false;
    drawHistogram();

}

KisLevelConfigWidget::~KisLevelConfigWidget()
{
    delete histogram;
}

void KisLevelConfigWidget::drawHistogram(bool logarithmic)
{
    int height = 256;

    if (m_histlog != logarithmic) {
        // Update the histogram
        if (logarithmic)
            histogram->setHistogramType(LOGARITHMIC);
        else
            histogram->setHistogramType(LINEAR);
        m_histlog = logarithmic;
    }

    QPixmap pix(256, height);
    pix.fill();
    QPainter p(&pix);
    p.setPen(QPen::QPen(Qt::gray,1, Qt::SolidLine));

    double highest = (double)histogram->calculations().getHighest();
    Q_INT32 bins = histogram->producer()->numberOfBins();

    if (histogram->getHistogramType() == LINEAR) {
        double factor = (double)height / highest;
        for( int i=0; i<bins; ++i ) {
            p.drawLine(i, height, i, height - int(histogram->getValue(i) * factor));
        }
    } else {
        double factor = (double)height / (double)log(highest);
        for( int i = 0; i < bins; ++i ) {
            p.drawLine(i, height, i, height - int(log((double)histogram->getValue(i)) * factor));
        }
    }

    m_page->histview->setPixmap(pix);
}

KisLevelFilterConfiguration * KisLevelConfigWidget::config()
{
    KisLevelFilterConfiguration * cfg = new KisLevelFilterConfiguration();

    cfg->blackvalue = m_page->blackspin->value();
    cfg->whitevalue = m_page->whitespin->value();
    cfg->gammavalue = m_page->ingradient->getGamma();

    cfg->outblackvalue = m_page->outblackspin->value() * 255;
    cfg->outwhitevalue = m_page->outwhitespin->value() * 255;

    return cfg;
}

void KisLevelConfigWidget::setConfiguration( KisFilterConfiguration * config )
{
    KisLevelFilterConfiguration * cfg = dynamic_cast<KisLevelFilterConfiguration *>(config);
    m_page->blackspin->setValue(cfg->blackvalue);
    m_page->whitespin->setValue(cfg->whitevalue);
    m_page->ingradient->modifyGamma(cfg->gammavalue);

    m_page->outblackspin->setValue(cfg->outblackvalue / 255);
    m_page->outwhitespin->setValue(cfg->outwhitevalue / 255);
}


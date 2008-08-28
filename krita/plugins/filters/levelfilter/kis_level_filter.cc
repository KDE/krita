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
#include "kis_level_filter.h"

#include <math.h>

#include <klocale.h>

#include <qlayout.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qlabel.h>
#include <qspinbox.h>

#include <KoBasicHistogramProducers.h>
#include <KoColorSpace.h>
#include <KoColorTransformation.h>
#include <KoProgressUpdater.h>


#include "kis_paint_device.h"
#include "kis_iterators_pixel.h"
#include "kis_iterator.h"
#include "kis_histogram.h"
#include "kis_painter.h"
#include "kgradientslider.h"
#include "kis_processing_information.h"
#include "kis_selection.h"
#include "kis_types.h"

KisLevelFilter::KisLevelFilter()
        : KisFilter(id(), CategoryAdjust, i18n("&Levels"))
{
    setSupportsPainting(true);
    setSupportsPreview(true);
    setSupportsIncrementalPainting(false);
    setColorSpaceIndependence(TO_LAB16);
}

KisLevelFilter::~KisLevelFilter()
{
}

KisConfigWidget * KisLevelFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, const KisImageSP image) const
{
    Q_UNUSED(image);
    return new KisLevelConfigWidget(parent, dev);
}

bool KisLevelFilter::workWith(KoColorSpace* cs) const
{
    Q_UNUSED(cs);
    return true;
}


void KisLevelFilter::process(KisConstProcessingInformation srcInfo,
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
    if (!config) {
        kWarning() << "No configuration object for level filter\n";
        return;
    }

    Q_ASSERT(config);

    KoColorTransformation * adjustment = 0;

    int blackvalue = config->getInt("blackvalue");
    int whitevalue = config->getInt("whitevalue", 255);
    double gammavalue = config->getDouble("gammavalue", 1.0);
    int outblackvalue = config->getInt("outblackvalue");
    int outwhitevalue = config->getInt("outwhitevalue", 255);

    Q_UINT16 transfer[256];
    for (int i = 0; i < 256; i++) {
        if (i <= blackvalue)
            transfer[i] = outblackvalue;
        else if (i < whitevalue) {
            double a = (double)(i - blackvalue) / (double)(whitevalue - blackvalue);
            a = (double)(outwhitevalue - outblackvalue) * pow(a, (1.0 / gammavalue));
            transfer[i] = int(outblackvalue + a);
        } else
            transfer[i] = outwhitevalue;
        // TODO use floats instead of integer in the configuration
        transfer[i] = ((int)transfer[i] * 0xFFFF) / 0xFF ;
    }
    adjustment = src->colorSpace()->createBrightnessContrastAdjustment(transfer);

    KisHLineConstIteratorPixel srcIt = src->createHLineConstIterator(srcTopLeft.x(), srcTopLeft.y(), size.width(), srcInfo.selection());
    KisHLineIteratorPixel dstIt = dst->createHLineIterator(dstTopLeft.x(), dstTopLeft.y(), size.width(), dstInfo.selection());

    if (progressUpdater) {
        progressUpdater->setRange(0, size.width() * size.height());
    }
    Q_INT32 pixelsProcessed = 0;

    for (int row = 0; row < size.height() && !(progressUpdater && progressUpdater->interrupted()); ++row) {
        while (! srcIt.isDone()  && !(progressUpdater && progressUpdater->interrupted())) {
            Q_UINT32 npix = 0, maxpix = qMin(srcIt.nConseqHPixels(), dstIt.nConseqHPixels());
            Q_UINT8 selectedness = dstIt.selectedness();
            // The idea here is to handle stretches of completely selected and completely unselected pixels.
            // Partially selected pixels are handled one pixel at a time.
            switch (selectedness) {
            case MIN_SELECTED:
                while (dstIt.selectedness() == MIN_SELECTED && maxpix) {
                    --maxpix;
                    ++srcIt;
                    ++dstIt;
                    ++npix;
                }
                pixelsProcessed += npix;
                break;

            case MAX_SELECTED: {
                const Q_UINT8 *firstPixelSrc = srcIt.oldRawData();
                Q_UINT8 *firstPixelDst = dstIt.rawData();
                while (dstIt.selectedness() == MAX_SELECTED && maxpix) {
                    --maxpix;
                    if (maxpix != 0) {
                        ++srcIt;
                        ++dstIt;
                    }
                    ++npix;
                }
                // adjust
                adjustment->transform(firstPixelSrc, firstPixelDst, npix);
                pixelsProcessed += npix;
                ++srcIt;
                ++dstIt;
                break;
            }

            default:
                // adjust, but since it's partially selected we also only partially adjust
                adjustment->transform(srcIt.oldRawData(), dstIt.rawData(), 1);
                const quint8 *pixels[2] = {srcIt.oldRawData(), dstIt.rawData()};
                qint16 weights[2] = {MAX_SELECTED - selectedness, selectedness};
                src->colorSpace()->mixColorsOp()->mixColors(pixels, weights, 2, dstIt.rawData());
                ++srcIt;
                ++dstIt;
                pixelsProcessed++;
                break;
            }
            if (progressUpdater) progressUpdater->setValue(pixelsProcessed);
        }
        srcIt.nextRow();
        dstIt.nextRow();
    }
}

KisLevelConfigWidget::KisLevelConfigWidget(QWidget * parent, KisPaintDeviceSP dev)
        : KisConfigWidget(parent)
{
    m_page.setupUi(this);
    histogram = NULL;

    m_page.ingradient->enableGamma(true);
    m_page.blackspin->setValue(0);
    m_page.whitespin->setValue(255);
    m_page.gammaspin->setNum(1.0);
    m_page.ingradient->modifyGamma(1.0);
    m_page.outblackspin->setValue(0);
    m_page.outwhitespin->setValue(255);

    connect(m_page.blackspin, SIGNAL(valueChanged(int)), SIGNAL(sigConfigChanged()));
    connect(m_page.whitespin, SIGNAL(valueChanged(int)), SIGNAL(sigConfigChanged()));
    connect(m_page.ingradient, SIGNAL(modifiedGamma(double)), SIGNAL(sigConfigChanged()));

    connect(m_page.blackspin, SIGNAL(valueChanged(int)), m_page.ingradient, SLOT(modifyBlack(int)));
    connect(m_page.whitespin, SIGNAL(valueChanged(int)), m_page.ingradient, SLOT(modifyWhite(int)));
    //connect( m_page.whitespin, SIGNAL(valueChanged(int)), m_page.ingradient, SLOT(modifyGamma()));

    connect(m_page.ingradient, SIGNAL(modifiedBlack(int)), m_page.blackspin, SLOT(setValue(int)));
    connect(m_page.ingradient, SIGNAL(modifiedWhite(int)), m_page.whitespin, SLOT(setValue(int)));
    connect(m_page.ingradient, SIGNAL(modifiedGamma(double)), m_page.gammaspin, SLOT(setNum(double)));


    connect(m_page.outblackspin, SIGNAL(valueChanged(int)), SIGNAL(sigConfigChanged()));
    connect(m_page.outwhitespin, SIGNAL(valueChanged(int)), SIGNAL(sigConfigChanged()));

    connect(m_page.outblackspin, SIGNAL(valueChanged(int)), m_page.outgradient, SLOT(modifyBlack(int)));
    connect(m_page.outwhitespin, SIGNAL(valueChanged(int)), m_page.outgradient, SLOT(modifyWhite(int)));

    connect(m_page.outgradient, SIGNAL(modifiedBlack(int)), m_page.outblackspin, SLOT(setValue(int)));
    connect(m_page.outgradient, SIGNAL(modifiedWhite(int)), m_page.outwhitespin, SLOT(setValue(int)));

    connect((QObject*)(m_page.chkLogarithmic), SIGNAL(toggled(bool)), this, SLOT(drawHistogram(bool)));

    KoHistogramProducerSP producer = KoHistogramProducerSP(new KoGenericLabHistogramProducer());
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
    p.setPen(QPen::QPen(Qt::gray, 1, Qt::SolidLine));

    double highest = (double)histogram->calculations().getHighest();
    Q_INT32 bins = histogram->producer()->numberOfBins();

    if (histogram->getHistogramType() == LINEAR) {
        double factor = (double)height / highest;
        for (int i = 0; i < bins; ++i) {
            p.drawLine(i, height, i, height - int(histogram->getValue(i) * factor));
        }
    } else {
        double factor = (double)height / (double)log(highest);
        for (int i = 0; i < bins; ++i) {
            p.drawLine(i, height, i, height - int(log((double)histogram->getValue(i)) * factor));
        }
    }

    m_page.histview->setPixmap(pix);
}

KisPropertiesConfiguration * KisLevelConfigWidget::configuration() const
{
    KisFilterConfiguration * config = new KisFilterConfiguration(KisLevelFilter::id().id(), 1);

    config->setProperty("blackvalue", m_page.blackspin->value());
    config->setProperty("whitevalue", m_page.whitespin->value());
    config->setProperty("gammavalue", m_page.ingradient->getGamma());
    config->setProperty("outblackvalue", m_page.outblackspin->value());
    config->setProperty("outwhitevalue", m_page.outwhitespin->value());

    return config;
}

void KisLevelConfigWidget::setConfiguration(KisPropertiesConfiguration * config)
{
    QVariant value;
    if (config->getProperty("blackvalue", value)) {
        m_page.blackspin->setValue(value.toUInt());
    }
    if (config->getProperty("whitevalue", value)) {
        m_page.whitespin->setValue(value.toUInt());
    }
    if (config->getProperty("gammavalue", value)) {
        m_page.ingradient->modifyGamma(value.toDouble());
    }
    if (config->getProperty("outblackvalue", value)) {
        m_page.outblackspin->setValue(value.toUInt());
    }
    if (config->getProperty("outwhitevalue", value)) {
        m_page.outwhitespin->setValue(value.toUInt());
    }
}


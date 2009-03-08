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

#include <cmath>

#include <klocale.h>

#include <QLayout>
#include <QPixmap>
#include <QPainter>
#include <QLabel>
#include <QSpinBox>

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
        warnKrita << "No configuration object for level filter\n";
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
    qint32 pixelsProcessed = 0;

    for (int row = 0; row < size.height() && !(progressUpdater && progressUpdater->interrupted()); ++row) {
        while (! srcIt.isDone()  && !(progressUpdater && progressUpdater->interrupted())) {
            quint32 npix = 0, maxpix = qMin(srcIt.nConseqHPixels(), dstIt.nConseqHPixels());
            quint8 selectedness = dstIt.selectedness();
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
                const quint8 *firstPixelSrc = srcIt.oldRawData();
                quint8 *firstPixelDst = dstIt.rawData();
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
    m_page.gammaspin->setValue(1.0);
    m_page.ingradient->slotModifyGamma(1.0);
    m_page.outblackspin->setValue(0);
    m_page.outwhitespin->setValue(255);

    connect(m_page.blackspin, SIGNAL(valueChanged(int)), SIGNAL(sigConfigChanged()));
    connect(m_page.whitespin, SIGNAL(valueChanged(int)), SIGNAL(sigConfigChanged()));
    connect(m_page.ingradient, SIGNAL(sigModifiedGamma(double)), SIGNAL(sigConfigChanged()));

    connect(m_page.blackspin, SIGNAL(valueChanged(int)), m_page.ingradient, SLOT(slotModifyBlack(int)));
    connect(m_page.whitespin, SIGNAL(valueChanged(int)), m_page.ingradient, SLOT(slotModifyWhite(int)));
    connect(m_page.gammaspin, SIGNAL(valueChanged(double)), m_page.ingradient, SLOT(slotModifyGamma(double)));

    connect(m_page.blackspin, SIGNAL(valueChanged(int)), this, SLOT(slotModifyInWhiteLimit(int)));
    connect(m_page.whitespin, SIGNAL(valueChanged(int)), this, SLOT(slotModifyInBlackLimit(int)));

    connect(m_page.ingradient, SIGNAL(sigModifiedBlack(int)), m_page.blackspin, SLOT(setValue(int)));
    connect(m_page.ingradient, SIGNAL(sigModifiedWhite(int)), m_page.whitespin, SLOT(setValue(int)));
    connect(m_page.ingradient, SIGNAL(sigModifiedGamma(double)), m_page.gammaspin, SLOT(setValue(double)));


    connect(m_page.outblackspin, SIGNAL(valueChanged(int)), SIGNAL(sigConfigChanged()));
    connect(m_page.outwhitespin, SIGNAL(valueChanged(int)), SIGNAL(sigConfigChanged()));

    connect(m_page.outblackspin, SIGNAL(valueChanged(int)), m_page.outgradient, SLOT(slotModifyBlack(int)));
    connect(m_page.outwhitespin, SIGNAL(valueChanged(int)), m_page.outgradient, SLOT(slotModifyWhite(int)));

    connect(m_page.outblackspin, SIGNAL(valueChanged(int)), this, SLOT(slotModifyOutWhiteLimit(int)));
    connect(m_page.outwhitespin, SIGNAL(valueChanged(int)), this, SLOT(slotModifyOutBlackLimit(int)));

    connect(m_page.outgradient, SIGNAL(sigModifiedBlack(int)), m_page.outblackspin, SLOT(setValue(int)));
    connect(m_page.outgradient, SIGNAL(sigModifiedWhite(int)), m_page.outwhitespin, SLOT(setValue(int)));

    connect((QObject*)(m_page.chkLogarithmic), SIGNAL(toggled(bool)), this, SLOT(slotDrawHistogram(bool)));

    KoHistogramProducerSP producer = KoHistogramProducerSP(new KoGenericLabHistogramProducer());
    histogram = new KisHistogram(dev, producer, LINEAR);
    m_histlog = false;
    slotDrawHistogram();

}

KisLevelConfigWidget::~KisLevelConfigWidget()
{
    delete histogram;
}

void KisLevelConfigWidget::slotDrawHistogram(bool logarithmic)
{
    int wHeight = height();
    int wHeightMinusOne = wHeight - 1;
    int wWidth = width();

    if (m_histlog != logarithmic) {
        // Update the histogram
        if (logarithmic)
            histogram->setHistogramType(LOGARITHMIC);
        else
            histogram->setHistogramType(LINEAR);
        m_histlog = logarithmic;
    }

    QPixmap pix(wWidth, wHeight);
    pix.fill();
    QPainter p(&pix);

    p.setPen(QPen::QPen(Qt::gray, 1, Qt::SolidLine));

    double highest = (double)histogram->calculations().getHighest();
    qint32 bins = histogram->producer()->numberOfBins();

    // use nearest neighbour interpolation
    if (histogram->getHistogramType() == LINEAR) {
        double factor = (double)(wHeight - wHeight / 5.0) / highest;
        for (int i = 0; i < wWidth; i++)
        {
            int binNo = (int)round((double)i / wWidth * (bins - 1));
            if ((int)histogram->getValue(binNo) != 0)
                p.drawLine(i, wHeightMinusOne, i, wHeightMinusOne - (int)histogram->getValue(binNo) * factor);
        }
    } else {
        double factor = (double)(wHeight - wHeight / 5.0) / (double)log(highest);
        for (int i = 0; i < wWidth; i++) {
            int binNo = (int)round((double)i / wWidth * (bins - 1)) ;
            if ((int)histogram->getValue(binNo) != 0)
              p.drawLine(i, wHeightMinusOne, i, wHeightMinusOne - log(histogram->getValue(binNo)) * factor);
        }
    }

    m_page.histview->setPixmap(pix);
}

void KisLevelConfigWidget::slotModifyInBlackLimit(int limit)
{
    m_page.blackspin->setMaximum(limit - 1);
}

void KisLevelConfigWidget::slotModifyInWhiteLimit(int limit)
{
    m_page.whitespin->setMinimum(limit + 1);
}

void KisLevelConfigWidget::slotModifyOutBlackLimit(int limit)
{
    m_page.outblackspin->setMaximum(limit - 1);
}

void KisLevelConfigWidget::slotModifyOutWhiteLimit(int limit)
{
    m_page.outwhitespin->setMinimum(limit + 1);
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

void KisLevelConfigWidget::setConfiguration(const KisPropertiesConfiguration * config)
{
    QVariant value;
    if (config->getProperty("blackvalue", value)) {
        m_page.blackspin->setValue(value.toUInt());
        m_page.ingradient->slotModifyBlack(value.toUInt());
    }
    if (config->getProperty("whitevalue", value)) {
        m_page.whitespin->setValue(value.toUInt());
        m_page.ingradient->slotModifyWhite(value.toUInt());
    }
    if (config->getProperty("gammavalue", value)) {
        m_page.gammaspin->setValue(value.toUInt());
        m_page.ingradient->slotModifyGamma(value.toDouble());
    }
    if (config->getProperty("outblackvalue", value)) {
        m_page.outblackspin->setValue(value.toUInt());
        m_page.outgradient->slotModifyBlack(value.toUInt());
    }
    if (config->getProperty("outwhitevalue", value)) {
        m_page.outwhitespin->setValue(value.toUInt());
        m_page.outgradient->slotModifyWhite(value.toUInt());
    }
}

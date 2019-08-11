/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "threshold.h"
#include <stdlib.h>
#include <vector>

#include <QPoint>
#include <QTime>

#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include "KisGradientSlider.h"
#include "kis_histogram.h"
#include <kis_layer.h>
#include "kis_paint_device.h"
#include "kis_painter.h"
#include <kis_processing_information.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <KisSequentialIteratorProgress.h>

#include <KoBasicHistogramProducers.h>
#include "KoColorModelStandardIds.h"
#include <KoColorSpace.h>
#include <KoColorTransformation.h>
#include <KoUpdater.h>

K_PLUGIN_FACTORY_WITH_JSON(KritaThresholdFactory, "kritathreshold.json", registerPlugin<KritaThreshold>();)

KritaThreshold::KritaThreshold(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisFilterThreshold());
}

KritaThreshold::~KritaThreshold()
{
}

KisFilterThreshold::KisFilterThreshold()
    : KisFilter(id(), FiltersCategoryAdjustId, i18n("&Threshold..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);

    setSupportsPainting(false);
    setShowConfigurationWidget(true);
    setSupportsLevelOfDetail(true);
    setSupportsAdjustmentLayers(true);
    setSupportsThreading(true);
}

void KisFilterThreshold::processImpl(KisPaintDeviceSP device,
                 const QRect& applyRect,
                 const KisFilterConfigurationSP config,
                 KoUpdater *progressUpdater) const
{
    Q_ASSERT(!device.isNull());

    const int threshold = config->getInt("threshold");

    KoColor white(Qt::white, device->colorSpace());
    KoColor black(Qt::black, device->colorSpace());

    KisSequentialIteratorProgress it(device, applyRect, progressUpdater);
    const int pixelSize = device->colorSpace()->pixelSize();

    while (it.nextPixel()) {
        if (device->colorSpace()->intensity8(it.oldRawData()) > threshold) {
            white.setOpacity(device->colorSpace()->opacityU8(it.oldRawData()));
            memcpy(it.rawData(), white.data(), pixelSize);
        }
        else {
            black.setOpacity(device->colorSpace()->opacityU8(it.oldRawData()));
            memcpy(it.rawData(), black.data(), pixelSize);
        }
    }

}


KisFilterConfigurationSP KisFilterThreshold::factoryConfiguration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("threshold", 1);
    config->setProperty("threshold", 128);
    return config;
}

KisConfigWidget *KisFilterThreshold::createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev, bool) const
{
    return new KisThresholdConfigWidget(parent, dev);
}

KisThresholdConfigWidget::KisThresholdConfigWidget(QWidget * parent, KisPaintDeviceSP dev)
    : KisConfigWidget(parent)
{
    Q_ASSERT(dev);
    m_page.setupUi(this);

    m_page.thresholdGradient->enableGamma(false);
    m_page.thresholdGradient->enableWhite(false);
    m_page.intThreshold->setValue(128);

    connect(m_page.intThreshold, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page.thresholdGradient, SIGNAL(sigModifiedGamma(double)), SIGNAL(sigConfigurationItemChanged()));

    connect(m_page.intThreshold, SIGNAL(valueChanged(int)), m_page.thresholdGradient, SLOT(slotModifyBlack(int)));

    connect(m_page.thresholdGradient, SIGNAL(sigModifiedBlack(int)), m_page.intThreshold, SLOT(setValue(int)));

    connect((QObject*)(m_page.chkLogarithmic), SIGNAL(toggled(bool)), this, SLOT(slotDrawHistogram(bool)));

    KoHistogramProducer *producer = new KoGenericLabHistogramProducer();
    m_histogram.reset( new KisHistogram(dev, dev->exactBounds(), producer, LINEAR) );
    m_histlog = false;
    m_page.histview->resize(288,100);
    slotDrawHistogram();

}

KisThresholdConfigWidget::~KisThresholdConfigWidget()
{
}

void KisThresholdConfigWidget::slotDrawHistogram(bool logarithmic)
{
    int wHeight = m_page.histview->height();
    int wHeightMinusOne = wHeight - 1;
    int wWidth = m_page.histview->width();

    if (m_histlog != logarithmic) {
        // Update the m_histogram
        if (logarithmic)
            m_histogram->setHistogramType(LOGARITHMIC);
        else
            m_histogram->setHistogramType(LINEAR);
        m_histlog = logarithmic;
    }

    QPalette appPalette = QApplication::palette();
    QPixmap pix(wWidth-100, wHeight);

    pix.fill(QColor(appPalette.color(QPalette::Base)));
    QPainter p(&pix);

    p.setPen(QPen(Qt::gray, 1, Qt::SolidLine));

    double highest = (double)m_histogram->calculations().getHighest();
    qint32 bins = m_histogram->producer()->numberOfBins();

    // use nearest neighbour interpolation
    if (m_histogram->getHistogramType() == LINEAR) {
        double factor = (double)(wHeight - wHeight / 5.0) / highest;
        for (int i = 0; i < wWidth; i++) {
            int binNo = qRound((double)i / wWidth * (bins - 1));
            if ((int)m_histogram->getValue(binNo) != 0)
                p.drawLine(i, wHeightMinusOne, i, wHeightMinusOne - (int)m_histogram->getValue(binNo) * factor);
        }
    } else {
        double factor = (double)(wHeight - wHeight / 5.0) / (double)log(highest);
        for (int i = 0; i < wWidth; i++) {
            int binNo = qRound((double)i / wWidth * (bins - 1)) ;
            if ((int)m_histogram->getValue(binNo) != 0)
                p.drawLine(i, wHeightMinusOne, i, wHeightMinusOne - log((double)m_histogram->getValue(binNo)) * factor);
        }
    }

    m_page.histview->setPixmap(pix);
}

void KisThresholdConfigWidget::slotSetThreshold(int limit)
{
    m_page.intThreshold->setMaximum(limit - 1);
}

KisPropertiesConfigurationSP KisThresholdConfigWidget::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("threshold", 1);
    config->setProperty("threshold", m_page.intThreshold->value());
    return config;
}

void KisThresholdConfigWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    QVariant value;
    if (config->getProperty("threshold", value)) {
        m_page.intThreshold->setValue(value.toUInt());
        m_page.thresholdGradient->slotModifyBlack(value.toUInt());
    }
}


#include "threshold.moc"

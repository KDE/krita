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

#include <klocalizedstring.h>

#include <QtGlobal>
#include <QLayout>
#include <QPixmap>
#include <QPainter>
#include <QLabel>
#include <QSpinBox>

#include <KoBasicHistogramProducers.h>
#include <KoColorSpace.h>
#include <KoColorTransformation.h>

#include "kis_paint_device.h"
#include "kis_histogram.h"
#include "kis_painter.h"
#include "KisGradientSlider.h"
#include "kis_processing_information.h"
#include "kis_selection.h"
#include "kis_types.h"
#include <filter/kis_filter_category_ids.h>
#include "filter/kis_color_transformation_configuration.h"

KisLevelFilter::KisLevelFilter()
        : KisColorTransformationFilter(id(), FiltersCategoryAdjustId, i18n("&Levels..."))
{
    setShortcut(QKeySequence(Qt::CTRL + Qt::Key_L));
    setSupportsPainting(false);
    setColorSpaceIndependence(TO_LAB16);
}

KisLevelFilter::~KisLevelFilter()
{
}

KisConfigWidget * KisLevelFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev) const
{
    return new KisLevelConfigWidget(parent, dev);
}

KoColorTransformation* KisLevelFilter::createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const
{
    if (!config) {
        warnKrita << "No configuration object for level filter\n";
        return 0;
    }

    Q_ASSERT(config);

    int blackvalue = config->getInt("blackvalue");
    int whitevalue = config->getInt("whitevalue", 255);
    double gammavalue = config->getDouble("gammavalue", 1.0);
    int outblackvalue = config->getInt("outblackvalue");
    int outwhitevalue = config->getInt("outwhitevalue", 255);

    quint16 transfer[256];
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
    return cs->createBrightnessContrastAdjustment(transfer);
}

KisLevelConfigWidget::KisLevelConfigWidget(QWidget * parent, KisPaintDeviceSP dev)
        : KisConfigWidget(parent)
{
    Q_ASSERT(dev);
    m_page.setupUi(this);

    m_page.ingradient->enableGamma(true);
    m_page.blackspin->setValue(0);
    m_page.whitespin->setValue(255);
    m_page.gammaspin->setValue(1.0);
    m_page.ingradient->slotModifyGamma(1.0);
    m_page.outblackspin->setValue(0);
    m_page.outwhitespin->setValue(255);

    connect(m_page.blackspin, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page.whitespin, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page.ingradient, SIGNAL(sigModifiedGamma(double)), SIGNAL(sigConfigurationItemChanged()));

    connect(m_page.blackspin, SIGNAL(valueChanged(int)), m_page.ingradient, SLOT(slotModifyBlack(int)));
    connect(m_page.whitespin, SIGNAL(valueChanged(int)), m_page.ingradient, SLOT(slotModifyWhite(int)));
    connect(m_page.gammaspin, SIGNAL(valueChanged(double)), m_page.ingradient, SLOT(slotModifyGamma(double)));

    connect(m_page.blackspin, SIGNAL(valueChanged(int)), this, SLOT(slotModifyInWhiteLimit(int)));
    connect(m_page.whitespin, SIGNAL(valueChanged(int)), this, SLOT(slotModifyInBlackLimit(int)));

    connect(m_page.ingradient, SIGNAL(sigModifiedBlack(int)), m_page.blackspin, SLOT(setValue(int)));
    connect(m_page.ingradient, SIGNAL(sigModifiedWhite(int)), m_page.whitespin, SLOT(setValue(int)));
    connect(m_page.ingradient, SIGNAL(sigModifiedGamma(double)), m_page.gammaspin, SLOT(setValue(double)));


    connect(m_page.outblackspin, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page.outwhitespin, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));

    connect(m_page.outblackspin, SIGNAL(valueChanged(int)), m_page.outgradient, SLOT(slotModifyBlack(int)));
    connect(m_page.outwhitespin, SIGNAL(valueChanged(int)), m_page.outgradient, SLOT(slotModifyWhite(int)));

    connect(m_page.outblackspin, SIGNAL(valueChanged(int)), this, SLOT(slotModifyOutWhiteLimit(int)));
    connect(m_page.outwhitespin, SIGNAL(valueChanged(int)), this, SLOT(slotModifyOutBlackLimit(int)));

    connect(m_page.outgradient, SIGNAL(sigModifiedBlack(int)), m_page.outblackspin, SLOT(setValue(int)));
    connect(m_page.outgradient, SIGNAL(sigModifiedWhite(int)), m_page.outwhitespin, SLOT(setValue(int)));

    connect(m_page.butauto, SIGNAL(clicked(bool)), this, SLOT(slotAutoLevel()));
    connect(m_page.butinvert, SIGNAL(clicked(bool)), this, SLOT(slotInvert()));

    connect((QObject*)(m_page.chkLogarithmic), SIGNAL(toggled(bool)), this, SLOT(slotDrawHistogram(bool)));

    KoHistogramProducer *producer = new KoGenericLabHistogramProducer();
    m_histogram.reset( new KisHistogram(dev, dev->exactBounds(), producer, LINEAR) );
    m_histlog = false;
    m_page.histview->resize(288,100);
    m_inverted = false;
    slotDrawHistogram();

}

KisLevelConfigWidget::~KisLevelConfigWidget()
{
}

void KisLevelConfigWidget::slotDrawHistogram(bool logarithmic)
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
    if (m_inverted) {
        m_page.outblackspin->setMinimum(limit + 1);
    } else {
        m_page.outblackspin->setMaximum(limit - 1);
    }
}

void KisLevelConfigWidget::slotModifyOutWhiteLimit(int limit)
{
    if (m_inverted) {
        m_page.outwhitespin->setMaximum(limit - 1);
    } else {
        m_page.outwhitespin->setMinimum(limit + 1);
    }
}

void KisLevelConfigWidget::slotAutoLevel(void)
{
    Q_ASSERT(m_histogram);

    qint32 num_bins = m_histogram->producer()->numberOfBins();

    Q_ASSERT(num_bins > 1);

    int chosen_low_bin = 0, chosen_high_bin = num_bins-1;
    int count_thus_far = m_histogram->getValue(0);
    const int total_count = m_histogram->producer()->count();
    const double threshold = 0.006;

    // find the low and hi point/bins based on summing count percentages
    //
    // this implementation is a port of GIMP's auto level implementation
    // (use a GPLv2 version as reference, specifically commit 51bfd07f18ef045a3e43632218fd92cae9ff1e48)

    for (int bin=0; bin<(num_bins-1); ++bin) {
        int next_count_thus_far = count_thus_far + m_histogram->getValue(bin+1);

        double this_percentage = static_cast<double>(count_thus_far) /  total_count;
        double next_percentage = static_cast<double>(next_count_thus_far) / total_count;

        //dbgKrita << "bin" << bin << "this_percentage" << this_percentage << "next_percentage" << next_percentage;
        if (fabs(this_percentage - threshold) < fabs(next_percentage - threshold)) {
            chosen_low_bin = bin;
            break;
        }

        count_thus_far = next_count_thus_far;
    }

    count_thus_far = m_histogram->getValue(num_bins-1);
    for (int bin=(num_bins-1); bin>0; --bin) {
        int next_count_thus_far = count_thus_far + m_histogram->getValue(bin-1);

        double this_percentage = static_cast<double>(count_thus_far) /  total_count;
        double next_percentage = static_cast<double>(next_count_thus_far) / total_count;

        //dbgKrita << "hi-bin" << bin << "this_percentage" << this_percentage << "next_percentage" << next_percentage;
        if (fabs(this_percentage - threshold) < fabs(next_percentage - threshold)) {
            chosen_high_bin = bin;
            break;
        }

        count_thus_far = next_count_thus_far;
    }

    if (chosen_low_bin < chosen_high_bin) {
        m_page.blackspin->setValue(chosen_low_bin);
        m_page.ingradient->slotModifyBlack(chosen_low_bin);

        m_page.whitespin->setValue(chosen_high_bin);
        m_page.ingradient->slotModifyWhite(chosen_high_bin);
    }
}

void KisLevelConfigWidget::slotInvert(void)
{
    m_inverted = !m_inverted;
    int white = m_page.outwhitespin->value();
    int black = m_page.outblackspin->value();

    resetOutSpinLimit();
    m_page.outgradient->setInverted(m_inverted);
    m_page.outwhitespin->setValue(black);
    m_page.outblackspin->setValue(white);
}

KisPropertiesConfigurationSP  KisLevelConfigWidget::configuration() const
{
    KisColorTransformationConfiguration * config = new KisColorTransformationConfiguration(KisLevelFilter::id().id(), 1);

    config->setProperty("blackvalue", m_page.blackspin->value());
    config->setProperty("whitevalue", m_page.whitespin->value());
    config->setProperty("gammavalue", m_page.gammaspin->value());
    config->setProperty("outblackvalue", m_page.outblackspin->value());
    config->setProperty("outwhitevalue", m_page.outwhitespin->value());

    return config;
}

void KisLevelConfigWidget::setConfiguration(const KisPropertiesConfigurationSP  config)
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

void KisLevelConfigWidget::resetOutSpinLimit() {
    if (m_inverted) {
        m_page.outblackspin->setMaximum(255);
        m_page.outwhitespin->setMinimum(0);
    } else {
        m_page.outblackspin->setMinimum(0);
        m_page.outwhitespin->setMaximum(255);
    }
}

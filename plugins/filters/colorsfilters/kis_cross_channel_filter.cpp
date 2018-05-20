/*
 * This file is part of Krita
 *
 * Copyright (c) 2018 Jouni Pentikainen <joupent@gmail.com>
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

#include "kis_cross_channel_filter.h"

#include <Qt>
#include <QLayout>
#include <QPixmap>
#include <QPainter>
#include <QLabel>
#include <QComboBox>
#include <QDomDocument>
#include <QHBoxLayout>

#include "KoChannelInfo.h"
#include "KoBasicHistogramProducers.h"
#include "KoColorModelStandardIds.h"
#include "KoColorSpace.h"
#include "KoColorTransformation.h"
#include "KoCompositeColorTransformation.h"
#include "KoCompositeOp.h"
#include "KoID.h"

#include "kis_signals_blocker.h"

#include "kis_bookmarked_configuration_manager.h"
#include "kis_config_widget.h"
#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>

#include "kis_histogram.h"
#include "kis_painter.h"
#include "widgets/kis_curve_widget.h"

// KisCrossChannelFilterConfiguration

KisCrossChannelFilterConfiguration::KisCrossChannelFilterConfiguration(int channelCount)
        : KisMultiChannelFilterConfigurationBase(channelCount, "crosschannel", 1)
{
    m_driverChannels.resize(channelCount);
}

KisCrossChannelFilterConfiguration::~KisCrossChannelFilterConfiguration()
{}

const QVector<int> KisCrossChannelFilterConfiguration::driverChannels() const
{
    return m_driverChannels;
}

void KisCrossChannelFilterConfiguration::setDriverChannels(QVector<int> driverChannels)
{
    m_driverChannels = driverChannels;
}

void KisCrossChannelFilterConfiguration::fromXML(const QDomElement& root)
{
    KisMultiChannelFilterConfigurationBase::fromXML(root);
    // TODO
}

void KisCrossChannelFilterConfiguration::toXML(QDomDocument& doc, QDomElement& root) const
{
    KisMultiChannelFilterConfigurationBase::toXML(doc, root);
    // TODO
}


KisCrossChannelConfigWidget::KisCrossChannelConfigWidget(QWidget * parent, KisPaintDeviceSP dev, Qt::WindowFlags f)
        : KisMultiChannelConfigWidgetBase(parent, dev, f)
{
    const int virtualChannelCount = m_virtualChannels.size();
    m_driverChannels.resize(virtualChannelCount);
    KisMultiChannelFilterConfigurationBase::initDefaultCurves(m_curves, virtualChannelCount);

    init();

    for (int i = 0; i < virtualChannelCount; i++) {
        const VirtualChannelInfo &info = m_virtualChannels[i];
        m_page->cmbDriverChannel->addItem(info.name(), info.pixelIndex());
    }

    connect(m_page->cmbDriverChannel, SIGNAL(activated(int)), this, SLOT(slotSetDriverChannel(int)));
}

// KisCrossChannelConfigWidget

KisCrossChannelConfigWidget::~KisCrossChannelConfigWidget()
{}

void KisCrossChannelConfigWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    // TODO
}

KisPropertiesConfigurationSP KisCrossChannelConfigWidget::configuration() const
{
    KisPropertiesConfigurationSP cfgSP = new KisCrossChannelFilterConfiguration(m_virtualChannels.count());
    auto *cfg = static_cast<KisCrossChannelFilterConfiguration*>(cfgSP.data());

    m_curves[m_activeVChannel] = m_page->curveWidget->curve();
    cfg->setCurves(m_curves);
    cfg->setDriverChannels(m_driverChannels);


    return cfgSP;
}

void KisCrossChannelConfigWidget::updateChannelRange()
{
    float m_shift = 0;
    float m_scale = 100.0;

    int min = 0;
    int max = 100;

    m_page->curveWidget->setupInOutControls(m_page->intIn, m_page->intOut, min, max);
}


void KisCrossChannelConfigWidget::slotSetDriverChannel(int channel)
{
    m_driverChannels[m_activeVChannel] = channel;
    updateChannelRange();

    // TODO: update histogram?
}

// KisCrossChannelFilter

KisCrossChannelFilter::KisCrossChannelFilter() : KisColorTransformationFilter(id(), categoryAdjust(), i18n("&Cross-channel adjustment curves..."))
{
    //setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));
    setSupportsPainting(true);
    setColorSpaceIndependence(TO_LAB16);
}

KisCrossChannelFilter::~KisCrossChannelFilter()
{}

KisConfigWidget * KisCrossChannelFilter::createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev) const
{
    return new KisCrossChannelConfigWidget(parent, dev);
}

KisFilterConfigurationSP  KisCrossChannelFilter::factoryConfiguration() const
{
    return new KisCrossChannelFilterConfiguration(0);
}

int mapChannel(const VirtualChannelInfo &channel) {
    // See KisHSVCurveAdjustment::enumChannel
    switch (channel.type()) {
    case VirtualChannelInfo::HUE:
        return 3;
    case VirtualChannelInfo::SATURATION:
        return 4;
    case VirtualChannelInfo::LIGHTNESS:
        return 5;
    default:
        // TODO
        return qBound(0, channel.pixelIndex(), 2);
    };
}

KoColorTransformation* KisCrossChannelFilter::createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const // TODO
{
    const KisCrossChannelFilterConfiguration* configBC =
        dynamic_cast<const KisCrossChannelFilterConfiguration*>(config.data());
    Q_ASSERT(configBC);

    const QVector<QVector<quint16> > &originalTransfers = configBC->transfers();
    const QList<KisCubicCurve> &curves = configBC->curves();
    const QVector<int> &drivers = configBC->driverChannels();

    const QVector<VirtualChannelInfo> virtualChannels =
            KisMultiChannelFilterConfigurationBase::getVirtualChannels(cs);

    if (originalTransfers.size() != int(virtualChannels.size())) {
        // We got an illegal number of colorchannels :(
        return 0;
    }

    QVector<KoColorTransformation*> transforms;
//    for (int i = 0; i < virtualChannels.size(); i++) {
      for (int i = virtualChannels.size() - 1; i >= 0; i--) {
        if (!curves[i].isNull()) {
            int channel = mapChannel(virtualChannels[i]);
            int driverChannel = mapChannel(virtualChannels[drivers[i]]);
            QHash<QString, QVariant> params;
            params["channel"] = channel;
            params["driverChannel"] = driverChannel; // FIXME: channel number mismatch
            params["curve"] = QVariant::fromValue(originalTransfers[i]);
            params["relative"] = true;
            params["lumaRed"]   = cs->lumaCoefficients()[0];
            params["lumaGreen"] = cs->lumaCoefficients()[1];
            params["lumaBlue"]  = cs->lumaCoefficients()[2];

            transforms << cs->createColorTransformation("hsv_curve_adjustment", params);
        }
    }

    return KoCompositeColorTransformation::createOptimizedCompositeTransform(transforms);
}

bool KisCrossChannelFilter::needsTransparentPixels(const KisFilterConfigurationSP config, const KoColorSpace *cs) const
{
    Q_UNUSED(config);
    return cs->colorModelId() == AlphaColorModelID;
}

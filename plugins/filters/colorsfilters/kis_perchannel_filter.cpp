/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 C. Boemann <cbo@boemann.dk>
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

#include "kis_perchannel_filter.h"

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

#include "../../color/colorspaceextensions/kis_hsv_adjustment.h"

KisPerChannelConfigWidget::KisPerChannelConfigWidget(QWidget * parent, KisPaintDeviceSP dev, Qt::WindowFlags f)
        : KisMultiChannelConfigWidget(parent, dev, f)
{
    init();

    // These are not used by this filter,
    // but the dialog is shared with KisCrossChannelFilter
    m_page->lblDriverChannel->hide();
    m_page->cmbDriverChannel->hide();
}

KisPerChannelConfigWidget::~KisPerChannelConfigWidget()
{}

#define BITS_PER_BYTE 8
#define pwr2(p) (1<<p)
void KisPerChannelConfigWidget::updateChannelControls()
{
    // Getting range accepted by channel
    VirtualChannelInfo &currentVChannel = m_virtualChannels[m_activeVChannel];

    KoChannelInfo::enumChannelValueType valueType = currentVChannel.valueType();

    int order = BITS_PER_BYTE * currentVChannel.channelSize();
    int maxValue = pwr2(order);
    int min;
    int max;

    m_page->curveWidget->dropInOutControls();

    switch (valueType) {
    case KoChannelInfo::UINT8:
    case KoChannelInfo::UINT16:
    case KoChannelInfo::UINT32:
        min = 0;
        max = maxValue - 1;
        break;
    case KoChannelInfo::INT8:
    case KoChannelInfo::INT16:
        min = -maxValue / 2;
        max = maxValue / 2 - 1;
        break;
    case KoChannelInfo::FLOAT16:
    case KoChannelInfo::FLOAT32:
    case KoChannelInfo::FLOAT64:
    default:
        //Hack Alert: should be changed to float
        min = 0;
        max = 100;
        break;
    }

    m_page->curveWidget->setupInOutControls(m_page->intIn, m_page->intOut, min, max, min, max);
}


KisPropertiesConfigurationSP KisPerChannelConfigWidget::configuration() const
{
    int numChannels = m_virtualChannels.size();
    KisPropertiesConfigurationSP cfg = new KisPerChannelFilterConfiguration(numChannels);

    KIS_ASSERT_RECOVER(m_activeVChannel < m_curves.size()) { return cfg; }

    m_curves[m_activeVChannel] = m_page->curveWidget->curve();
    static_cast<KisPerChannelFilterConfiguration*>(cfg.data())->setCurves(m_curves);

    return cfg;
}

KisPropertiesConfigurationSP KisPerChannelConfigWidget::getDefaultConfiguration()
{
    return new KisPerChannelFilterConfiguration(m_virtualChannels.size());
}

KisPerChannelFilterConfiguration::KisPerChannelFilterConfiguration(int channelCount)
        : KisMultiChannelFilterConfiguration(channelCount, "perchannel", 1)
{
    init();
}

KisPerChannelFilterConfiguration::~KisPerChannelFilterConfiguration()
{
}

KisCubicCurve KisPerChannelFilterConfiguration::getDefaultCurve()
{
    return KisCubicCurve();
}

// KisPerChannelFilter

KisPerChannelFilter::KisPerChannelFilter() : KisMultiChannelFilter(id(), i18n("&Color Adjustment curves..."))
{
    setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));
}

KisConfigWidget * KisPerChannelFilter::createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev, bool) const
{
    return new KisPerChannelConfigWidget(parent, dev);
}

KisFilterConfigurationSP  KisPerChannelFilter::factoryConfiguration() const
{
    return new KisPerChannelFilterConfiguration(0);
}

KoColorTransformation* KisPerChannelFilter::createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const
{
    const KisPerChannelFilterConfiguration* configBC =
        dynamic_cast<const KisPerChannelFilterConfiguration*>(config.data()); // Somehow, this shouldn't happen
    Q_ASSERT(configBC);

    const QVector<QVector<quint16> > &originalTransfers = configBC->transfers();
    const QList<KisCubicCurve> &originalCurves = configBC->curves();

    /**
     * TODO: What about the order of channels? (DK)
     *
     * Virtual channels are sorted in display order, does Lcms accepts
     * transforms in display order? Why on Earth it works?! Is it
     * documented anywhere?
     */
    const QVector<VirtualChannelInfo> virtualChannels =
        KisMultiChannelFilter::getVirtualChannels(cs, originalTransfers.size());

    if (originalTransfers.size() > int(virtualChannels.size())) {
        // We got an illegal number of colorchannels :(
        return 0;
    }

    bool colorsNull = true;
    bool hueNull = true;
    bool saturationNull = true;
    bool lightnessNull = true;
    bool allColorsNull = true;
    int alphaIndexInReal = -1;

    QVector<QVector<quint16> > realTransfers;
    QVector<quint16> hueTransfer;
    QVector<quint16> saturationTransfer;
    QVector<quint16> lightnessTransfer;
    QVector<quint16> allColorsTransfer;

    for (int i = 0; i < virtualChannels.size(); i++) {
        if (virtualChannels[i].type() == VirtualChannelInfo::REAL) {
            realTransfers << originalTransfers[i];

            if (virtualChannels[i].isAlpha()) {
                alphaIndexInReal = realTransfers.size() - 1;
            }

            if (colorsNull && !originalCurves[i].isIdentity()) {
                colorsNull = false;
            }
        } else if (virtualChannels[i].type() == VirtualChannelInfo::HUE) {
            KIS_ASSERT_RECOVER_NOOP(hueTransfer.isEmpty());
            hueTransfer = originalTransfers[i];

            if (hueNull && !originalCurves[i].isIdentity()) {
                hueNull = false;
            }
        } else if (virtualChannels[i].type() == VirtualChannelInfo::SATURATION) {
            KIS_ASSERT_RECOVER_NOOP(saturationTransfer.isEmpty());
            saturationTransfer = originalTransfers[i];

            if (saturationNull && !originalCurves[i].isIdentity()) {
                saturationNull = false;
            }
        } else if (virtualChannels[i].type() == VirtualChannelInfo::LIGHTNESS) {
            KIS_ASSERT_RECOVER_NOOP(lightnessTransfer.isEmpty());
            lightnessTransfer = originalTransfers[i];

            if (lightnessNull && !originalCurves[i].isIdentity()) {
                lightnessNull = false;
            }
        } else if (virtualChannels[i].type() == VirtualChannelInfo::ALL_COLORS) {
            KIS_ASSERT_RECOVER_NOOP(allColorsTransfer.isEmpty());
            allColorsTransfer = originalTransfers[i];

            if (allColorsNull && !originalCurves[i].isIdentity()) {
                allColorsNull = false;
            }
        }
    }

    KoColorTransformation *hueTransform = 0;
    KoColorTransformation *saturationTransform = 0;
    KoColorTransformation *lightnessTransform = 0;
    KoColorTransformation *allColorsTransform = 0;
    KoColorTransformation *colorTransform = 0;

    if (!colorsNull) {
        const quint16** transfers = new const quint16*[realTransfers.size()];
        for(int i = 0; i < realTransfers.size(); ++i) {
            transfers[i] = realTransfers[i].constData();

            /**
             * createPerChannelAdjustment() expects alpha channel to
             * be the last channel in the list, so just it here
             */
            KIS_ASSERT_RECOVER_NOOP(i != alphaIndexInReal ||
                                    alphaIndexInReal == (realTransfers.size() - 1));
        }

        colorTransform = cs->createPerChannelAdjustment(transfers);
        delete [] transfers;
    }

    if (!hueNull) {
        QHash<QString, QVariant> params;
        params["curve"] = QVariant::fromValue(hueTransfer);
        params["channel"] = KisHSVCurve::Hue;
        params["relative"] = false;
        params["lumaRed"]   = cs->lumaCoefficients()[0];
        params["lumaGreen"] = cs->lumaCoefficients()[1];
        params["lumaBlue"]  = cs->lumaCoefficients()[2];

        hueTransform = cs->createColorTransformation("hsv_curve_adjustment", params);
    }

    if (!saturationNull) {
        QHash<QString, QVariant> params;
        params["curve"] = QVariant::fromValue(saturationTransfer);
        params["channel"] = KisHSVCurve::Saturation;
        params["relative"] = false;
        params["lumaRed"]   = cs->lumaCoefficients()[0];
        params["lumaGreen"] = cs->lumaCoefficients()[1];
        params["lumaBlue"]  = cs->lumaCoefficients()[2];

        saturationTransform = cs->createColorTransformation("hsv_curve_adjustment", params);
    }

    if (!lightnessNull) {
        lightnessTransform = cs->createBrightnessContrastAdjustment(lightnessTransfer.constData());
    }

    if (!allColorsNull) {
        const quint16** allColorsTransfers = new const quint16*[realTransfers.size()];
        for(int i = 0; i < realTransfers.size(); ++i) {
            allColorsTransfers[i] = (i != alphaIndexInReal) ?
                allColorsTransfer.constData() : 0;

            /**
             * createPerChannelAdjustment() expects alpha channel to
             * be the last channel in the list, so just it here
             */
            KIS_ASSERT_RECOVER_NOOP(i != alphaIndexInReal ||
                                    alphaIndexInReal == (realTransfers.size() - 1));
        }

        allColorsTransform = cs->createPerChannelAdjustment(allColorsTransfers);
        delete[] allColorsTransfers;
    }

    QVector<KoColorTransformation*> allTransforms;
    allTransforms << colorTransform;
    allTransforms << allColorsTransform;
    allTransforms << hueTransform;
    allTransforms << saturationTransform;
    allTransforms << lightnessTransform;

    return KoCompositeColorTransformation::createOptimizedCompositeTransform(allTransforms);
}

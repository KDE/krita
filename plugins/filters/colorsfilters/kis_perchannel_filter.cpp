/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2005 C. Boemann <cbo@boemann.dk>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include <KisGlobalResourcesInterface.h>

#include "kis_multichannel_utils.h"

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
        if (m_dev->colorSpace()->colorModelId() == LABAColorModelID || m_dev->colorSpace()->colorModelId() == CMYKAColorModelID) {
            if (m_dev->colorSpace()->channels().length() > m_activeVChannel) {
                min = m_dev->colorSpace()->channels()[m_activeVChannel]->getUIMin();
                max = m_dev->colorSpace()->channels()[m_activeVChannel]->getUIMax();
            } else {
                // it must be "Hue", "Saturation" or other "channel" that isn't actually accessible in the color space
                min = 0;
                // specific number apparently doesn't matter,
                // if there is 255, it will work just fine, too
                max = 100;
            }
        } else {
            min = 0;
            max = 100;
        }
        break;
    }

    m_page->curveWidget->setupInOutControls(m_page->intIn, m_page->intOut, min, max, min, max);
}


KisPropertiesConfigurationSP KisPerChannelConfigWidget::configuration() const
{
    int numChannels = m_virtualChannels.size();
    KisPropertiesConfigurationSP cfg = new KisPerChannelFilterConfiguration(numChannels, KisGlobalResourcesInterface::instance());

    KIS_ASSERT_RECOVER(m_activeVChannel < m_curves.size()) { return cfg; }

    m_curves[m_activeVChannel] = m_page->curveWidget->curve();
    static_cast<KisPerChannelFilterConfiguration*>(cfg.data())->setCurves(m_curves);

    return cfg;
}

KisPropertiesConfigurationSP KisPerChannelConfigWidget::getDefaultConfiguration()
{
    return new KisPerChannelFilterConfiguration(m_virtualChannels.size(), KisGlobalResourcesInterface::instance());
}

KisPerChannelFilterConfiguration::KisPerChannelFilterConfiguration(int channelCount, KisResourcesInterfaceSP resourcesInterface)
        : KisMultiChannelFilterConfiguration(channelCount, "perchannel", 1, resourcesInterface)
{
    init();
}

KisPerChannelFilterConfiguration::KisPerChannelFilterConfiguration(const KisPerChannelFilterConfiguration &rhs)
    : KisMultiChannelFilterConfiguration(rhs)
{
}

KisPerChannelFilterConfiguration::~KisPerChannelFilterConfiguration()
{
}

KisFilterConfigurationSP KisPerChannelFilterConfiguration::clone() const
{
    return new KisPerChannelFilterConfiguration(*this);
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

KisFilterConfigurationSP  KisPerChannelFilter::factoryConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    return new KisPerChannelFilterConfiguration(0, resourcesInterface);
}

KoColorTransformation* KisPerChannelFilter::createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const
{
    const KisPerChannelFilterConfiguration* configBC =
        dynamic_cast<const KisPerChannelFilterConfiguration*>(config.data()); // Somehow, this shouldn't happen
    Q_ASSERT(configBC);

    QList<bool> isIdentityList;
    for (const KisCubicCurve &curve : configBC->curves()) {
        isIdentityList.append(curve.isIdentity());
    }

    return KisMultiChannelUtils::createPerChannelTransformationFromTransfers(cs, configBC->transfers(), isIdentityList);
}

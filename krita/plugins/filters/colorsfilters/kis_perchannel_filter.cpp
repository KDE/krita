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

QVector<VirtualChannelInfo> getVirtualChannels(const KoColorSpace *cs)
{
    const bool supportsLightness =
        cs->colorModelId() != LABAColorModelID &&
        cs->colorModelId() != GrayAColorModelID &&
        cs->colorModelId() != GrayColorModelID &&
        cs->colorModelId() != AlphaColorModelID;

    QVector<VirtualChannelInfo> vchannels;

    QList<KoChannelInfo *> sortedChannels =
        KoChannelInfo::displayOrderSorted(cs->channels());

    if (supportsLightness) {
        vchannels << VirtualChannelInfo(VirtualChannelInfo::ALL_COLORS, -1, 0, cs);
    }

    foreach(KoChannelInfo *channel, sortedChannels) {
        int pixelIndex = KoChannelInfo::displayPositionToChannelIndex(channel->displayPosition(), sortedChannels);
        vchannels << VirtualChannelInfo(VirtualChannelInfo::REAL, pixelIndex, channel, cs);
    }

    if (supportsLightness) {
        vchannels << VirtualChannelInfo(VirtualChannelInfo::LIGHTNESS, -1, 0, cs);
    }

    return vchannels;
}

KisPerChannelConfigWidget::KisPerChannelConfigWidget(QWidget * parent, KisPaintDeviceSP dev, Qt::WFlags f)
        : KisConfigWidget(parent, f), m_histogram(0)
{
    Q_ASSERT(dev);
    m_page = new WdgPerChannel(this);

    QHBoxLayout * layout = new QHBoxLayout(this);
    Q_CHECK_PTR(layout);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_page);

    m_dev = dev;
    m_activeVChannel = 0;

    // fill in the channel chooser, in the display order, but store the pixel index as well.

    m_virtualChannels = getVirtualChannels(dev->colorSpace());
    const int virtualChannelCount = m_virtualChannels.size();

    KisPerChannelFilterConfiguration::initDefaultCurves(m_curves,
                                                        virtualChannelCount);
    for (int i = 0; i < virtualChannelCount; i++) {
        const VirtualChannelInfo &info = m_virtualChannels[i];

        m_page->cmbChannel->addItem(info.name(), info.pixelIndex());
        m_curves[i].setName(info.name());
    }

    connect(m_page->cmbChannel, SIGNAL(activated(int)), this, SLOT(setActiveChannel(int)));

    // create the horizontal and vertical gradient labels
    m_page->hgradient->setPixmap(createGradient(Qt::Horizontal));
    m_page->vgradient->setPixmap(createGradient(Qt::Vertical));

    // init histogram calculator
    QList<QString> keys =
        KoHistogramProducerFactoryRegistry::instance()->keysCompatibleWith(m_dev->colorSpace());

    if(keys.size() > 0) {
        KoHistogramProducerFactory *hpf;
        hpf = KoHistogramProducerFactoryRegistry::instance()->get(keys.at(0));
	m_histogram = new KisHistogram(m_dev, m_dev->exactBounds(), hpf->generate(), LINEAR);
    }

    connect(m_page->curveWidget, SIGNAL(modified()), this, SIGNAL(sigConfigurationItemChanged()));

    m_page->curveWidget->setupInOutControls(m_page->intIn, m_page->intOut, 0, 100);

    {
        KisSignalsBlocker b(m_page->curveWidget);
        m_page->curveWidget->setCurve(m_curves[0]);
        setActiveChannel(0);
    }
}

KisPerChannelConfigWidget::~KisPerChannelConfigWidget()
{
    delete m_histogram;
}

inline QPixmap KisPerChannelConfigWidget::createGradient(Qt::Orientation orient /*, int invert (not used yet) */)
{
    int width;
    int height;
    int *i, inc, col;
    int x = 0, y = 0;

    if (orient == Qt::Horizontal) {
        i = &x; inc = 1; col = 0;
        width = 256; height = 1;
    } else {
        i = &y; inc = -1; col = 255;
        width = 1; height = 256;
    }

    QPixmap gradientpix(width, height);
    QPainter p(&gradientpix);
    p.setPen(QPen(QColor(0, 0, 0), 1, Qt::SolidLine));
    for (; *i < 256; (*i)++, col += inc) {
        p.setPen(QColor(col, col, col));
        p.drawPoint(x, y);
    }
    return gradientpix;
}

inline QPixmap KisPerChannelConfigWidget::getHistogram()
{
    int i;
    int height = 256;
    QPixmap pix(256, height);
    pix.fill();
    QPainter p(&pix);
    p.setPen(QPen(Qt::gray, 1, Qt::SolidLine));

    const VirtualChannelInfo &info = m_virtualChannels[m_activeVChannel];

    if (m_histogram && info.type() == VirtualChannelInfo::REAL)
    {
        m_histogram->setChannel(info.pixelIndex());

        double highest = (double)m_histogram->calculations().getHighest();
        qint32 bins = m_histogram->producer()->numberOfBins();

        if (m_histogram->getHistogramType() == LINEAR) {
            double factor = (double)height / highest;
            for (i = 0; i < bins; ++i) {
                p.drawLine(i, height, i, height - int(m_histogram->getValue(i) * factor));
            }
        } else {
            double factor = (double)height / (double)log(highest);
            for (i = 0; i < bins; ++i) {
                p.drawLine(i, height, i, height - int(log((double)m_histogram->getValue(i)) * factor));
            }
        }
    }
    return pix;
}

#define BITS_PER_BYTE 8
#define pwr2(p) (1<<p)

void KisPerChannelConfigWidget::setActiveChannel(int ch)
{
    m_curves[m_activeVChannel] = m_page->curveWidget->curve();

    m_activeVChannel = ch;
    m_page->curveWidget->setCurve(m_curves[m_activeVChannel]);
    m_page->curveWidget->setPixmap(getHistogram());
    m_page->cmbChannel->setCurrentIndex(m_activeVChannel);


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
        m_shift = 0;
        m_scale = double(maxValue);
        min = 0;
        max = maxValue - 1;
        break;
    case KoChannelInfo::INT8:
    case KoChannelInfo::INT16:
        m_shift = 0.5;
        m_scale = double(maxValue);
        min = -maxValue / 2;
        max = maxValue / 2 - 1;
        break;
    case KoChannelInfo::FLOAT16:
    case KoChannelInfo::FLOAT32:
    case KoChannelInfo::FLOAT64:
    default:
        m_shift = 0;
        m_scale = 100.0;
        //Hack Alert: should be changed to float
        min = 0;
        max = 100;
        break;
    }

    m_page->curveWidget->setupInOutControls(m_page->intIn, m_page->intOut, min, max);
}


KisPropertiesConfiguration * KisPerChannelConfigWidget::configuration() const
{
    int numChannels = m_virtualChannels.size();
    KisPerChannelFilterConfiguration * cfg = new KisPerChannelFilterConfiguration(numChannels);

    KIS_ASSERT_RECOVER(m_activeVChannel < m_curves.size()) { return cfg; }

    m_curves[m_activeVChannel] = m_page->curveWidget->curve();
    cfg->setCurves(m_curves);

    return cfg;
}

void KisPerChannelConfigWidget::setConfiguration(const KisPropertiesConfiguration * config)
{
    const KisPerChannelFilterConfiguration * cfg = dynamic_cast<const KisPerChannelFilterConfiguration *>(config);
    if (!cfg)
        return;

    if (cfg->curves().size() == 0) {
        /**
         * HACK ALERT: our configuration factory generates
         * default configuration with nTransfers==0.
         * Catching it here. Just reset all the transfers.
         */

        const int virtualChannelCount = m_virtualChannels.size();
        KisPerChannelFilterConfiguration::initDefaultCurves(m_curves,
                                                            virtualChannelCount);

        for (int i = 0; i < virtualChannelCount; i++) {
            const VirtualChannelInfo &info = m_virtualChannels[i];
            m_curves[i].setName(info.name());
        }

    } else if (cfg->curves().size() != int(m_virtualChannels.size())) {
        qWarning() << "WARNING: trying to load a curve with incorrect  number of channels!";
        qWarning() << "WARNING:   expected:" << m_virtualChannels.size();
        qWarning() << "WARNING:        got:" << cfg->curves().size();
        return;
    } else {
        for (int ch = 0; ch < cfg->curves().size(); ch++)
            m_curves[ch] = cfg->curves()[ch];
    }

    // HACK: we save the previous curve in setActiveChannel, so just copy it
    m_page->curveWidget->setCurve(m_curves[m_activeVChannel]);

    setActiveChannel(0);
}


KisPerChannelFilterConfiguration::KisPerChannelFilterConfiguration(int nCh)
        : KisFilterConfiguration("perchannel", 1)
{
    initDefaultCurves(m_curves, nCh);
    updateTransfers();
}

KisPerChannelFilterConfiguration::~KisPerChannelFilterConfiguration()
{
}

bool KisPerChannelFilterConfiguration::isCompatible(const KisPaintDeviceSP dev) const
{
    return (int)dev->colorSpace()->channelCount() == m_curves.size();
}

void KisPerChannelFilterConfiguration::setCurves(QList<KisCubicCurve> &curves)
{
    m_curves.clear();
    m_curves = curves;

    updateTransfers();
}

void KisPerChannelFilterConfiguration::initDefaultCurves(QList<KisCubicCurve> &curves, int nCh)
{
    curves.clear();
    for (int i = 0; i < nCh; i++) {
        curves.append(KisCubicCurve());
    }
}

void KisPerChannelFilterConfiguration::updateTransfers()
{
    m_transfers.resize(m_curves.size());
    for (int i = 0; i < m_curves.size(); i++) {
        m_transfers[i] = m_curves[i].uint16Transfer();
    }
}

const QVector<QVector<quint16> >&
KisPerChannelFilterConfiguration::transfers() const
{
    return m_transfers;
}

const QList<KisCubicCurve>&
KisPerChannelFilterConfiguration::curves() const
{
    return m_curves;
}

void KisPerChannelFilterConfiguration::fromLegacyXML(const QDomElement& root)
{
    fromXML(root);
}

void KisPerChannelFilterConfiguration::fromXML(const QDomElement& root)
{
    QList<KisCubicCurve> curves;
    quint16 numTransfers = 0;
    int version;
    version  = root.attribute("version").toInt();

    QDomElement e = root.firstChild().toElement();
    QString attributeName;
    KisCubicCurve curve;
    quint16 index;
    while (!e.isNull()) {
        if ((attributeName = e.attribute("name")) == "nTransfers") {
            numTransfers = e.text().toUShort();
        } else {
            QRegExp rx("curve(\\d+)");
            if (rx.indexIn(attributeName, 0) != -1) {

                index = rx.cap(1).toUShort();
                index = qMin(index, quint16(curves.count()));

                if (!e.text().isEmpty()) {
                    curve.fromString(e.text());
                }
                curves.insert(index, curve);
            }
        }
        e = e.nextSiblingElement();
    }

    if (!numTransfers)
        return;

    setVersion(version);
    setCurves(curves);
}

/**
 * Inherited from KisPropertiesConfiguration
 */
//void KisPerChannelFilterConfiguration::fromXML(const QString& s)

void addParamNode(QDomDocument& doc,
                  QDomElement& root,
                  const QString &name,
                  const QString &value)
{
    QDomText text = doc.createTextNode(value);
    QDomElement t = doc.createElement("param");
    t.setAttribute("name", name);
    t.appendChild(text);
    root.appendChild(t);
}

void KisPerChannelFilterConfiguration::toXML(QDomDocument& doc, QDomElement& root) const
{
    /**
     * <params version=1>
     *       <param name="nTransfers">3</param>
     *       <param name="curve0">0,0;0.5,0.5;1,1;</param>
     *       <param name="curve1">0,0;1,1;</param>
     *       <param name="curve2">0,0;1,1;</param>
     * </params>
     */

    root.setAttribute("version", version());

    QDomText text;
    QDomElement t;

    addParamNode(doc, root, "nTransfers", QString::number(m_curves.size()));

    KisCubicCurve curve;
    QString paramName;

    for (int i = 0; i < m_curves.size(); ++i) {
        QString name = QLatin1String("curve") + QString::number(i);
        QString value = m_curves[i].toString();

        addParamNode(doc, root, name, value);
    }
}

/**
 * Inherited from KisPropertiesConfiguration
 */
//QString KisPerChannelFilterConfiguration::toXML()


KisPerChannelFilter::KisPerChannelFilter() : KisColorTransformationFilter(id(), categoryAdjust(), i18n("&Color Adjustment curves..."))
{
    setSupportsPainting(true);
    setColorSpaceIndependence(TO_LAB16);
}

KisConfigWidget * KisPerChannelFilter::createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev) const
{
    return new KisPerChannelConfigWidget(parent, dev);
}

KisFilterConfiguration * KisPerChannelFilter::factoryConfiguration(const KisPaintDeviceSP) const
{
    return new KisPerChannelFilterConfiguration(0);
}

KoColorTransformation* KisPerChannelFilter::createTransformation(const KoColorSpace* cs, const KisFilterConfiguration* config) const
{
    const KisPerChannelFilterConfiguration* configBC =
        dynamic_cast<const KisPerChannelFilterConfiguration*>(config); // Somehow, this shouldn't happen
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
    const QVector<VirtualChannelInfo> virtualChannels = getVirtualChannels(cs);

    if (originalTransfers.size() != int(virtualChannels.size())) {
        // We got an illegal number of colorchannels :(
        return 0;
    }

    bool colorsNull = true;
    bool lightnessNull = true;
    bool allColorsNull = true;
    int alphaIndexInReal = -1;

    QVector<QVector<quint16> > realTransfers;
    QVector<quint16> lightnessTransfer;
    QVector<quint16> allColorsTransfer;

    for (int i = 0; i < virtualChannels.size(); i++) {
        if (virtualChannels[i].type() == VirtualChannelInfo::REAL) {
            realTransfers << originalTransfers[i];

            if (virtualChannels[i].isAlpha()) {
                alphaIndexInReal = realTransfers.size() - 1;
            }

            if (colorsNull && !originalCurves[i].isNull()) {
                colorsNull = false;
            }
        } else if (virtualChannels[i].type() == VirtualChannelInfo::LIGHTNESS) {
            KIS_ASSERT_RECOVER_NOOP(lightnessTransfer.isEmpty());
            lightnessTransfer = originalTransfers[i];

            if (lightnessNull && !originalCurves[i].isNull()) {
                lightnessNull = false;
            }
        } else if (virtualChannels[i].type() == VirtualChannelInfo::ALL_COLORS) {
            KIS_ASSERT_RECOVER_NOOP(allColorsTransfer.isEmpty());
            allColorsTransfer = originalTransfers[i];

            if (allColorsNull && !originalCurves[i].isNull()) {
                allColorsNull = false;
            }
        }
    }

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
    allTransforms << lightnessTransform;
    allTransforms << allColorsTransform;
    allTransforms << colorTransform;

    return KoCompositeColorTransformation::createOptimizedCompositeTransform(allTransforms);
}

